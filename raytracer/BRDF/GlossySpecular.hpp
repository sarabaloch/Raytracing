#pragma once

/**
 * GlossySpecular.hpp
 *
 * Implements a glossy specular BRDF using the Blinn-Phong model.
 *
 * The Blinn-Phong model computes specular highlights using the half-vector
 * between the incoming light direction and the outgoing view direction.
 *
 * BRDF value = ks * cs * (N dot H)^exp
 *   ks  = specular coefficient
 *   cs  = specular color
 *   H   = normalized half-vector between wi and wo
 *   exp = shininess exponent (higher = sharper highlight)
 */

#include "BRDF.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/Vector3D.hpp"
#include "../utilities/ShadeInfo.hpp"

#include <cmath>
#include <algorithm>

class GlossySpecular : public BRDF {
private:

    float    ks;        // specular reflection coefficient
    RGBColor cs;        // specular color
    float    exp;       // shininess exponent

public:

    // Set coefficient, color, and shininess.
    GlossySpecular(float specular_coefficient,
                   const RGBColor& specular_color,
                   float shininess)
        : ks(specular_coefficient), cs(specular_color), exp(shininess) {}

    // Copy constructor and assignment operator.
    GlossySpecular(const GlossySpecular& other) = default;
    GlossySpecular& operator=(const GlossySpecular& other) = default;

    virtual ~GlossySpecular() = default;

    // Computes the Blinn-Phong specular term.
    virtual RGBColor evaluate(const ShadeInfo& sinfo,
                              const Vector3D& wi,
                              const Vector3D& wo) const override {

        // Get the surface normal from the shade info.
        Vector3D N = sinfo.normal;
        N.normalize();

        // Compute the half-vector between incoming and outgoing directions.
        Vector3D H = wi + wo;
        H.normalize();

        // Dot product of normal and half-vector (clamped to avoid negatives).
        double n_dot_h = N * H;
        if (n_dot_h < 0.0) {
            n_dot_h = 0.0;
        }

        // Blinn-Phong: ks * cs * (N.H)^exp
        float specular_intensity = static_cast<float>(std::pow(n_dot_h, exp));
        RGBColor result = cs * (ks * specular_intensity);
        return result;
    }
};