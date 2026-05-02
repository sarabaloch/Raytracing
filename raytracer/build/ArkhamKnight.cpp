/**
 * Rooftop overlook: VP at z≈10, camera farther +z. Moon + skylight routed via SceneLights.hpp.
 */
#include "../cameras/Perspective.hpp"
#include "../extras/Box.hpp"
#include "../extras/Emissive.hpp"
#include "../extras/MoonLitWetConcrete.hpp"
#include "../extras/RainStreakMat.hpp"
#include "../extras/Reflective.hpp"
#include "../extras/SceneLights.hpp"
#include "../extras/StreetWetAsphalt.hpp"
#include "../geometry/Plane.hpp"
#include "../geometry/Sphere.hpp"
#include "../materials/Cosine.hpp"
#include "../samplers/Simple.hpp"
#include "../world/World.hpp"

#include <cmath>

void World::build(void) {
    /* Suffern-style view plane — fixed z slice; rays march into −Z territory */
    vplane.top_left = Point3D(-26.f, 48.f, 10.f);
    vplane.bottom_right = Point3D(26.f, 12.f, 10.f);
    vplane.hres = 960;
    vplane.vres = 540;

    bg_color = RGBColor(0.042f, 0.058f, 0.098f); /* bruised raining sky */

    /* Standing on deck: elevated Y, VP just in front toward the sprawl (−Z) */
    set_camera(new Perspective(1.95f, 41.92f, 23.85f));
    sampler_ptr = new Simple(camera_ptr, &vplane);

    auto tower = [&](float x0, float x1, float z0, float z1, float y0, float y1, float w) {
        Box *b = new Box(Point3D(x0, y0, z0), Point3D(x1, y1, z1));
        b->set_material(new MoonLitWetConcrete(0.052f, 0.057f, 0.074f, w));
        add_geometry(b);
    };

    auto glass_strip = [&](float x0, float x1, float y0, float y1, float z_near, float z_thick) {
        Box *g = new Box(Point3D(x0, y0, z_near), Point3D(x1, y1, z_near + z_thick));
        g->set_material(new Reflective(0.12f, 0.16f, 0.24f, 0.88f, 2.55f));
        add_geometry(g);
    };

    auto window_band = [&](float x0, float x1, float y0, float y1, float z_face, float inset) {
        Box *w = new Box(Point3D(x0, y0, z_face - inset), Point3D(x1, y1, z_face + 0.06f));
        w->set_material(new Cosine(0.022f, 0.028f, 0.038f));
        add_geometry(w);
    };

    auto neon = [&](float x, float y, float z, float r, float g, float b, float rad) {
        Sphere *s = new Sphere(Point3D(x, y, z), 0.62f);
        s->set_material(new Emissive(r, g, b, rad));
        add_geometry(s);
    };

    auto rain_bar = [&](float x, float y_mid, float z, float span) {
        Box *rb = new Box(Point3D(x - 0.048f, y_mid - span * 0.5f, z - 0.048f),
                          Point3D(x + 0.048f, y_mid + span * 0.5f, z + 0.048f));
        rb->set_material(new RainStreakMat());
        add_geometry(rb);
    };

    /* Moon — MUST match SceneLights::moon_center() + MOON_SPHERE_RADIUS */
    Sphere *moon =
        new Sphere(SceneLights::moon_center(), SceneLights::MOON_SPHERE_RADIUS);
    moon->set_material(new Emissive(0.8f, 0.87f, 1.06f, 2.62f));
    add_geometry(moon);

    /*
     * Roof deck slab: capped below the VP slice so rays always leave z=10 into the city (−z),
     * but still cross the planks underfoot.
     */
    Box *roof = new Box(Point3D(-34.f, 40.35f, -52.f), Point3D(58.f, 41.03f, 9.92f));
    roof->set_material(new Reflective(0.07f, 0.074f, 0.084f, 0.52f, 1.94f));
    add_geometry(roof);

    Sphere *puddle_a = new Sphere(Point3D(14.f, 41.06f, -6.f), 3.05f);
    puddle_a->set_material(new Reflective(0.04f, 0.049f, 0.058f, 0.74f, 2.08f));
    add_geometry(puddle_a);
    Sphere *puddle_b = new Sphere(Point3D(-10.f, 41.03f, 2.f), 2.55f);
    puddle_b->set_material(new Reflective(0.048f, 0.056f, 0.065f, 0.7f, 1.96f));
    add_geometry(puddle_b);

    /* Parapet + roof clutter for scale */
    Box *par_w = new Box(Point3D(-34.f, 41.f, -52.f), Point3D(-31.2f, 42.4f, 9.92f));
    par_w->set_material(new MoonLitWetConcrete(0.068f, 0.069f, 0.074f, 0.73f));
    add_geometry(par_w);
    Box *par_e = new Box(Point3D(55.f, 41.f, -52.f), Point3D(58.f, 42.35f, 9.92f));
    par_e->set_material(new MoonLitWetConcrete(0.068f, 0.069f, 0.074f, 0.73f));
    add_geometry(par_e);
    Box *hvac_a = new Box(Point3D(4.f, 41.02f, -18.f), Point3D(22.f, 43.95f, 2.f));
    hvac_a->set_material(new MoonLitWetConcrete(0.06f, 0.063f, 0.069f, 0.71f));
    add_geometry(hvac_a);
    Box *hvac_b = new Box(Point3D(-18.f, 41.f, -32.f), Point3D(-7.f, 42.95f, -16.f));
    hvac_b->set_material(new MoonLitWetConcrete(0.061f, 0.065f, 0.072f, 0.69f));
    add_geometry(hvac_b);

    /* Street basin + arterial roads readable from above */
    Plane *bowl =
        new Plane(Point3D(0.f, -54.5f, 0.f), Vector3D(0.f, 1.f, 0.f));
    bowl->set_material(new StreetWetAsphalt(0.03f, 0.032f, 0.038f, 0.74f));
    add_geometry(bowl);

    Box *artery_ns = new Box(Point3D(-11.f, -54.32f, -240.f), Point3D(11.f, -53.9f, 28.f));
    artery_ns->set_material(new StreetWetAsphalt(0.029f, 0.032f, 0.042f, 0.76f));
    add_geometry(artery_ns);

    Box *artery_ns2 = new Box(Point3D(118.f, -54.32f, -220.f), Point3D(148.f, -53.9f, 20.f));
    artery_ns2->set_material(new StreetWetAsphalt(0.03f, 0.032f, 0.04f, 0.73f));
    add_geometry(artery_ns2);

    Box *artery_we = new Box(Point3D(-240.f, -54.34f, -138.f), Point3D(240.f, -53.95f, -112.f));
    artery_we->set_material(new StreetWetAsphalt(0.029f, 0.032f, 0.041f, 0.74f));
    add_geometry(artery_we);

    Box *boulevard_loop =
        new Box(Point3D(-120.f, -54.37f, -58.f), Point3D(190.f, -54.f, -32.f));
    boulevard_loop->set_material(new StreetWetAsphalt(0.028f, 0.032f, 0.043f, 0.71f));
    add_geometry(boulevard_loop);

    /* Skyline silhouettes farther along −Z */
    tower(-118.f, -56.f, -210.f, -170.f, -54.f, 108.f, 0.82f);
    glass_strip(-116.f, -57.f, -15.f, 84.f, -170.f, 2.6f);
    for (float y = -12.f; y < 94.f; y += 17.f)
        window_band(-110.f, -63.f, y, y + 7.f, -170.f, 0.4f);

    tower(-62.f, 6.f, -198.f, -154.f, -54.f, 96.f, 0.79f);
    glass_strip(-60.f, 4.f, -8.f, 76.f, -154.f, 2.3f);
    for (float y = 6.f; y < 82.f; y += 13.f)
        window_band(-55.f, 0.f, y, y + 6.f, -154.f, 0.35f);

    tower(26.f, 84.f, -208.f, -166.f, -54.f, 118.f, 0.81f);
    glass_strip(28.f, 82.f, 0.f, 98.f, -166.f, 2.5f);
    for (float y = 8.f; y < 100.f; y += 14.f)
        window_band(33.f, 78.f, y, y + 7.f, -166.f, 0.38f);

    tower(-92.f, -18.f, -130.f, -94.f, -54.f, 72.f, 0.76f);
    glass_strip(-90.f, -20.f, -20.f, 56.f, -94.f, 1.95f);

    tower(94.f, 168.f, -182.f, -158.f, -54.f, 104.f, 0.8f);
    glass_strip(96.f, 165.f, -6.f, 90.f, -158.f, 2.55f);

    tower(-154.f, -108.f, -146.f, -124.f, -54.f, 92.f, 0.74f);
    glass_strip(-152.f, -110.f, -4.f, 72.f, -124.f, 2.05f);

    tower(136.f, 192.f, -140.f, -118.f, -54.f, 78.f, 0.73f);

    tower(-36.f, 24.f, -122.f, -98.f, -54.f, 62.f, 0.71f);

    neon(-92.f, 34.f, -118.f, 0.95f, 0.22f, 1.07f, 9.8f);
    neon(58.f, 58.f, -152.f, 0.32f, 0.93f, 1.06f, 8.9f);
    neon(154.f, 66.f, -134.f, 1.06f, 0.52f, 0.74f, 8.35f);
    neon(-154.f, 44.f, -136.f, 0.94f, 0.93f, 1.06f, 7.85f);

    /* Rain veil between camera slice and skyline */
    for (int i = 0; i < 54; ++i) {
        float t = static_cast<float>(i) / 53.f;
        float x = -24.f + 48.f * t + std::sin(t * 6.283f * 5.17f) * 6.f;
        float z = 6.f - 210.f * t;
        rain_bar(x, 46.f + std::sin(t * 43.f) * 14.f, z, 16.f + std::cos(t * 31.f) * 4.f);
    }
}
