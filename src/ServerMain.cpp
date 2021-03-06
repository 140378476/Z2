//
// Created by liyicheng on 2019/7/25.
//



using namespace std;

#define ASIO_STANDALONE

#include <iostream>
#include <string>
#include <core/Lobby.h>
#include <plog/Log.h>
#include "world/World.h"
#include "core/Server.h"
#include "config/EntityRepository.h"
#include "entity/Farmer.h"
#include "core/LocalClient.h"
#include "commandLine/CommandLineGameGui.h"
#include "entity/ConstructionBase.h"
#include "bot/BotClientProxy.h"
#include "config/GameConfiguration.h"
#include "config/SerializableRegistry.h"
#include "config/MapRepository.h"

using namespace std;
using namespace z2;


shared_ptr<World> buildWorld() {
    shared_ptr<World> w(new World(5, 5, 3));
    w->getPlayer(0).setGroupId(0);
    w->getPlayer(1).setDead(true);
    w->getPlayer(2).setGroupId(2);
//    w->createEntity(Point(2, 2), ConstructionBase::className(), 0);
    w->createEntity(Point(1, 1), "Killer", 0);
    w->createEntity(Point(3, 3), "Killer", 2);
    w->getTile(0, 3).setResource(Resource::MINE);
    return w;
}

void setupWorld(const shared_ptr<World>& w){
    w->getPlayer(0).setGroupId(0);
    w->getPlayer(1).setDead(true);
    w->getPlayer(2).setGroupId(2);
//    w->createEntity(Point(2, 2), ConstructionBase::className(), 0);
    w->createEntity(Point(1, 1), "Killer", 0);
    w->createEntity(Point(3, 3), "Killer", 2);
    w->getTile(0, 3).setResource(Resource::MINE);
}

//    std::thread t([lobby](){
//        try{
//            lobby->closeLobby();
//        }catch (...){
//            PLOG_INFO << "!"<<endl;
//        }
//    });
//    try{
//        t.detach();
//    }catch (...){
//        PLOG_INFO << "!!"<<endl;
//    }
//    return 0;
const int port = 23456;

GameInitSetting getSetting(){
//    vector<PlayerType> players{PlayerType::LOCAL_PLAYER, PlayerType::BOT_PLAYER, PlayerType::REMOTE_PLAYER};
    auto map = MapRepository::instance().getMap("TestMap");
    GameInitSetting gis(3,map);
    gis.setupPlayer(0, PlayerSetting(0, 0, 0, PlayerType::LOCAL_PLAYER));
    gis.setupPlayer(1, PlayerSetting(1, 1, 1, PlayerType::REMOTE_PLAYER));
    gis.setupPlayer(2, PlayerSetting(2, 2, 2, PlayerType::REMOTE_PLAYER));
    return gis;
}

int main() {
    cout << "Starting..." << endl;
    GameConfiguration::initAll();

    auto setting = getSetting();
    cout << setting.getPlayers().size() << endl;
    shared_ptr<Lobby> lobby(new Lobby(port,setting));
    lobby->openLobby();
//192.168.0.28

    shared_ptr<CommandLineGameGui> gui(new CommandLineGameGui());
//    local->setView(static_pointer_cast<GameGui>(gui));
//    gui->setControllerAndView(static_pointer_cast<Client>(local));

    auto server = lobby->startGame(gui, 1000000);
    setupWorld(server->getWorld());
    if(!server){
        return 0;
    }
    gui->mainLoop();
}