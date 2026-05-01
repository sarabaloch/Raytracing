#include "ShadeInfo.hpp"
#include "../world/World.hpp"

ShadeInfo::ShadeInfo(const World &world) : hit(false), material_ptr(nullptr), hit_point(Point3D(0.0f)),normal(Vector3D(0.0)), 
ray(), depth(0), t(kHugeValue), w(&world) {}