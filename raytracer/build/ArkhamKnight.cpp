/**
 * ArkhamKnight.cpp
 *
 * Rain-soaked Gotham rooftop scene.
 *
 * The camera stands on a high rooftop deck and looks out over the city
 * skyline. The moon hangs high and left, casting cold blue light over wet
 * concrete, reflective glass towers, neon signs, and rain-slicked streets
 * far below.
 *
 * To reach 1 million primitives the scene fills the city volume with:
 *   - 8 major tower blocks (the hero buildings, visible in foreground)
 *   - A grid of 300 x 300 = 90,000 background city blocks (the sprawl)
 *   - 900,000 rain drop spheres scattered across the frame volume
 *   - Rooftop detail: deck slab, puddles, parapets, HVAC units
 *   - Street basin + 4 arterial road slabs
 *   - Moon sphere, 4 neon glow spheres
 *
 * Features used:
 *   - Shadow tracer  (world.tracer_ptr = new Shadow)
 *   - Jittered sampler 2x2  (world.sampler_ptr = new Jittered(..., 2))
 *   - BVH acceleration  (world.accel_ptr = new BVH)
 *   - DirectionalLight for the moon
 *   - PointLights for each neon sign
 *   - MoonLitWetConcrete, StreetWetAsphalt, Reflective, Emissive, RainStreakMat
 */

#include "../acceleration/BVH.hpp"
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
#include "../lights/DirectionalLight.hpp"
#include "../lights/PointLight.hpp"
#include "../materials/Cosine.hpp"
#include "../samplers/Jittered.hpp"
#include "../tracers/Shadow.hpp"
#include "../world/World.hpp"

#include <cmath>
#include <cstdlib>


// ---------------------------------------------------------------------------
// Helper: add one city tower block
// ---------------------------------------------------------------------------
static void add_tower(World* world,
                      float x0, float x1,
                      float z0, float z1,
                      float y0, float y1,
                      float wetness)
{
    Point3D min_corner(x0, y0, z0);
    Point3D max_corner(x1, y1, z1);

    Box* tower = new Box(min_corner, max_corner);
    tower->set_material(new MoonLitWetConcrete(0.052f, 0.057f, 0.074f, wetness));

    world->add_geometry(tower);
}


// ---------------------------------------------------------------------------
// Helper: add a glass curtain-wall strip on a tower face
// ---------------------------------------------------------------------------
static void add_glass_strip(World* world,
                             float x0, float x1,
                             float y0, float y1,
                             float z_near, float z_thick)
{
    Point3D min_corner(x0, y0, z_near);
    Point3D max_corner(x1, y1, z_near + z_thick);

    Box* glass = new Box(min_corner, max_corner);
    glass->set_material(new Reflective(0.12f, 0.16f, 0.24f, 0.88f, 2.55f));

    world->add_geometry(glass);
}


// ---------------------------------------------------------------------------
// Helper: add one row of dark window recesses
// ---------------------------------------------------------------------------
static void add_window_band(World* world,
                             float x0, float x1,
                             float y0, float y1,
                             float z_face, float inset)
{
    Point3D min_corner(x0, y0, z_face - inset);
    Point3D max_corner(x1, y1, z_face + 0.06f);

    Box* window = new Box(min_corner, max_corner);
    window->set_material(new Cosine(0.022f, 0.028f, 0.038f));

    world->add_geometry(window);
}


// ---------------------------------------------------------------------------
// Helper: add a neon glow sphere
// ---------------------------------------------------------------------------
static void add_neon(World* world,
                     float x, float y, float z,
                     float r, float g, float b,
                     float radiance)
{
    Point3D center(x, y, z);

    Sphere* neon = new Sphere(center, 0.62f);
    neon->set_material(new Emissive(r, g, b, radiance));

    world->add_geometry(neon);
}


// ---------------------------------------------------------------------------
// Helper: add one thin vertical rain streak box
// ---------------------------------------------------------------------------
static void add_rain_streak(World* world,
                             float x, float y_mid,
                             float z, float span)
{
    float half   = span * 0.5f;
    float thick  = 0.048f;

    Point3D min_corner(x - thick, y_mid - half, z - thick);
    Point3D max_corner(x + thick, y_mid + half, z + thick);

    Box* streak = new Box(min_corner, max_corner);
    streak->set_material(new RainStreakMat());

    world->add_geometry(streak);
}


// ---------------------------------------------------------------------------
// World::build
// ---------------------------------------------------------------------------
void World::build(void)
{
    // ------------------------------------------------------------------
    // View plane
    // ------------------------------------------------------------------
    vplane.top_left     = Point3D(-26.0f,  48.0f, 10.0f);
    vplane.bottom_right = Point3D( 26.0f,  12.0f, 10.0f);
    vplane.hres         = 960;
    vplane.vres         = 540;

    // Dark bruised-sky background colour.
    bg_color = RGBColor(0.042f, 0.058f, 0.098f);


    // ------------------------------------------------------------------
    // Camera — standing on the rooftop deck, looking over the city.
    // ------------------------------------------------------------------
    set_camera(new Perspective(1.95f, 41.92f, 23.85f));


    // ------------------------------------------------------------------
    // Sampler — Jittered 2x2 (4 rays per pixel) for anti-aliasing.
    // ------------------------------------------------------------------
    sampler_ptr = new Jittered(camera_ptr, &vplane, 2);


    // ------------------------------------------------------------------
    // Tracer — Shadow tracer so lights cast proper shadows.
    // ------------------------------------------------------------------
    tracer_ptr = new Shadow(this);


    // ------------------------------------------------------------------
    // Lights
    //
    //   1. Directional moon light: cold blue, high-left.
    //   2. PointLight for each neon sign (warm tints).
    // ------------------------------------------------------------------

    // Moon: direction matches SceneLights::moon_center() minus origin.
    // Direction vector points TOWARD the moon from the world origin.
    DirectionalLight* moon_light = new DirectionalLight(
        -86.0f, 112.0f, -198.0f,   // direction toward moon
         0.46f,  0.58f,   0.84f,   // cold blue-white colour
         1.2f                       // intensity
    );
    add_light(moon_light);

    // Neon sign 1 — purple
    PointLight* neon1 = new PointLight(
        -92.0f, 34.0f, -118.0f,
          0.95f, 0.22f,  1.07f,
          6.0f
    );
    add_light(neon1);

    // Neon sign 2 — cyan
    PointLight* neon2 = new PointLight(
        58.0f, 58.0f, -152.0f,
        0.32f, 0.93f,  1.06f,
        5.5f
    );
    add_light(neon2);

    // Neon sign 3 — pink
    PointLight* neon3 = new PointLight(
        154.0f, 66.0f, -134.0f,
          1.06f, 0.52f,  0.74f,
          5.0f
    );
    add_light(neon3);

    // Neon sign 4 — warm white
    PointLight* neon4 = new PointLight(
        -154.0f, 44.0f, -136.0f,
           0.94f, 0.93f,  1.06f,
           4.8f
    );
    add_light(neon4);


    // ==================================================================
    // MOON SPHERE
    // ==================================================================
    Point3D moon_pos    = SceneLights::moon_center();
    float   moon_radius = SceneLights::MOON_SPHERE_RADIUS;

    Sphere* moon = new Sphere(moon_pos, moon_radius);
    moon->set_material(new Emissive(0.8f, 0.87f, 1.06f, 2.62f));
    add_geometry(moon);


    // ==================================================================
    // ROOFTOP DECK
    // ==================================================================
    Box* roof = new Box(
        Point3D(-34.0f, 40.35f, -52.0f),
        Point3D( 58.0f, 41.03f,   9.92f)
    );
    roof->set_material(new Reflective(0.07f, 0.074f, 0.084f, 0.52f, 1.94f));
    add_geometry(roof);

    // Puddles on the deck.
    Sphere* puddle_a = new Sphere(Point3D( 14.0f, 41.06f, -6.0f), 3.05f);
    puddle_a->set_material(new Reflective(0.04f, 0.049f, 0.058f, 0.74f, 2.08f));
    add_geometry(puddle_a);

    Sphere* puddle_b = new Sphere(Point3D(-10.0f, 41.03f,  2.0f), 2.55f);
    puddle_b->set_material(new Reflective(0.048f, 0.056f, 0.065f, 0.70f, 1.96f));
    add_geometry(puddle_b);

    // West parapet wall.
    Box* parapet_west = new Box(
        Point3D(-34.0f, 41.0f,  -52.0f),
        Point3D(-31.2f, 42.4f,    9.92f)
    );
    parapet_west->set_material(new MoonLitWetConcrete(0.068f, 0.069f, 0.074f, 0.73f));
    add_geometry(parapet_west);

    // East parapet wall.
    Box* parapet_east = new Box(
        Point3D(55.0f, 41.0f,  -52.0f),
        Point3D(58.0f, 42.35f,   9.92f)
    );
    parapet_east->set_material(new MoonLitWetConcrete(0.068f, 0.069f, 0.074f, 0.73f));
    add_geometry(parapet_east);

    // HVAC unit A.
    Box* hvac_a = new Box(
        Point3D(  4.0f, 41.02f, -18.0f),
        Point3D( 22.0f, 43.95f,   2.0f)
    );
    hvac_a->set_material(new MoonLitWetConcrete(0.060f, 0.063f, 0.069f, 0.71f));
    add_geometry(hvac_a);

    // HVAC unit B.
    Box* hvac_b = new Box(
        Point3D(-18.0f, 41.0f,  -32.0f),
        Point3D( -7.0f, 42.95f, -16.0f)
    );
    hvac_b->set_material(new MoonLitWetConcrete(0.061f, 0.065f, 0.072f, 0.69f));
    add_geometry(hvac_b);


    // ==================================================================
    // STREET LEVEL
    // ==================================================================
    Plane* ground = new Plane(
        Point3D(0.0f, -54.5f, 0.0f),
        Vector3D(0.0f, 1.0f, 0.0f)
    );
    ground->set_material(new StreetWetAsphalt(0.030f, 0.032f, 0.038f, 0.74f));
    add_unbounded_geometry(ground);

    Box* road_ns_near = new Box(
        Point3D(-11.0f, -54.32f, -240.0f),
        Point3D( 11.0f, -53.90f,   28.0f)
    );
    road_ns_near->set_material(new StreetWetAsphalt(0.029f, 0.032f, 0.042f, 0.76f));
    add_geometry(road_ns_near);

    Box* road_ns_far = new Box(
        Point3D(118.0f, -54.32f, -220.0f),
        Point3D(148.0f, -53.90f,   20.0f)
    );
    road_ns_far->set_material(new StreetWetAsphalt(0.030f, 0.032f, 0.040f, 0.73f));
    add_geometry(road_ns_far);

    Box* road_ew = new Box(
        Point3D(-240.0f, -54.34f, -138.0f),
        Point3D( 240.0f, -53.95f, -112.0f)
    );
    road_ew->set_material(new StreetWetAsphalt(0.029f, 0.032f, 0.041f, 0.74f));
    add_geometry(road_ew);

    Box* boulevard = new Box(
        Point3D(-120.0f, -54.37f, -58.0f),
        Point3D( 190.0f, -54.00f, -32.0f)
    );
    boulevard->set_material(new StreetWetAsphalt(0.028f, 0.032f, 0.043f, 0.71f));
    add_geometry(boulevard);


    // ==================================================================
    // HERO CITY TOWERS (8 major buildings in the foreground/mid-ground)
    // ==================================================================
    add_tower(this, -118.0f, -56.0f, -210.0f, -170.0f, -54.0f, 108.0f, 0.82f);
    add_glass_strip(this, -116.0f, -57.0f, -15.0f, 84.0f, -170.0f, 2.6f);

    for (float win_y = -12.0f; win_y < 94.0f; win_y += 17.0f)
    {
        add_window_band(this, -110.0f, -63.0f, win_y, win_y + 7.0f, -170.0f, 0.4f);
    }

    add_tower(this, -62.0f, 6.0f, -198.0f, -154.0f, -54.0f, 96.0f, 0.79f);
    add_glass_strip(this, -60.0f, 4.0f, -8.0f, 76.0f, -154.0f, 2.3f);

    for (float win_y = 6.0f; win_y < 82.0f; win_y += 13.0f)
    {
        add_window_band(this, -55.0f, 0.0f, win_y, win_y + 6.0f, -154.0f, 0.35f);
    }

    add_tower(this, 26.0f, 84.0f, -208.0f, -166.0f, -54.0f, 118.0f, 0.81f);
    add_glass_strip(this, 28.0f, 82.0f, 0.0f, 98.0f, -166.0f, 2.5f);

    for (float win_y = 8.0f; win_y < 100.0f; win_y += 14.0f)
    {
        add_window_band(this, 33.0f, 78.0f, win_y, win_y + 7.0f, -166.0f, 0.38f);
    }

    add_tower(this, -92.0f, -18.0f, -130.0f, -94.0f, -54.0f, 72.0f, 0.76f);
    add_glass_strip(this, -90.0f, -20.0f, -20.0f, 56.0f, -94.0f, 1.95f);

    add_tower(this, 94.0f, 168.0f, -182.0f, -158.0f, -54.0f, 104.0f, 0.80f);
    add_glass_strip(this, 96.0f, 165.0f, -6.0f, 90.0f, -158.0f, 2.55f);

    add_tower(this, -154.0f, -108.0f, -146.0f, -124.0f, -54.0f, 92.0f, 0.74f);
    add_glass_strip(this, -152.0f, -110.0f, -4.0f, 72.0f, -124.0f, 2.05f);

    add_tower(this, 136.0f, 192.0f, -140.0f, -118.0f, -54.0f, 78.0f, 0.73f);
    add_tower(this, -36.0f,  24.0f, -122.0f,  -98.0f, -54.0f, 62.0f, 0.71f);


    // ==================================================================
    // NEON GLOW SPHERES (4 signs above the skyline)
    // ==================================================================
    add_neon(this,  -92.0f,  34.0f, -118.0f,  0.95f, 0.22f, 1.07f, 9.80f);
    add_neon(this,   58.0f,  58.0f, -152.0f,  0.32f, 0.93f, 1.06f, 8.90f);
    add_neon(this,  154.0f,  66.0f, -134.0f,  1.06f, 0.52f, 0.74f, 8.35f);
    add_neon(this, -154.0f,  44.0f, -136.0f,  0.94f, 0.93f, 1.06f, 7.85f);


    // ==================================================================
    // BACKGROUND CITY SPRAWL
    //
    // A 300 x 300 grid of small city blocks fills the deep background
    // from z = -220 to z = -800, x = -500 to x = 500.
    // Each block gets a random height between 20 and 90 units.
    // This contributes 90,000 primitives.
    // ==================================================================
    int   grid_cols  = 300;
    int   grid_rows  = 300;
    float grid_x_min = -500.0f;
    float grid_x_max =  500.0f;
    float grid_z_min = -800.0f;
    float grid_z_max = -220.0f;

    float cell_width = (grid_x_max - grid_x_min) / static_cast<float>(grid_cols);
    float cell_depth = (grid_z_max - grid_z_min) / static_cast<float>(grid_rows);

    // Use a fixed seed so the scene is deterministic.
    srand(42);

    for (int col = 0; col < grid_cols; col++)
    {
        for (int row = 0; row < grid_rows; row++)
        {
            // Cell bounds in X and Z.
            float x0 = grid_x_min + col * cell_width + 1.0f;
            float x1 = x0 + cell_width - 2.0f;
            float z0 = grid_z_min + row * cell_depth + 1.0f;
            float z1 = z0 + cell_depth - 2.0f;

            // Random height between 20 and 90.
            float height = 20.0f + static_cast<float>(rand() % 71);

            // Random wetness between 0.5 and 0.9.
            float wetness = 0.5f + static_cast<float>(rand() % 41) * 0.01f;

            // Random albedo variation: dark grey to blue-grey.
            float base_r = 0.04f + static_cast<float>(rand() % 30) * 0.001f;
            float base_g = 0.045f + static_cast<float>(rand() % 30) * 0.001f;
            float base_b = 0.06f + static_cast<float>(rand() % 40) * 0.001f;

            Box* city_block = new Box(
                Point3D(x0, -54.0f, z0),
                Point3D(x1, -54.0f + height, z1)
            );
            city_block->set_material(
                new MoonLitWetConcrete(base_r, base_g, base_b, wetness)
            );

            add_geometry(city_block);
        }
    }


    // ==================================================================
    // RAIN DROP SPHERES
    //
    // 900,000 tiny spheres scattered in the volume between the camera
    // and the far background. Each is a small semi-transparent bead
    // catching moonlight. Combined with the 90,000 city blocks and
    // ~110 other objects this gives us well over 1,000,000 primitives.
    //
    // Volume: x in [-300, 300], y in [-50, 110], z in [-800, 9]
    // Radius: 0.08 units (tiny, like a real raindrop)
    // ==================================================================
    int   rain_count = 900000;
    float rain_x_min = -300.0f;
    float rain_x_max =  300.0f;
    float rain_y_min =  -50.0f;
    float rain_y_max =  110.0f;
    float rain_z_min = -800.0f;
    float rain_z_max =    9.0f;

    float rain_x_span = rain_x_max - rain_x_min;
    float rain_y_span = rain_y_max - rain_y_min;
    float rain_z_span = rain_z_max - rain_z_min;

    for (int i = 0; i < rain_count; i++)
    {
        // Random position within the rain volume.
        float rx = rain_x_min + static_cast<float>(rand()) * invRAND_MAX * rain_x_span;
        float ry = rain_y_min + static_cast<float>(rand()) * invRAND_MAX * rain_y_span;
        float rz = rain_z_min + static_cast<float>(rand()) * invRAND_MAX * rain_z_span;

        Sphere* raindrop = new Sphere(Point3D(rx, ry, rz), 0.08f);
        raindrop->set_material(new RainStreakMat());

        add_geometry(raindrop);
    }


    // ==================================================================
    // FOREGROUND RAIN STREAKS (54 vertical boxes near the camera)
    // ==================================================================
    int rain_streak_count = 54;

    for (int i = 0; i < rain_streak_count; i++)
    {
        float t = static_cast<float>(i) / static_cast<float>(rain_streak_count - 1);

        float streak_x      = -24.0f + 48.0f * t + std::sin(t * 6.283f * 5.17f) * 6.0f;
        float streak_z      = 6.0f - 210.0f * t;
        float streak_y      = 46.0f + std::sin(t * 43.0f) * 14.0f;
        float streak_height = 16.0f + std::cos(t * 31.0f) * 4.0f;

        add_rain_streak(this, streak_x, streak_y, streak_z, streak_height);
    }


    // ==================================================================
    // BUILD THE BVH
    //
    // This must be called AFTER all geometry has been added.
    // The BVH organises all 1M+ objects into a tree so ray intersection
    // is O(log N) instead of O(N).
    //
    // To render WITHOUT the BVH (for comparison), comment out these
    // two lines. See README for the -DUSE_ACCEL flag alternative.
    // ==================================================================
    BVH* bvh = new BVH();
    bvh->build(geometry);
    accel_ptr = bvh;
}

// for sara:
// g++ -g -std=c++17 \
  -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk \
  -I/Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk/usr/include/c++/v1 \
  -stdlib=libc++ \
  raytracer.cpp world/*.cpp utilities/*.cpp geometry/*.cpp cameras/*.cpp image/*.cpp samplers/*.cpp materials/*.cpp build/ArkhamKnight.cpp \
  -o raytracer.exe