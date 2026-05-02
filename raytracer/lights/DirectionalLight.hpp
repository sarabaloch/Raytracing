#pragma once

/**
 * DirectionalLight.hpp
 *
 * A directional light that shines equally from one direction everywhere
 * in the scene (like sunlight or moonlight from very far away).
 *
 * The direction is fixed — every surface point receives light from
 * exactly the same angle. The distance to the light is infinite,
 * so shadow rays travel forever in that direction.
 */

#include "Light.hpp"
#include "../utilities/Vector3D.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Constants.hpp"

class DirectionalLight : public Light {
private:

    Vector3D direction;     // unit direction TOWARD the light
    RGBColor color;         // color of the light
    float    intensity;     // brightness multiplier

public:

    // Set direction (will be normalized), color, and intensity.
    DirectionalLight(const Vector3D& dir, const RGBColor& col, float ls)
        : direction(dir), color(col), intensity(ls) {
        direction.normalize();
    }

    // Convenience: set direction by components.
    DirectionalLight(float dx, float dy, float dz,
                     float r,  float g,  float b,
                     float ls)
        : direction(dx, dy, dz), color(r, g, b), intensity(ls) {
        direction.normalize();
    }

    // Copy constructor and assignment operator.
    DirectionalLight(const DirectionalLight& other) = default;
    DirectionalLight& operator=(const DirectionalLight& other) = default;

    virtual ~DirectionalLight() = default;

    // The direction toward the light is constant for all surface points.
    virtual Vector3D get_direction(const ShadeInfo& sinfo) const override {
        return direction;
    }

    // Returns color * intensity.
    virtual RGBColor get_radiance(const ShadeInfo& sinfo) const override {
        return color * intensity;
    }

    // Directional lights are infinitely far away.
    virtual float get_distance(const ShadeInfo& sinfo) const override {
        return kHugeValue;
    }
};