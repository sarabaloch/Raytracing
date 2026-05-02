#pragma once

/**
   This file declares the Camera class which is an abstract class for concrete
   cameras to inherit from. A camera views the world through a view plane.

   Courtesy Kevin Suffern.
*/

class Point3D;
class Vector3D;

class Camera {
public:
  // Constructors.
  Camera() = default; // does nothing.

  // Copy constuctor and assignment operator.
  Camera(const Camera &camera) = default;
  Camera &operator=(const Camera &other) = default;

  // Desctructor.
  virtual ~Camera() = default;

  // Get direction of projection for a point.
  virtual Vector3D get_direction(const Point3D &p) const = 0;

  // Get the ray origin for a given sample point on the view plane.
  // Perspective: always returns the eye position (ignores p).
  // Parallel: returns the sample point itself (ray starts on view plane).
  virtual Point3D get_origin(const Point3D &sample_point) const = 0;
};