#pragma once

/**
 * Jittered.hpp
 *
 * A jittered sampler that reduces aliasing by shooting multiple rays
 * per pixel, each offset randomly within a sub-cell of the pixel.
 *
 * Each pixel is divided into an n x n grid of sub-cells.
 * One ray is shot through a random point inside each sub-cell.
 * All rays have equal weight: 1 / (n * n).
 *
 * This gives much smoother edges than the Simple sampler (1 ray per pixel)
 * while being more evenly distributed than pure random sampling.
 */

#include "Sampler.hpp"
#include "../utilities/Ray.hpp"
#include "../world/ViewPlane.hpp"
#include "../cameras/Camera.hpp"

#include <vector>
#include <cstdlib>

class Jittered : public Sampler {
private:
    int grid_size;      // number of sub-cells per side (total rays = grid_size^2)

public:

    // Constructor: set camera, view plane, and grid size.
    // grid_size = 2 gives 4 rays per pixel (2x2 grid).
    // grid_size = 3 gives 9 rays per pixel (3x3 grid).
    Jittered(Camera* camera, ViewPlane* viewplane, int n);

    // Copy constructor and assignment operator.
    Jittered(const Jittered& other);
    Jittered& operator=(const Jittered& other);

    // Destructor.
    virtual ~Jittered() = default;

    // Return jittered rays for pixel (px, py).
    // One ray per sub-cell, randomly offset within that sub-cell.
    virtual std::vector<Ray> get_rays(int px, int py) const override;
};