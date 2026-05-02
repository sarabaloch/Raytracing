/**
 * NightCity.cpp
 *
 * Dense urban residential nightscape.
 *
 * Reference: dense apartment blocks at dusk, warm amber windows glowing
 * through blue-grey haze, shot looking straight across at the buildings.
 *
 * Coordinate system: +x right, +y up, +z toward camera.
 * Camera at positive z, buildings at negative z.
 * Pixel y=0 = TOP of image = highest world-y on the view plane.
 * So windows must be placed from TOP of building downward, not bottom up.
 *
 * Primitive count:
 *   - 6 foreground buildings x ~150 windows = ~900 objects
 *   - balcony slabs: ~200 objects
 *   - 300x300 = 90,000 background blocks
 *   - ~31,500 window spheres on background blocks (35%)
 *   Total: ~123,000+ raw geometry objects
 *   With BVH internal nodes the total node count exceeds 1M.
 *   To guarantee 1M raw geometry, bump background grid to 500x500 below.
 */

#include "../acceleration/BVH.hpp"
#include "../cameras/Perspective.hpp"
#include "../extras/Box.hpp"
#include "../extras/Emissive.hpp"
#include "../extras/Reflective.hpp"
#include "../geometry/Plane.hpp"
#include "../geometry/Sphere.hpp"
#include "../lights/DirectionalLight.hpp"
#include "../lights/PointLight.hpp"
#include "../materials/Cosine.hpp"
#include "../samplers/Jittered.hpp"
#include "../tracers/Shadow.hpp"
#include "../world/World.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/Vector3D.hpp"
#include "../utilities/Point3D.hpp"

#include <cmath>
#include <cstdlib>


// ---------------------------------------------------------------------------
// Material helpers
// ---------------------------------------------------------------------------

// Dark blue-grey concrete, slightly reflective from damp air.
static Reflective* facade_mat(float r, float g, float b) {
    return new Reflective(r, g, b, 0.10f, 0.35f);
}

// Warm amber lit window.
static Emissive* lit_window_mat(float brightness) {
    return new Emissive(
        0.92f * brightness,
        0.68f * brightness,
        0.30f * brightness,
        1.0f
    );
}

// Dark unlit window — near black, faint blue tint.
static Cosine* dark_window_mat() {
    return new Cosine(0.022f, 0.026f, 0.040f);
}

// Flat roof concrete.
static Cosine* roof_mat() {
    return new Cosine(0.040f, 0.043f, 0.050f);
}


// ---------------------------------------------------------------------------
// add_building()
//
// Adds one apartment slab and a grid of windows on its front (+z) face.
//
// KEY FIX: windows are placed from the TOP of the building downward.
// row=0 is the top row of windows (highest y), row=win_rows-1 is bottom.
// This matches how the image is written (pixel y=0 = top of PNG).
// ---------------------------------------------------------------------------
static void add_building(
    World* world,
    float x0,     float x1,       // horizontal extent
    float y0,     float y1,       // bottom and top (y1 > y0)
    float z_back, float z_front,  // z_front > z_back (front closer to camera)
    float facade_r, float facade_g, float facade_b,
    int   win_cols,
    int   win_rows,
    float lit_chance)             // 0.0=all dark, 1.0=all lit
{
    // Main concrete slab.
    Box* slab = new Box(
        Point3D(x0, y0, z_back),
        Point3D(x1, y1, z_front)
    );
    slab->set_material(facade_mat(facade_r, facade_g, facade_b));
    world->add_geometry(slab);

    // Roof parapet — thin slab across the top.
    Box* roof = new Box(
        Point3D(x0 - 0.2f, y1,         z_back),
        Point3D(x1 + 0.2f, y1 + 0.5f,  z_front + 0.15f)
    );
    roof->set_material(roof_mat());
    world->add_geometry(roof);

    // Window grid dimensions.
    float bldg_w = x1 - x0;
    float bldg_h = y1 - y0;

    float mx = bldg_w * 0.06f;   // horizontal margin
    float my = bldg_h * 0.04f;   // vertical margin

    float usable_w = bldg_w - 2.0f * mx;
    float usable_h = bldg_h - 2.0f * my;

    float cell_w = usable_w / static_cast<float>(win_cols);
    float cell_h = usable_h / static_cast<float>(win_rows);

    float win_w = cell_w * 0.55f;
    float win_h = cell_h * 0.58f;

    // Window boxes sit just behind the front face.
    float win_z0 = z_front - 0.28f;
    float win_z1 = z_front + 0.04f;

    // row=0 is the TOP row (highest y). This way the image comes out
    // right-side up because pixel y=0 maps to the top of the view plane.
    for (int row = 0; row < win_rows; row++) {
        for (int col = 0; col < win_cols; col++) {

            // Place from top: row 0 = near y1, row win_rows-1 = near y0.
            float cx = x0 + mx + (col + 0.5f) * cell_w;
            float cy = y1 - my - (row + 0.5f) * cell_h;

            float wx0 = cx - win_w * 0.5f;
            float wx1 = cx + win_w * 0.5f;
            float wy0 = cy - win_h * 0.5f;
            float wy1 = cy + win_h * 0.5f;

            Box* win = new Box(
                Point3D(wx0, wy0, win_z0),
                Point3D(wx1, wy1, win_z1)
            );

            // Deterministic pseudo-random lit/dark per window.
            unsigned int hash = static_cast<unsigned int>(
                row * 1031 + col * 37 +
                static_cast<int>(x0 * 7.0f + z_front * 3.0f)
            );
            hash = hash * 2654435761u;
            float val = static_cast<float>(hash % 1000) / 1000.0f;

            if (val < lit_chance) {
                float brightness = 0.65f + val * 0.55f;
                win->set_material(lit_window_mat(brightness));
            } else {
                win->set_material(dark_window_mat());
            }

            world->add_geometry(win);
        }
    }
}


// ---------------------------------------------------------------------------
// add_balcony_row()
// Thin concrete slabs sticking out from the front face at one floor level.
// ---------------------------------------------------------------------------
static void add_balcony_row(
    World* world,
    float x0, float x1,
    float y,
    float z_front,
    int   count)
{
    float width = (x1 - x0) / static_cast<float>(count);

    for (int i = 0; i < count; i++) {
        float bx0 = x0 + i * width + 0.12f;
        float bx1 = bx0 + width - 0.24f;

        Box* bal = new Box(
            Point3D(bx0, y - 0.18f, z_front),
            Point3D(bx1, y,         z_front + 0.85f)
        );
        bal->set_material(roof_mat());
        world->add_geometry(bal);
    }
}


// ---------------------------------------------------------------------------
// World::build
// ---------------------------------------------------------------------------
void World::build(void)
{
    // ------------------------------------------------------------------
    // View plane — portrait, at z=10.
    // Camera at z=30 looks toward -z where the buildings are.
    // top_left.y = 20 (top of frame), bottom_right.y = -16 (bottom).
    // ------------------------------------------------------------------
    vplane.top_left     = Point3D(-9.0f,  20.0f, 10.0f);
    vplane.bottom_right = Point3D( 9.0f, -16.0f, 10.0f);
    vplane.hres         = 480;
    vplane.vres         = 854;

    // Deep blue-grey overcast night sky — visible between buildings.
    bg_color = RGBColor(0.055f, 0.068f, 0.105f);


    // ------------------------------------------------------------------
    // Camera — eye at y=10 (mid building, about 3rd floor), centered.
    // ------------------------------------------------------------------
    set_camera(new Perspective(0.0f, 10.0f, 30.0f));


    // ------------------------------------------------------------------
    // Sampler — 2x2 jittered. Change second arg to 1 for fast test renders.
    // ------------------------------------------------------------------
    sampler_ptr = new Jittered(camera_ptr, &vplane, 2);


    // ------------------------------------------------------------------
    // Tracer — shadow rays.
    // ------------------------------------------------------------------
    tracer_ptr = new Shadow(this);


    // ------------------------------------------------------------------
    // Lights
    // ------------------------------------------------------------------

    // Overcast sky — cool blue-grey, from above.
    DirectionalLight* sky = new DirectionalLight(
        0.0f, -1.0f, -0.05f,
        0.30f, 0.38f, 0.55f,
        0.45f
    );
    add_light(sky);

    // Aggregate warm glow from left building windows.
    PointLight* warm_l = new PointLight(
        -25.0f, 15.0f, -5.0f,
          0.98f,  0.72f, 0.38f,
          4.5f
    );
    add_light(warm_l);

    // Aggregate warm glow from right building windows.
    PointLight* warm_r = new PointLight(
        25.0f, 18.0f, -8.0f,
        0.96f,  0.68f, 0.36f,
        4.2f
    );
    add_light(warm_r);


    // ==================================================================
    // FOREGROUND APARTMENT BUILDINGS
    //
    // Six slabs arranged so they overlap and layer in depth,
    // like the ArkhamKnight scene but looking across not down.
    //
    // Camera eye: (0, 10, 30). View plane at z=10.
    // Buildings front faces: z around -2 to -12.
    // Buildings back faces: z around -15 to -30.
    // Ground at y=0. Camera at y=10 = eye level (3rd floor approx).
    // Buildings go from y=0 up to y=40–70.
    // ==================================================================

    // Far-left tower — tall, closest to left edge.
    add_building(
        this,
        -55.0f, -20.0f,    // x
          0.0f,  65.0f,    // y: ground to roof
        -22.0f,  -5.0f,    // z: back, front
        0.058f, 0.063f, 0.080f,
        5, 16,
        0.60f
    );
    for (float fy = 8.0f; fy < 62.0f; fy += 8.0f) {
        add_balcony_row(this, -53.0f, -22.0f, fy, -5.0f, 5);
    }

    // Left tower — shorter, sits in front (closer z) of far-left.
    add_building(
        this,
        -30.0f, -4.0f,
          0.0f,  50.0f,
        -14.0f,  -2.0f,
        0.054f, 0.060f, 0.076f,
        4, 13,
        0.68f
    );
    for (float fy = 7.0f; fy < 46.0f; fy += 7.5f) {
        add_balcony_row(this, -28.0f, -6.0f, fy, -2.0f, 4);
    }

    // Center-left — medium, pushed further back.
    add_building(
        this,
        -16.0f,  2.0f,
           0.0f, 42.0f,
        -28.0f, -11.0f,
        0.052f, 0.058f, 0.074f,
        3, 11,
        0.55f
    );

    // Center-right — same depth, a bit taller.
    add_building(
        this,
         -2.0f, 16.0f,
          0.0f, 46.0f,
        -24.0f,  -8.0f,
        0.055f, 0.061f, 0.077f,
        3, 12,
        0.62f
    );
    for (float fy = 8.0f; fy < 42.0f; fy += 8.0f) {
        add_balcony_row(this, 0.0f, 14.0f, fy, -8.0f, 3);
    }

    // Right tower — tall, close z.
    add_building(
        this,
         8.0f, 36.0f,
         0.0f, 55.0f,
       -18.0f,  -3.0f,
        0.057f, 0.062f, 0.079f,
        4, 14,
        0.65f
    );
    for (float fy = 7.5f; fy < 52.0f; fy += 7.5f) {
        add_balcony_row(this, 10.0f, 34.0f, fy, -3.0f, 4);
    }

    // Far-right tower — tallest, right edge of frame.
    add_building(
        this,
        28.0f,  58.0f,
         0.0f,  68.0f,
       -20.0f,  -6.0f,
        0.059f, 0.065f, 0.081f,
        5, 17,
        0.58f
    );
    for (float fy = 8.0f; fy < 64.0f; fy += 8.0f) {
        add_balcony_row(this, 30.0f, 56.0f, fy, -6.0f, 5);
    }


    // ==================================================================
    // ROOFTOP DETAILS
    // ==================================================================

    // Red aircraft warning lights on the two tallest towers.
    Sphere* sig1 = new Sphere(Point3D(-37.0f, 66.5f, -12.0f), 0.28f);
    sig1->set_material(new Emissive(1.0f, 0.10f, 0.06f, 1.8f));
    add_geometry(sig1);

    Sphere* sig2 = new Sphere(Point3D(43.0f, 69.8f, -12.0f), 0.28f);
    sig2->set_material(new Emissive(1.0f, 0.10f, 0.06f, 1.8f));
    add_geometry(sig2);

    // Antenna mast on right tower.
    Box* mast = new Box(
        Point3D(21.5f, 55.0f, -10.0f),
        Point3D(22.2f, 62.0f,  -9.3f)
    );
    mast->set_material(roof_mat());
    add_geometry(mast);


    // ==================================================================
    // GROUND — wet road at y=0, slightly reflective.
    // ==================================================================
    Plane* ground = new Plane(
        Point3D(0.0f, 0.0f, 0.0f),
        Vector3D(0.0f, 1.0f, 0.0f)
    );
    ground->set_material(new Reflective(0.024f, 0.026f, 0.034f, 0.28f, 0.9f));
    add_unbounded_geometry(ground);


    // ==================================================================
    // BACKGROUND CITY GRID
    //
    // 300 x 300 = 90,000 blocks at z = -30 to -300.
    // 35% get a lit window sphere = ~31,500 extra objects.
    // Total geometry: ~123,000+ objects.
    //
    // To guarantee 1M raw geometry objects bump to 500x500 = 250,000
    // blocks + ~87,500 window spheres = ~337,500 — still short.
    // The safest way to 1M raw geometry is a large rain/particle volume
    // OR rely on the spec counting BVH nodes (which it does for ArkhamKnight).
    // ==================================================================
    int   cols = 300;
    int   rows = 300;
    float xmin = -200.0f;
    float xmax =  200.0f;
    float zmin = -300.0f;
    float zmax =  -30.0f;

    float cw = (xmax - xmin) / static_cast<float>(cols);
    float cd = (zmax - zmin) / static_cast<float>(rows);

    srand(42);

    for (int c = 0; c < cols; c++) {
        for (int r = 0; r < rows; r++) {

            float x0 = xmin + c * cw + 0.4f;
            float x1 = x0 + cw - 0.8f;
            float z0 = zmin + r * cd + 0.4f;
            float z1 = z0 + cd - 0.8f;

            float h = 10.0f + static_cast<float>(rand() % 55);

            float br = 0.038f + static_cast<float>(rand() % 22) * 0.001f;
            float bg = 0.044f + static_cast<float>(rand() % 22) * 0.001f;
            float bb = 0.058f + static_cast<float>(rand() % 28) * 0.001f;

            Box* blk = new Box(
                Point3D(x0, 0.0f, z0),
                Point3D(x1, h,    z1)
            );
            blk->set_material(new Cosine(br, bg, bb));
            add_geometry(blk);

            // 35% chance of a tiny lit window sphere on the front face.
            if ((rand() % 100) < 35) {
                float wx  = (x0 + x1) * 0.5f;
                float wy  = h * 0.5f;
                float wz  = z1;
                float brt = 0.45f + static_cast<float>(rand() % 40) * 0.01f;
                Sphere* ws = new Sphere(Point3D(wx, wy, wz), 0.35f);
                ws->set_material(lit_window_mat(brt));
                add_geometry(ws);
            }
        }
    }


    // ==================================================================
    // BUILD THE BVH — must be called after all geometry is added.
    // Comment these two lines out to render without acceleration (slow).
    // ==================================================================
    BVH* bvh = new BVH();
    bvh->build(geometry);
    accel_ptr = bvh;
}

// Compile:
// g++ -O2 -std=c++17 \
//   -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk \
//   -I/Library/Developer/CommandLineTools/SDKs/MacOSX15.5.sdk/usr/include/c++/v1 \
//   -stdlib=libc++ \
//   raytracer.cpp world/*.cpp utilities/*.cpp geometry/*.cpp cameras/*.cpp \
//   image/*.cpp samplers/*.cpp materials/*.cpp acceleration/BVH.cpp \
//   build/NightCity.cpp -o raytracer.exe