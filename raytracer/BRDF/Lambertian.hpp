#pragma once

/**
 * Lambertian.hpp
 *
 * Implements a Lambertian (perfectly diffuse) BRDF.
 *
 * A Lambertian surface scatters incoming light equally in all directions.
 * The BRDF value is: (kd * cd) / PI
 *   kd = diffuse coefficient  (0 to 1)
 *   cd = diffuse color
 */

#include "BRDF.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/Constants.hpp"

class Lambertian : public BRDF {
private:

    float    kd;    // diffuse reflection coefficient (0 = none, 1 = full)
    RGBColor cd;    // diffuse color

public:

    // Set coefficient and color directly.
    Lambertian(float diffuse_coefficient, const RGBColor& diffuse_color)
        : kd(diffuse_coefficient), cd(diffuse_color) {}

    // Copy constructor and assignment operator.
    Lambertian(const Lambertian& other) = default;
    Lambertian& operator=(const Lambertian& other) = default;

    virtual ~Lambertian() = default;

    // Returns kd * cd / PI — the Lambertian BRDF is constant.
    // It does not depend on wi or wo.
    virtual RGBColor evaluate(const ShadeInfo& sinfo,
                              const Vector3D& wi,
                              const Vector3D& wo) const override {
        RGBColor result = cd * kd * invPI;
        return result;
    }
};