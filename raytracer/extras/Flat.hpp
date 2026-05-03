#ifndef FLAT_HPP
#define FLAT_HPP

/**
 * Flat.hpp
 *
 * A material that returns a constant colour regardless of the surface normal
 * or lighting direction.  Used for building silhouettes, where we want a
 * solid, unlit dark face that will then receive coloured light from the
 * Shadow tracer's diffuse/specular pass.
 *
 * shade() returns:   ambient_scale * color
 * The Shadow tracer then adds diffuse + specular on top of this.
 */

#include "../materials/Material.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/ShadeInfo.hpp"

class Flat : public Material {
private:
    RGBColor color;
    float    ambient; // fraction of base color returned as ambient (0–1)

public:
    Flat(const RGBColor& c, float amb = 0.08f) : color(c), ambient(amb) {}
    Flat(float r, float g, float b, float amb = 0.08f)
        : color(r, g, b), ambient(amb) {}

    virtual ~Flat() = default;

    virtual RGBColor shade(const ShadeInfo& /*sinfo*/) const override {
        return color * ambient;
    }
};

#endif // FLAT_HPP