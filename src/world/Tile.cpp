//
// Created by liyicheng on 2019/7/3.
//

#include "Tile.h"

using namespace z2;

Terrain Tile::getTerrain() {
    return terrain;
}

std::shared_ptr<Entity> Tile::getEntity() {
    return entity;
}

std::shared_ptr<Entity> Tile::removeEntity() {
    auto ptr = entity.get();
    entity.reset();
    return shared_ptr<Entity>(ptr);
}

Resource Tile::getResource() {
    return resource;
}

void Tile::setResource(const Resource &resource_) {
    Tile::resource = resource_;
}

bool Tile::hasEntity() {
    return bool(entity);
}

bool Tile::isOccupied() {
    return false;
}

void Tile::setTerrain(Terrain terrain_) {
    Tile::terrain = terrain_;
}

void Tile::setEntity(const shared_ptr<Entity>& entity_) {
    entity = entity_;
}

bool isBlocking(const Terrain t) {
    switch(t){
        case Terrain::PLAIN:
            return false;
        case Terrain::MOUNTAIN:
            return true;
    }
}