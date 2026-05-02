#pragma once

/**
 * PointLight.hpp
 *
 * A point light source that radiates equally in all directions
 * from a single position in space (like a bare light bulb).
 *
 * The direction toward the light changes depending on where the
 * surface point is. The light does not attenuate with distance
 * (this keeps scenes looking good without needing HDR tonemapping).
 */

#include "Light.hpp"
#include "../utilities/Point3D.hpp"
#include "../utilities/Vector3D.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Constants.hpp"

class PointLight : public Light {
private:

    Point3D  position;      // where the light is in the world
    RGBColor color;         // color of the light
    float    intensity;     // brightness multiplier

public:

    // Set position, color, and intensity.
    PointLight(const Point3D& pos, const RGBColor& col, float ls)
        : position(pos), color(col), intensity(ls) {}

    // Convenience: set position by coordinates.
    PointLight(float x, float y, float z,
               float r, float g, float b,
               float ls)
        : position(x, y, z), color(r, g, b), intensity(ls) {}

    // Copy constructor and assignment operator.
    PointLight(const PointLight& other) = default;
    PointLight& operator=(const PointLight& other) = default;

    virtual ~PointLight() = default;

    // Returns the unit vector from the hit point toward this light.
    virtual Vector3D get_direction(const ShadeInfo& sinfo) const override {
        Vector3D direction = position - sinfo.hit_point;
        direction.normalize();
        return direction;
    }

    // Returns color * intensity.
    virtual RGBColor get_radiance(const ShadeInfo& sinfo) const override {
        return color * intensity;
    }

    // Returns the actual distance from the hit point to the light.
    virtual float get_distance(const ShadeInfo& sinfo) const override {
        return sinfo.hit_point.distance(position);
    }
};