/**
 * NightCity.cpp
 *
 * Cyberpunk night city - purple/magenta aesthetic.
 *
 * Design:
 *  - Camera looks slightly upward so the upper 40% of frame is open sky.
 *  - Large moon (emissive sphere) sits clearly in the upper sky area.
 *  - Buildings occupy the lower 60% of the frame — short silhouettes.
 *  - Rooftop / fence is at the very bottom edge, barely visible.
 *  - Rain uses Sphere primitives (small glowing dots) instead of Box,
 *    which produces natural-looking droplets rather than stiff rectangles.
 *  - Puddles on rooftop reflect neon and moonlight.
 *  - 1M+ primitives via horizon window grids.
 */

#include "../cameras/Perspective.hpp"
#include "../extras/Box.hpp"
#include "../extras/Emissive.hpp"
#include "../extras/Puddle.hpp"
#include "../geometry/Plane.hpp"
#include "../geometry/Sphere.hpp"
#include "../lights/PointLight.hpp"
#include "../lights/DirectionalLight.hpp"
#include "../materials/Cosine.hpp"
#include "../samplers/Simple.hpp"
#include "../tracers/Shadow.hpp"
#include "../acceleration/BVH.hpp"
#include "../utilities/Constants.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/Vector3D.hpp"
#include "../utilities/Point3D.hpp"
#include "../world/World.hpp"
#include <cmath>
#include <cstdlib>

static float frand(unsigned int& seed) {
    seed = seed * 1664525u + 1013904223u;
    return static_cast<float>(seed & 0xFFFFFF) / float(0x1000000);
}

// ---------------------------------------------------------------------------
// Rain using Sphere primitives.
// Each raindrop is a small emissive sphere — produces soft glowing dots
// that look like real rain droplets catching neon/moonlight.
// Two spheres per drop: a bright core and a larger faint halo.
// ---------------------------------------------------------------------------
static void rainField(World& w,
    float xMin, float xMax,
    float yMin, float yMax,
    float zMin, float zMax,
    int count,
    const RGBColor& tint,
    float brightness,
    unsigned int& seed)
{
    float xR = xMax - xMin;
    float yR = yMax - yMin;
    float zR = zMax - zMin;

    for (int i = 0; i < count; i++) {
        float rx = xMin + frand(seed) * xR;
        float ry = yMin + frand(seed) * yR;
        float rz = zMin + frand(seed) * zR;

        // Core drop: small bright sphere.
        float coreR  = 0.25f + frand(seed) * 0.35f;
        float coreBr = brightness * (0.6f + frand(seed) * 0.6f);
        Sphere* core = new Sphere(Point3D(rx, ry, rz), coreR);
        core->set_material(new Emissive(tint, coreBr));
        w.add_geometry(core);

        // Halo: slightly larger, much dimmer — simulates water-droplet scatter.
        float haloR  = coreR * 2.2f;
        float haloBr = coreBr * 0.18f;
        Sphere* halo = new Sphere(Point3D(rx, ry, rz), haloR);
        halo->set_material(new Emissive(tint, haloBr));
        w.add_geometry(halo);
    }
}

// ---------------------------------------------------------------------------
// Puddle patches on the rooftop.
// ---------------------------------------------------------------------------
static void puddleField(World& w,
    float xMin, float xMax,
    float zMin, float zMax,
    float yTop,
    int count,
    const RGBColor& reflColor,
    float wetness,
    unsigned int& seed)
{
    float xR = xMax - xMin;
    float zR = zMax - zMin;
    for (int i = 0; i < count; i++) {
        float px  = xMin + frand(seed) * xR;
        float pz  = zMin + frand(seed) * zR;
        float pWx = 6.f  + frand(seed) * 22.f;
        float pWz = 5.f  + frand(seed) * 14.f;
        Box* b = new Box(
            Point3D(px,       yTop - 0.4f, pz),
            Point3D(px + pWx, yTop,        pz + pWz));
        b->set_material(new Puddle(reflColor, wetness * (0.55f + frand(seed) * 0.45f)));
        w.add_geometry(b);
    }
}

// ---------------------------------------------------------------------------
// Emissive window grid on a building face (face_z = front face z coord).
// ---------------------------------------------------------------------------
static void windowGrid(World& w,
    float x0, float x1, float y0, float y1, float face_z,
    const RGBColor& col, float brightness, float density, unsigned int& seed)
{
    float cW = 9.f, cH = 11.f, wW = 5.f, wH = 5.f;
    int cols = (int)((x1 - x0) / cW); if (cols < 1) cols = 1;
    int rows = (int)((y1 - y0) / cH); if (rows < 1) rows = 1;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (frand(seed) > density) continue;
            float wx = x0 + c * cW + 2.f;
            float wy = y0 + r * cH + 3.f;
            if (wx + wW > x1 || wy + wH > y1) continue;
            float br = brightness * (0.5f + frand(seed) * 0.8f);
            bool warm = frand(seed) > 0.4f;
            RGBColor wc = warm
                ? RGBColor(col.r, col.g * 0.5f + 0.3f, col.b * 0.1f + 0.1f)
                : col;
            Box* win = new Box(
                Point3D(wx,      wy,      face_z),
                Point3D(wx + wW, wy + wH, face_z + 0.6f));
            win->set_material(new Emissive(wc, br));
            w.add_geometry(win);
        }
    }
}

// ===========================================================================
void World::build() {
// ===========================================================================

    // ── View plane ───────────────────────────────────────────────────────────
    // Camera eye: z=600, y=0 (level).
    // View plane at z=200. Looking slightly upward: eye y=0, plane centre y=10.
    // This shifts the horizon DOWN so the upper frame is open sky.
    vplane.top_left     = Point3D(-120, 160, 200);
    vplane.bottom_right = Point3D( 120, -80, 200);
    vplane.hres = 600;
    vplane.vres = 600;

    bg_color = RGBColor(0.008f, 0.000f, 0.025f); // near-black deep purple

    set_camera(new Perspective(0.f, 0.f, 600.f));
    sampler_ptr = new Simple(camera_ptr, &vplane);
    tracer_ptr  = new Shadow(this);

    // ── Palette ──────────────────────────────────────────────────────────────
    RGBColor bldBlack      (0.00f, 0.00f, 0.00f);
    RGBColor bldDarkPurple (0.025f,0.00f, 0.045f);
    RGBColor bldMidPurple  (0.035f,0.00f, 0.07f);
    RGBColor bldSlatePurple(0.015f,0.00f, 0.032f);

    RGBColor neonMagenta(1.0f,  0.05f, 0.85f);
    RGBColor neonPurple (0.75f, 0.02f, 1.00f);
    RGBColor neonOrange (1.0f,  0.45f, 0.02f);
    RGBColor neonRed    (1.0f,  0.05f, 0.10f);
    RGBColor neonYellow (1.0f,  0.90f, 0.05f);
    RGBColor neonPink   (1.0f,  0.15f, 0.60f);
    RGBColor neonWhite  (0.95f, 0.90f, 1.00f);
    RGBColor winWarm    (1.0f,  0.70f, 0.30f);
    RGBColor winPurple  (0.80f, 0.20f, 1.00f);
    RGBColor moonColor  (0.88f, 0.90f, 1.00f); // cool blue-white

    RGBColor roofColor  (0.025f,0.00f, 0.045f);
    RGBColor detailColor(0.018f,0.00f, 0.032f);

    unsigned int seed = 0xDEADBEEF;

    // Rooftop y-level. View plane bottom is y=-80, so roofY=-72 puts the
    // rooftop just inside the bottom of the frame — barely visible.
    float roofY = -72.f;

    // ── MOON ─────────────────────────────────────────────────────────────────
    // Placed at upper-centre of sky, large and bright.
    // y=120 maps to upper portion of the frame (top_left y=160).
    // z=-350 puts it behind all buildings.
    float moonX = -40.f, moonY = 108.f, moonZ = -350.f;
    float moonRad = 32.f;
    {
        // Solid bright disc.
        Sphere* moon = new Sphere(Point3D(moonX, moonY, moonZ), moonRad);
        moon->set_material(new Emissive(moonColor, 9.0f));
        add_geometry(moon);
    }
    {
        // Inner soft glow ring.
        Sphere* g1 = new Sphere(Point3D(moonX, moonY, moonZ - 1.f), moonRad * 1.35f);
        g1->set_material(new Emissive(moonColor, 1.8f));
        add_geometry(g1);
    }
    {
        // Outer atmospheric halo.
        Sphere* g2 = new Sphere(Point3D(moonX, moonY, moonZ - 2.f), moonRad * 1.80f);
        g2->set_material(new Emissive(moonColor, 0.55f));
        add_geometry(g2);
    }

    // ── FOREGROUND ROOFTOP ───────────────────────────────────────────────────
    // Thin slab at the very bottom — just enough to show the ledge.
    { Box* b = new Box(Point3D(-320, roofY,     180), Point3D(320, roofY + 0.5f, 270));
      b->set_material(new Cosine(roofColor)); add_geometry(b); }

    // Parapet wall — thin, low.
    { Box* b = new Box(Point3D(-320, roofY, 175), Point3D(320, roofY + 8.f, 184));
      b->set_material(new Cosine(detailColor)); add_geometry(b); }

    // Thin railing bar across the top of the parapet.
    { Box* b = new Box(Point3D(-320, roofY + 7.f, 176), Point3D(320, roofY + 9.f, 180));
      b->set_material(new Cosine(detailColor)); add_geometry(b); }

    // A few AC units just above roofY — small, not dominating.
    { Box* b = new Box(Point3D(-150, roofY + 0.5f, 210), Point3D(-110, roofY + 18.f, 238));
      b->set_material(new Cosine(detailColor)); add_geometry(b); }
    { Box* b = new Box(Point3D( 120, roofY + 0.5f, 212), Point3D( 155, roofY + 15.f, 234));
      b->set_material(new Cosine(detailColor)); add_geometry(b); }
    { Box* b = new Box(Point3D(  20, roofY + 0.5f, 220), Point3D(  55, roofY + 12.f, 240));
      b->set_material(new Cosine(detailColor)); add_geometry(b); }

    // Antenna on right.
    { Box* b = new Box(Point3D(235, roofY + 0.5f, 210), Point3D(239, roofY + 70.f, 214));
      b->set_material(new Cosine(detailColor)); add_geometry(b); }
    { Box* b = new Box(Point3D(222, roofY + 55.f, 210), Point3D(252, roofY + 57.f, 214));
      b->set_material(new Cosine(detailColor)); add_geometry(b); }

    // Water tower left.
    { Box* b = new Box(Point3D(-270, roofY + 0.5f, 222), Point3D(-248, roofY + 30.f, 244));
      b->set_material(new Cosine(detailColor)); add_geometry(b); }
    { Box* b = new Box(Point3D(-268, roofY + 30.f, 224), Point3D(-250, roofY + 33.f, 242));
      b->set_material(new Cosine(detailColor)); add_geometry(b); }

    // ── PUDDLES ──────────────────────────────────────────────────────────────
    puddleField(*this, -280, -20, 186, 255, roofY + 0.5f, 30, neonMagenta, 0.88f, seed);
    puddleField(*this,  -60, 110, 186, 255, roofY + 0.5f, 28, neonPurple,  0.82f, seed);
    puddleField(*this,   70, 280, 186, 255, roofY + 0.5f, 25, neonOrange,  0.78f, seed);
    // Moon reflection — elongated bright puddle in centre.
    { Box* b = new Box(Point3D(-35, roofY + 0.1f, 200), Point3D(35, roofY + 0.5f, 245));
      b->set_material(new Puddle(moonColor, 0.88f)); add_geometry(b); }
    // Small emissive glow strip at ledge base.
    { Box* b = new Box(Point3D(-100, roofY + 0.5f, 184), Point3D(60, roofY + 0.9f, 188));
      b->set_material(new Emissive(neonMagenta, 2.5f)); add_geometry(b); }

    // ── BUILDINGS ────────────────────────────────────────────────────────────
    // All buildings are SHORT — tops reach y=40 to y=100 max.
    // The view plane top is y=160, so the upper 40% of the image is pure sky.

    // Left flanking block.
    { Box* b = new Box(Point3D(-370, -55, -25), Point3D(-182, 85, 148));
      b->set_material(new Cosine(bldBlack)); add_geometry(b); }
    windowGrid(*this, -370, -182, -55, 85, 148, winPurple, 3.8f, 0.32f, seed);

    // Right flanking block.
    { Box* b = new Box(Point3D(182, -55, -25), Point3D(370, 95, 148));
      b->set_material(new Cosine(bldBlack)); add_geometry(b); }
    windowGrid(*this, 182, 370, -55, 95, 148, winWarm, 3.8f, 0.30f, seed);

    // Central tower — tallest building, still well below sky zone.
    { Box* b = new Box(Point3D(18, -55, -128), Point3D(112, 120, -12));
      b->set_material(new Cosine(bldMidPurple)); add_geometry(b); }
    // Horizontal ledge band.
    { Box* b = new Box(Point3D(16, 55, -130), Point3D(114, 59, -10));
      b->set_material(new Cosine(bldDarkPurple)); add_geometry(b); }
    // Thin spire on top.
    { Box* b = new Box(Point3D(55, 120, -90), Point3D(75, 185, -50));
      b->set_material(new Cosine(detailColor)); add_geometry(b); }
    windowGrid(*this, 18, 112, -55, 120, -12, winPurple, 4.2f, 0.50f, seed);

    // Centre-left tower.
    { Box* b = new Box(Point3D(-128, -55, -142), Point3D(-22, 90, -38));
      b->set_material(new Cosine(bldDarkPurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(-130, 42, -144), Point3D(-20, 46, -36));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    windowGrid(*this, -128, -22, -55, 90, -38, winPurple, 3.8f, 0.46f, seed);

    // Centre block (wide, shorter).
    { Box* b = new Box(Point3D(-72, -55, -82), Point3D(12, 70, 20));
      b->set_material(new Cosine(bldMidPurple)); add_geometry(b); }
    windowGrid(*this, -72, 12, -55, 70, 20, winWarm, 3.5f, 0.42f, seed);

    // Right-centre tower.
    { Box* b = new Box(Point3D(90, -55, -162), Point3D(160, 80, -80));
      b->set_material(new Cosine(bldMidPurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(88, 38, -164), Point3D(162, 42, -78));
      b->set_material(new Cosine(bldDarkPurple)); add_geometry(b); }
    windowGrid(*this, 90, 160, -55, 80, -80, winPurple, 3.8f, 0.44f, seed);

    // Far background row — very short, just horizon silhouettes.
    { Box* b = new Box(Point3D(-242, -55, -485), Point3D(-70, 55, -360));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    windowGrid(*this, -242, -70, -55, 55, -360, winPurple, 2.2f, 0.35f, seed);
    { Box* b = new Box(Point3D(-95, -55, -452), Point3D(14, 48, -325));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    windowGrid(*this, -95, 14, -55, 48, -325, winWarm, 2.0f, 0.33f, seed);
    { Box* b = new Box(Point3D(8, -55, -500), Point3D(105, 52, -370));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    windowGrid(*this, 8, 105, -55, 52, -370, winPurple, 2.0f, 0.35f, seed);
    { Box* b = new Box(Point3D(90, -55, -470), Point3D(195, 45, -345));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(170, -55, -510), Point3D(288, 40, -380));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(-315, -55, -460), Point3D(-175, 42, -330));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    windowGrid(*this, -315, -175, -55, 42, -330, winPurple, 1.8f, 0.30f, seed);

    // Procedural horizon silhouettes — tiny buildings at the very horizon.
    for (int i = 0; i < 120; i++) {
        float w  = 8.f  + frand(seed) * 80.f;
        float h  = 10.f + frand(seed) * 55.f;   // very short
        float d  = -620.f - frand(seed) * 1600.f;
        float xp = -820.f + frand(seed) * 1640.f;
        float th = 18.f + frand(seed) * 80.f;
        float dk = 0.012f + (d + 620.f) / -2000.f;
        if (dk < 0.003f) dk = 0.003f;
        RGBColor col(dk * 0.65f, dk * 0.04f, dk * 1.05f);
        Box* b = new Box(Point3D(xp, -55, d), Point3D(xp + w, h, d + th));
        b->set_material(new Cosine(col));
        add_geometry(b);
    }

    // ── NEON SIGNS ───────────────────────────────────────────────────────────
    // Repositioned to match shorter building heights.

    // Left tower.
    { Box* s = new Box(Point3D(-192, 28, 184), Point3D(-128,  72, 187));
      s->set_material(new Emissive(neonMagenta, 22.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-210,  5, 184), Point3D(-128,  20, 186));
      s->set_material(new Emissive(neonPurple,  20.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-208,-30, 184), Point3D(-188,   4, 186));
      s->set_material(new Emissive(neonPink,    17.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-196, 78, 183), Point3D(-120,  90, 185));
      s->set_material(new Emissive(neonWhite,   21.f)); add_geometry(s); }

    // Right tower.
    { Box* s = new Box(Point3D(128,  42, 184), Point3D(190,  80, 187));
      s->set_material(new Emissive(neonRed,    20.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(124,  10, 184), Point3D(206,  24, 186));
      s->set_material(new Emissive(neonOrange, 18.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(138,  88, 183), Point3D(180,  98, 185));
      s->set_material(new Emissive(neonMagenta,15.f)); add_geometry(s); }

    // Central tower signs — vertical neon stripes.
    { Box* s = new Box(Point3D(20,  -20, 22), Point3D(30, 115, 25));
      s->set_material(new Emissive(neonMagenta, 23.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(100, -20, 22), Point3D(110, 95, 25));
      s->set_material(new Emissive(neonPurple,  21.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(48,   30, 23), Point3D(80,  38, 25));
      s->set_material(new Emissive(neonWhite,   19.f)); add_geometry(s); }

    // Centre-left signs.
    { Box* s = new Box(Point3D(-104, 30, 33), Point3D(-26, 58, 35));
      s->set_material(new Emissive(neonPurple,  19.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-104, -5, 33), Point3D(-86, 28, 35));
      s->set_material(new Emissive(neonMagenta, 17.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-102, 62, 33), Point3D(-84, 90, 35));
      s->set_material(new Emissive(neonOrange,  18.f)); add_geometry(s); }

    // Mid-scene signs.
    { Box* s = new Box(Point3D(-50, 15, 63), Point3D(20, 40, 65));
      s->set_material(new Emissive(neonOrange, 17.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(108, 18, -17), Point3D(162, 55, -15));
      s->set_material(new Emissive(neonYellow, 18.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(108,  0, -17), Point3D(162, 15, -15));
      s->set_material(new Emissive(neonOrange, 16.f)); add_geometry(s); }

    // Distant horizon neon glow bands.
    { Box* s = new Box(Point3D(-275, 10, -218), Point3D( -78, 24, -214));
      s->set_material(new Emissive(neonMagenta, 12.f)); add_geometry(s); }
    { Box* s = new Box(Point3D(  14, 12, -218), Point3D( 195, 25, -214));
      s->set_material(new Emissive(neonPurple,  11.f)); add_geometry(s); }
    { Box* s = new Box(Point3D( 180,  4, -218), Point3D( 245, 14, -216));
      s->set_material(new Emissive(neonOrange,  10.f)); add_geometry(s); }

    // ── STARS ────────────────────────────────────────────────────────────────
    // Concentrated in the upper sky area (y = 60 to 155).
    for (int i = 0; i < 350; i++) {
        float sx = -450.f + frand(seed) * 900.f;
        float sy =   60.f + frand(seed) * 95.f;   // upper sky band
        float sz = -580.f - frand(seed) * 320.f;
        float sr =   0.6f + frand(seed) * 1.4f;
        float br =   3.0f + frand(seed) * 5.0f;
        float t  = frand(seed);
        RGBColor col = (t < 0.55f) ? neonWhite : (t < 0.78f) ? winPurple : neonYellow;
        Sphere* star = new Sphere(Point3D(sx, sy, sz), sr);
        star->set_material(new Emissive(col, br));
        add_geometry(star);
    }

    // ── 1M PRIMITIVES: horizon building window grids ──────────────────────────
    unsigned int ws = 0xDEADBEEF;
    for (int i = 0; i < 120; i++) {
        float w  = 8.f  + frand(ws) * 80.f;
        float h  = 10.f + frand(ws) * 55.f;
        float d  = -620.f - frand(ws) * 1600.f;
        float xp = -820.f + frand(ws) * 1640.f;
        float th = 18.f + frand(ws) * 80.f;
        float fz = d + th;
        float cw = std::max(1.2f, w / 9.f);
        float ch = std::max(1.2f, h / 40.f);
        int wc = std::max(1, (int)(w / cw));
        int wr = std::max(1, (int)(h / ch));
        float ww = cw * 0.48f, wh = ch * 0.50f;
        unsigned int wsub = (unsigned int)(i * 1000003u + 7u);
        for (int r = 0; r < wr; r++) {
            for (int c = 0; c < wc; c++) {
                if (frand(wsub) < 0.45f) continue;
                float wx = xp + c * cw;
                float wy = -55.f + r * ch;
                bool warm = frand(wsub) > 0.45f;
                float br = 0.4f + frand(wsub) * 1.4f;
                Box* win = new Box(
                    Point3D(wx,      wy,      fz),
                    Point3D(wx + ww, wy + wh, fz + 0.5f));
                win->set_material(new Emissive(warm ? winWarm : winPurple, br));
                add_geometry(win);
            }
        }
    }

    // ── RAIN ─────────────────────────────────────────────────────────────────
    // Sphere-based droplets — soft glowing dots, NOT stiff rectangles.
    // Each call places core + halo spheres, giving a natural bokeh-like look.
    // All zones are inside the scene (z < 200).
    // Brightness kept low so droplets are subtle against the dark sky.

    // Moonlit rain (pale blue-white, centre of scene).
    rainField(*this, -220, 220, roofY, 155, -15, 195,
              800, moonColor, 0.55f, seed);

    // Left zone — purple tinted (near left neon signs).
    rainField(*this, -320, -10, roofY, 120, -70, 140,
              400, neonPurple, 0.45f, seed);

    // Right zone — pink/orange tinted (near right neon signs).
    rainField(*this, 10, 320, roofY, 120, -70, 140,
              400, neonPink, 0.45f, seed);

    // Mid-distance layer (fainter, smaller drops).
    rainField(*this, -300, 300, roofY, 100, -280, -30,
              600, RGBColor(0.55f, 0.30f, 0.80f), 0.28f, seed);

    // Far background (very faint, deep purple).
    rainField(*this, -380, 380, 0, 80, -580, -250,
              500, RGBColor(0.30f, 0.02f, 0.48f), 0.18f, seed);

    // ── LIGHTS ───────────────────────────────────────────────────────────────

    // Moon: primary cool directional light from upper-left.
    add_light(new DirectionalLight(
        -0.25f, -1.0f, -0.35f,
        moonColor.r, moonColor.g, moonColor.b,
        0.60f));

    // Dim purple ambient fill.
    add_light(new DirectionalLight(
        0.0f, -1.0f, 0.0f,
        0.12f, 0.00f, 0.18f,
        0.18f));

    // Moon as point light (from moon sphere position).
    add_light(new PointLight(Point3D(moonX, moonY, moonZ), moonColor, 200.f));

    // Neon sign point lights.
    add_light(new PointLight(Point3D(-160.f,  50.f, 200.f), neonMagenta, 125.f));
    add_light(new PointLight(Point3D(-168.f,  12.f, 200.f), neonPurple,  115.f));
    add_light(new PointLight(Point3D( 159.f,  61.f, 200.f), neonRed,     120.f));
    add_light(new PointLight(Point3D( 165.f,  17.f, 200.f), neonOrange,  110.f));
    add_light(new PointLight(Point3D(  25.f,  48.f,  25.f), neonMagenta, 110.f));
    add_light(new PointLight(Point3D( 105.f,  38.f,  25.f), neonPurple,  100.f));
    add_light(new PointLight(Point3D( -65.f,  22.f,  36.f), neonPink,    105.f));
    add_light(new PointLight(Point3D(-100.f,  75.f,  65.f), neonOrange,   95.f));
    add_light(new PointLight(Point3D( 135.f,  30.f, -16.f), neonYellow,   90.f));
    add_light(new PointLight(Point3D(-185.f,  84.f, 200.f), neonPurple,  110.f));
    add_light(new PointLight(Point3D( 134.f,  93.f, 200.f), neonMagenta, 110.f));

    // Rooftop puddle fill lights — illuminate puddles and near rain drops.
    add_light(new PointLight(Point3D( -80.f, roofY + 4.f, 215.f), neonMagenta, 70.f));
    add_light(new PointLight(Point3D(  40.f, roofY + 4.f, 215.f), neonPurple,  65.f));
    add_light(new PointLight(Point3D( 140.f, roofY + 4.f, 215.f), neonOrange,  60.f));
    add_light(new PointLight(Point3D(   0.f, roofY + 4.f, 225.f), moonColor,   55.f));

    // ── BVH ──────────────────────────────────────────────────────────────────
    BVH* bvh = new BVH();
    bvh->build(geometry);
    accel_ptr = bvh;
}
