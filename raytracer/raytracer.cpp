/**
 * raytracer.cpp
 *
 * Main entry point. Builds the world, then traces rays through every pixel.
 * The actual shading logic lives in the Tracer subclass set by World::build().
 *
 * Extended for CS 440 Project 2:
 *   - Uses world.tracer_ptr to trace rays (supports Basic and Shadow tracers).
 *   - Uses world.sampler_ptr to get rays per pixel (Simple or Jittered).
 *   - Uses world.accel_ptr for fast intersection if set (BVH).
 */

#include <iostream>

#include "image/Image.hpp"
#include "samplers/Sampler.hpp"
#include "tracers/Tracer.hpp"
#include "utilities/RGBColor.hpp"
#include "utilities/Ray.hpp"
#include "utilities/ShadeInfo.hpp"
#include "world/ViewPlane.hpp"
#include "world/World.hpp"

int main() {

    // Build the scene.
    World world;
    world.build();

    // Get references to the sampler and view plane.
    Sampler*   sampler   = world.sampler_ptr;
    Tracer*    tracer    = world.tracer_ptr;
    ViewPlane& viewplane = world.vplane;

    // Create an image the same size as the view plane.
    Image result(viewplane);

    // Loop over every pixel in the image.
    for (int x = 0; x < viewplane.hres; x++) {
        for (int y = 0; y < viewplane.vres; y++) {

            // Accumulate color from all rays for this pixel.
            RGBColor pixel_color(0.0f);

            // Get all rays for this pixel (one for Simple, many for Jittered).
            std::vector<Ray> rays = sampler->get_rays(x, y);

            // Trace each ray and accumulate the weighted color.
            for (int r = 0; r < (int)rays.size(); r++) {
                RGBColor ray_color = tracer->trace_ray(rays[r]);
                pixel_color = pixel_color + rays[r].w * ray_color;
            }

            // Write the final color to the image.
            result.set_pixel(x, y, pixel_color);
        }
    }

    // Save the image to disk.
    result.write_png("Raytracing.png");

    std::cout << "Done. Saved to Raytracing.png" << std::endl;

    return 0;
}