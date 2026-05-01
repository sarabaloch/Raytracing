#include "Parallel.hpp"
#include "../utilities/Point3D.hpp"

Parallel::Parallel() : Camera(), dir(Vector3D(0.0, 0.0, -1.0)) {}

Parallel::Parallel(float value): Camera(), dir(Vector3D(value, value, value)) {
    dir.normalize();
}

Parallel::Parallel(float x, float y, float z): Camera(), dir(Vector3D(x, y, z)) {
    dir.normalize();
}

Parallel::Parallel(const Vector3D &direction): Camera(), dir(direction) {
    dir.normalize();
}

Parallel::Parallel(const Parallel &other): Camera(other), dir(other.dir) {}

Parallel &Parallel::operator=(const Parallel &rhs) {
    Camera::operator=(rhs);
    dir = rhs.dir;
    return *this;
}

Vector3D Parallel::get_direction(const Point3D &p) const {
    (void)p;
    return dir;
}