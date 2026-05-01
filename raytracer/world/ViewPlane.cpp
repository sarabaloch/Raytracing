#include "ViewPlane.hpp"

ViewPlane::ViewPlane() : top_left(Point3D(-320.0f, 240.0f, 100.0f)), bottom_right(Point3D(320.0f, -240.0f, 100.0f)), normal(Vector3D(0.0, 0.0, -1.0)), hres(1500),
                         vres(1200){}

int ViewPlane::get_hres() const
{
  return hres;
}

void ViewPlane::set_hres(int h)
{
  hres = h;
}

int ViewPlane::get_vres() const
{
  return vres;
}

void ViewPlane::set_vres(int v)
{
  vres = v;
}