// Clip.h
// -- clipping polygons in clip coordinates
// cs250 7/17

#ifndef CS250_CLIP_H
#define CS250_CLIP_H

#include <vector>
#include "Affine.h"
#include "HalfSpace.h"


class Clip {
  public:
    Clip(void) { }
    Clip(const Polyhedron& poly) : half_spaces(poly.half_spaces) { }
    bool operator()(std::vector<Hcoord>& vertices);
  private:
    std::vector<HalfSpace> half_spaces;
    std::vector<Hcoord> temp_vertices;
};


#endif

