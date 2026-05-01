#include "Geometry.hpp"

Geometry::Geometry() : material_ptr(nullptr) {}
Material *Geometry::get_material() const { return material_ptr; }

void Geometry::set_material(Material *materialPointer) {
  material_ptr = materialPointer;
}
