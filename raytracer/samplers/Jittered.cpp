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

    double pixel_width  = (viewplane_ptr->bottom_right.x - viewplane_ptr->top_left.x)
                          / static_cast<double>(viewplane_ptr->hres);
    double pixel_height = (viewplane_ptr->top_left.y - viewplane_ptr->bottom_right.y)
                          / static_cast<double>(viewplane_ptr->vres);

    double cell_width  = pixel_width  / static_cast<double>(grid_size);
    double cell_height = pixel_height / static_cast<double>(grid_size);

    double pixel_left = viewplane_ptr->top_left.x + px * pixel_width;
    double pixel_top  = viewplane_ptr->top_left.y - py * pixel_height;

    int   total_rays = grid_size * grid_size;
    float ray_weight = 1.0f / static_cast<float>(total_rays);

    std::vector<Ray> rays;
    rays.reserve(total_rays);

    for (int row = 0; row < grid_size; row++) {
        for (int col = 0; col < grid_size; col++) {

            double jitter_x = static_cast<double>(rand()) * invRAND_MAX;
            double jitter_y = static_cast<double>(rand()) * invRAND_MAX;

            double sample_x = pixel_left + (col + jitter_x) * cell_width;
            double sample_y = pixel_top  - (row + jitter_y) * cell_height;

            Point3D sample_point(
                static_cast<float>(sample_x),
                static_cast<float>(sample_y),
                viewplane_ptr->top_left.z
            );

            Vector3D direction = camera_ptr->get_direction(sample_point);
            Point3D  origin    = camera_ptr->get_origin(sample_point);

            Ray ray(origin, direction);
            ray.w = ray_weight;
            rays.push_back(ray);
        }
    }

    return rays;
}