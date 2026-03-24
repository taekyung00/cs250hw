// SnubDodecMesh.h
// -- mesh for a snub dodecahedron
// cs250 1/15

#ifndef CS250_SNUBDODEC_H
#define CS250_SNUBDODEC_H

#include "Mesh.h"


class SnubDodecMesh : public Mesh {
  public:
    int VertexCount(void) { return 60; }
    Point GetVertex(int i) { return vertices[i]; }
    Vector Dimensions(void) { return Vector(2,2,2); }
    Point Center(void) { return Point(0,0,0); }
    int FaceCount(void) { return 116; }
    Face GetFace(int i) { return faces[i]; }
    int EdgeCount(void) { return 150; }
    Edge GetEdge(int i) { return edges[i]; }
  private:
    static Point vertices[60];
    static Face faces[116];
    static Edge edges[150];
};


#endif

