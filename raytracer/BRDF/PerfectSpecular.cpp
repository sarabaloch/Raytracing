#pragma once

/**
 * PerfectSpecular.hpp
 *
 * Implements a perfect specular (mirror) BRDF.
 *
 * A perfect mirror reflects light in exactly one direction: the
 * reflection of the incoming ray about the surface normal.
 *
 * BRDF value = kr * cr
 *   kr = reflection coefficient
 *   cr = reflection color (usually white for a clean mirror)
 *
 * Note: This BRDF is a Dirac delta — it only contributes when
 * wi is exactly the reflected direction of wo. In a basic tracer
 * without recursive rays this is used as an approximation only.
 */

#include "BRDF.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/Vector3D.hpp"
#include "../utilities/ShadeInfo.hpp"

class PerfectSpecular : public BRDF {
private:

    float    kr;    // reflection coefficient
    RGBColor cr;    // reflection color

public:

    // Set coefficient and color.
    PerfectSpecular(float reflection_coefficient, const RGBColor& reflection_color)
        : kr(reflection_coefficient), cr(reflection_color) {}

    // Copy constructor and assignment operator.
    PerfectSpecular(const PerfectSpecular& other) = default;
    PerfectSpecular& operator=(const PerfectSpecular& other) = default;

    virtual ~PerfectSpecular() = default;

    // Returns kr * cr.
    // In a full path tracer this would use a delta function, but here
    // we return the coefficient directly for use in Phong-style materials.
    virtual RGBColor evaluate(const ShadeInfo& sinfo,
                              const Vector3D& wi,
                              const Vector3D& wo) const override {
        RGBColor result = cr * kr;
        return result;
    }

    // Helper: compute the reflected direction of wo about the normal.
    // reflected = 2(N.wo)N - wo
    Vector3D reflect(const Vector3D& wo, const Vector3D& N) const {
        double n_dot_wo = N * wo;
        Vector3D reflected = N * (2.0 * n_dot_wo) - wo;
        reflected.normalize();
        return reflected;
    }
};