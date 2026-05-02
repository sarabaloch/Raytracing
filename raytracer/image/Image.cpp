#include "Image.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "../utilities/RGBColor.hpp"
#include "../world/ViewPlane.hpp"
#include <vector>
#include <cmath>

Image::Image(int horizontalRes, int verticalRes): hres(horizontalRes), vres(verticalRes) {

    colors = new RGBColor *[hres];
    for (int x = 0; x < hres; x++) {
        colors[x] = new RGBColor[vres];
    }
}

Image::Image(const ViewPlane &viewPlane): Image(viewPlane.hres, viewPlane.vres) {}

Image::~Image() {
    for (int x = 0; x < hres; x++) {
        delete[] colors[x];
    }
    delete[] colors;
}

void Image::set_pixel(int x, int y, const RGBColor &color) {
    colors[x][y] = color;
}

void Image::write_png(std::string path) const {
    int colours = 3;
    unsigned char *imageBytes = new unsigned char[hres * vres * colours];

    for (int y = 0; y < vres; y++) {
        for (int x = 0; x < hres; x++) {
            const RGBColor &pixelColor = colors[x][y];

            int index = (y * hres + x) * colours;
    auto tobyte = [](float v) -> unsigned char {
        // Reinhard tone mapping: maps HDR [0,inf) to [0,1), then to [0,255]
        v = v / (1.0f + v);
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
        // Gamma correction (approximate sRGB)
        v = std::pow(v, 1.0f / 2.2f);
        return static_cast<unsigned char>(v * 255.0f);
    };
    imageBytes[index + 0] = tobyte(pixelColor.r);
    imageBytes[index + 1] = tobyte(pixelColor.g);
    imageBytes[index + 2] = tobyte(pixelColor.b);
        }
    }

    int stride = hres * colours;
    stbi_write_png(path.c_str(), hres, vres, colours, imageBytes, stride);

    delete[] imageBytes;
}