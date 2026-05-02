#include "Geometry.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../world/World.hpp"

Geometry::Geometry() : material_ptr(nullptr) {}
Material *Geometry::get_material() const { return material_ptr; }

void Geometry::set_material(Material *materialPointer) {
  material_ptr = materialPointer;
}

bool Geometry::shadow_hit(const Ray &ray, float max_distance) const {
  float t;
  ShadeInfo sinfo;
  return hit(ray, t, sinfo) && t > 0.0001f && t < max_distance;
}