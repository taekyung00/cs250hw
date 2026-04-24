/******************************************************************************
 * File: HalfSpace.cpp
 * Author: taekyung.ho
 * Course: CS250
 * Assignment: Clipping Programming Assignment
 * Description: Implements constructors for HalfSpace and Polyhedron classes.
 * Defines clipping volumes for standard cubes and camera frustums.
 *****************************************************************************/

#include "HalfSpace.h"
#include <cmath>

/**
 * Creates a half-space with outwardly pointing normal 'm' and boundary containing point 'C'.
 */
HalfSpace::HalfSpace(const Vector& m, const Point& C) {
    x = m.x;
    y = m.y;
    z = m.z;
    w = -(m.x * C.x + m.y * C.y + m.z * C.z);
}

/**
 * Creates a half-space containing A, B, C on its boundary and P in its interior.
 */
HalfSpace::HalfSpace(const Point& A, const Point& B, const Point& C, const Point& P) {
    Vector v1 = B - A;
    Vector v2 = C - A;
    
    // Normal calculation via cross product
    Vector n;
    n.x = v1.y * v2.z - v1.z * v2.y;
    n.y = v1.z * v2.x - v1.x * v2.z;
    n.z = v1.x * v2.y - v1.y * v2.x;

    x = n.x;
    y = n.y;
    z = n.z;
    w = -(n.x * A.x + n.y * A.y + n.z * A.z);

    // Ensure the normal points outward by checking point P's orientation
    if ((x * P.x + y * P.y + z * P.z + w * P.w) > 0.0f) {
        x = -x;
        y = -y;
        z = -z;
        w = -w;
    }
}

/**
 * Constructs a polyhedron. If 'cube' is true, initializes a standard NDC cube.
 */
Polyhedron::Polyhedron(bool cube) {
    if (cube) {
        // Standard cube decomposition order to match expected test output
        half_spaces.push_back(HalfSpace( 1,  0,  0, -1)); // Right
        half_spaces.push_back(HalfSpace(-1,  0,  0, -1)); // Left
        half_spaces.push_back(HalfSpace( 0,  1,  0, -1)); // Top
        half_spaces.push_back(HalfSpace( 0, -1,  0, -1)); // Bottom
        half_spaces.push_back(HalfSpace( 0,  0,  1, -1)); // Front
        half_spaces.push_back(HalfSpace( 0,  0, -1, -1)); // Back
    }
}

/**
 * Constructs a camera frustum polyhedron based on FOV, aspect ratio, and near/far planes.
 */
Polyhedron::Polyhedron(float fov, float a, float N, float F) {
    float W = std::tan(fov / 2.0f);
    
    // Frustum plane order adjusted for correct vertex sequence in clipping
    half_spaces.push_back(HalfSpace( 1,  0, a * W, 0));   // Right
    half_spaces.push_back(HalfSpace(-1,  0, a * W, 0));   // Left
    half_spaces.push_back(HalfSpace( 0,  1, W, 0));       // Top
    half_spaces.push_back(HalfSpace( 0, -1, W, 0));       // Bottom
    half_spaces.push_back(HalfSpace( 0,  0,  1,  N));     // Near
    half_spaces.push_back(HalfSpace( 0,  0, -1, -F));     // Far
}

/**
 * Returns true if point P is inside the polyhedron (dot product <= 0 for all planes).
 */
bool contains(const Polyhedron& polyhedron, const Hcoord& P) {
    if (P.w <= 0.0f) return false;
    for (const auto& hs : polyhedron.half_spaces) {
        if (dot(hs, P) > 0.0f) return false;
    }
    return true;
}