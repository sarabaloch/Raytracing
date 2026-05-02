#ifndef RAIN_STREAK_MAT_HPP
#define RAIN_STREAK_MAT_HPP

#include "../materials/Material.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Vector3D.hpp"
#include "SceneLights.hpp"

#include <algorithm>
#include <cmath>

/**
 * Dark, razor-thin streaks that pop only under moon/specular peaks (no transparency support).
 */
class RainStreakMat : public Material {
public:
    virtual RGBColor shade(const ShadeInfo &sinfo) const override {
        Vector3D N(sinfo.normal);
        N.normalize();
        Vector3D V = -sinfo.ray.d;
        V.normalize();

        Vector3D LtowardMoon = SceneLights::moon_direction_toward(sinfo.hit_point);
        float moon_k = SceneLights::moon_illum_scale(sinfo.hit_point);

        Vector3D LtowardSky = SceneLights::rain_skylight_up();

        float moon_face = std::clamp(static_cast<float>(N * LtowardMoon), 0.f, 1.f);
        float sky_face = std::clamp(static_cast<float>(N * LtowardSky), 0.f, 1.f);

        /* Heavy rain reads as silver-blue streaks catching moon + billboard spill */
        RGBColor tint(0.11f, 0.13f, 0.158f);

        RGBColor diffuse =
            tint * (0.065f + 0.155f * sky_face + 0.58f * moon_k * moon_face);

        Vector3D Hmoon = (LtowardMoon + V);
        Hmoon.normalize();
        float nh = std::clamp(static_cast<float>(N * Hmoon), 0.f, 1.f);
        RGBColor glitter = RGBColor(0.78f, 0.87f, 1.06f) *
                           moon_k * (2.45f * std::pow(nh, 88.f) + 0.55f * std::pow(nh, 24.f));

        Vector3D Hwarm(-0.1f, -0.35f, 0.88f); // grazing street-lamp halo from deep scene
        Hwarm.normalize();
        Hwarm = (Hwarm + V);
        Hwarm.normalize();
        float nhw = std::clamp(static_cast<float>(N * Hwarm), 0.f, 1.f);
        RGBColor amber = RGBColor(0.74f, 0.62f, 0.48f) * (0.12f * std::pow(nhw, 74.f)); /* faint city bounce */

        RGBColor sum = diffuse + glitter + amber;
        return RGBColor(std::min(2.85f, sum.r), std::min(3.05f, sum.g), std::min(3.35f, sum.b));
    }
};

#endif
