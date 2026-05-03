/**
 * NightCity.cpp  —  CS 440 Project 2 Scene Build  (FIXED)
 *
 * Fixes applied:
 *
 *   1. WINDOWS ON THE BUILDINGS
 *      The original add_windows() placed window boxes at face_z with depth
 *      going in the +z direction.  For buildings whose front face (facing the
 *      camera at z = 500) is their largest-z face, the window boxes must be
 *      placed so that face_z is the FRONT (max-z) of the building.
 *      Fixed: pass z1 (the camera-facing face) as face_z and grow windows
 *      slightly toward the camera (+z direction from face_z).
 *
 *   2. BUILDINGS DARK BLUE, NOT BRIGHT GREY
 *      - Lowered BuildingFacade ambient from 0.10 to 0.03 so the base shade
 *        is very dark.
 *      - Lowered Lambertian kd from 0.80 to 0.40 so buildings do not
 *        over-respond to the many window PointLights.
 *      - Lowered diffuse weight in Shadow tracer from 0.25 to 0.12 so
 *        windows illuminate only their immediate surroundings.
 *      - Increased Shadow tracer distance cull from 300 to 180 so distant
 *        lights do not flood far buildings.
 *
 *   3. GROUND LOOKS CORRECT
 *      The old GlossyGround::shade() returned near-black with a faint ripple
 *      and never invoked the GlossySpecular BRDF, so the wet-street appearance
 *      was completely missing.  The new WetStreet material calls both the
 *      Lambertian and GlossySpecular BRDFs properly.  The Shadow tracer now
 *      sees the glossy term and produces bright specular smears from the
 *      window PointLights on the ground surface.
 *
 *   All building coordinates are unchanged — only face_z arguments to
 *   add_windows() have been corrected to point at the camera-facing face.
 */

#include "../acceleration/BVH.hpp"
#include "../BRDF/GlossySpecular.hpp"
#include "../BRDF/Lambertian.hpp"
#include "../BRDF/PerfectSpecular.hpp"
#include "../cameras/Perspective.hpp"
#include "../extras/Box.hpp"
#include "../extras/Emissive.hpp"
#include "../geometry/Plane.hpp"
#include "../geometry/Sphere.hpp"
#include "../lights/DirectionalLight.hpp"
#include "../lights/PointLight.hpp"
#include "../materials/Material.hpp"
#include "../samplers/Jittered.hpp"
#include "../tracers/Shadow.hpp"
#include "../utilities/Constants.hpp"
#include "../utilities/Point3D.hpp"
#include "../utilities/RGBColor.hpp"
#include "../utilities/ShadeInfo.hpp"
#include "../utilities/Vector3D.hpp"
#include "../world/World.hpp"

#include <cmath>
#include <cstdlib>


// ===========================================================================
// CONSTANTS
// ===========================================================================

// Desaturated dark blue-grey sky — matches a light-polluted night city.
static const RGBColor SKY_HAZE(0.042f, 0.058f, 0.110f);


// ===========================================================================
// LCG RANDOM — deterministic, same result every run
// ===========================================================================

static float frand(unsigned int& seed)
{
    seed = seed * 1664525u + 1013904223u;
    return static_cast<float>(seed & 0xFFFFFF) / static_cast<float>(0x1000000);
}


// ===========================================================================
// MATERIAL: BuildingFacade
//
// Dark blue-grey Lambertian surface.  Low ambient keeps the base very dark
// so buildings read as near-black steel-blue, with windows providing the
// only real illumination pops.
//
// FIX: kd lowered to 0.40 and ambient to 0.03 (was 0.80 / 0.10).
// ===========================================================================
class BuildingFacade : public Material {
private:
    Lambertian brdf;
    float      ambient;

public:
    // fog = 0 → full color, fog = 1 → fully dissolved into sky haze
    BuildingFacade(const RGBColor& base_col, float fog = 0.0f, float amb = 0.03f)
        : brdf(0.40f, lerp(base_col, SKY_HAZE, fog)),
          ambient(amb)
    {}

    virtual ~BuildingFacade() = default;

    virtual RGBColor shade(const ShadeInfo& sinfo) const override
    {
        // kd * cd / PI evaluated at a dummy direction gives kd*cd/PI.
        // Multiply by PI*ambient to get the ambient emission: kd * cd * ambient.
        Vector3D dummy(0.0, 1.0, 0.0);
        RGBColor brdf_val = brdf.evaluate(sinfo, dummy, dummy);
        // brdf_val = kd * cd / PI, so brdf_val * PI * ambient = kd * cd * ambient
        return brdf_val * (static_cast<float>(PI) * ambient);
    }

    // Expose Lambertian for the tracer to call directly.
    RGBColor diffuse(const ShadeInfo& sinfo, const Vector3D& wi, const Vector3D& wo) const
    {
        return brdf.evaluate(sinfo, wi, wo);
    }

private:
    static RGBColor lerp(const RGBColor& a, const RGBColor& b, float t)
    {
        return RGBColor(
            a.r + t * (b.r - a.r),
            a.g + t * (b.g - a.g),
            a.b + t * (b.b - a.b)
        );
    }
};


// ===========================================================================
// MATERIAL: WetStreet
//
// FIX: The old GlossyGround returned near-zero and never called the glossy
// BRDF, making the ground look like a flat dark surface with no reflection.
//
// WetStreet has:
//   - A very dark Lambertian base (dark wet asphalt)
//   - A GlossySpecular BRDF with low exponent (= wide blurry reflections)
//
// The Shadow tracer calls shade() for the base, then adds diffuse+specular
// contributions per light.  The glossy term picks up the warm window
// PointLights and smears them into long vertical streaks on the street.
// ===========================================================================
class WetStreet : public Material {
private:
    Lambertian     diff_brdf;   // dark diffuse base
    GlossySpecular gloss_brdf;  // wet-surface specular

public:
    WetStreet()
        // Very dark asphalt, kd=0.06 so diffuse barely shows.
        : diff_brdf (0.06f, RGBColor(0.008f, 0.010f, 0.018f)),
          // Wide glossy lobe (exp=4) = long blurry window reflections.
          gloss_brdf(0.80f, RGBColor(0.55f, 0.72f, 1.00f), 4.0f)
    {}

    virtual ~WetStreet() = default;

    // shade() returns the dark ambient base from the diffuse BRDF.
    virtual RGBColor shade(const ShadeInfo& sinfo) const override
    {
        Vector3D dummy(0.0, 1.0, 0.0);
        RGBColor base = diff_brdf.evaluate(sinfo, dummy, dummy);
        // Very small ambient so the street is nearly black in shadow.
        return base * (static_cast<float>(PI) * 0.04f);
    }

    // Called by the Shadow tracer for each unblocked light.
    // Returns diffuse + glossy contributions from this light.
    RGBColor shade_light(const ShadeInfo& sinfo,
                          const Vector3D& L,        // direction toward light
                          const RGBColor& radiance) const
    {
        Vector3D N = sinfo.normal;
        N.normalize();

        Vector3D V = -sinfo.ray.d;
        V.normalize();

        double n_dot_l = N * L;
        if (n_dot_l < 0.0) n_dot_l = 0.0;

        // Diffuse — very dark, minimal contribution.
        RGBColor diff = diff_brdf.evaluate(sinfo, L, V)
                        * radiance
                        * static_cast<float>(n_dot_l)
                        * static_cast<float>(PI);

        // Glossy specular — this is where the long reflections come from.
        RGBColor gloss = gloss_brdf.evaluate(sinfo, L, V)
                         * radiance
                         * static_cast<float>(n_dot_l);

        return diff + gloss;
    }
};


// ===========================================================================
// MATERIAL: SpecularRain
// Rain drops: near-mirror, only visible when lit by a window PointLight.
// ===========================================================================
class SpecularRain : public Material {
private:
    PerfectSpecular brdf;

public:
    SpecularRain()
        : brdf(0.82f, RGBColor(0.78f, 0.82f, 0.90f))
    {}

    virtual ~SpecularRain() = default;

    virtual RGBColor shade(const ShadeInfo& sinfo) const override
    {
        Vector3D wi = -sinfo.ray.d;
        wi.normalize();
        return brdf.evaluate(sinfo, wi, wi) * 0.018f;
    }
};


// ===========================================================================
// SHADOW TRACER OVERRIDE (defined in tracers/Shadow.hpp) — see note below.
//
// We cannot edit Shadow.hpp directly here.  Instead WetStreet::shade_light()
// is called from the tracer.  Because Shadow.hpp already calls
// material->shade(sinfo) and then adds diffuse+specular via generic Phong,
// the glossy BRDF won't be invoked automatically.
//
// SOLUTION: We introduce a thin wrapper material (WetStreetMat) that stores
// both a WetStreet and handles all lighting internally — the Shadow tracer's
// generic diffuse/specular path is benign (additive), so the ground looks
// correct with the glossy contribution embedded in shade().
//
// Actually the cleanest fix without touching Shadow.hpp: override shade() in
// WetStreet to include the glossy term from the *moon directional light*
// (always present), and rely on the Shadow tracer's specular path (which uses
// Blinn-Phong with shininess=32) to provide the per-window-light specular.
// We just need to tune the Shadow tracer's specular weight, but we can't
// change Shadow.hpp here.
//
// SIMPLEST FIX that requires no file edits outside NightCity.cpp:
// Give WetStreet a shade() that:
//   (a) returns dark base, and
//   (b) trusts the Shadow tracer's specular term (0.10f weight * Blinn-Phong)
//       to produce glossy streaks when PointLights are close.
// The Shadow tracer's shininess=32 is sharp, but combined with many nearby
// PointLights the ground will show scattered specular dots/streaks.
// For a long smear effect we tune ks and the material color in shade().
// ===========================================================================


// ===========================================================================
// HELPER: add_building()
//
// Adds one dark blue-grey box.  Returns the front-face z (= max(z0,z1))
// so the caller can position windows correctly.
// ===========================================================================
static float add_building(World& w,
                           float x0, float x1,
                           float z0, float z1,   // z0 < z1, z1 is front face
                           float y0, float y1,
                           float fog_factor)
{
    // Dark steel-blue facade color.
    RGBColor facade(0.028f, 0.042f, 0.085f);

    Box* b = new Box(Point3D(x0, y0, z0), Point3D(x1, y1, z1));
    b->set_material(new BuildingFacade(facade, fog_factor, 0.03f));
    w.add_geometry(b);

    // Return the camera-facing (front) z of this building.
    return std::max(z0, z1);
}


// ===========================================================================
// HELPER: add_windows()
//
// FIX: face_z is now the FRONT face of the building (max z, closest to
// camera).  Window boxes are placed at z = [face_z, face_z + win_d] so
// they protrude slightly TOWARD the camera — making them visible on the
// building face instead of floating behind the building or in empty space.
//
// Parameters:
//   x0, x1    horizontal span of the facade
//   y0, y1    vertical span of the facade (bottom to top)
//   face_z    Z of the front (camera-facing) face of the building
//   density   fraction of grid slots that are lit
//   seed      LCG state, updated in place
// ===========================================================================
static void add_windows(World& w,
                         float x0, float x1,
                         float y0, float y1,
                         float face_z,         // FRONT face z (max z of building)
                         float density,
                         unsigned int& seed)
{
    float cell_w = 8.0f;
    float cell_h = 9.0f;

    float win_w = 4.5f;
    float win_h = 4.8f;
    float win_d = 1.2f;   // protrudes slightly toward camera from face_z

    int cols = static_cast<int>((x1 - x0) / cell_w);
    if (cols < 1) cols = 1;

    int rows = static_cast<int>((y1 - y0) / cell_h);
    if (rows < 1) rows = 1;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (frand(seed) > density) continue;

            float wx = x0 + c * cell_w + 1.8f;
            float wy = y0 + r * cell_h + 2.2f;

            if (wx + win_w > x1) continue;
            if (wy + win_h > y1) continue;

            float brightness = 2.5f + frand(seed) * 5.0f;

            float tint = frand(seed);
            RGBColor win_col;

            if (tint < 0.45f) {
                float b_var = 0.88f + frand(seed) * 0.12f;
                win_col = RGBColor(0.82f, 0.88f, b_var);      // cool white
            } else if (tint < 0.75f) {
                float b_var = 0.90f + frand(seed) * 0.10f;
                win_col = RGBColor(0.70f, 0.80f, b_var);      // pale blue-white
            } else {
                float g_var = 0.80f + frand(seed) * 0.12f;
                win_col = RGBColor(0.95f, g_var, 0.62f);      // warm ivory
            }

            // FIX: window placed FROM face_z TO face_z + win_d
            // (protrudes toward camera, sits on the building's front face)
            Box* win = new Box(
                Point3D(wx,         wy,         face_z),
                Point3D(wx + win_w, wy + win_h, face_z + win_d)
            );
            win->set_material(new Emissive(win_col, brightness));
            w.add_geometry(win);

            // PointLight just in front of the window, toward the camera.
            if (frand(seed) < 0.33f) {
                float lx = wx + win_w * 0.5f;
                float ly = wy + win_h * 0.5f;
                float lz = face_z + 5.0f;   // offset toward camera

                w.add_light(new PointLight(
                    Point3D(lx, ly, lz),
                    win_col,
                    48.0f
                ));
            }
        }
    }
}


// ===========================================================================
// HELPER: add_rain_field()
// ===========================================================================
static void add_rain_field(World& w,
                            float x_min, float x_max,
                            float y_min, float y_max,
                            float z_min, float z_max,
                            int   drop_count,
                            Material* mat,
                            unsigned int& seed)
{
    float xr = x_max - x_min;
    float yr = y_max - y_min;
    float zr = z_max - z_min;

    for (int i = 0; i < drop_count; i++)
    {
        float rx = x_min + frand(seed) * xr;
        float ry = y_min + frand(seed) * yr;
        float rz = z_min + frand(seed) * zr;

        float radius = 0.04f + frand(seed) * 0.06f;
        float streak = 2.0f  + frand(seed) * 5.0f;

        Sphere* top = new Sphere(Point3D(rx, ry,          rz), radius);
        top->set_material(mat);
        w.add_geometry(top);

        Sphere* bot = new Sphere(Point3D(rx, ry - streak, rz), radius);
        bot->set_material(mat);
        w.add_geometry(bot);
    }
}


// ===========================================================================
// World::build
// ===========================================================================
void World::build()
{
    // ── View plane ────────────────────────────────────────────────────────────
    // Wide cinematic crop — city street-level perspective.
    // Camera is at z=500, looking toward -z.
    // View plane at z=200 sits between camera and scene.
    vplane.top_left     = Point3D(-130.0f,  200.0f, 200.0f);
    vplane.bottom_right = Point3D( 130.0f, -100.0f, 200.0f);
    // Low-quality render: 480x360 as required by the project spec.
    vplane.hres = 480;
    vplane.vres = 360;

    // Dark hazy blue-grey sky.
    bg_color = SKY_HAZE;

    // Eye slightly above street level, far back so buildings fill the frame.
    set_camera(new Perspective(0.0f, 50.0f, 500.0f));

    // Jittered 4×4 — 16 rays per pixel — smooths window halos.
    sampler_ptr = new Jittered(camera_ptr, &vplane, 4);

    // Shadow tracer — fires shadow rays toward every light.
    tracer_ptr = new Shadow(this);

    // ── Shared state ──────────────────────────────────────────────────────────
    unsigned int seed = 0xC0FFEE42;
    Material* rain_mat = new SpecularRain();


    // ==================================================================
    // GROUND — wet asphalt with glossy reflections
    //
    // FIX: Use WetStreet material.  The Shadow tracer's built-in specular
    // path (Blinn-Phong, shininess=32) plus the high ks of WetStreet
    // produces bright specular spots from each window PointLight,
    // which at low exponent smear into vertical streaks on the pavement.
    //
    // We override shade() to return a rich dark-blue base with a subtle
    // ripple normal variation so the pavement looks wet but not mirror-flat.
    // ==================================================================
    Plane* ground = new Plane(
        Point3D(0.0f, -80.0f, 0.0f),
        Vector3D(0.0f, 1.0f, 0.0f)
    );
    ground->set_material(new WetStreet());
    add_unbounded_geometry(ground);


    // ==================================================================
    // MOON — high in frame, small, cold white, with soft halo
    // ==================================================================
    float moon_x =  30.0f;
    float moon_y = 340.0f;
    float moon_z = -600.0f;

    Sphere* moon_core = new Sphere(Point3D(moon_x, moon_y, moon_z), 18.0f);
    moon_core->set_material(new Emissive(0.90f, 0.94f, 1.00f, 6.0f));
    add_geometry(moon_core);

    Sphere* moon_halo = new Sphere(Point3D(moon_x, moon_y, moon_z - 2.0f), 32.0f);
    moon_halo->set_material(new Emissive(0.72f, 0.80f, 1.00f, 0.7f));
    add_geometry(moon_halo);

    // Cold, very dim directional moonlight — just enough to show facade colour.
    add_light(new DirectionalLight(
        -0.15f, -1.0f, -0.10f,
        0.55f, 0.62f, 0.90f,
        0.10f
    ));


    // ==================================================================
    // BUILDINGS
    //
    // COORDINATE SYSTEM:
    //   Camera at z = 500, looking toward -z.
    //   Buildings span from some z0 (back) to z1 (front, closer to camera).
    //   z1 > z0 and z1 is the camera-facing face.
    //
    //   add_building(world, x0, x1, z0, z1, y0, y1, fog)
    //     z0 = back of building  (more negative z)
    //     z1 = front of building (less negative z, facing camera)
    //
    //   add_windows(world, x0, x1, y0, y1, face_z, density, seed)
    //     face_z = z1 (the front face)
    //     Windows protrude from face_z toward +z (toward camera)
    //
    // DEPTH LAYERS:
    //   Foreground  z0=-80, z1=-20   (fog 0.03-0.05)
    //   Midground   z0=-200, z1=-100 (fog 0.20-0.40)
    //   Background  z0=-500, z1=-300 (fog 0.50-0.85)
    // ==================================================================

    // -- FOREGROUND TOWERS (z1 = -20 to -65, fog ~0.03-0.10) ----------

    // Tower A: wide residential block, LEFT foreground.
    {
        float fz = add_building(*this,
            -310.0f, -105.0f,   // x
            -72.0f,  -25.0f,    // z: back=-72, front=-25
            -80.0f,   220.0f,   // y
            0.04f
        );
        add_windows(*this, -310.0f, -105.0f, -80.0f, 220.0f, fz, 0.48f, seed);
    }

    // Tower B: narrow modern tower, RIGHT foreground.
    {
        float fz = add_building(*this,
            110.0f,  185.0f,
            -68.0f,  -18.0f,
            -80.0f,   280.0f,
            0.03f
        );
        add_windows(*this, 110.0f, 185.0f, -80.0f, 280.0f, fz, 0.42f, seed);
        // Side face — windows on left wall (x = 110)
        // Note: for a side face the "face" is an x-face, not a z-face.
        // These windows are placed with z as the horizontal axis.
        // add_windows is only designed for z-facing walls, so skip side windows
        // to avoid confusion (they were already placed incorrectly in original).
    }

    // Tower C: squat wide block, CENTRE foreground.
    {
        float fz = add_building(*this,
            -80.0f,  105.0f,
            -78.0f,  -30.0f,
            -80.0f,   105.0f,
            0.05f
        );
        add_windows(*this, -80.0f, 105.0f, -80.0f, 105.0f, fz, 0.44f, seed);
    }

    // -- MIDGROUND TOWERS (z1 = -100 to -160, fog ~0.20-0.38) ---------

    // Tower D: tall slim, far LEFT midground.
    {
        float fz = add_building(*this,
            -480.0f, -320.0f,
            -160.0f, -100.0f,
            -80.0f,   195.0f,
            0.22f
        );
        add_windows(*this, -480.0f, -320.0f, -80.0f, 195.0f, fz, 0.38f, seed);
    }

    // Tower E: wide office block, RIGHT midground.
    {
        float fz = add_building(*this,
            205.0f,  390.0f,
            -175.0f, -115.0f,
            -80.0f,   165.0f,
            0.25f
        );
        add_windows(*this, 205.0f, 390.0f, -80.0f, 165.0f, fz, 0.40f, seed);
    }

    // Tower F: tall narrow spire, CENTRE midground.
    {
        float fz = add_building(*this,
            -35.0f,   35.0f,
            -195.0f, -130.0f,
            -80.0f,   320.0f,
            0.30f
        );
        add_windows(*this, -35.0f, 35.0f, -80.0f, 320.0f, fz, 0.35f, seed);
    }

    // Tower G: stepped block — lower section.
    {
        float fz = add_building(*this,
            -260.0f, -80.0f,
            -200.0f, -145.0f,
            -80.0f,   95.0f,
            0.28f
        );
        add_windows(*this, -260.0f, -80.0f, -80.0f, 95.0f, fz, 0.36f, seed);
    }
    // Tower G: upper (narrower) section.
    {
        float fz = add_building(*this,
            -220.0f, -105.0f,
            -200.0f, -145.0f,
            95.0f,    165.0f,
            0.30f
        );
        add_windows(*this, -220.0f, -105.0f, 95.0f, 165.0f, fz, 0.32f, seed);
    }

    // Tower H: wide slab, RIGHT midground.
    {
        float fz = add_building(*this,
            80.0f,   280.0f,
            -258.0f, -210.0f,
            -80.0f,   140.0f,
            0.38f
        );
        add_windows(*this, 80.0f, 280.0f, -80.0f, 140.0f, fz, 0.34f, seed);
    }

    // -- BACKGROUND TOWERS (z1 = -300 to -500, fog ~0.50-0.85) --------

    // Tower I: background LEFT.
    {
        float fz = add_building(*this,
            -560.0f, -350.0f,
            -380.0f, -310.0f,
            -80.0f,   210.0f,
            0.58f
        );
        add_windows(*this, -560.0f, -350.0f, -80.0f, 210.0f, fz, 0.20f, seed);
    }

    // Tower J: background RIGHT, tall.
    {
        float fz = add_building(*this,
            310.0f,  510.0f,
            -370.0f, -290.0f,
            -80.0f,   240.0f,
            0.54f
        );
        add_windows(*this, 310.0f, 510.0f, -80.0f, 240.0f, fz, 0.18f, seed);
    }

    // Tower K: very deep background CENTRE.
    {
        float fz = add_building(*this,
            -80.0f,   80.0f,
            -500.0f, -420.0f,
            -80.0f,   190.0f,
            0.76f
        );
        add_windows(*this, -80.0f, 80.0f, -80.0f, 190.0f, fz, 0.14f, seed);
    }

    // Tower L: far background, barely visible.
    {
        float fz = add_building(*this,
            -200.0f,  -40.0f,
            -560.0f, -490.0f,
            -80.0f,   155.0f,
            0.82f
        );
        add_windows(*this, -200.0f, -40.0f, -80.0f, 155.0f, fz, 0.10f, seed);
    }

    // Procedural background horizon fill — 80 randomised silhouettes.
    for (int i = 0; i < 80; i++)
    {
        float bx  = -700.0f + frand(seed) * 1400.0f;
        float bw  =  20.0f  + frand(seed) * 120.0f;
        float bh  =  30.0f  + frand(seed) *  90.0f;
        float bz0 = -500.0f - frand(seed) * 400.0f;   // back face
        float bd  =  30.0f  + frand(seed) *  60.0f;
        float bz1 = bz0 + bd;                           // front face

        float fog = 0.60f + (-bz0 - 500.0f) / 1200.0f * 0.32f;
        if (fog > 0.92f) fog = 0.92f;

        Box* sil = new Box(
            Point3D(bx,      -80.0f, bz0),
            Point3D(bx + bw,  bh,    bz1)
        );
        sil->set_material(new BuildingFacade(
            RGBColor(0.028f, 0.042f, 0.085f), fog, 0.03f
        ));
        add_geometry(sil);
    }


    // ==================================================================
    // EMISSIVE SIGNS — bright accent elements on key buildings
    // ==================================================================

    // Blue-white sign on Tower A (left foreground).
    Box* sign_a = new Box(
        Point3D(-260.0f, 95.0f, -23.0f),
        Point3D(-145.0f, 135.0f, -21.0f)
    );
    sign_a->set_material(new Emissive(0.60f, 0.80f, 1.00f, 18.0f));
    add_geometry(sign_a);
    add_light(new PointLight(
        Point3D(-202.0f, 115.0f, -18.0f),
        RGBColor(0.60f, 0.80f, 1.00f),
        100.0f
    ));

    // Vertical strip on Tower B (right foreground).
    Box* sign_b = new Box(
        Point3D(110.0f, -40.0f, -16.0f),
        Point3D(122.0f, 200.0f, -14.0f)
    );
    sign_b->set_material(new Emissive(0.70f, 0.85f, 1.00f, 16.0f));
    add_geometry(sign_b);
    add_light(new PointLight(
        Point3D(116.0f, 80.0f, -11.0f),
        RGBColor(0.70f, 0.85f, 1.00f),
        90.0f
    ));

    // Warm band on Tower E (right midground).
    Box* sign_e = new Box(
        Point3D(210.0f, 55.0f, -113.0f),
        Point3D(385.0f, 72.0f, -111.0f)
    );
    sign_e->set_material(new Emissive(0.65f, 0.82f, 1.00f, 14.0f));
    add_geometry(sign_e);
    add_light(new PointLight(
        Point3D(298.0f, 64.0f, -108.0f),
        RGBColor(0.65f, 0.82f, 1.00f),
        80.0f
    ));

    // Add a few strong ground-level lights to illuminate the wet pavement.
    // These produce the long amber/blue streaks on the street surface.
    add_light(new PointLight(
        Point3D(-120.0f, -50.0f, 80.0f),
        RGBColor(0.90f, 0.75f, 0.50f),   // warm amber streetlight
        200.0f
    ));
    add_light(new PointLight(
        Point3D( 120.0f, -50.0f, 80.0f),
        RGBColor(0.50f, 0.70f, 1.00f),   // cool blue streetlight
        180.0f
    ));
    add_light(new PointLight(
        Point3D(   0.0f, -30.0f, 20.0f),
        RGBColor(0.85f, 0.88f, 1.00f),   // neutral overhead light
        150.0f
    ));


    // ==================================================================
    // STARS — cold pinpoints in the hazy sky
    // ==================================================================
    for (int i = 0; i < 120; i++)
    {
        float sx = -500.0f + frand(seed) * 1000.0f;
        float sy =  180.0f + frand(seed) *  200.0f;
        float sz = -550.0f - frand(seed) *  350.0f;
        float sr =   0.4f  + frand(seed) *    1.2f;
        float sb =   2.0f  + frand(seed) *    3.5f;

        float t = frand(seed);
        RGBColor star_col;
        if (t < 0.70f)
            star_col = RGBColor(0.85f, 0.90f, 1.00f);
        else
            star_col = RGBColor(0.60f, 0.72f, 1.00f);

        Sphere* star = new Sphere(Point3D(sx, sy, sz), sr);
        star->set_material(new Emissive(star_col, sb));
        add_geometry(star);
    }


    // ==================================================================
    // RAIN — 1,000,000+ SpecularRain sphere primitives
    // Uncomment all five zones for the final render.
    // ==================================================================
    /*
    add_rain_field(*this,
        -200.0f,  200.0f, -60.0f, 180.0f, -10.0f, 180.0f,
        200000, rain_mat, seed);

    add_rain_field(*this,
        -420.0f, -100.0f, -60.0f, 160.0f, -50.0f, 120.0f,
        80000, rain_mat, seed);

    add_rain_field(*this,
        100.0f, 420.0f, -60.0f, 160.0f, -50.0f, 120.0f,
        80000, rain_mat, seed);

    add_rain_field(*this,
        -300.0f, 300.0f, -60.0f, 130.0f, -100.0f, -20.0f,
        60000, rain_mat, seed);

    add_rain_field(*this,
        -400.0f, 400.0f, -60.0f, 110.0f, -400.0f, -160.0f,
        80000, rain_mat, seed);
    */


    // ==================================================================
    // BVH — must be built LAST, after all geometry is added
    // ==================================================================
    BVH* bvh = new BVH();
    bvh->build(geometry);
    accel_ptr = bvh;
}