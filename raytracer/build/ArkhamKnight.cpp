/**
 * ArkhamKnight.cpp
 *
 * A rain-soaked Gotham rooftop scene viewed from above a building deck.
 * The camera looks out over the city skyline. The moon hangs high and
 * left, casting cold blue light over wet concrete, glass towers, and
 * neon-lit streets far below.
 *
 * Scene elements:
 *   - Moon (emissive sphere, matches SceneLights::moon_center())
 *   - Rooftop deck with puddles, parapets, and HVAC units
 *   - Six city tower blocks with glass strips and window bands
 *   - Wet street basin with arterial roads
 *   - Four neon light sources floating above the skyline
 *   - Rain streaks falling across the frame
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


// ---------------------------------------------------------------------------
// Helper: add one city tower block to the scene
//   x0, x1  = left and right X bounds
//   z0, z1  = far and near Z bounds
//   y0, y1  = bottom and top Y bounds
//   wetness = how rain-soaked the concrete looks (0=dry, 1=soaked)
// ---------------------------------------------------------------------------
static void add_tower(World* world,
                      float x0, float x1,
                      float z0, float z1,
                      float y0, float y1,
                      float wetness)
{
    Point3D corner_min(x0, y0, z0);
    Point3D corner_max(x1, y1, z1);

    Box* tower = new Box(corner_min, corner_max);
    tower->set_material(new MoonLitWetConcrete(0.052f, 0.057f, 0.074f, wetness));

    world->add_geometry(tower);
}


// ---------------------------------------------------------------------------
// Helper: add a glass curtain-wall strip on one face of a tower
//   x0, x1    = horizontal span
//   y0, y1    = vertical span
//   z_near    = Z position of the glass face
//   z_thick   = how thick the glass slab is
// ---------------------------------------------------------------------------
static void add_glass_strip(World* world,
                             float x0, float x1,
                             float y0, float y1,
                             float z_near, float z_thick)
{
    Point3D corner_min(x0, y0, z_near);
    Point3D corner_max(x1, y1, z_near + z_thick);

    Box* glass = new Box(corner_min, corner_max);
    glass->set_material(new Reflective(0.12f, 0.16f, 0.24f, 0.88f, 2.55f));

    world->add_geometry(glass);
}


// ---------------------------------------------------------------------------
// Helper: add one row of dark window recesses on a tower face
//   x0, x1   = horizontal span of the window row
//   y0, y1   = vertical span of this row
//   z_face   = Z position of the wall face the windows sit on
//   inset    = how far the window is recessed into the wall
// ---------------------------------------------------------------------------
static void add_window_band(World* world,
                             float x0, float x1,
                             float y0, float y1,
                             float z_face, float inset)
{
    Point3D corner_min(x0, y0, z_face - inset);
    Point3D corner_max(x1, y1, z_face + 0.06f);

    Box* window = new Box(corner_min, corner_max);
    window->set_material(new Cosine(0.022f, 0.028f, 0.038f));

    world->add_geometry(window);
}


// ---------------------------------------------------------------------------
// Helper: add a neon glow sphere at a point in the skyline
//   x, y, z       = position
//   r, g, b       = colour of the neon light
//   radiance      = brightness multiplier
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
//   x         = X position of the streak
//   y_mid     = Y centre of the streak
//   z         = Z position
//   span      = total height of the streak
// ---------------------------------------------------------------------------
static void add_rain_streak(World* world,
                             float x, float y_mid,
                             float z, float span)
{
    float half_span = span * 0.5f;
    float thickness = 0.048f;

    Point3D corner_min(x - thickness, y_mid - half_span, z - thickness);
    Point3D corner_max(x + thickness, y_mid + half_span, z + thickness);

    Box* streak = new Box(corner_min, corner_max);
    streak->set_material(new RainStreakMat());

    world->add_geometry(streak);
}


// ---------------------------------------------------------------------------
// World::build
//   Sets up the entire Arkham Knight rooftop scene.
// ---------------------------------------------------------------------------
void World::build(void)
{
    // ------------------------------------------------------------------
    // View plane
    //   The plane sits at z=10, just in front of the camera.
    //   Rays travel into negative Z (toward the city).
    // ------------------------------------------------------------------
    vplane.top_left     = Point3D(-26.0f,  48.0f, 10.0f);
    vplane.bottom_right = Point3D( 26.0f,  12.0f, 10.0f);
    vplane.hres         = 960;
    vplane.vres         = 540;

    // Dark bruised-sky background colour
    bg_color = RGBColor(0.042f, 0.058f, 0.098f);

    // ------------------------------------------------------------------
    // Camera and sampler
    //   Camera stands on the rooftop deck, looking out over the city.
    // ------------------------------------------------------------------
    set_camera(new Perspective(1.95f, 41.92f, 23.85f));
    sampler_ptr = new Simple(camera_ptr, &vplane);


    // ==================================================================
    // MOON
    //   Must match SceneLights::moon_center() and MOON_SPHERE_RADIUS
    //   exactly, so the lighting in all materials stays consistent.
    // ==================================================================
    Point3D   moon_pos    = SceneLights::moon_center();
    float     moon_radius = SceneLights::MOON_SPHERE_RADIUS;

    Sphere* moon = new Sphere(moon_pos, moon_radius);
    moon->set_material(new Emissive(0.8f, 0.87f, 1.06f, 2.62f));
    add_geometry(moon);


    // ==================================================================
    // ROOFTOP DECK
    //   A thin reflective slab representing the roof we are standing on.
    //   Capped at z=9.92 so it never crosses the view plane at z=10.
    // ==================================================================
    Box* roof = new Box(
        Point3D(-34.0f, 40.35f, -52.0f),
        Point3D( 58.0f, 41.03f,   9.92f)
    );
    roof->set_material(new Reflective(0.07f, 0.074f, 0.084f, 0.52f, 1.94f));
    add_geometry(roof);

    // -- Puddles on the deck ------------------------------------------
    Sphere* puddle_a = new Sphere(Point3D(14.0f, 41.06f, -6.0f), 3.05f);
    puddle_a->set_material(new Reflective(0.04f, 0.049f, 0.058f, 0.74f, 2.08f));
    add_geometry(puddle_a);

    Sphere* puddle_b = new Sphere(Point3D(-10.0f, 41.03f, 2.0f), 2.55f);
    puddle_b->set_material(new Reflective(0.048f, 0.056f, 0.065f, 0.70f, 1.96f));
    add_geometry(puddle_b);

    // -- West parapet wall --------------------------------------------
    Box* parapet_west = new Box(
        Point3D(-34.0f, 41.0f, -52.0f),
        Point3D(-31.2f, 42.4f,   9.92f)
    );
    parapet_west->set_material(new MoonLitWetConcrete(0.068f, 0.069f, 0.074f, 0.73f));
    add_geometry(parapet_west);

    // -- East parapet wall --------------------------------------------
    Box* parapet_east = new Box(
        Point3D(55.0f, 41.0f, -52.0f),
        Point3D(58.0f, 42.35f,  9.92f)
    );
    parapet_east->set_material(new MoonLitWetConcrete(0.068f, 0.069f, 0.074f, 0.73f));
    add_geometry(parapet_east);

    // -- HVAC unit A (large box near centre-right) --------------------
    Box* hvac_a = new Box(
        Point3D(  4.0f, 41.02f, -18.0f),
        Point3D( 22.0f, 43.95f,   2.0f)
    );
    hvac_a->set_material(new MoonLitWetConcrete(0.060f, 0.063f, 0.069f, 0.71f));
    add_geometry(hvac_a);

    // -- HVAC unit B (smaller box on the left) ------------------------
    Box* hvac_b = new Box(
        Point3D(-18.0f, 41.0f,  -32.0f),
        Point3D( -7.0f, 42.95f, -16.0f)
    );
    hvac_b->set_material(new MoonLitWetConcrete(0.061f, 0.065f, 0.072f, 0.69f));
    add_geometry(hvac_b);


    // ==================================================================
    // STREET LEVEL
    //   A large flat plane for the ground, plus four raised road slabs
    //   sitting just above it to represent arterial streets.
    // ==================================================================

    // -- Ground plane (wet asphalt basin) -----------------------------
    Plane* ground = new Plane(
        Point3D(0.0f, -54.5f, 0.0f),
        Vector3D(0.0f, 1.0f, 0.0f)
    );
    ground->set_material(new StreetWetAsphalt(0.030f, 0.032f, 0.038f, 0.74f));
    add_geometry(ground);

    // -- North-south artery near the base of our building -------------
    Box* road_ns_near = new Box(
        Point3D(-11.0f, -54.32f, -240.0f),
        Point3D( 11.0f, -53.90f,   28.0f)
    );
    road_ns_near->set_material(new StreetWetAsphalt(0.029f, 0.032f, 0.042f, 0.76f));
    add_geometry(road_ns_near);

    // -- North-south artery on the far right of frame -----------------
    Box* road_ns_far = new Box(
        Point3D(118.0f, -54.32f, -220.0f),
        Point3D(148.0f, -53.90f,   20.0f)
    );
    road_ns_far->set_material(new StreetWetAsphalt(0.030f, 0.032f, 0.040f, 0.73f));
    add_geometry(road_ns_far);

    // -- East-west artery cutting across the mid-ground ---------------
    Box* road_ew = new Box(
        Point3D(-240.0f, -54.34f, -138.0f),
        Point3D( 240.0f, -53.95f, -112.0f)
    );
    road_ew->set_material(new StreetWetAsphalt(0.029f, 0.032f, 0.041f, 0.74f));
    add_geometry(road_ew);

    // -- Wide boulevard loop in the foreground ------------------------
    Box* boulevard = new Box(
        Point3D(-120.0f, -54.37f, -58.0f),
        Point3D( 190.0f, -54.00f, -32.0f)
    );
    boulevard->set_material(new StreetWetAsphalt(0.028f, 0.032f, 0.043f, 0.71f));
    add_geometry(boulevard);


    // ==================================================================
    // CITY TOWERS
    //   Each tower gets: a concrete body, a glass curtain-wall strip on
    //   its front face, and several rows of dark window recesses.
    // ==================================================================

    // -- Tower 1: tall block far left ---------------------------------
    add_tower(this, -118.0f, -56.0f, -210.0f, -170.0f, -54.0f, 108.0f, 0.82f);
    add_glass_strip(this, -116.0f, -57.0f, -15.0f, 84.0f, -170.0f, 2.6f);

    for (float win_y = -12.0f; win_y < 94.0f; win_y += 17.0f)
    {
        add_window_band(this, -110.0f, -63.0f, win_y, win_y + 7.0f, -170.0f, 0.4f);
    }

    // -- Tower 2: mid-left block --------------------------------------
    add_tower(this, -62.0f, 6.0f, -198.0f, -154.0f, -54.0f, 96.0f, 0.79f);
    add_glass_strip(this, -60.0f, 4.0f, -8.0f, 76.0f, -154.0f, 2.3f);

    for (float win_y = 6.0f; win_y < 82.0f; win_y += 13.0f)
    {
        add_window_band(this, -55.0f, 0.0f, win_y, win_y + 6.0f, -154.0f, 0.35f);
    }

    // -- Tower 3: tallest block, right of centre ----------------------
    add_tower(this, 26.0f, 84.0f, -208.0f, -166.0f, -54.0f, 118.0f, 0.81f);
    add_glass_strip(this, 28.0f, 82.0f, 0.0f, 98.0f, -166.0f, 2.5f);

    for (float win_y = 8.0f; win_y < 100.0f; win_y += 14.0f)
    {
        add_window_band(this, 33.0f, 78.0f, win_y, win_y + 7.0f, -166.0f, 0.38f);
    }

    // -- Tower 4: shorter block, left mid-ground (no windows) ---------
    add_tower(this, -92.0f, -18.0f, -130.0f, -94.0f, -54.0f, 72.0f, 0.76f);
    add_glass_strip(this, -90.0f, -20.0f, -20.0f, 56.0f, -94.0f, 1.95f);

    // -- Tower 5: wide block far right --------------------------------
    add_tower(this, 94.0f, 168.0f, -182.0f, -158.0f, -54.0f, 104.0f, 0.80f);
    add_glass_strip(this, 96.0f, 165.0f, -6.0f, 90.0f, -158.0f, 2.55f);

    // -- Tower 6: block far left background ---------------------------
    add_tower(this, -154.0f, -108.0f, -146.0f, -124.0f, -54.0f, 92.0f, 0.74f);
    add_glass_strip(this, -152.0f, -110.0f, -4.0f, 72.0f, -124.0f, 2.05f);

    // -- Tower 7: far right background, no glass ----------------------
    add_tower(this, 136.0f, 192.0f, -140.0f, -118.0f, -54.0f, 78.0f, 0.73f);

    // -- Tower 8: small block in the foreground mid-left --------------
    add_tower(this, -36.0f, 24.0f, -122.0f, -98.0f, -54.0f, 62.0f, 0.71f);


    // ==================================================================
    // NEON SIGNS
    //   Four coloured glow spheres sitting above rooftops in the skyline.
    //   Colours: purple, cyan, pink, warm white.
    // ==================================================================
    add_neon(this,  -92.0f,  34.0f, -118.0f,  0.95f, 0.22f, 1.07f, 9.80f);  // purple
    add_neon(this,   58.0f,  58.0f, -152.0f,  0.32f, 0.93f, 1.06f, 8.90f);  // cyan
    add_neon(this,  154.0f,  66.0f, -134.0f,  1.06f, 0.52f, 0.74f, 8.35f);  // pink
    add_neon(this, -154.0f,  44.0f, -136.0f,  0.94f, 0.93f, 1.06f, 7.85f);  // warm white


    // ==================================================================
    // RAIN VEIL
    //   54 thin vertical streak boxes placed across the frame, between
    //   the camera and the skyline. Each streak is offset slightly in X
    //   and Z using sine/cosine so they feel random without using rand().
    // ==================================================================
    int   rain_count = 54;

    for (int i = 0; i < rain_count; i++)
    {
        // t goes from 0.0 to 1.0 across all streaks
        float t = static_cast<float>(i) / static_cast<float>(rain_count - 1);

        // X position: spread across the frame with a gentle sine wobble
        float streak_x = -24.0f + 48.0f * t + std::sin(t * 6.283f * 5.17f) * 6.0f;

        // Z position: streaks start near the camera and go deep into the scene
        float streak_z = 6.0f - 210.0f * t;

        // Y centre: floats up and down slightly along the veil
        float streak_y = 46.0f + std::sin(t * 43.0f) * 14.0f;

        // Height: varies slightly so not all streaks are identical
        float streak_height = 16.0f + std::cos(t * 31.0f) * 4.0f;

        add_rain_streak(this, streak_x, streak_y, streak_z, streak_height);
    }
}

// for sara:
// g++ -g -std=c++17 \
  -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk \
  -I/Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk/usr/include/c++/v1 \
  -stdlib=libc++ \
  raytracer.cpp world/*.cpp utilities/*.cpp geometry/*.cpp cameras/*.cpp image/*.cpp samplers/*.cpp materials/*.cpp build/ArkhamKnight.cpp \
  -o raytracer.exe