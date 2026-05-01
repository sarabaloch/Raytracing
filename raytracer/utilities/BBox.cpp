#include "BBox.hpp"
#include "Ray.hpp"
#include "../geometry/Geometry.hpp"

BBox::BBox(const Point3D &minPoint, const Point3D &maxPoint): pmin(minPoint), pmax(maxPoint) {}

std::string BBox::to_string() const {
    return "BBox[min=" + pmin.to_string() + ", max=" + pmax.to_string() + "]";
}

bool BBox::hit(const Ray &ray, float &tEnter, float &tExit) const {
    double tMinX, tMaxX;
    double tMinY, tMaxY;
    double tMinZ, tMaxZ;

    if (ray.d.x >= 0.0) {
        tMinX = (pmin.x - ray.o.x) / ray.d.x;
        tMaxX = (pmax.x - ray.o.x) / ray.d.x;
    } else {
        tMinX = (pmax.x - ray.o.x) / ray.d.x;
        tMaxX = (pmin.x - ray.o.x) / ray.d.x;
    }

    if (ray.d.y >= 0.0) {
        tMinY = (pmin.y - ray.o.y) / ray.d.y;
        tMaxY = (pmax.y - ray.o.y) / ray.d.y;
    } else {
        tMinY = (pmax.y - ray.o.y) / ray.d.y;
        tMaxY = (pmin.y - ray.o.y) / ray.d.y;
    }

    if (ray.d.z >= 0.0) {
        tMinZ = (pmin.z - ray.o.z) / ray.d.z;
        tMaxZ = (pmax.z - ray.o.z) / ray.d.z;
    } else {
        tMinZ = (pmax.z - ray.o.z) / ray.d.z;
        tMaxZ = (pmin.z - ray.o.z) / ray.d.z;
    }

    double tNear = std::max(std::max(tMinX, tMinY), tMinZ);
    double tFar = std::min(std::min(tMaxX, tMaxY), tMaxZ);

    if (tNear > tFar) {
        return false;
    }

    tEnter = static_cast<float>(tNear);
    tExit = static_cast<float>(tFar);

    return true;
}

void BBox::extend(Geometry *geometryObject) {
    extend(geometryObject->getBBox());
}

void BBox::extend(const BBox &other) {
    pmin = min(pmin, other.pmin);
    pmax = max(pmax, other.pmax);
}

bool BBox::contains(const Point3D &point) {
    return (point.x >= pmin.x && point.x <= pmax.x) &&
           (point.y >= pmin.y && point.y <= pmax.y) &&
           (point.z >= pmin.z && point.z <= pmax.z);
}

bool BBox::overlaps(Geometry *geometryObject) {
    return overlaps(geometryObject->getBBox());
}

bool BBox::overlaps(const BBox &other) {
    return (pmin.x <= other.pmax.x && pmax.x >= other.pmin.x) &&
           (pmin.y <= other.pmax.y && pmax.y >= other.pmin.y) &&
           (pmin.z <= other.pmax.z && pmax.z >= other.pmin.z);
}