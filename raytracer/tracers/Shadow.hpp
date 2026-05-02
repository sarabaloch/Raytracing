#pragma once

/**
 * Shadow.hpp
 *
 * An extended tracer that adds shadow rays.
 *
 * For each primary ray hit, it loops over all lights in the scene
 * and shoots a shadow ray toward each one. If the shadow ray is
 * blocked, that light does not contribute to the final color.
 *
 * Shading model used here:
 *   color = ambient + sum over unblocked lights of (diffuse + specular)
 *
 * The diffuse term uses Lambert's law: max(0, N dot L).
 * The specular term uses Blinn-Phong: max(0, N dot H)^shininess.
 */

#include "Tracer.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Vector3D.hpp"
#include "../utilities/Ray.hpp"
#include "../utilities/Constants.hpp"
#include "../world/World.hpp"

#include <cmath>
#include <algorithm>

class Shadow : public Tracer {
public:

    // Constructor.
    Shadow(World* world) : Tracer(world) {}

    // Copy constructor and assignment operator.
    Shadow(const Shadow& other) = default;
    Shadow& operator=(const Shadow& other) = default;

    virtual ~Shadow() = default;

    // Trace a primary ray, then test shadow rays for each light.
    virtual RGBColor trace_ray(const Ray& ray) const override {

        // Test the primary ray against all geometry.
        ShadeInfo sinfo = world_ptr->hit_objects(ray);

        // If nothing was hit, return the background color.
        if (!sinfo.hit) {
            return world_ptr->bg_color;
        }

        // Start with the material's own shade() result as the base.
        // This includes ambient and any baked-in lighting from the material.
        RGBColor color = sinfo.material_ptr->shade(sinfo);

        // Get the surface normal at the hit point.
        Vector3D N = sinfo.normal;
        N.normalize();

        // View direction: from hit point toward the camera.
        Vector3D V = -sinfo.ray.d;
        V.normalize();

        // Loop over every light in the scene.
        for (int i = 0; i < (int)world_ptr->lights.size(); i++) {

            // Get the direction from hit point toward this light.
            Vector3D L = world_ptr->lights[i]->get_direction(sinfo);
            L.normalize();

            // Get how far away this light is.
            float light_distance = world_ptr->lights[i]->get_distance(sinfo);

            // Build a shadow ray starting just above the surface.
            // We offset by kEpsilon along the normal to avoid self-intersection.
            Point3D shadow_origin = sinfo.hit_point + N * kEpsilon;
            Ray shadow_ray(shadow_origin, L);

            // Check if anything blocks the path to this light.
            bool blocked = world_ptr->in_shadow(shadow_ray, light_distance);

            // If the light is blocked, skip it — this light casts a shadow here.
            if (blocked) {
                continue;
            }

            // Lambert diffuse: how much the surface faces this light.
            double n_dot_l = N * L;
            if (n_dot_l < 0.0) {
                n_dot_l = 0.0;
            }

            // Blinn-Phong specular: half-vector between L and V.
            Vector3D H = L + V;
            H.normalize();
            double n_dot_h = N * H;
            if (n_dot_h < 0.0) {
                n_dot_h = 0.0;
            }

            // Get this light's radiance.
            RGBColor light_radiance = world_ptr->lights[i]->get_radiance(sinfo);

            // Diffuse contribution.
            RGBColor diffuse = light_radiance * static_cast<float>(n_dot_l) * 0.6f;

            // Specular contribution (shininess = 32 for a moderate gloss).
            float specular_factor = static_cast<float>(std::pow(n_dot_h, 32.0));
            RGBColor specular = light_radiance * specular_factor * 0.3f;

            // Add both contributions to the final color.
            color = color + diffuse + specular;
        }

        return color;
    }
};