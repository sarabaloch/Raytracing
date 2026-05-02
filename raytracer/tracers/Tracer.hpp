#pragma once

/**
 * Tracer.hpp
 *
 * Abstract base class for all tracers.
 * A tracer takes a ray and a world, and returns the color seen along that ray.
 */

#include "../utilities/RGBColor.hpp"
#include "../utilities/Ray.hpp"

class World;

class Tracer {
protected:
    World* world_ptr;   // pointer to the world being traced

public:

    // Constructor — store a pointer to the world.
    Tracer(World* world) : world_ptr(world) {}

    // Copy constructor and assignment operator.
    Tracer(const Tracer& other) = default;
    Tracer& operator=(const Tracer& other) = default;

    // Virtual destructor.
    virtual ~Tracer() = default;

    // Trace a ray through the world and return the color seen.
    virtual RGBColor trace_ray(const Ray& ray) const = 0;
};