// HalfSpace.h
// -- half-spaces and perspective interpolation
// cs250 3/17

#ifndef CS250_HALFSPACE_H
#define CS250_HALFSPACE_H

#include <vector>
#include "Affine.h"


struct HalfSpace : Hcoord {
  HalfSpace(float x=0, float y=0, float z=0, float w=0) : Hcoord(x,y,z,w) {}
  HalfSpace(const Vector& m, const Point& C);
  HalfSpace(const Point& A, const Point& B, const Point& C, const Point& P);
};


inline
float dot(const HalfSpace& h, const Hcoord& P)
  { return h.x*P.x+h.y*P.y+h.z*P.z+h.w*P.w; }



struct Polyhedron {
  std::vector<HalfSpace> half_spaces;
  Polyhedron(bool cube=false);
  Polyhedron(float fov, float a, float N, float F);
};


bool contains(const Polyhedron& polyhedron, const Hcoord& P);


#endif

