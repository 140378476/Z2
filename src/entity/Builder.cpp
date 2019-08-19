/*
 * Created by liyicheng on 2019/8/18.
 */

#include "Builder.h"

z2::Builder::Builder(unsigned int objectId) : GameUnit(objectId) {}

void z2::Builder::initialize(const Properties &prop) {
    GameUnit::initialize(prop);
    buildingName = prop.get("buildingName","");
}

const string &z2::Builder::getClassName() const {
    return className();
}

const string &z2::Builder::className() {
    static string name = "Builder";
    return name;
}

void z2::Builder::serializeTo(ostream &output) {
    output << className() << ' ';
    serializeDataPart(output);
}

z2::Builder *z2::Builder::loadFrom(istream &input) {
    Builder* b = new Builder(0);
    deserializeDataPart(input, b);
    return b;
}

z2::Entity *z2::Builder::create(int objectId, const Properties &initializer) {
    auto x = new Builder(objectId);
    x->initialize(initializer);
    return x;
}

void z2::Builder::serializeDataPart(ostream &output) {
    GameUnit::serializeDataPart(output);
    output << buildingName << ' ';

}

void z2::Builder::deserializeDataPart(istream &input, z2::Builder *en) {
    GameUnit::deserializeDataPart(input, en);
    input >> en->buildingName;
}