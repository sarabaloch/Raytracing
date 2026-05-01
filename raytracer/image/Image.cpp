#include "Image.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "../utilities/RGBColor.hpp"
#include "../world/ViewPlane.hpp"
#include <vector>

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
            imageBytes[index + 0] = static_cast<unsigned char>(pixelColor.r * 255.0f);
            imageBytes[index + 1] = static_cast<unsigned char>(pixelColor.g * 255.0f);
            imageBytes[index + 2] = static_cast<unsigned char>(pixelColor.b * 255.0f);
        }
    }

    int stride = hres * colours;
    stbi_write_png(path.c_str(), hres, vres, colours, imageBytes, stride);

    delete[] imageBytes;
}