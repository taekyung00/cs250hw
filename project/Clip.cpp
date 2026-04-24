/******************************************************************************
 * File: Clip.cpp
 * Author: Taekyung Ho
 * Course: CS250
 * Assignment: Clipping Programming Assignment
 * Description: Implements the Sutherland-Hodgman polygon clipping algorithm
 * to clip polygons against a 3D convex polyhedron (viewing frustum).
 *****************************************************************************/

#include "Clip.h"

 /**
  * Clips a polygon (defined by a vector of Hcoord vertices) against the
  * internal polyhedron using the Sutherland-Hodgman algorithm.
  */
bool Clip::operator()(std::vector<Hcoord>& vertices) {
    // Iterate through each bounding plane of the viewing volume
    for (size_t p = 0; p < half_spaces.size(); ++p) {
        const HalfSpace& plane = half_spaces[p];

        // If the polygon is completely clipped away by previous planes, abort early
        if (vertices.empty()) {
            return false;
        }

        temp_vertices.clear();
        size_t num_vertices = vertices.size();

        // Process each edge of the polygon against the current plane
        for (size_t i = 0; i < num_vertices; ++i) {
            const Hcoord& A = (i == 0) ? vertices.back() : vertices[i - 1];
            const Hcoord& B = vertices[i];

            float dA = dot(plane, A);
            float dB = dot(plane, B);

            // According to HalfSpace specifications, dot <= 0 means the point is inside
            bool A_in = (dA <= 0.0f);
            bool B_in = (dB <= 0.0f);

            // Case 1: Both inside -> Add the end point B
            if (A_in && B_in) {
                temp_vertices.push_back(B);
            }
            // Case 2: Inside to Outside -> Compute and add intersection point
            else if (A_in && !B_in) {
                float s = dA / (dA - dB);
                Hcoord I;
                I.x = A.x + s * (B.x - A.x);
                I.y = A.y + s * (B.y - A.y);
                I.z = A.z + s * (B.z - A.z);
                I.w = A.w + s * (B.w - A.w);
                temp_vertices.push_back(I);
            }
            // Case 3: Outside to Inside -> Compute intersection, add it, then add end point B
            else if (!A_in && B_in) {
                float s = dA / (dA - dB);
                Hcoord I;
                I.x = A.x + s * (B.x - A.x);
                I.y = A.y + s * (B.y - A.y);
                I.z = A.z + s * (B.z - A.z);
                I.w = A.w + s * (B.w - A.w);
                temp_vertices.push_back(I);
                temp_vertices.push_back(B);
            }
            // Case 4: Both outside -> Do nothing
        }

        // Overwrite the vertices with the clipped result for the next plane
        vertices = temp_vertices;
    }

    // A valid polygon must have at least 3 vertices remaining
    return vertices.size() >= 3;
}