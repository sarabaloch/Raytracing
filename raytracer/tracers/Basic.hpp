#pragma once

/**
 * Basic.hpp
 *
 * The simplest possible tracer.
 * Shoots a primary ray, finds the closest hit, shades it using only
 * the material's shade() function. No shadow rays, no lighting loop.
 *
 * This is essentially what raytracer.cpp was doing before.
 */

#include "Tracer.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../world/World.hpp"

class Basic : public Tracer {
public:

    // Constructor.
    Basic(World* world) : Tracer(world) {}

    // Copy constructor and assignment operator.
    Basic(const Basic& other) = default;
    Basic& operator=(const Basic& other) = default;

    virtual ~Basic() = default;

    // Trace a primary ray.
    // If it hits something, shade it. Otherwise return background color.
    virtual RGBColor trace_ray(const Ray& ray) const override {

        // Test ray against all geometry.
        ShadeInfo sinfo = world_ptr->hit_objects(ray);

        // If we hit something, call its material's shade function.
        if (sinfo.hit) {
            RGBColor color = sinfo.material_ptr->shade(sinfo);
            return color;
        }

        // Nothing hit — return the background color.
        return world_ptr->bg_color;
    }
};