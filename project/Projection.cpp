/******************************************************************************
 * File: Projection.cpp
 * Author: Taekyung Ho
 * Course: CS250
 * Assignment: Transform Projection Programming Assignment
 * Description: Implements camera-to-world, world-to-camera, and
 * camera-to-NDC transformation matrices.
 *****************************************************************************/
#include "Projection.h"

namespace {
    /**
 * @brief Casting function: Converts a Vector to a Point.
 */
    Point VectorToPoint(const Vector& v)
    {
        return Point(v.x, v.y, v.z);
    }

    /**
     * @brief Casting function: Converts a Point to a Vector.
     */
    Vector PointToVector(const Point& p)
    {
        return Vector(p.x, p.y, p.z);
    }
}

 /**
  * Maps camera space coordinates to world space coordinates.
  * Uses the camera's orthonormal basis (Right, Up, Back) and position (Eye).
  */
Affine CameraToWorld(const Camera& cam)
{
    return Affine(cam.Right(), cam.Up(), cam.Back(), cam.Eye());
}

/**
 * Maps world space coordinates to camera space coordinates.
 * This is the inverse of CameraToWorld, moving the camera to the origin
 * and aligning its axes with the canonical axes (X, Y, Z).
 */
Affine WorldToCamera(const Camera& cam)
{
    // R is the rotation part (transpose of the camera-to-world rotation)
    Affine rotate_t = Affine(cam.Right(), cam.Up(), cam.Back(), Point(0.f, 0.f, 0.f));
    float temp;

    // Transpose the 3x3 linear part
    temp = rotate_t[0].y; rotate_t[0].y = rotate_t[1].x; rotate_t[1].x = temp;
    temp = rotate_t[0].z; rotate_t[0].z = rotate_t[2].x; rotate_t[2].x = temp;
    temp = rotate_t[1].z; rotate_t[1].z = rotate_t[2].y; rotate_t[2].y = temp;

    // T is the translation part (moving eye to origin)
    Affine translate_neg = translate(-PointToVector(cam.Eye()));

    return rotate_t * translate_neg;
}

/**
 * Maps camera coordinates to Normalized Device Coordinates (NDC).
 * Transforms the view frustum into a standard cube [-1, 1]^3.
 */
Matrix CameraToNDC(const Camera& cam)
{
    Matrix proj_and_NDC;
    float near = cam.NearDistance();
    float far = cam.FarDistance();
    float width = cam.ViewportGeometry().x;
    float height = cam.ViewportGeometry().y;
    float distance = cam.ViewportGeometry().z;

    // Apply the perspective projection matrix formula (Pi)
    proj_and_NDC[0].x = 2.0f * distance / width;
    proj_and_NDC[1].y = 2.0f * distance / height;
    proj_and_NDC[2].z = (far + near) / (near - far);
    proj_and_NDC[2].w = (2.0f * far * near) / (near - far);
    proj_and_NDC[3].z = -1.0f;
    proj_and_NDC[3].w = 0.0f;

    return proj_and_NDC;
}