#pragma once

/**
 * BRDF.hpp
 *
 * Abstract base class for all BRDFs (Bidirectional Reflectance
 * Distribution Functions).
 *
 * A BRDF describes how light is reflected at a surface point.
 * Given an incoming light direction and an outgoing view direction,
 * it returns how much light is reflected.
 */

class RGBColor;
class Vector3D;
class ShadeInfo;

class BRDF {
public:

    // Default constructor — does nothing.
    BRDF() = default;

    // Copy constructor and assignment operator.
    BRDF(const BRDF& other) = default;
    BRDF& operator=(const BRDF& other) = default;

    // Virtual destructor so subclasses can clean up.
    virtual ~BRDF() = default;

    // Returns the BRDF value for the given incoming and outgoing directions.
    // wi  = incoming light direction (toward the light)
    // wo  = outgoing view direction  (toward the camera)
    // Returns the color/weight contribution of this BRDF.
    virtual RGBColor evaluate(const ShadeInfo& sinfo,
                              const Vector3D& wi,
                              const Vector3D& wo) const = 0;
};