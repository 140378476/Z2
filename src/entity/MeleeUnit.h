/*
 * Created by liyicheng on 2019/7/11.
 */

#ifndef Z2_MELEEUNIT_H
#define Z2_MELEEUNIT_H

#include "GameUnit.h"
#include "BattleUnit.h"

namespace z2 {
class MeleeUnit : public BattleUnit {
public:
    explicit MeleeUnit(unsigned int objectId);

    void initialize(const Properties &prop) override;

public:
    const string &getClassName() const override;

    static const string& className();

    void serializeTo(ostream &output) override;

    static MeleeUnit* loadFrom(istream& input);

    static Entity* create(int objectId, const Properties& initializer);

protected:
    void serializeDataPart(ostream &output) override;

    static void deserializeDataPart(istream &input, MeleeUnit *en);
};

}

#endif //Z2_MELEEUNIT_H
