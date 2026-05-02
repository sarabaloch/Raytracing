#pragma once

/**
 * Light.hpp
 *
 * Abstract base class for all light sources.
 *
 * A light provides:
 *   - A direction from a surface point toward the light.
 *   - A radiance (color * intensity) arriving at that point.
 *   - A distance to the light (used for shadow ray termination).
 */

class RGBColor;
class Vector3D;
class ShadeInfo;

class Light {
public:

    // Default constructor.
    Light() = default;

    // Copy constructor and assignment operator.
    Light(const Light& other) = default;
    Light& operator=(const Light& other) = default;

    // Virtual destructor.
    virtual ~Light() = default;

    // Returns the unit direction from the hit point toward this light.
    virtual Vector3D get_direction(const ShadeInfo& sinfo) const = 0;

    // Returns the radiance (color scaled by intensity) of this light.
    virtual RGBColor get_radiance(const ShadeInfo& sinfo) const = 0;

    // Returns the distance from the hit point to the light source.
    // Used to limit shadow rays so they don't go past the light.
    virtual float get_distance(const ShadeInfo& sinfo) const = 0;
};