#include "World.hpp"
#include "../cameras/Camera.hpp"
#include "../geometry/Geometry.hpp"
#include "../samplers/Sampler.hpp"
#include "../utilities/Constants.hpp"
#include "../utilities/Ray.hpp"
#include "../utilities/ShadeInfo.hpp"

World::World()
    : vplane(), bg_color(black), camera_ptr(nullptr), sampler_ptr(nullptr) {}

World::~World() {
    for (Geometry *currentObject : geometry) {
        delete currentObject;
    }

    geometry.clear();

    delete sampler_ptr;
    sampler_ptr = nullptr;

    delete camera_ptr;
    camera_ptr = nullptr;
}

void World::add_geometry(Geometry *newObject) {
    geometry.push_back(newObject);
}

void World::set_camera(Camera *newCamera) {
    delete camera_ptr;
    camera_ptr = newCamera;
}

ShadeInfo World::hit_objects(const Ray &ray) {
    ShadeInfo closestIntersection(*this);
    float smallestHitDistance = kHugeValue;

    for (const Geometry *currentObject : geometry) {
        float objectHitDistance = kHugeValue;
        ShadeInfo objectShadeInfo(*this);

        bool didHit = currentObject->hit(ray, objectHitDistance, objectShadeInfo);

        if (didHit) {
            if (objectHitDistance < smallestHitDistance) {
                smallestHitDistance = objectHitDistance;
                closestIntersection = objectShadeInfo;
            }
        }
    }

    return closestIntersection;
}