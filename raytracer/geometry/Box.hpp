#ifndef BOX_HPP
#define BOX_HPP

#include "../geometry/Geometry.hpp"
#include "../utilities/Point3D.hpp"
#include "../utilities/Vector3D.hpp"
#include "../utilities/Ray.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/BBox.hpp"
#include <cmath>
#include <string>
#include <algorithm>

/**
 * Axis-Aligned Bounding Box (AABB)
 * Defined by two corner points: min and max
 */
class Box : public Geometry {
private:
    Point3D p0;  // Minimum corner (x_min, y_min, z_min)
    Point3D p1;  // Maximum corner (x_max, y_max, z_max)

public:
    // Constructor
    Box(const Point3D& corner1, const Point3D& corner2);
    
    virtual ~Box() {}
    
    // String representation
    virtual std::string to_string() const;
    
    // Get bounding box
    virtual BBox getBBox() const;
    
    // Ray-box intersection test
    virtual bool hit(const Ray& ray, float& t, ShadeInfo& sinfo) const;
    
    // Shadow ray intersection (optimized, no shading info needed)
    virtual bool shadow_hit(const Ray& ray, float& tmin) const;

private:
    // Helper to compute normal based on which face was hit
    Vector3D get_normal(const Point3D& hit_point) const;
};

// Implementation

Box::Box(const Point3D& corner1, const Point3D& corner2) {
    // Ensure p0 has minimum coordinates, p1 has maximum
    p0.x = std::min(corner1.x, corner2.x);
    p0.y = std::min(corner1.y, corner2.y);
    p0.z = std::min(corner1.z, corner2.z);
    
    p1.x = std::max(corner1.x, corner2.x);
    p1.y = std::max(corner1.y, corner2.y);
    p1.z = std::max(corner1.z, corner2.z);
}

bool Box::hit(const Ray& ray, float& t, ShadeInfo& sinfo) const {
    float tx_min, ty_min, tz_min;
    float tx_max, ty_max, tz_max;
    
    // X slab
    float a = 1.0f / ray.d.x;
    if (a >= 0) {
        tx_min = (p0.x - ray.o.x) * a;
        tx_max = (p1.x - ray.o.x) * a;
    } else {
        tx_min = (p1.x - ray.o.x) * a;
        tx_max = (p0.x - ray.o.x) * a;
    }
    
    // Y slab
    float b = 1.0f / ray.d.y;
    if (b >= 0) {
        ty_min = (p0.y - ray.o.y) * b;
        ty_max = (p1.y - ray.o.y) * b;
    } else {
        ty_min = (p1.y - ray.o.y) * b;
        ty_max = (p0.y - ray.o.y) * b;
    }
    
    // Z slab
    float c = 1.0f / ray.d.z;
    if (c >= 0) {
        tz_min = (p0.z - ray.o.z) * c;
        tz_max = (p1.z - ray.o.z) * c;
    } else {
        tz_min = (p1.z - ray.o.z) * c;
        tz_max = (p0.z - ray.o.z) * c;
    }
    
    // Find largest entering t and smallest exiting t
    float t0 = std::max(tx_min, std::max(ty_min, tz_min));
    float t1 = std::min(tx_max, std::min(ty_max, tz_max));
    
    // Check if ray intersects box
    if (t0 > t1) return false;
    
    // Use t0 (entry point) unless it's behind ray origin
    if (t0 > kEpsilon) {
        t = t0;
    } else if (t1 > kEpsilon) {
        t = t1;  // Ray origin is inside box
    } else {
        return false;  // Box is behind ray
    }
    
    // Fill in ShadeInfo
    sinfo.hit = true;
    sinfo.material_ptr = material_ptr;
    sinfo.hit_point = ray.o + ray.d * t;
    sinfo.normal = get_normal(sinfo.hit_point);
    sinfo.ray = ray;
    sinfo.t = t;
    
    return true;
}

bool Box::shadow_hit(const Ray& ray, float& tmin) const {
    float tx_min, ty_min, tz_min;
    float tx_max, ty_max, tz_max;
    
    float a = 1.0f / ray.d.x;
    if (a >= 0) {
        tx_min = (p0.x - ray.o.x) * a;
        tx_max = (p1.x - ray.o.x) * a;
    } else {
        tx_min = (p1.x - ray.o.x) * a;
        tx_max = (p0.x - ray.o.x) * a;
    }
    
    float b = 1.0f / ray.d.y;
    if (b >= 0) {
        ty_min = (p0.y - ray.o.y) * b;
        ty_max = (p1.y - ray.o.y) * b;
    } else {
        ty_min = (p1.y - ray.o.y) * b;
        ty_max = (p0.y - ray.o.y) * b;
    }
    
    float c = 1.0f / ray.d.z;
    if (c >= 0) {
        tz_min = (p0.z - ray.o.z) * c;
        tz_max = (p1.z - ray.o.z) * c;
    } else {
        tz_min = (p1.z - ray.o.z) * c;
        tz_max = (p0.z - ray.o.z) * c;
    }
    
    float t0 = std::max(tx_min, std::max(ty_min, tz_min));
    float t1 = std::min(tx_max, std::min(ty_max, tz_max));
    
    if (t0 > t1) return false;
    
    if (t0 > kEpsilon) {
        tmin = t0;
        return true;
    }
    
    if (t1 > kEpsilon) {
        tmin = t1;
        return true;
    }
    
    return false;
}

Vector3D Box::get_normal(const Point3D& hit_point) const {
    const float epsilon = 0.0001f;
    
    // Determine which face was hit by checking proximity to each face
    if (std::abs(hit_point.x - p0.x) < epsilon)
        return Vector3D(-1, 0, 0);  // Left face
    if (std::abs(hit_point.x - p1.x) < epsilon)
        return Vector3D(1, 0, 0);   // Right face
    
    if (std::abs(hit_point.y - p0.y) < epsilon)
        return Vector3D(0, -1, 0);  // Bottom face
    if (std::abs(hit_point.y - p1.y) < epsilon) {
        float gx =
            std::cos(hit_point.x * 14.17f + hit_point.z * 6.43f) * 0.085f +
            std::sin(hit_point.z * 11.71f + hit_point.x * 3.83f) * 0.048f;
        float gz =
            std::sin(hit_point.z * 13.21f + hit_point.x * 5.53f) * 0.088f +
            std::cos(hit_point.x * 9.93f + hit_point.z * 7.91f) * 0.041f;

        Vector3D wet(gx, 1.0, gz);
        wet.normalize();
        return wet;
    }

    if (std::abs(hit_point.z - p0.z) < epsilon)
        return Vector3D(0, 0, -1);  // Front face
    if (std::abs(hit_point.z - p1.z) < epsilon)
        return Vector3D(0, 0, 1);   // Back face
    
    // Fallback (shouldn't happen)
    return Vector3D(0, 1, 0);
}

std::string Box::to_string() const {
    std::string str = "Box: min=(";
    str += std::to_string(p0.x) + ", " + std::to_string(p0.y) + ", " + std::to_string(p0.z) + "), max=(";
    str += std::to_string(p1.x) + ", " + std::to_string(p1.y) + ", " + std::to_string(p1.z) + ")";
    return str;
}

BBox Box::getBBox() const {
    return BBox(p0, p1);
}

#endif // BOX_HPP