// CameraRender2D.h
// -- display edges/faces of a mesh using a camera
// cs250 9/16

#ifndef CS250_CAMERARENDER2D_H
#define CS250_CAMERARENDER2D_H

#include <vector>
#include "Render.h"
#include "Camera.h"
#include "Mesh.h"


class CameraRender2D {
  public:
    CameraRender2D(Render &r);
    ~CameraRender2D(void);
    void SetCamera(const Camera &cam);
    void DisplayEdges(Mesh& m, const Affine& A, const Vector& color);
    void DisplayFaces(Mesh& m, const Affine& A, const Vector& color);
  private:
    Render &render;
    Affine world2camera;
    Matrix camera2ndc;
    std::vector<Point> cam_vertices;
    std::vector<Point> ndc_vertices;
};

#endif

