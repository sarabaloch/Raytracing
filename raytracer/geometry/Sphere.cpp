#include "Sphere.hpp"
#include "../utilities/Ray.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/BBox.hpp"
#include "../utilities/Constants.hpp"
#include <cmath>

Sphere::Sphere() : Geometry(), c(Point3D(0, 0, 0)), r(0.0f) {}

Sphere::Sphere(const Point3D &center, float radius) : Geometry(), c(center), r(radius) {}

Sphere::Sphere(const Sphere &object) : Geometry(object), c(object.c), r(object.r) {}

Sphere &Sphere::operator=(const Sphere &rhs) {
    Geometry::operator=(rhs);
    c = rhs.c;
    r = rhs.r;
    return *this;
}

std::string Sphere::to_string() const {
    return "Sphere[center=" + c.to_string() + ", radius=" + std::to_string(r) + "]";
}

bool Sphere::hit(const Ray &ray, float &t, ShadeInfo &shadeInfo) const {
  Vector3D centerToOrigin = ray.o - c;

  double a = ray.d * ray.d;
  double b = 2.0 * (centerToOrigin * ray.d);
  double C = (centerToOrigin * centerToOrigin) - (r * r);

  double quadDisc = b * b - 4.0 * a * C;

  if (quadDisc < 0.0) {
    return false;
  }

  double disc = sqrt(quadDisc);
  double den = 2.0 * a;

  double rootA = (-b - disc) / den;
  double rootB = (-b + disc) / den;

  double t_hit;

  if (rootA > kEpsilon) {
      t_hit = rootA;
  } else if (rootB > kEpsilon) {
      t_hit = rootB;
  } else {
      return false;
  }

  t = static_cast<float>(t_hit);

  shadeInfo.hit = true;
  shadeInfo.material_ptr = material_ptr;
  shadeInfo.t = t;
  shadeInfo.ray = ray;
  shadeInfo.hit_point = ray.o + t_hit * ray.d;
  Vector3D normal = (shadeInfo.hit_point - c) / r;
  normal.normalize();
  shadeInfo.normal = normal;

  return true;

}
BBox Sphere::getBBox() const {
    float minX = c.x - r;
    float minY = c.y - r;
    float minZ = c.z - r;

    float maxX = c.x + r;
    float maxY = c.y + r;
    float maxZ = c.z + r;

    Point3D minimumCorner(minX, minY, minZ);
    Point3D maximumCorner(maxX, maxY, maxZ);

    return BBox(minimumCorner, maximumCorner);
}