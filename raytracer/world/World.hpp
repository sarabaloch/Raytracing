#pragma once

/**
 * World.hpp
 *
 * The World holds everything in the scene:
 *   - All geometry objects
 *   - All light sources
 *   - The view plane, camera, and sampler
 *   - A tracer (Basic or Shadow)
 *   - An optional acceleration structure (BVH)
 *
 * Courtesy Kevin Suffern. Extended for CS 440 Project 2.
 */

#include <vector>

#include "../utilities/RGBColor.hpp"
#include "ViewPlane.hpp"

class Camera;
class Geometry;
class Light;
class Ray;
class Sampler;
class ShadeInfo;
class Tracer;
class Acceleration;

class World {
public:

    ViewPlane               vplane;         // the view plane
    RGBColor                bg_color;       // background color
    std::vector<Geometry*>  geometry;       // all geometry in the scene
    std::vector<Light*>     lights;         // all light sources
    Camera*                 camera_ptr;     // the camera
    Sampler*                sampler_ptr;    // the sampler
    Tracer*                 tracer_ptr;     // the tracer (Basic or Shadow)
    Acceleration*           accel_ptr;      // BVH (null = brute force)

public:

    // Constructor — sets all pointers to null.
    World();

    // Destructor — frees all heap memory.
    ~World();

    // Add geometry to the scene.
    void add_geometry(Geometry* geom_ptr);

    // Add a light source to the scene.
    void add_light(Light* light_ptr);

    // Set the camera (deletes the old one first).
    void set_camera(Camera* c_ptr);

    // Build the scene — defined in a separate build/*.cpp file.
    void build();

    // Test a ray against all geometry and return shading info for the closest hit.
    // Uses BVH if accel_ptr is set, otherwise brute-force.
    ShadeInfo hit_objects(const Ray& ray);

    // Test a shadow ray: returns true if anything blocks it before max_distance.
    // Uses BVH if accel_ptr is set, otherwise brute-force.
    bool in_shadow(const Ray& shadow_ray, float max_distance) const;
};