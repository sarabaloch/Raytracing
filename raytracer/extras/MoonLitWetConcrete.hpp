#ifndef MOONLIT_WET_CONCRETE_HPP
#define MOONLIT_WET_CONCRETE_HPP

#include "../materials/Material.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Vector3D.hpp"
#include "SceneLights.hpp"

#include <algorithm>
#include <cmath>

/**
 * Damp Gotham masonry / asphalt paint: moon key light, skylight bounce, oily wet specular lobes.
 */
class MoonLitWetConcrete : public Material {
private:
    RGBColor albedo;
    float damp; /* 0=dry-ish, 1=soaked */

public:
    MoonLitWetConcrete(float r, float g, float b, float wetness = 0.55f)
        : albedo(r, g, b), damp(std::clamp(wetness, 0.f, 1.f)) {}

    MoonLitWetConcrete(const RGBColor &c, float wetness = 0.55f)
        : albedo(c), damp(std::clamp(wetness, 0.f, 1.f)) {}

    virtual RGBColor shade(const ShadeInfo &sinfo) const override {
        Vector3D N(sinfo.normal);
        N.normalize();
        Vector3D V = -sinfo.ray.d;
        V.normalize();

        Vector3D Lmoon = SceneLights::moon_direction_toward(sinfo.hit_point);
        float moon_k = SceneLights::moon_illum_scale(sinfo.hit_point);

        Vector3D LcoolFill = SceneLights::rain_skylight_up();

        float moon_ndotl = std::clamp(static_cast<float>(N * Lmoon), 0.f, 1.f);
        float fill_ndotl =
            std::clamp(static_cast<float>(N * LcoolFill), 0.f, 1.f); /* skylight hits upward faces */

        RGBColor moon_tint = SceneLights::moon_spectrum();

        RGBColor amb_base = SceneLights::night_ambient_cool();

        RGBColor ambient(amb_base.r + 0.012f * damp, amb_base.g + 0.028f * damp,
                         amb_base.b + 0.042f * damp);
        RGBColor diff =
            albedo *
            (ambient + moon_tint * moon_k * (0.28f * moon_ndotl + 0.15f * damp * fill_ndotl));

        Vector3D Hmoon = (Lmoon + V);
        Hmoon.normalize();
        float spec_m = std::clamp(static_cast<float>(N * Hmoon), 0.f, 1.f);
        float exp_m = 10.f + damp * 90.f;
        RGBColor moon_spec =
            moon_tint * moon_k *
            ((0.12f + 0.75f * damp) * std::pow(spec_m, exp_m));

        Vector3D Lstreet(0.15f, -0.82f, -0.55f); // aggregated warm bounce from roadway
        Lstreet.normalize();
        Vector3D Hw = ((-Lstreet) + V);
        Hw.normalize();
        float street_gloss = std::clamp(static_cast<float>(N * Hw), 0.f, 1.f);
        RGBColor warm_bounce(0.55f, 0.32f, 0.22f);
        RGBColor amber_spec =
            warm_bounce * ((0.04f + 0.52f * damp) * std::pow(street_gloss, exp_m * 0.82f));

        float NV = std::clamp(static_cast<float>(N * V), 0.f, 1.f);
        float rim = damp * std::pow(1.f - NV, 4.2f) * 1.05f;

        RGBColor sum = diff + moon_spec + amber_spec + moon_tint * rim;
        return RGBColor(std::min(2.2f, sum.r), std::min(2.2f, sum.g), std::min(2.4f, sum.b));
    }
};

#endif
