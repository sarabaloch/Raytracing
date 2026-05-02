#ifndef REFLECTIVE_HPP
#define REFLECTIVE_HPP

#include "../materials/Material.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "SceneLights.hpp"

#include <algorithm>
#include <cmath>

/**
 * Wet pavement / glazing: faux indirect + moon + skylight highlights (no recursion in this tracer).
 */
class Reflective : public Material {
private:
    RGBColor base_color;
    float reflectivity;
    float wet_boost; /* boosts spec lobes beyond reflectivity knob */

public:
    Reflective(float r, float g, float b, float refl = 0.5f, float glossy = 1.0f)
        : base_color(r, g, b), reflectivity(std::clamp(refl, 0.f, 1.f)),
          wet_boost(std::max(0.15f, glossy)) {}

    Reflective(const RGBColor &c, float refl = 0.5f, float glossy = 1.0f)
        : base_color(c), reflectivity(std::clamp(refl, 0.f, 1.f)), wet_boost(std::max(0.15f, glossy)) {}

    virtual ~Reflective() {}

    virtual RGBColor shade(const ShadeInfo &sinfo) const override {
        Vector3D N(sinfo.normal);
        N.normalize();
        Vector3D V = -sinfo.ray.d;
        V.normalize();

        Vector3D LtowardMoon = SceneLights::moon_direction_toward(sinfo.hit_point);
        float moon_k = SceneLights::moon_illum_scale(sinfo.hit_point);

        Vector3D LtowardSky = SceneLights::rain_skylight_up();

        float moon_ndotl = std::clamp(static_cast<float>(N * LtowardMoon), 0.f, 1.f);
        float sky_ndotl = std::clamp(static_cast<float>(N * LtowardSky), 0.f, 1.f);

        RGBColor moon_rgb = SceneLights::moon_spectrum();

        RGBColor cool_amb = SceneLights::night_ambient_cool();

        float diffuse_weight = std::clamp(1.f - reflectivity * 0.58f, 0.09f, 1.f);

        RGBColor diffuse =
            base_color *
            (cool_amb + moon_rgb * moon_k * (0.28f * moon_ndotl + 0.085f * sky_ndotl));

        diffuse = diffuse * diffuse_weight;

        Vector3D Hmoon = (LtowardMoon + V);
        Hmoon.normalize();
        float spec_moon = std::clamp(static_cast<float>(N * Hmoon), 0.f, 1.f);
        float shininess_m = (14.f + reflectivity * 110.f + wet_boost * 52.f);

        RGBColor glossy_moon = moon_rgb * moon_k * ((0.16f + 2.95f * reflectivity * wet_boost) *
                                                    std::pow(spec_moon, shininess_m));

        Vector3D LneonBounce(-0.35f, -0.82f, 0.45f); // aggregated neon bounce from alley depth
        LneonBounce.normalize();

        Vector3D Hneon = ((-LneonBounce) + V);
        Hneon.normalize();
        float spec_neon = std::clamp(static_cast<float>(N * Hneon), 0.f, 1.f);
        RGBColor neon_paint(1.08f, 0.52f, 0.92f); // magenta/pink bleed
        RGBColor electric(0.32f, 0.74f, 1.18f);   // bar blues
        RGBColor bleed = neon_paint * 0.62f + electric * 0.38f;
        RGBColor glossy_neon =
            bleed *
            (0.09f + 0.55f * reflectivity * wet_boost) *
            std::pow(spec_neon, shininess_m * 0.9f); /* subdued — moon owns the rim */

        Vector3D Lwarm(-0.05f, -0.93f, -0.28f); // sodium / tungsten pooled on asphalt
        Lwarm.normalize();
        Vector3D Hw = ((-Lwarm) + V);
        Hw.normalize();
        float wet_spec = std::clamp(static_cast<float>(N * Hw), 0.f, 1.f);
        RGBColor lamp(1.06f, 0.71f, 0.43f);

        RGBColor glossy_warm =
            lamp *
            ((0.08f + 2.05f * reflectivity * wet_boost) * std::pow(wet_spec, shininess_m * 1.06f));

        float NV = std::clamp(static_cast<float>(N * V), 0.f, 1.f);
        float fresnel =
            reflectivity *
            std::clamp(0.11f + 0.87f * std::pow(1.f - NV, 4.85f), 0.f, 1.f);

        RGBColor ref_tint =
            (moon_rgb * 0.52f + bleed * RGBColor(0.28f, 0.36f, 0.62f)) * fresnel * wet_boost *
            RGBColor(0.48f, 0.55f, 0.88f);

        RGBColor sum = diffuse + glossy_moon + glossy_neon + glossy_warm + ref_tint;

        return RGBColor(std::min(3.25f, sum.r), std::min(3.25f, sum.g), std::min(3.45f, sum.b));
    }
};

#endif // REFLECTIVE_HPP
