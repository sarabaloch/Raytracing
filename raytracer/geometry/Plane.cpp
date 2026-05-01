#include "Plane.hpp"
#include "../utilities/BBox.hpp"
#include "../utilities/Constants.hpp"
#include "../utilities/Ray.hpp"
#include "../utilities/ShadeInfo.hpp"

Plane::Plane() : Geometry(), a(Point3D(0.0f)), n(Vector3D(0.0, 1.0, 0.0)) {}

Plane::Plane(const Point3D &pointOnPlane, const Vector3D &planeNormal): Geometry(), a(pointOnPlane), n(planeNormal) {n.normalize();}

Plane::Plane(const Plane &object): Geometry(object), a(object.a), n(object.n) {}

Plane &Plane::operator=(const Plane &rhs) {
    Geometry::operator=(rhs);
    a = rhs.a;
    n = rhs.n;
    return *this;
}

std::string Plane::to_string() const {
    return "Plane[point=" + a.to_string() + ", normal=" + n.to_string() + "]";
}

bool Plane::hit(const Ray &ray, float &t, ShadeInfo &shadeInfo) const {
    double den = ray.d * n;

    if (std::abs(den) < kEpsilon) {
        return false;
    }

    double t_hit = ((a - ray.o) * n) / den;

    if (t_hit <= kEpsilon) {
        return false;
    }

    t = static_cast<float>(t_hit);

    shadeInfo.hit = true;
    shadeInfo.material_ptr = material_ptr;
    shadeInfo.t = t;
    shadeInfo.ray = ray;
    shadeInfo.hit_point = ray.o + ray.d * t_hit;
    shadeInfo.normal = n;
    shadeInfo.normal.normalize();

    return true;
}

BBox Plane::getBBox() const {
    return BBox(Point3D(-kHugeValue), Point3D(kHugeValue));
}