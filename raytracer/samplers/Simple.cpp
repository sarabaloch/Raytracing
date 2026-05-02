#include "Simple.hpp"
#include "../cameras/Camera.hpp"
#include "../utilities/Ray.hpp"
#include "../world/ViewPlane.hpp"

Simple::Simple(Camera *cameraPtr, ViewPlane *viewPlanePtr): Sampler(cameraPtr, viewPlanePtr) {}

Simple::Simple(const Simple &other): Sampler(other) {}

Simple &Simple::operator=(const Simple &rhs) {
    if (this != &rhs) {
        Sampler::operator=(rhs);
    }
    return *this;
}

std::vector<Ray> Simple::get_rays(int pixelX, int pixelY) const {
    const Point3D &topLeft     = viewplane_ptr->top_left;
    const Point3D &bottomRight = viewplane_ptr->bottom_right;

    double pixelWidth  = (bottomRight.x - topLeft.x) / static_cast<double>(viewplane_ptr->hres);
    double pixelHeight = (topLeft.y - bottomRight.y)  / static_cast<double>(viewplane_ptr->vres);

    double sampleX = topLeft.x + (pixelX + 0.5) * pixelWidth;
    double sampleY = topLeft.y - (pixelY + 0.5) * pixelHeight;

    Point3D samplePoint(
        static_cast<float>(sampleX),
        static_cast<float>(sampleY),
        topLeft.z
    );

    Vector3D direction = camera_ptr->get_direction(samplePoint);

    // Ray origin is the camera eye, not the view plane point.
    Point3D origin = camera_ptr->get_origin(samplePoint);

    Ray ray(origin, direction);
    ray.w = 1.0f;
    return { ray };
}