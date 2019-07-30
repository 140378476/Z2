//
// Created by liyicheng on 2019/7/29.
//

#ifndef Z2_GAMEINITSETTING_H
#define Z2_GAMEINITSETTING_H
#include <memory>
#include <string>
#include <vector>
using namespace std;
namespace z2 {
class GameMap;
class World;
enum class PlayerType {
    LOCAL_PLAYER,
    BOT_PLAYER,
    REMOTE_PLAYER,
};
class PlayerSetting{
public:
    PlayerSetting(int playerId, int positionId, int groupId, PlayerType type);
    PlayerSetting();
    int playerId;
    int positionId;
    int groupId;
    PlayerType type;
};

class GameInitSetting {
private:
    vector<PlayerSetting> players;
    shared_ptr<GameMap> map;
public:
    GameInitSetting(int playerCount, shared_ptr<GameMap> map);

    GameInitSetting(vector<PlayerSetting> players, shared_ptr<GameMap> map);

    void setupPlayer(int idx,const PlayerSetting& s);

    bool isValidLocalSetting(bool isLocalGame);

    shared_ptr<World> buildGame();

};

}


#endif //Z2_GAMEINITSETTING_H
