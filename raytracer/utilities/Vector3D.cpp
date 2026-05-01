#include "Vector3D.hpp"
#include "Point3D.hpp"
#include <cmath>
#include <sstream>

Vector3D::Vector3D() : x(0.0), y(0.0), z(0.0) {}

Vector3D::Vector3D(double value) : x(value), y(value), z(value) {}

Vector3D::Vector3D(double xComp, double yComp, double zComp) : x(xComp), y(yComp), z(zComp) {}

Vector3D::Vector3D(const Point3D &point): x(point.x), y(point.y), z(point.z) {}

Vector3D &Vector3D::operator=(const Point3D &point) {
    x = point.x;
    y = point.y;
    z = point.z;
    return *this;
}

std::string Vector3D::to_string() const {
    return "Vector3D(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
}

Vector3D Vector3D::operator-() const {
    return Vector3D(-x, -y, -z);
}

Vector3D Vector3D::operator+(const Vector3D &otherVector) const {
    double newX = x + otherVector.x;
    double newY = y + otherVector.y;
    double newZ = z + otherVector.z;

    return Vector3D(newX, newY, newZ);
}

Vector3D &Vector3D::operator+=(const Vector3D &otherVector) {
    x = x + otherVector.x;
    y = y + otherVector.y;
    z = z + otherVector.z;
    return *this;
}

Vector3D Vector3D::operator-(const Vector3D &otherVector) const {
    double newX = x - otherVector.x;
    double newY = y - otherVector.y;
    double newZ = z - otherVector.z;

    return Vector3D(newX, newY, newZ);
}

Vector3D &Vector3D::operator-=(const Vector3D &otherVector) {
    x = x - otherVector.x;
    y = y - otherVector.y;
    z = z - otherVector.z;
    return *this;
}

Vector3D Vector3D::operator*(double scaleFactor) const {
    return Vector3D(x * scaleFactor, y * scaleFactor, z * scaleFactor);
}

Vector3D Vector3D::operator/(double divisor) const {
    return Vector3D(x / divisor, y / divisor, z / divisor);
}

void Vector3D::normalize() {
    double vectorLength = length();

    if (vectorLength > 0.0) {
        x = x / vectorLength;
        y = y / vectorLength;
        z = z / vectorLength;
    }
}

double Vector3D::length() const {
    double squaredLength = len_squared();
    return std::sqrt(squaredLength);
}

double Vector3D::len_squared() const {
    return x * x + y * y + z * z;
}

double Vector3D::operator*(const Vector3D &otherVector) const {
    double xProduct = x * otherVector.x;
    double yProduct = y * otherVector.y;
    double zProduct = z * otherVector.z;

    return xProduct + yProduct + zProduct;
}

Vector3D Vector3D::operator^(const Vector3D &otherVector) const {
    double crossX = y * otherVector.z - z * otherVector.y;
    double crossY = z * otherVector.x - x * otherVector.z;
    double crossZ = x * otherVector.y - y * otherVector.x;

    return Vector3D(crossX, crossY, crossZ);
}

Vector3D operator*(double scaleFactor, const Vector3D &vector) {
    return vector * scaleFactor;
}