// CubeMesh.h
// -- standard cube
// cs250 5/15

#ifndef CS250_CUBEMESH_H
#define CS250_CUBEMESH_H

#include "Mesh.h"

class CubeMesh : public Mesh {
  public:
    int VertexCount(void) override;
    Point GetVertex(int i) override;
    Vector Dimensions(void) override;
    Point Center(void) override;
    int FaceCount(void) override;
    Face GetFace(int i) override;
    int EdgeCount(void) override;
    Edge GetEdge(int i) override;
  private:
    static const Point vertices[8];
    static const Face faces[12];
    static const Edge edges[12];
};

#endif

