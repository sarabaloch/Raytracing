#ifndef PUDDLE_HPP
#define PUDDLE_HPP

/**
 * Puddle.hpp
 *
 * Material for wet rooftop puddle patches.
 *
 * A puddle on a dark rooftop acts like a dark mirror — it reflects the
 * bright neon signs above it. We simulate this with a strong self-luminous
 * reflection component (not dependent on light attenuation), using:
 *   1. A very dark water-body base colour.
 *   2. A bright Fresnel-boosted reflection of the neon colour, strongest
 *      at grazing angles (looking across the puddle surface).
 *   3. A sinusoidal ripple pattern that breaks up the reflection so it
 *      looks like disturbed water rather than a flat mirror.
 *   4. A rain-drop ripple distortion based on world position.
 *
 * The reflectColor should be the dominant neon sign colour visible from
 * above the puddle (magenta, purple, orange etc).
 */

#include "../materials/Material.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Vector3D.hpp"
#include "../utilities/Point3D.hpp"

#include <cmath>
#include <algorithm>

class Puddle : public Material {
private:
    RGBColor reflectColor;
    float    wetness;

public:
    Puddle(const RGBColor& ref, float wet = 0.85f)
        : reflectColor(ref), wetness(wet) {}

    virtual ~Puddle() = default;

    virtual RGBColor shade(const ShadeInfo& sinfo) const override {
        Vector3D V = -sinfo.ray.d;
        V.normalize();
        Vector3D N = sinfo.normal;
        N.normalize();

        float ndotv = std::max(0.0f, static_cast<float>(N * V));

        // Schlick Fresnel — water is very reflective at grazing angles.
        float F0  = 0.04f;
        float fre = F0 + (1.0f - F0) * std::pow(1.0f - ndotv, 5.0f);

        // Boost the base fresnel so puddles are visible even straight-on.
        float reflStrength = fre * 3.5f + 0.25f;
        if (reflStrength > 1.0f) reflStrength = 1.0f;

        // Ripple pattern: two overlapping sine waves to simulate disturbed water.
        float px = sinfo.hit_point.x * 0.22f;
        float pz = sinfo.hit_point.z * 0.28f;
        float ripple1 = 0.5f + 0.5f * std::sin(px * 1.3f) * std::cos(pz * 0.9f);
        float ripple2 = 0.5f + 0.5f * std::sin(px * 0.7f + 1.2f) * std::cos(pz * 1.5f);
        float ripple  = 0.5f * ripple1 + 0.5f * ripple2;

        // Rain-drop concentric rings (finer detail).
        float dist = std::sqrt(px * px + pz * pz);
        float rings = 0.5f + 0.5f * std::sin(dist * 4.5f);
        ripple = 0.7f * ripple + 0.3f * rings;

        // Very dark water base with slight purple tint.
        RGBColor base(0.01f, 0.00f, 0.02f);

        // Neon reflection: bright, ripple-modulated, fresnel-boosted.
        RGBColor refl = reflectColor * (wetness * reflStrength * (0.4f + 0.6f * ripple));

        return base + refl;
    }
};

#endif // PUDDLE_HPP