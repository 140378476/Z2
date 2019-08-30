//
// Created by chenzc18 on 2019/8/29.
//

#ifndef Z2_ALPHABOT_H
#define Z2_ALPHABOT_H

#include <entity/Farmer.h>
#include "Bot.h"
#include "TaskCaptureResource.h"

namespace z2{
    namespace bot{
        class AlphaBot : public Bot {
        protected:
            string name;
            int minFarmerCount = 2;
            int minFarmerGuardianCount = 1;
            int maxFarmerGuardianCount = 2;
            int extraGuardianForGem = 2;
            int maxNumOfTurnsToWaitForResearch = 6;
            int extraBattleUnitCountToLaunchAttack = 2;
            bool saveMoneyThisTurn = false;
            double goldMultiplier = 2;
            long operationInterval = 100;

            vector<shared_ptr<TaskCaptureResource>> farming;
            vector<shared_ptr<EntityTask>> combating;
            vector<shared_ptr<TaskGuard>> guarding;
            vector<shared_ptr<EntityTask>> extraBattleUnits;
            shared_ptr<TaskResearch> research;
            Point conBase;

            void doInit() override;
            void makeOperation(const shared_ptr<GameMessage> &msg) override;
            void performMine(const Point &pos);
            void perform(const shared_ptr<Entity> &en) override;
            void refreshInfo();
            int computeIncomePerTurn();
            virtual void doFarm();
            void launchAttackTo(EntityId enemy, int maxCount);
            bool launchAttackToFarmer();
            bool launchAttackToBase();
            bool launchAttackToAll();
            bool assignFarmer(const shared_ptr<Entity> &farmer);
            bool assignStandBy(EntityId unit);
            /**
             * Returns `true` if a new resource is found and entities are prepared, which implies that `farming.size()` is
             * increased.
             */
            virtual bool planForNewResource();
            virtual bool strengthenMilitaryForce();
            virtual void consolidateFarming(const shared_ptr<Entity> &en);
            virtual void doCombat();
            virtual void doCommandUnits();
            virtual void doBasicProduction();
            virtual void doExtraProduction();
            virtual void doResearch();
            Point findNextNearestResource(const Point& start);
            Point findRandomEmptyTile(int d=1);
        public:
            void doBotTurn() override;
            const string &getBotName() override;
            AlphaBot(BotDifficulty difficulty, const string &name);
            friend class AlphaBotProvider;
        };

        class AlphaBotProvider{
        public:
            explicit AlphaBotProvider(const string & name);
            shared_ptr<Bot> operator()(BotDifficulty difficulty);
        private:
            string name;
        };
    }
}

#endif //Z2_ALPHABOT_H
