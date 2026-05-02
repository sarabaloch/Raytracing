/**
 * World.cpp
 *
 * Implementation of the World class.
 * Extended for CS 440 Project 2 to support lights, tracers, and acceleration.
 */

#include "World.hpp"
#include "../acceleration/Acceleration.hpp"
#include "../cameras/Camera.hpp"
#include "../geometry/Geometry.hpp"
#include "../lights/Light.hpp"
#include "../samplers/Sampler.hpp"
#include "../tracers/Tracer.hpp"
#include "../utilities/Constants.hpp"
#include "../utilities/Ray.hpp"
#include "../utilities/ShadeInfo.hpp"


// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
World::World()
    : vplane(),
      bg_color(black),
      camera_ptr(nullptr),
      sampler_ptr(nullptr),
      tracer_ptr(nullptr),
      accel_ptr(nullptr)
{}


// ---------------------------------------------------------------------------
// Destructor — free everything we own
// ---------------------------------------------------------------------------
World::~World() {

    // Delete all geometry.
    for (int i = 0; i < (int)geometry.size(); i++) {
        delete geometry[i];
    }
    geometry.clear();

    // Delete unbounded geometry.
    for (int i = 0; i < (int)unbounded_geometry.size(); i++) {
        delete unbounded_geometry[i];
    }
    unbounded_geometry.clear();

    // Delete all lights.
    for (int i = 0; i < (int)lights.size(); i++) {
        delete lights[i];
    }
    lights.clear();

    // Delete camera, sampler, tracer, and acceleration structure.
    delete camera_ptr;
    camera_ptr = nullptr;

    delete sampler_ptr;
    sampler_ptr = nullptr;

    delete tracer_ptr;
    tracer_ptr = nullptr;

    delete accel_ptr;
    accel_ptr = nullptr;
}


// ---------------------------------------------------------------------------
// add_geometry()
// ---------------------------------------------------------------------------
void World::add_geometry(Geometry* geom_ptr) {
    geometry.push_back(geom_ptr);
}

void World::add_unbounded_geometry(Geometry* geom_ptr) {
    unbounded_geometry.push_back(geom_ptr);
}


// ---------------------------------------------------------------------------
// add_light()
// ---------------------------------------------------------------------------
void World::add_light(Light* light_ptr) {
    lights.push_back(light_ptr);
}


// ---------------------------------------------------------------------------
// set_camera()
// ---------------------------------------------------------------------------
void World::set_camera(Camera* new_camera) {
    delete camera_ptr;
    camera_ptr = new_camera;
}


// ---------------------------------------------------------------------------
// hit_objects()
//   Find the closest geometry hit by this ray.
//   Uses BVH if accel_ptr is set, otherwise tests every object.
// ---------------------------------------------------------------------------
ShadeInfo World::hit_objects(const Ray& ray) {

    ShadeInfo closest(*this);
    float smallest_t = kHugeValue;

    // Test bounded geometry (via BVH or brute force).
    if (accel_ptr != nullptr) {
        ShadeInfo bvh_info = accel_ptr->hit(ray, *this);
        if (bvh_info.hit && bvh_info.t < smallest_t) {
            smallest_t = bvh_info.t;
            closest    = bvh_info;
        }
    } else {
        for (int i = 0; i < (int)geometry.size(); i++) {
            float     object_t    = kHugeValue;
            ShadeInfo object_info(*this);
            if (geometry[i]->hit(ray, object_t, object_info) && object_t < smallest_t) {
                smallest_t = object_t;
                closest    = object_info;
            }
        }
    }

    // Always test unbounded geometry (planes etc.) directly — never through BVH.
    for (int i = 0; i < (int)unbounded_geometry.size(); i++) {
        float     object_t    = kHugeValue;
        ShadeInfo object_info(*this);
        if (unbounded_geometry[i]->hit(ray, object_t, object_info) && object_t < smallest_t) {
            smallest_t = object_t;
            closest    = object_info;
        }
    }

    return closest;
}


// ---------------------------------------------------------------------------
// in_shadow()
//   Test whether a shadow ray is blocked before reaching max_distance.
//   Uses BVH if accel_ptr is set, otherwise tests every object.
// ---------------------------------------------------------------------------
bool World::in_shadow(const Ray& shadow_ray, float max_distance) const {

    // Test via BVH or brute force.
    if (accel_ptr != nullptr) {
        if (accel_ptr->shadow_hit(shadow_ray, max_distance)) return true;
    } else {
        for (int i = 0; i < (int)geometry.size(); i++) {
            if (geometry[i]->shadow_hit(shadow_ray, max_distance)) return true;
        }
    }

    // Also test unbounded geometry.
    for (int i = 0; i < (int)unbounded_geometry.size(); i++) {
        if (unbounded_geometry[i]->shadow_hit(shadow_ray, max_distance)) return true;
    }

    return false;
}