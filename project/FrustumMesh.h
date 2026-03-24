// FrustumMesh.h
// -- 3D mesh for a view frustum
// cs250 5/15

#ifndef CS250_FRUSTUMMESH_H
#define CS250_FRUSTUMMESH_H

#include "Mesh.h"


class FrustumMesh : public Mesh {
  public:
    FrustumMesh(float fov, float a, float n, float f);
    int VertexCount(void);
    Point GetVertex(int i);
    Vector Dimensions(void);
    Point Center(void);
    int FaceCount(void);
    Face GetFace(int i);
    int EdgeCount(void);
    Edge GetEdge(int i);
  private:
    Point vertices[9];
    Point center;
    Vector dimensions;
    static const Edge edges[16];
    static const Face faces[12];
};


#endif

