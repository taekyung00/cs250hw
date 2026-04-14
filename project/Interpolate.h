// Interpolate.h
// -- perspective-correct texture mapping and clipping
// cs250 4/17

#ifndef CS250_INTERPOLATE_H
#define CS250_INTERPOLATE_H


#include <vector>
#include "Affine.h"
#include "HalfSpace.h"
#include "Raster.h"
#include "Texture.h"  // from CS 200


class TexturedCoord : public Hcoord {
  public:
    float u, v;
    explicit TexturedCoord(const Hcoord& P=Hcoord(), float _u=0, float _v=0)
                  : Hcoord(P), u(_u), v(_v) { }
    static void SetTexture(Texture *tptr) { texture = tptr; }
    static void SetColorScale(float mu) { color_scale = mu; }
    friend void FillTriangle(Raster&, const TexturedCoord&,
                             const TexturedCoord&, const TexturedCoord&);
  private:
    static float color_scale;
    static Texture *texture;
};


class TextureClip {
  public:
    TextureClip(void) { }
    TextureClip(const Polyhedron& poly) : half_spaces(poly.half_spaces) { }
    bool operator()(std::vector<TexturedCoord>& vertices);
  private:
    std::vector<HalfSpace> half_spaces;
    std::vector<TexturedCoord> temp_vertices;
};


#endif

