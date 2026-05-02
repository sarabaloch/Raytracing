/**
 * Jittered.cpp
 *
 * Implementation of the jittered sampler.
 */

#include "Jittered.hpp"
#include "../utilities/Constants.hpp"

Jittered::Jittered(Camera* camera, ViewPlane* viewplane, int n)
    : Sampler(camera, viewplane), grid_size(n) {}

Jittered::Jittered(const Jittered& other)
    : Sampler(other), grid_size(other.grid_size) {}

Jittered& Jittered::operator=(const Jittered& other) {
    if (this != &other) {
        Sampler::operator=(other);
        grid_size = other.grid_size;
    }
    return *this;
}

std::vector<Ray> Jittered::get_rays(int px, int py) const {

    // Compute pixel dimensions in world space.
    double pixel_width  = (viewplane_ptr->bottom_right.x - viewplane_ptr->top_left.x)
                          / static_cast<double>(viewplane_ptr->hres);
    double pixel_height = (viewplane_ptr->top_left.y    - viewplane_ptr->bottom_right.y)
                          / static_cast<double>(viewplane_ptr->vres);

    // Sub-cell dimensions.
    double cell_width  = pixel_width  / static_cast<double>(grid_size);
    double cell_height = pixel_height / static_cast<double>(grid_size);

    // World-space top-left corner of this pixel.
    double pixel_left = viewplane_ptr->top_left.x + px * pixel_width;
    double pixel_top  = viewplane_ptr->top_left.y - py * pixel_height;

    // Total rays = grid_size * grid_size.
    int total_rays = grid_size * grid_size;

    // Weight of each ray: equal share of the pixel.
    float ray_weight = 1.0f / static_cast<float>(total_rays);

    std::vector<Ray> rays;
    rays.reserve(total_rays);

    // Loop over each row of sub-cells.
    for (int row = 0; row < grid_size; row++) {

        // Loop over each column of sub-cells.
        for (int col = 0; col < grid_size; col++) {

            // Random offset within this sub-cell.
            // invRAND_MAX converts rand() output to [0, 1].
            double jitter_x = static_cast<double>(rand()) * invRAND_MAX;
            double jitter_y = static_cast<double>(rand()) * invRAND_MAX;

            // World-space X of this sample.
            double sample_x = pixel_left + (col + jitter_x) * cell_width;

            // World-space Y of this sample (Y decreases downward).
            double sample_y = pixel_top  - (row + jitter_y) * cell_height;

            // Z is the view plane Z.
            float sample_z = viewplane_ptr->top_left.z;

            // Build the sample point on the view plane.
            Point3D sample_point(
                static_cast<float>(sample_x),
                static_cast<float>(sample_y),
                sample_z
            );

            // Get ray direction from camera.
            Vector3D direction = camera_ptr->get_direction(sample_point);

            // Build the ray and assign its weight.
            Ray ray(sample_point, direction);
            ray.w = ray_weight;

            rays.push_back(ray);
        }
    }

    return rays;
}