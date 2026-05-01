#include "Ray.hpp"
#include <sstream>

Ray::Ray() : o(Point3D(0.0f)), d(Vector3D(0.0)), w(1.0f) {}

Ray::Ray(const Point3D &origin, const Vector3D &dir) : o(origin), d(dir), w(1.0f) {d.normalize();}

std::string Ray::to_string() const {
    return "Ray[origin=" + o.to_string() + ", dir=" + d.to_string() + ", w=" + std::to_string(w) + "]";
}