#ifndef STREET_WET_ASPHALT_HPP
#define STREET_WET_ASPHALT_HPP

/**
 * Wet road: moon key, rain skylift, pooled sodium-orange, faint lane modulation.
 */
#include "../materials/Material.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Vector3D.hpp"
#include "SceneLights.hpp"

#include <algorithm>
#include <cmath>

class StreetWetAsphalt : public Material {
private:
    RGBColor albedo;
    float soak;

public:
    StreetWetAsphalt(float r, float g, float b, float wet = 0.72f)
        : albedo(r, g, b), soak(std::clamp(wet, 0.f, 1.f)) {}

    RGBColor shade(const ShadeInfo &sinfo) const override {
        Vector3D N(sinfo.normal);
        N.normalize();
        Vector3D V = -sinfo.ray.d;
        V.normalize();

        Vector3D Lmoon = SceneLights::moon_direction_toward(sinfo.hit_point);
        Vector3D Lsky = SceneLights::rain_skylight_up();
        float k_moon = SceneLights::moon_illum_scale(sinfo.hit_point);

        float ndotl_m = std::clamp(static_cast<float>(N * Lmoon), 0.f, 1.f);
        float ndotl_sk = std::clamp(static_cast<float>(N * Lsky), 0.f, 1.f);

        RGBColor moon_tint = SceneLights::moon_spectrum();
        RGBColor amb = SceneLights::night_ambient_cool();

        float lane =
            std::abs(std::sin(sinfo.hit_point.x * 0.052f + sinfo.hit_point.z * 0.023f +
                              sinfo.hit_point.y * 0.11f));
        float pothole =
            std::abs(std::cos(sinfo.hit_point.z * 0.031f + sinfo.hit_point.x * 0.019f));

        RGBColor toned = albedo * (0.94f + 0.085f * lane - 0.06f * pothole);

        RGBColor diff = toned *
                        (amb * (1.06f + 0.62f * soak) +
                         moon_tint * k_moon * (0.32f * ndotl_m + 0.12f * soak * ndotl_sk));

        Vector3D Hmoon = (Lmoon + V);
        Hmoon.normalize();
        float spec_m = std::clamp(static_cast<float>(N * Hmoon), 0.f, 1.f);
        float exp_m = 9.f + soak * 88.f;

        RGBColor moon_gloss =
            moon_tint * k_moon * ((0.12f + 0.94f * soak) * std::pow(spec_m, exp_m));

        Vector3D Lpole(0.12, -0.88f, -0.45f);
        Lpole.normalize();
        Vector3D Hpole = ((-Lpole) + V);
        Hpole.normalize();
        float pole = std::clamp(static_cast<float>(N * Hpole), 0.f, 1.f);
        RGBColor sodium(1.06f, 0.74f, 0.42f);
        RGBColor street_pool =
            sodium * ((0.07f + 0.96f * soak) * std::pow(pole, exp_m * 0.74f));

        float NV = std::clamp(static_cast<float>(N * V), 0.f, 1.f);
        float wet_sheen = soak * std::pow(1.f - NV, 5.8f);

        RGBColor sum = diff + moon_gloss + street_pool + moon_tint * wet_sheen * 0.22f;

        return RGBColor(std::min(2.5f, sum.r), std::min(2.5f, sum.g),
                        std::min(2.85f, sum.b));
    }
};

#endif
