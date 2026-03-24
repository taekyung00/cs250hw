// SimpleRender.h
// -- first attempt at 3D rendering (fixed camera, no depth buffering)
// cs250 5/17

#ifndef CS250_SIMPLERENDER_H
#define CS250_SIMPLERENDER_H

#include <vector>
#include "Render.h"
#include "Affine.h"
#include "Mesh.h"


class SimpleRender {
  public:
    SimpleRender(Render &r);
    ~SimpleRender(void);
    void DisplayEdges(Mesh& m, const Affine& A, const Vector& color);
    void DisplayFaces(Mesh& m, const Affine& A, const Vector& color);
  private:
    Render &render;
    Matrix PersProj;
    //std::vector<Point> world_vertices;
    //std::vector<Point> proj_vertices;
};


#endif

