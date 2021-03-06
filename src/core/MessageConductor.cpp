#include <memory>

#include <utility>
#include <sstream>

//
// Created by liyicheng on 2019/7/22.
//

#include "MessageConductor.h"
#include "plog/Log.h"
#include "config/GameConfiguration.h"

using namespace z2;
using namespace std;

const MessageProcessor &z2::MessageConductor::getProcessor() const {
    return processor;
}

void z2::MessageConductor::setProcessor(const MessageProcessor &processor) {
//    std::lock_guard<mutex> guard(processorMutex);
    MessageConductor::processor = processor;
}

bool z2::MessageConductor::isStopped() const {
    return stopped;
}

void z2::MessageConductor::sendMessage(const MessagePtr &msg, int id) {
    if (stopped) {
        PLOG_WARNING << "[Conductor] Conductor is stopped!";
        return;
    }
    auto sock = connections[id];

    if (!sock) {
        PLOG_WARNING << "[Conductor] Connection not available: " << id;
        return;
    }
    asio::error_code err = MessageConductor::sendMessageToSocket(sock, msg);
    if (err) {
        PLOG_WARNING << "Failed to send message, error code = " << err.value();
        failureProcessor(err, id);
    }
}


z2::MessageConductor::~MessageConductor() = default;

z2::MessageConductor::MessageConductor(MessageProcessor processor) : processor(std::move(processor)) {
}


void z2::MessageConductor::start(int port, int count, const shared_ptr<MessageConductor> &self) {
    // creates the socket
    service.reset(new io_service);
    ip::tcp::endpoint ep(ip::tcp::v4(), port);
    acceptor.reset(new ip::tcp::acceptor(*service, ep));
    acceptor->listen(count);
//    acceptor->close().
    connections = vector<socket_ptr>(count);
    buffers = vector<asio::streambuf>(count);
    for (int i = 0; i < count; i++) {
        auto sock = new ip::tcp::socket(*service);
        connections[i].reset(sock);
        acceptor->async_accept(*sock, [&self, i](const error_code &error) {
            if (self) {
                self->handleAccept(error, i);
            }
            return;
        });
    }

    // start the thread
    auto f = [this, &self]() {
        service->run();
        PLOG(plog::info) << "[Conductor] Ended!";
        stopped = true;
    };
    workingThread = std::thread(f);
//    workingThread.detach();
}


void MessageConductor::stop() {
    for (auto &conn : connections) {
        if (conn) {
            conn->close();
            conn.reset();
        }
    }
    PLOG_INFO << "[MessageConductor] Connections reset";
    if (acceptor) {
        acceptor->close();
        PLOG_INFO << "[MessageConductor] Acceptor cancelled";
    }
    if (service) {
        service->stop();
        if(workingThread.joinable()){
            workingThread.join();
        }else{
            PLOG_ERROR << "[MessageConductor] Working thread is not joinable!: ";
        }

        PLOG_INFO << "[MessageConductor] Service stopped: " << service->stopped();
    }

    acceptor.reset();
    service.reset();
}


void MessageConductor::handleAccept(const error_code &error, int id) {
    if (error) {
        failureProcessor(error, id);
        PLOG_WARNING << "[Conductor] Failed to connect, error code = " << error.value() << ", message = "
                     << error.message();
        return;
    }
    PLOG_INFO << "[Conductor] Accepted id = " << id;
    socket_ptr sock = connections[id];
    async_read_until(*sock, buffers[id], '\n', [this, id](const error_code &error, size_t length) {
        handleReceive(error, length, id);
    });
}

void MessageConductor::handleReceive(const error_code &error, size_t length, int id) {
    if (error) {
        PLOG_WARNING << "[Conductor] Failed to receive, error = " << error.value() << ", message = "
                     << error.message();
//        connections[id]->close();
        connections[id].reset();
        failureProcessor(error, id);
        return;
    }
    PLOG(plog::info) << "[Conductor] Received from " << id;

    asio::streambuf &buf = buffers[id];
//    std::cout << &buf;
    auto msg = MessageConductor::readMessageFromSocket(buf);
    {
        std::lock_guard<mutex> guard(processorMutex);
        processor(msg,id);
    }


    async_read_until(*connections[id], buffers[id], '\n', [this, id](const error_code &error, size_t length) {
        handleReceive(error, length, id);
        return;
    });
}

asio::error_code MessageConductor::sendMessageToSocket(const socket_ptr& socket, const MessagePtr &message) {
    PLOG(plog::info) << "Sending message: " << message->getClassName();
    std::stringstream ss;
    message->serializeTo(ss);
    ss << '\n';
    asio::error_code err;
    socket->write_some(buffer(ss.str()), err);
    return err;

}

MessagePtr MessageConductor::readMessageFromSocket(asio::streambuf &buffer) {
    std::istream input(&buffer);
    MessagePtr msg(dynamic_cast<Message *>(SerializableRegistry::instance().deserialize(input)));
    string s;
    std::getline(input, s);
    return msg;
}

void MessageConductor::setFailureProcessor(const FailureProcessor &failureProcessor) {
    MessageConductor::failureProcessor = failureProcessor;
}

string MessageConductor::getLocalAddressInfo() {
//    if(connections.empty()){
//        return "127.0.0.1";
//    }else{
//        auto& conn = connections[0];
//        try{
//            return conn->local_endpoint().address().to_string();
//        }catch(...){
//            return "127.0.0.1";
//        }
//    }
    ip::tcp::resolver resolver(*service);
    ip::tcp::resolver::query query(asio::ip::host_name(), "");
    ip::tcp::resolver::iterator iter = resolver.resolve(query);
    ip::tcp::resolver::iterator end; // End marker.
    while(iter != end){
        ip::tcp::endpoint ep = *iter++;
        if(ep.address().is_v4()){
            return ep.address().to_string();
        }
    }
    return "127.0.0.1";
//    while (iter != end) {
//
//    }
}

string MessageConductor::getLocalHostNameInfo() {
    return asio::ip::host_name();
}
