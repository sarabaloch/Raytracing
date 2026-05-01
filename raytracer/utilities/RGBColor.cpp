#include "RGBColor.hpp"
#include <cmath>
#include <sstream>

RGBColor::RGBColor() : r(0.0f), g(0.0f), b(0.0f) {}

RGBColor::RGBColor(float value) : r(value), g(value), b(value) {}

RGBColor::RGBColor(float redChannel, float greenChannel, float blueChannel) : r(redChannel), g(greenChannel), b(blueChannel) {}

std::string RGBColor::to_string() const {
    return "RGBColor(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ")";
}

RGBColor RGBColor::operator+(const RGBColor &otherColor) const {
    float newRed = r + otherColor.r;
    float newGreen = g + otherColor.g;
    float newBlue = b + otherColor.b;

    return RGBColor(newRed, newGreen, newBlue);
}

RGBColor &RGBColor::operator+=(const RGBColor &otherColor) {
    r = r + otherColor.r;
    g = g + otherColor.g;
    b = b + otherColor.b;
    return *this;
}

RGBColor RGBColor::operator*(float scaleFactor) const {
    return RGBColor(r * scaleFactor, g * scaleFactor, b * scaleFactor);
}

RGBColor &RGBColor::operator*=(float scaleFactor) {
    r = r * scaleFactor;
    g = g * scaleFactor;
    b = b * scaleFactor;
    return *this;
}

RGBColor RGBColor::operator/(float divisor) const {
    return RGBColor(r / divisor, g / divisor, b / divisor);
}

RGBColor &RGBColor::operator/=(float divisor) {
    r = r / divisor;
    g = g / divisor;
    b = b / divisor;
    return *this;
}

RGBColor RGBColor::operator*(const RGBColor &otherColor) const {
    float mixedRed = r * otherColor.r;
    float mixedGreen = g * otherColor.g;
    float mixedBlue = b * otherColor.b;

    return RGBColor(mixedRed, mixedGreen, mixedBlue);
}

bool RGBColor::operator==(const RGBColor &otherColor) const {
    bool orgRed = (r == otherColor.r);
    bool orgGreen = (g == otherColor.g);
    bool orgBlue = (b == otherColor.b);

    return orgRed && orgGreen && orgBlue;
}

RGBColor RGBColor::powc(float powerValue) const {
    float newRed = std::pow(r, powerValue);
    float newGreen = std::pow(g, powerValue);
    float newBlue = std::pow(b, powerValue);

    return RGBColor(newRed, newGreen, newBlue);
}

float RGBColor::average() const {
    float total = r + g + b;
    return total / 3.0f;
}

RGBColor operator*(float scaleFactor, const RGBColor &color) {
    return color * scaleFactor;
}