/**
 * NightCity.cpp
 *
 * Raytraced cyberpunk night city scene - Purple/Magenta aesthetic.
 *
 * This scene features:
 * - Lower foreground rooftop (less tall) with explicit fence,
 *   AC units, utility boxes, pipes, and cables.
 * - Progressively darker, high-density background skyscrapers.
 * - Purple/magenta/orange neon signage (NO BLUE).
 * - Deep purple ambient lighting with vibrant neon contrasts.
 */

#include "../cameras/Perspective.hpp"

#include "../extras/Box.hpp"
#include "../extras/Emissive.hpp"

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

void World::build() {

    // ── View plane ────────────────────────────────────────────────────────────
    vplane.top_left     = Point3D(-120, 150, 200);
    vplane.bottom_right = Point3D( 120, -150, 200);
    vplane.hres = 600;
    vplane.vres = 750;

    // Deep purple night sky background (NO BLUE)
    bg_color = RGBColor(0.02f, 0.00f, 0.06f);  // Purple-black

    set_camera(new Perspective(0, 30, 600));
    sampler_ptr = new Simple(camera_ptr, &vplane);
    tracer_ptr  = new Shadow(this);

    // ── Palette: Purple/Magenta focus, NO BLUE ─────────────────────────────────
    RGBColor bldDarkPurple (0.03f, 0.00f, 0.06f);  // Deep purple
    RGBColor bldMidPurple  (0.04f, 0.00f, 0.09f);  // Medium purple
    RGBColor bldWarmPurple (0.05f, 0.00f, 0.07f);  // Warm-leaning purple
    RGBColor bldSlatePurple(0.01f, 0.00f, 0.04f);  // Very dark purple-black
    RGBColor bldBlack      (0.00f, 0.00f, 0.00f);  // Pure black framing

    // Intensified Neon colors (Purple, Magenta, Orange, Red, Yellow - NO BLUE/CYAN)
    RGBColor neonMagenta(1.0f, 0.05f, 0.85f);  // Strong magenta
    RGBColor neonPurple (0.85f, 0.02f, 1.0f);  // Deep purple neon
    RGBColor neonOrange (1.0f,  0.45f, 0.02f); // Warm orange
    RGBColor neonRed    (1.0f,  0.05f, 0.10f); // Bright red
    RGBColor neonYellow (1.0f,  0.90f, 0.05f); // Yellow accent
    RGBColor neonPink   (1.0f,  0.15f, 0.60f); // Hot pink
    RGBColor neonWhite  (0.95f, 0.85f, 1.0f);  // Slightly purple-tinted white

    RGBColor winWarm    (1.0f,  0.70f, 0.30f);  // Warm orange windows
    RGBColor winPurple  (0.85f, 0.20f, 1.0f);   // Purple windows (replaces winCool)
    RGBColor roofColor  (0.04f, 0.00f, 0.08f);  // Dark purple roof
    RGBColor ledgeColor (0.02f, 0.00f, 0.04f);  // Very dark purple
    RGBColor detailsColor(0.03f, 0.00f, 0.05f); // Utility structures

    unsigned int seed = 0xDEADBEEF;

    // ── Foreground rooftop (clean + aligned) ─────────────────────────────

    float roofY = -25.0f;

    // Thin roof surface (invisible thickness)
    { Box* b = new Box(Point3D(-320, roofY, 180), Point3D(320, roofY + 0.1f, 260));
    b->set_material(new Cosine(roofColor)); add_geometry(b); }

    // Ledge (front lip)
    { Box* b = new Box(Point3D(-320, roofY, 175), Point3D(320, roofY + 12, 185));
    b->set_material(new Cosine(ledgeColor)); add_geometry(b); }

    // ── Fence ────────────────────────────────────────────────────────────

    // Vertical posts
    for(int i=-3; i<=3; i++) {
        if(i==0) continue;
        { Box* post = new Box(
            Point3D(i*80.f - 2.5f, roofY + 12, 178),
            Point3D(i*80.f + 2.5f, roofY + 70, 182)
        );
        post->set_material(new Cosine(detailsColor)); add_geometry(post); }
    }

    // Horizontal rails
    { Box* railT = new Box(Point3D(-320, roofY + 65, 178), Point3D(320, roofY + 70, 182));
    railT->set_material(new Cosine(detailsColor)); add_geometry(railT); }

    { Box* railB = new Box(Point3D(-320, roofY + 20, 178), Point3D(320, roofY + 25, 182));
    railB->set_material(new Cosine(detailsColor)); add_geometry(railB); }

    // ── AC units ─────────────────────────────────────────────────────────

    { Box* ac1 = new Box(Point3D(-160, roofY + 0.1f,210), Point3D(-110, roofY + 30,245));
    ac1->set_material(new Cosine(detailsColor)); add_geometry(ac1); }

    { Box* ac2 = new Box(Point3D(-95, roofY + 0.1f,215), Point3D(-65, roofY + 25,240));
    ac2->set_material(new Cosine(detailsColor)); add_geometry(ac2); }

    { Box* ac3 = new Box(Point3D(-50, roofY + 0.1f,205), Point3D(-10, roofY + 28,235));
    ac3->set_material(new Cosine(detailsColor)); add_geometry(ac3); }

    { Box* ac4 = new Box(Point3D(180, roofY + 0.1f,210), Point3D(220, roofY + 22,240));
    ac4->set_material(new Cosine(detailsColor)); add_geometry(ac4); }

    { Box* ac5 = new Box(Point3D(120, roofY + 0.1f,215), Point3D(150, roofY + 24,240));
    ac5->set_material(new Cosine(detailsColor)); add_geometry(ac5); }

    // ── Pipes / utility boxes ────────────────────────────────────────────

    { Box* p1 = new Box(Point3D(-190, roofY + 0.1f,190), Point3D(-186, roofY + 30,194));
    p1->set_material(new Cosine(detailsColor)); add_geometry(p1); }

    { Box* p2 = new Box(Point3D(-195, roofY + 25,185), Point3D(-181, roofY + 35,199));
    p2->set_material(new Cosine(detailsColor)); add_geometry(p2); }

    { Box* box1 = new Box(Point3D(240, roofY + 0.1f,180), Point3D(270, roofY + 13,200));
    box1->set_material(new Cosine(detailsColor)); add_geometry(box1); }

    { Box* p3 = new Box(Point3D(250, roofY + 0.1f,190), Point3D(254, roofY + 28,194));
    p3->set_material(new Cosine(detailsColor)); add_geometry(p3); }

    // ── Large duct ───────────────────────────────────────────────────────

    { Box* duct = new Box(Point3D(-300, roofY + 18,250), Point3D(300, roofY + 26,258));
    duct->set_material(new Cosine(detailsColor)); add_geometry(duct); }

    // ── Wires / conduits ─────────────────────────────────────────────────

    { Box* w1 = new Box(Point3D(-130, roofY + 2,200), Point3D(-100, roofY + 4,202));
    w1->set_material(new Cosine(bldBlack)); add_geometry(w1); }

    { Box* w2 = new Box(Point3D(140, roofY + 1,205), Point3D(170, roofY + 3,207));
    w2->set_material(new Cosine(bldBlack)); add_geometry(w2); }

    // ── Antenna ──────────────────────────────────────────────────────────

    { Box* b = new Box(Point3D(240, roofY + 40,210), Point3D(245, roofY + 125,215));
    b->set_material(new Cosine(detailsColor)); add_geometry(b); }

    // ── Layer 1: Flanking towers (Framing - Pitch Black) ───────────────────
    { Box* b = new Box(Point3D(-370,-55,-30), Point3D(-185,680,140));
      b->set_material(new Cosine(bldBlack)); add_geometry(b); }
    { Box* b = new Box(Point3D(185,-55,-30), Point3D(370,700,140));
      b->set_material(new Cosine(bldBlack)); add_geometry(b); }

    // ── Layer 2: Central skyscrapers (ALL PURPLE TONES - NO BLUE) ───────────
    { Box* b = new Box(Point3D(15,-55,-130), Point3D(125,740, -10));
      b->set_material(new Cosine(bldMidPurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(55,740,-85), Point3D(85,960,-35));
      b->set_material(new Cosine(detailsColor)); add_geometry(b); }
    { Box* b = new Box(Point3D(-135,-55,-150), Point3D(-15,630,-35));
      b->set_material(new Cosine(bldDarkPurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(-80,-55,-90), Point3D(5,490,15));
      b->set_material(new Cosine(bldWarmPurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(85,-55,-170), Point3D(165,540,-75));
      b->set_material(new Cosine(bldMidPurple)); add_geometry(b); }

    // ── Layer 3: Far background row (Purple-black) ──────────────────────────
    { Box* b = new Box(Point3D(-250,-55,-500), Point3D(-70,430,-360));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(-100,-55,-460), Point3D(10,380,-330));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(0,-55,-510), Point3D(110,410,-370));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(85,-55,-480), Point3D(200,470,-345));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(175,-55,-520), Point3D(295,390,-380));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }
    { Box* b = new Box(Point3D(-320,-55,-470), Point3D(-180,420,-330));
      b->set_material(new Cosine(bldSlatePurple)); add_geometry(b); }

    // ── Layer 4: Horizon skyscrapers (purple-black canyons) ──────────────────
    {
        for (int i = 0; i < 120; i++) {
            float width  = 10.f + frand(seed) * 120.f;
            float height = 150.f + frand(seed) * 1200.f;
            float depth  = -600.f - frand(seed) * 1800.f;
            float xPos   = -800.f + frand(seed) * 1600.f;
            float thickness = 20.f + frand(seed) * 120.f;

            float darkness = 0.02f + (depth + 600.f) / -2000.f;
            if (darkness < 0.005f) darkness = 0.005f;
            
            // Purple-tinted darkness (no blue)
            RGBColor col(darkness * 0.8f, darkness * 0.1f, darkness * 1.2f);

            Box* b = new Box(
                Point3D(xPos, -55, depth),
                Point3D(xPos + width, height, depth + thickness)
            );
            b->set_material(new Cosine(col));
            add_geometry(b);
        }
    }

    // ── Neon signs (PURPLE/MAGENTA/ORANGE only - NO BLUE/CYAN) ────────────────

    // Left tower: purple/magenta vertical sign
    { Box* s = new Box(Point3D(-195,215,182), Point3D(-125,295,185));
      s->set_material(new Emissive(neonMagenta,20.0f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-215,165,182), Point3D(-125,185,184));
      s->set_material(new Emissive(neonPurple,18.5f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-205,85,182), Point3D(-185,160,184));
      s->set_material(new Emissive(neonPink,16.8f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-190, 310, 180), Point3D(-180, 480, 182));
      s->set_material(new Emissive(neonPurple,15.0f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-140, 110, 180), Point3D(-130, 150, 182));
      s->set_material(new Emissive(neonRed,14.0f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-200, 320, 182), Point3D(-120, 335, 184));
      s->set_material(new Emissive(neonWhite,22.0f)); add_geometry(s); }

    // Right tower: orange/red vertical sign
    { Box* s = new Box(Point3D(125,240,182), Point3D(195,305,185));
      s->set_material(new Emissive(neonRed,18.0f)); add_geometry(s); }
    { Box* s = new Box(Point3D(120,195,182), Point3D(210,212,184));
      s->set_material(new Emissive(neonOrange,16.0f)); add_geometry(s); }
    { Box* s = new Box(Point3D(135, 320, 180), Point3D(185, 340, 182));
      s->set_material(new Emissive(neonMagenta,14.5f)); add_geometry(s); }
    { Box* s = new Box(Point3D(150, 130, 180), Point3D(160, 185, 182));
      s->set_material(new Emissive(neonPurple,15.5f)); add_geometry(s); }
    { Box* s = new Box(Point3D(190, 220, 182), Point3D(205, 235, 184));
      s->set_material(new Emissive(neonYellow,16.0f)); add_geometry(s); }

    // Central tower: magenta/purple stripes (replaced cyan/blue)
    { Box* s = new Box(Point3D(32,310,21), Point3D(42,610,24));
      s->set_material(new Emissive(neonMagenta,22.0f)); add_geometry(s); }
    { Box* s = new Box(Point3D(98,310,21), Point3D(108,550,24));
      s->set_material(new Emissive(neonPurple,21.0f)); add_geometry(s); }
    { Box* s = new Box(Point3D(50, 350, 23), Point3D(80, 360, 25));
      s->set_material(new Emissive(neonWhite,19.0f)); add_geometry(s); }

    // Centre-left tower: purple/orange bands
    { Box* s = new Box(Point3D(-108,260,31), Point3D(-30,320,33));
      s->set_material(new Emissive(neonPurple,18.5f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-108,180,31), Point3D(-88,255,33));
      s->set_material(new Emissive(neonMagenta,16.2f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-105, 330, 31), Point3D(-85, 410, 33));
      s->set_material(new Emissive(neonOrange,17.0f)); add_geometry(s); }

    // Mid-scene: orange/magenta bands (replaced green)
    { Box* s = new Box(Point3D(-55,190,61), Point3D(25,235,63));
      s->set_material(new Emissive(neonOrange,16.5f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-30, 160, 61), Point3D(-10, 185, 63));
      s->set_material(new Emissive(neonMagenta,14.8f)); add_geometry(s); }

    // Right mid building: warm billboard
    { Box* s = new Box(Point3D(105,210,-19), Point3D(168,285,-17));
      s->set_material(new Emissive(neonYellow,17.8f)); add_geometry(s); }
    { Box* s = new Box(Point3D(105,170,-19), Point3D(168,198,-17));
      s->set_material(new Emissive(neonOrange,16.2f)); add_geometry(s); }

    // ── Far background neon (purple/magenta only) ──────────────────────────
    { Box* s = new Box(Point3D(-280,75,-220), Point3D(-80,105,-216));
      s->set_material(new Emissive(neonMagenta,13.0f)); add_geometry(s); }
    { Box* s = new Box(Point3D(10,80,-220), Point3D(200,110,-216));
      s->set_material(new Emissive(neonPurple,12.8f)); add_geometry(s); }
    { Box* s = new Box(Point3D(180, 60, -220), Point3D(250, 75, -218));
      s->set_material(new Emissive(neonOrange,11.0f)); add_geometry(s); }
    { Box* s = new Box(Point3D(-220, 50, -220), Point3D(-160, 62, -218));
      s->set_material(new Emissive(neonRed,10.0f)); add_geometry(s); }

    // ── Window lights (PURPLE/WARM tones - NO COOL BLUES) ────────────────────

    // Left framing tower windows
    for (int row = 0; row < 18; row++) {
        for (int col = 0; col < 6; col++) {
            float wx0 = -215.f + col*11.f;
            float wy0 =   25.f + row*28.f;
            bool warm = ((row+col)%2==0);
            Box* w = new Box(Point3D(wx0,wy0,139.9f), Point3D(wx0+6,wy0+4,140.0f));
            w->set_material(new Emissive(warm ? winWarm : winPurple, 2.0f+frand(seed)*2.5f));
            add_geometry(w);
        }
    }
    // Right framing tower windows
    for (int row = 0; row < 18; row++) {
        for (int col = 0; col < 6; col++) {
            float wx0 = 135.f + col*11.f;
            float wy0 =  20.f + row*28.f;
            bool warm = ((row+col+1)%2==0);
            Box* w = new Box(Point3D(wx0,wy0,-10.1f), Point3D(wx0+7,wy0+5,-10.0f));
            w->set_material(new Emissive(warm ? winWarm : winPurple, 2.0f+frand(seed)*2.5f));
            add_geometry(w);
        }
    }
    // Central tower windows
    for (int row = 0; row < 22; row++) {
        for (int col = 0; col < 5; col++) {
            float wx0 = 22.f + col*16.f;
            float wy0 = 10.f + row*28.f;
            if (frand(seed) < 0.12f) continue;
            bool warm = frand(seed) > 0.5f;
            Box* w = new Box(Point3D(wx0,wy0,-9.5f), Point3D(wx0+7,wy0+5,-8.5f));
            w->set_material(new Emissive(warm ? winWarm : winPurple, 1.5f+frand(seed)*3.5f));
            add_geometry(w);
        }
    }
    // Background scattered windows
    for (int i = 0; i < 220; i++) {
        float rx = -240.f + frand(seed)*460.f;
        float ry =     5.f + frand(seed)*360.f;
        float rz = -290.f + frand(seed)*420.f;
        float rad = 1.2f + frand(seed)*3.5f;
        bool  warm = frand(seed) > 0.5f;
        Box* w = new Box(Point3D(rx,ry,rz), Point3D(rx+6,ry+4,rz+1.0f));
        w->set_material(new Emissive(warm ? winWarm : winPurple, rad));
        add_geometry(w);
    }

    // ── Stars (warm/purple tint) ─────────────────────────────────────────────
    for (int i = 0; i < 300; i++) {
        float sx = -450.f + frand(seed)*900.f;
        float sy =  100.f + frand(seed)*480.f;
        float sz = -600.f - frand(seed)*300.f;
        float sr =    0.8f + frand(seed)*1.8f;
        float br =    4.0f + frand(seed)*6.0f;
        float t  = frand(seed);
        RGBColor col = (t<0.5f) ? neonWhite : (t<0.75f) ? winPurple : neonYellow;
        Sphere* star = new Sphere(Point3D(sx,sy,sz), sr);
        star->set_material(new Emissive(col, br));
        add_geometry(star);
    }

    // ── Lights (PURPLE/MAGENTA focus - NO BLUE) ───────────────────────────────

    // Deep purple ambient directional (moonlight)
    add_light(new DirectionalLight(0.0f, -1.0f, -0.3f, 0.45f, 0.10f, 0.65f, 0.45f));

    // Neon point lights (purple/magenta/orange only)
    add_light(new PointLight(Point3D(-162.f,230.f,200.f), neonMagenta, 140.f));
    add_light(new PointLight(Point3D(-165.f,167.f,200.f), neonPurple,   135.f));
    add_light(new PointLight(Point3D( 162.f,242.f,200.f), neonRed,     135.f));
    add_light(new PointLight(Point3D( 165.f,187.f,200.f), neonOrange,  120.f));
    add_light(new PointLight(Point3D(  37.f,420.f, 25.f), neonMagenta,  120.f));
    add_light(new PointLight(Point3D(  98.f,380.f, 25.f), neonPurple,   110.f));
    add_light(new PointLight(Point3D( -68.f,275.f, 35.f), neonPink,     115.f));
    add_light(new PointLight(Point3D( -17.f,197.f, 65.f), neonOrange,   105.f));
    add_light(new PointLight(Point3D( 136.f, 60.f, 15.f), winWarm,      100.f));
    add_light(new PointLight(Point3D(-190.f,330.f,200.f), neonPurple,   120.f));
    add_light(new PointLight(Point3D( 135.f,320.f,200.f), neonMagenta,  120.f));
    add_light(new PointLight(Point3D(-100.f,370.f, 65.f), neonOrange,   110.f));

    // ── BVH acceleration ─────────────────────────────────────────────────────
    BVH* bvh = new BVH();
    bvh->build(geometry);
    accel_ptr = bvh;
}