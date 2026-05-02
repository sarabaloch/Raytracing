#ifndef EMISSIVE_HPP
#define EMISSIVE_HPP

#include "../materials/Material.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Vector3D.hpp"

#include <algorithm>

/**
 * Self-luminous surfaces with softer centers and brighter rims (neon bloom & moon disc falloff).
 */
class Emissive : public Material {
private:
    RGBColor color;
    float radiance;

public:
    Emissive(float r, float g, float b, float rad = 1.0f) : color(r, g, b), radiance(rad) {}

    Emissive(const RGBColor &c, float rad = 1.0f) : color(c), radiance(rad) {}

    virtual ~Emissive() {}

    virtual RGBColor shade(const ShadeInfo &sinfo) const override {
        Vector3D view = -sinfo.ray.d;
        view.normalize();
        float mu = std::clamp(static_cast<float>(sinfo.normal * view), 0.f, 1.f);
        float limb = std::sqrt(mu);
        return color * (radiance * (0.62f + 0.58f * limb));
    }

    virtual RGBColor get_Le(const ShadeInfo &sinfo) const { return color * radiance; }
};

#endif // EMISSIVE_HPP
