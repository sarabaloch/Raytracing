#pragma once

/**
 * Acceleration.hpp
 *
 * Abstract base class for acceleration structures.
 *
 * An acceleration structure speeds up ray-geometry intersection
 * by avoiding testing every ray against every object.
 *
 * Subclasses (e.g. BVH) implement build() and hit().
 */

#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Ray.hpp"

#include <vector>

class Geometry;
class World;

class Acceleration {
public:

    // Default constructor.
    Acceleration() = default;

    // Copy constructor and assignment operator.
    Acceleration(const Acceleration& other) = default;
    Acceleration& operator=(const Acceleration& other) = default;

    // Virtual destructor.
    virtual ~Acceleration() = default;

    // Build the acceleration structure from a list of geometry objects.
    // Call this once after all geometry has been added to the world.
    virtual void build(const std::vector<Geometry*>& objects) = 0;

    // Test a ray against the acceleration structure.
    // Returns ShadeInfo for the closest hit, or ShadeInfo.hit = false if no hit.
    virtual ShadeInfo hit(const Ray& ray, const World& world) const = 0;

    // Test a shadow ray: returns true if anything is hit before max_distance.
    virtual bool shadow_hit(const Ray& ray, float max_distance) const = 0;
};