#include "Sampler.hpp"

Sampler::Sampler() : camera_ptr(nullptr), viewplane_ptr(nullptr) {}

Sampler::Sampler(Camera *cameraPointer, ViewPlane *viewPlanePointer) : camera_ptr(cameraPointer), viewplane_ptr(viewPlanePointer) {}
