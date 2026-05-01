#include "Triangle.hpp"
#include "../utilities/BBox.hpp"
#include "../utilities/Constants.hpp"
#include "../utilities/Ray.hpp"
#include "../utilities/ShadeInfo.hpp"
#include <cmath>

Triangle::Triangle(): Geometry(),v0(Point3D(0.0f)),v1(Point3D(0.0f)),v2(Point3D(0.0f)) {}

Triangle::Triangle(const Point3D &firstVertex,const Point3D &secondVertex, const Point3D &thirdVertex): Geometry(),v0(firstVertex),
      v1(secondVertex),v2(thirdVertex) {}

Triangle::Triangle(const Triangle &object): Geometry(object), v0(object.v0), v1(object.v1), v2(object.v2) {}

Triangle &Triangle::operator=(const Triangle &rhs) {
    Geometry::operator=(rhs);
    v0 = rhs.v0;
    v1 = rhs.v1;
    v2 = rhs.v2;
    return *this;
}

std::string Triangle::to_string() const {
    return "Triangle[v0=" + v0.to_string() + ", v1=" + v1.to_string() + ", v2=" + v2.to_string() + "]";
}

bool Triangle::hit(const Ray &ray, float &t, ShadeInfo &shadeInfo) const {
    Vector3D e1 = v1 - v0;
    Vector3D e2 = v2 - v0;

    Vector3D pvec = ray.d ^ e2;
    double det = e1 * pvec;

    if (std::abs(det) < kEpsilon) {
        return false;
    }

    double invD= 1.0 / det;

    Vector3D tvec = ray.o - v0;
    double u = (tvec * pvec) * invD;

    if (u < 0.0 || u > 1.0) {
        return false;
    }

    Vector3D qvec = tvec ^ e1;
    double v = (ray.d * qvec) * invD;

    if (v < 0.0 || (u + v) > 1.0) {
        return false;
    }

    double t_hit = (e2 * qvec) * invD;

    if (t_hit <= kEpsilon) {
        return false;
    }

    t = static_cast<float>(t_hit);

    shadeInfo.hit = true;
    shadeInfo.material_ptr = material_ptr;
    shadeInfo.t = t;
    shadeInfo.ray = ray;
    shadeInfo.hit_point = ray.o + ray.d * t_hit;

    Vector3D normal = e1 ^ e2;
    normal.normalize();
    shadeInfo.normal = normal;

    return true;
}

BBox Triangle::getBBox() const {
    Point3D smallestBetweenV1AndV2 = min(v1, v2);
    Point3D overallMinimum = min(v0, smallestBetweenV1AndV2);

    Point3D largestBetweenV1AndV2 = max(v1, v2);
    Point3D overallMaximum = max(v0, largestBetweenV1AndV2);

    return BBox(overallMinimum, overallMaximum);
}