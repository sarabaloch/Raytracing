#include "Point3D.hpp"
#include "Vector3D.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

Point3D::Point3D() : x(0.0f), y(0.0f), z(0.0f) {}

Point3D::Point3D(float value) : x(value), y(value), z(value) {}

Point3D::Point3D(float xValue, float yValue, float zValue): x(xValue), y(yValue), z(zValue) {}

std::string Point3D::to_string() const
{
    return "Point3D(" + std::to_string(x) + ", " +std::to_string(y) + ", " +std::to_string(z) + ")";
}

Point3D Point3D::operator-() const
{
    return Point3D(-x, -y, -z);
}

Vector3D Point3D::operator-(const Point3D &point) const
{
    return Vector3D(x - point.x, y - point.y, z - point.z);
}

Point3D Point3D::operator+(const Vector3D &vector) const
{
    return Point3D(x + vector.x, y + vector.y, z + vector.z);
}

Point3D Point3D::operator-(const Vector3D &vector) const
{
    return Point3D(x - vector.x, y - vector.y, z - vector.z);
}

Point3D Point3D::operator*(const float scale) const
{
    return Point3D(x * scale, y * scale, z * scale);
}

float Point3D::d_squared(const Point3D &point) const
{
    float dx = x - point.x;
    float dy = y - point.y;
    float dz = z - point.z;

    return dx * dx + dy * dy + dz * dz;
}

float Point3D::distance(const Point3D &point) const
{
    return std::sqrt(d_squared(point));
}

Point3D operator*(const float scale, const Point3D &point)
{
    return point * scale;
}

Point3D min(const Point3D &a, const Point3D &b)
{
    return Point3D(
        std::min(a.x, b.x),
        std::min(a.y, b.y),
        std::min(a.z, b.z));
}

Point3D max(const Point3D &a, const Point3D &b)
{
    return Point3D(
        std::max(a.x, b.x),
        std::max(a.y, b.y),
        std::max(a.z, b.z));
}