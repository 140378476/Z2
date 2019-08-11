/*
 * Created by liyicheng on 2019/7/3.
 */

#include "Server.h"
#include "Message.h"
#include "ClientProxy.h"
#include "messages/PlayerMessage.h"
#include "messages/ControlMessage.h"
#include "plog/Log.h"
#include "event/StateEvent.h"

using namespace z2;

void Server::acceptMessage(const shared_ptr<Message> &command) {
    if (gameState == GameState::PAUSED) {
        PLOG(plog::info) << "Reject message: Game Paused";
        return;
    }
    PLOG_INFO << "[Server] Accepted message: " << command->getClassName();
//    ancono::info"Accepted message!";
    switch (command->getGeneralType()) {
        case GeneralMessageType::ControlMessage: {
            dealWithControlMessage(static_pointer_cast<ControlMessage>(command));
            break;
        }
        case GeneralMessageType::GameMessage: {
            world->dealWithMessage(static_pointer_cast<GameMessage>(command));
            //TODO
            broadcastMessage(command);
            break;
        }
        case GeneralMessageType::ChatMessage:
            break;
    }
}

bool z2::Server::registerClient(const shared_ptr<z2::ClientProxy> &client) {
    int cid = clients.size();
    registerClient(client, cid);
}

bool Server::registerClient(const shared_ptr<ClientProxy> &client, int clientId) {
    client->setClientId(clientId);
    if (clients.size() <= clientId) {
        clients.resize(clientId + 1);
    }
    clients[clientId] = client;

    bool re = client->syncWorld(world);
    if (re) {
        PLOG(plog::info) << "Client registered";
        return true;
    } else {
        PLOG_WARNING << "Client register failed!";
        return false;
    }
}


void z2::Server::startGame() {
    if (!checkGameReady()) {
        return;
    }
    gameState = GameState::RUNNING;
    PLOG(plog::info) << "Game Started!";
    shared_ptr<Message> startGame(new ControlMessage(ControlMessageType::StartGame));
    broadcastMessage(startGame);
    int playerId = world->nextPlayerFromCurrent();
    callPlayer(playerId);
}

void Server::sendMessage(const shared_ptr<Message> &message, int clientId) {
    auto &c = clients[clientId];
    if (c) {
        c->sendMessage(message);
    }
}

void Server::broadcastMessage(const shared_ptr<Message> &message) {
    for (auto &c : clients) {
        if (c) {
            c->sendMessage(message);
        }
    }
}

void Server::callPlayer(int playerId) {
    shared_ptr<Message> msg(new PlayerMessage(ControlMessageType::PlayerTurnStart, playerId));
    broadcastMessage(msg);
    world->onPlayerTurnStart();
}

void Server::shiftTurn() {
    callPlayer(world->nextPlayer());
    world->onPlayerTurnFinish();
}

void Server::onPlayerTurnFinish(const shared_ptr<Message> &message) {
    shared_ptr<PlayerMessage> msg = static_pointer_cast<PlayerMessage>(message);
    int playerId = msg->getPlayerId();
    if (playerId != world->getCurrentPlayer()) {
        PLOG_WARNING << "Turn finish with not current player!";
        return;
    }
    broadcastMessage(msg);
    shiftTurn();
}

const shared_ptr<World> &Server::getWorld() const {
    return world;
}

Server::GameState Server::getGameState() const {
    return gameState;
}

void Server::attachListeners(const shared_ptr<World> &world) {
    auto listener = [this](const GameEventPtr &event) {
        shared_ptr<GroupEvent> gw = dynamic_pointer_cast<GroupEvent>(event);
        if (!gw) {
            return;
        }
        localEndGame(gw->getGroupId());
    };
    world->addEventListener(listener);
}

void Server::setWorld(const shared_ptr<World> &world) {
    Server::world = world;
    attachListeners(world);
}

bool Server::checkGameReady() {
    if (gameState == GameState::RUNNING) {
        PLOG_WARNING << "Already started!";
        return false;
    }
    if (!world) {
        PLOG_WARNING << "The world is not set!";
        return false;
    }
    if (clients.size() != world->getPlayerCount()) {
        PLOG_WARNING << "Player count != client count";
        return false;
    }
    if (!world->checkReady()) {
        PLOG_WARNING << "The world is not correctly loaded!";
        return false;
    }
    return true;
}

void Server::dealWithControlMessage(const shared_ptr<z2::ControlMessage> &message) {
    switch (message->getControlType()) {
        case ControlMessageType::PlayerTurnFinish: {
            onPlayerTurnFinish(message);
            break;
        }
        case ControlMessageType::EndGame: {
            exceptionalEndGame("Received EndGame message.");
            break;
        }
        case ControlMessageType::PlayerTurnStart:
            break;
        case ControlMessageType::PlayerDefeated:
            break;
        case ControlMessageType::PlayerWin:
            break;
        case ControlMessageType::StartGame:
            break;
        case ControlMessageType::RegisterPlayer:
            break;
        case ControlMessageType::SyncWorld:
            break;
        case ControlMessageType::Signal:
            break;
    }
}

//void Server::onEndGame() {
//    gameState = GameState::PAUSED;
//    broadcastMessage(make_shared<ControlMessage>(ControlMessageType::EndGame));
//    PLOG(plog::info) << "Game ended!";
//}

void Server::localEndGame(int winnerGroupId) {
    PLOG(plog::info) << "[Server] The group " << winnerGroupId << " wins!";
    gameState = GameState::PAUSED;
//    onEndGame();
}

void Server::exceptionalEndGame(const string &cause) {
    gameState = GameState::PAUSED;
    PLOG(plog::info) << "Game ended, cause: " << cause;
    broadcastMessage(make_shared<ControlMessage>(ControlMessageType::EndGame));
}


z2::Server::~Server() = default;
