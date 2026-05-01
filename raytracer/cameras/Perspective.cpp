#include "Perspective.hpp"
#include "../utilities/Vector3D.hpp"

Perspective::Perspective() : Camera(), pos(Point3D(0.0f)) {}

Perspective::Perspective(float value): Camera(), pos(Point3D(value)) {}

Perspective::Perspective(float x, float y, float z): Camera(), pos(Point3D(x, y, z)) {}

Perspective::Perspective(const Point3D &camera_pos): Camera(), pos(camera_pos) {}

Perspective::Perspective(const Perspective &other): Camera(other), pos(other.pos) {}

Perspective &Perspective::operator=(const Perspective &rhs) {
    Camera::operator=(rhs);
    pos = rhs.pos;
    return *this;
}

Vector3D Perspective::get_direction(const Point3D &targetPoint) const {
    Vector3D directionToPixel = targetPoint - pos;
    directionToPixel.normalize();
    return directionToPixel;
}