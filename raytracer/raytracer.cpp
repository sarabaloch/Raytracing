#include <iostream>
#include "image/Image.hpp"
#include "materials/Cosine.hpp"
#include "samplers/Sampler.hpp"
#include "utilities/RGBColor.hpp"
#include "utilities/Ray.hpp"
#include "utilities/ShadeInfo.hpp"
#include "world/ViewPlane.hpp"
#include "world/World.hpp"

int main() {
    World sceneWorld;
    sceneWorld.build();

    Sampler *sceneSampler = sceneWorld.sampler_ptr;
    ViewPlane &sceneViewPlane = sceneWorld.vplane;
    Image result(sceneViewPlane);
    std::vector<Ray> pixelRays;

    for (int x = 0; x < sceneViewPlane.hres; x++) {
        for (int y = 0; y < sceneViewPlane.vres; y++) {
            RGBColor finalPixelColor(0.0f);
            pixelRays = sceneSampler->get_rays(x, y);
            for (const Ray &currentRay : pixelRays) {
                float rayWeight = currentRay.w;
                ShadeInfo hitResult = sceneWorld.hit_objects(currentRay);

                if (hitResult.hit) {
                    RGBColor shadedColor = hitResult.material_ptr->shade(hitResult);
                    finalPixelColor = finalPixelColor + rayWeight * shadedColor;
                } else {
                    finalPixelColor = finalPixelColor + rayWeight * sceneWorld.bg_color;
                }
            }
            result.set_pixel(x, y, finalPixelColor);
        }
    }
    result.write_png("Raytracing.png");
    return 0;
}