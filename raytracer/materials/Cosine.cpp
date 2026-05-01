#include <iostream>
#include "Cosine.hpp"
#include "../utilities/ShadeInfo.hpp"

using namespace std;

Cosine::Cosine() : Material(), color(RGBColor(0.0f)) {}

Cosine::Cosine(float value) : Material(), color(RGBColor(value)) {}

Cosine::Cosine(float red, float green, float blue) : Material(), color(RGBColor(red, green, blue)) {}

Cosine::Cosine(const RGBColor& materialColor) : Material(), color(materialColor) {}

Cosine::Cosine(const Cosine& other) : Material(other), color(other.color) {}

Cosine& Cosine::operator=(const Cosine& rhs) {
    Material::operator=(rhs);
    color = rhs.color;
    return *this;
}

RGBColor Cosine::shade(const ShadeInfo& shadeInfo) const {
    Vector3D surfaceNormal = shadeInfo.normal;
    surfaceNormal.normalize();
    Vector3D lightDirection = -shadeInfo.ray.d;
    lightDirection.normalize();

    double brightness = max(0.0, surfaceNormal * lightDirection);
    return color * static_cast<float>(brightness);
}