/******************************************************************************
 * File: HalfSpace.cpp
 * Author: Taekyung Ho
 * Course: CS250
 * Assignment: Clipping Programming Assignment
 * Description: Implements constructors for creating HalfSpaces from normal
 * vectors and points, and generates Polyhedron clipping volumes for
 * both standard cubes and camera viewing frustums.
 *****************************************************************************/

#include "HalfSpace.h"
#include "Affine.h"
#include <cmath>

 /**
  * Creates a half-space with an outwardly pointing surface normal 'm'
  * and whose boundary contains the point 'C'.
  * Plane Equation: dot(m, P - C) = 0 => m.x*P.x + m.y*P.y + m.z*P.z - dot(m, C) = 0
  */
HalfSpace::HalfSpace(const Vector& m, const Point& C) {
    x = m.x;
    y = m.y;
    z = m.z;
    w = -(m.x * C.x + m.y * C.y + m.z * C.z);
}

/**
 * Creates a half-space whose boundary contains points A, B, C and
 * whose interior contains the point P.
 */
HalfSpace::HalfSpace(const Point& A, const Point& B, const Point& C, const Point& P) {
    // 1. Calculate the normal vector using the cross product of (B-A) and (C-A)
    Vector v1(B.x - A.x, B.y - A.y, B.z - A.z);
    Vector v2(C.x - A.x, C.y - A.y, C.z - A.z);

    // Assuming cross() is available in Affine.h. Manual fallback is used for safety.
    Vector n;
    n.x = v1.y * v2.z - v1.z * v2.y;
    n.y = v1.z * v2.x - v1.x * v2.z;
    n.z = v1.x * v2.y - v1.y * v2.x;

    x = n.x;
    y = n.y;
    z = n.z;
    w = -(n.x * A.x + n.y * A.y + n.z * A.z);

    // 2. Ensure point P is on the "interior" side (dot product < 0)
    // If dot > 0, the normal is pointing the wrong way, so we flip the signs.
    if ((x * P.x + y * P.y + z * P.z + w * P.w) > 0.0f) {
        x = -x;
        y = -y;
        z = -z;
        w = -w;
    }
}

/**
 * Constructs a polyhedron. If 'cube' is true, it creates the standard
 * NDC cube boundaries [-1, 1].
 */
Polyhedron::Polyhedron(bool cube) {
    if (cube) {
        // Standard cube boundaries: x, y, z in [-1, 1].
        // In clip coordinates, interior means: P.x - P.w <= 0, etc.
        half_spaces.push_back(HalfSpace(1, 0, 0, -1)); // Right: x <= w
        half_spaces.push_back(HalfSpace(-1, 0, 0, -1)); // Left: -x <= w
        half_spaces.push_back(HalfSpace(0, 1, 0, -1)); // Top: y <= w
        half_spaces.push_back(HalfSpace(0, -1, 0, -1)); // Bottom: -y <= w
        half_spaces.push_back(HalfSpace(0, 0, 1, -1)); // Front: z <= w
        half_spaces.push_back(HalfSpace(0, 0, -1, -1)); // Back: -z <= w
    }
}

/**
 * Constructs a camera view frustum polyhedron in camera space.
 */
Polyhedron::Polyhedron(float fov, float a, float N, float F) {
    // W represents tan(fov / 2)
    float W = std::tan(fov / 2.0f);

    // In standard right-handed camera space, the camera looks down the -Z axis.
    half_spaces.push_back(HalfSpace(0, 0, 1, N));     // Near plane: z <= -N  =>  z + N <= 0
    half_spaces.push_back(HalfSpace(0, 0, -1, -F));     // Far plane:  z >= -F  => -z - F <= 0

    half_spaces.push_back(HalfSpace(1, 0, a * W, 0));   // Right plane: x <= -z * a * W
    half_spaces.push_back(HalfSpace(-1, 0, a * W, 0));   // Left plane: -x <= -z * a * W

    half_spaces.push_back(HalfSpace(0, 1, W, 0));       // Top plane: y <= -z * W
    half_spaces.push_back(HalfSpace(0, -1, W, 0));       // Bottom plane: -y <= -z * W
}

/**
 * Checks if a point P is inside the convex polyhedron.
 */
bool contains(const Polyhedron& polyhedron, const Hcoord& P) {
    // Condition 1: P.w must be positive
    if (P.w <= 0.0f) {
        return false;
    }

    // Condition 2: P must be interior to all half-spaces (dot product <= 0)
    for (size_t i = 0; i < polyhedron.half_spaces.size(); ++i) {
        if (dot(polyhedron.half_spaces[i], P) > 0.0f) {
            return false;
        }
    }
    return true;
}