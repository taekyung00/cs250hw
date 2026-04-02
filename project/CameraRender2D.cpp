/******************************************************************************
 * File: CameraRender2D.cpp
 * Author: Taekyung Ho
 * Course: CS250
 * Assignment: Transform Projection Programming Assignment
 * Description: Renders 3D meshes from a camera's point of view with
 * backface culling, frustum culling, and diffuse shading.
 *****************************************************************************/
#include "CameraRender2D.h"
#include "Projection.h"

namespace
{
    /**
     * Backface culling check: returns true if the face normal points
     * toward the eye (dot(n, PE) > 0).
     */
    bool is_frontface(const Vector& v1, const Vector& v2)
    {
        return dot(v1, v2) > 0;
    }

    /**
     * Simple frustum culling: returns true if a vertex is in front of
     * the viewer (z < 0 in camera space).
     */
    bool is_front_of_camera(const Point& camera_vertices)
    {
        return camera_vertices.z < 0;
    }
}

CameraRender2D::CameraRender2D(Render& r) : render(r)
{
}

CameraRender2D::~CameraRender2D(void)
{
}

/**
 * Updates the current camera and recalculates necessary transforms.
 */
void CameraRender2D::SetCamera(const Camera& cam)
{
    cam_vertices.clear();
    world2camera = WorldToCamera(cam);
    camera2ndc = CameraToNDC(cam);

    // Store camera position for PE vector and Back vector for light direction
    cam_vertices.push_back(cam.Eye());
    cam_vertices.push_back(VectorToPoint(cam.Back()));
}

/**
 * Draws the mesh edges. Clips any edge that has a vertex behind the viewer.
 */
void CameraRender2D::DisplayEdges(Mesh& m, const Affine& ModelToWorld, const Vector& color)
{
    render.SetColor(color);
    int vertexCount = m.VertexCount();
    std::vector<Point> vertices;
    vertices.reserve(vertexCount);

    // Combined transform from Object -> World -> Camera -> NDC
    Matrix M = camera2ndc * world2camera * ModelToWorld;

    for (int i = 0; i < vertexCount; i++)
    {
        Hcoord result = M * m.GetVertex(i);

        // Explicit Perspective Divide to get NDC
        float w = result.w;
        vertices.push_back(Point(result.x / w, result.y / w, result.z / w));
    }

    int edgeCount = m.EdgeCount();
    for (int i = 0; i < edgeCount; i++)
    {
        int index1 = m.GetEdge(i).index1;
        int index2 = m.GetEdge(i).index2;

        // Clip edges if at least one vertex is behind the camera (z >= 0)
        const Point cam_v1 = world2camera * ModelToWorld * m.GetVertex(index1);
        const Point cam_v2 = world2camera * ModelToWorld * m.GetVertex(index2);

        if (is_front_of_camera(cam_v1) && is_front_of_camera(cam_v2))
        {
            render.DrawLine(vertices[index1], vertices[index2]);
        }
    }
}

/**
 * Draws visible mesh faces with diffuse shading. Clips faces behind the viewer.
 */
void CameraRender2D::DisplayFaces(Mesh& m, const Affine& ModelToWorld, const Vector& color)
{
    int vertexCount = m.VertexCount();
    std::vector<Point> vertices;
    vertices.reserve(vertexCount);

    Matrix M = camera2ndc * world2camera * ModelToWorld;
    for (int i = 0; i < vertexCount; i++)
    {
        Hcoord result = M * m.GetVertex(i);
        float w = result.w;
        vertices.push_back(Point(result.x / w, result.y / w, result.z / w));
    }

    int faceCount = m.FaceCount();
    // Light shines parallel to the direction the camera is looking (Back vector)
    const Vector line_shines = PointToVector(cam_vertices[1]);

    for (int i = 0; i < faceCount; i++)
    {
        int index1 = m.GetFace(i).index1;
        int index2 = m.GetFace(i).index2;
        int index3 = m.GetFace(i).index3;

        // World space positions for backface culling and shading
        const Point world_v[3] = {
            ModelToWorld * m.GetVertex(index1),
            ModelToWorld * m.GetVertex(index2),
            ModelToWorld * m.GetVertex(index3)
        };

        // Camera space positions for frustum culling
        const Point cam_v[3] = {
            world2camera * world_v[0],
            world2camera * world_v[1],
            world2camera * world_v[2]
        };

        Vector normal = cross(world_v[1] - world_v[0], world_v[2] - world_v[0]);
        Vector PE = PointToVector(cam_vertices[0]) - PointToVector(world_v[0]);

        // Draw only if it's a front-facing face and all vertices are in front of the camera
        if (is_frontface(normal, PE) &&
            is_front_of_camera(cam_v[0]) && is_front_of_camera(cam_v[1]) && is_front_of_camera(cam_v[2]))
        {
            // Calculate diffuse shading coefficient mu = |L.n| / (||L|| ||n||)
            float diffuse = std::abs(dot(line_shines, normal)) / (abs(line_shines) * abs(normal));

            render.SetColor(Vector(diffuse * color.x, diffuse * color.y, diffuse * color.z));
            render.FillTriangle(vertices[index1], vertices[index2], vertices[index3]);
        }
    }
}