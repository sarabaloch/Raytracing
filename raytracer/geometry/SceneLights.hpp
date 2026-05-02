#ifndef SCENE_LIGHTS_HPP
#define SCENE_LIGHTS_HPP

/**
 * Shared night-scene lighting for Gotham / moonlit materials.
 * Keep moon_center() in sync with the emissive moon sphere in build/ArkhamKnight.cpp.
 */
#include "../utilities/Point3D.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/Vector3D.hpp"

#include <algorithm>
#include <cmath>

namespace SceneLights {

inline Point3D moon_center() { return Point3D(-86.f, 112.f, -198.f); }

inline constexpr float MOON_SPHERE_RADIUS = 44.f;

/** Unit vector from surface point toward the moon disc. */
inline Vector3D moon_direction_toward(const Point3D &surf) {
    Vector3D L = moon_center() - surf;
    L.normalize();
    return L;
}

/** Soft inverse-square style falloff so distant streets dim slightly vs. rooftops. */
inline float moon_illum_scale(const Point3D &surf) {
    float dsq = surf.d_squared(moon_center());
    return std::clamp(210000.f / std::max(dsq, 5200.f), 0.46f, 1.18f);
}

inline Vector3D rain_skylight_up() {
    Vector3D L(0.05, 0.992, -0.12);
    L.normalize();
    return L;
}

inline RGBColor moon_spectrum() { return RGBColor(0.46f, 0.58f, 0.84f); }

inline RGBColor night_ambient_cool() { return RGBColor(0.014f, 0.02f, 0.032f); }

} // namespace SceneLights

#endif
