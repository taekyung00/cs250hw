/******************************************************************************
 * File: Clip.cpp
 * Author: taekyung.ho
 * Course: CS250
 * Assignment: Clipping Programming Assignment
 * Description: Implements the Sutherland-Hodgman polygon clipping algorithm.
 * Optimizes computations by caching dot product results across iterations.
 *****************************************************************************/

#include "Clip.h"

 /**
  * Clips a polygon against the convex polyhedron.
  * Modifies the 'vertices' vector to hold the clipped result.
  */
bool Clip::operator()(std::vector<Hcoord>& vertices) {
    // Process the polygon through each half-space plane
    for (const auto& plane : half_spaces) {
        if (vertices.empty()) return false;

        temp_vertices.clear();
        size_t n = vertices.size();

        // Cache the dot product for the last vertex to avoid redundant computation
        float dA = dot(plane, vertices.back());

        for (size_t i = 0; i < n; ++i) {
            const Hcoord& A = (i == 0) ? vertices.back() : vertices[i - 1];
            const Hcoord& B = vertices[i];

            // Re-use dA from the previous iteration; compute only the current vertex dB
            float dB = dot(plane, B);

            // Using <= 0 to treat boundary as 'inside'
            bool A_in = (dA <= 0.0f);
            bool B_in = (dB <= 0.0f);

            // Case-based clipping logic
            if (A_in) {
                if (B_in) {
                    // Case 1: Both inside - output current point
                    temp_vertices.push_back(B);
                }
                else {
                    // Case 2: Exiting - output intersection
                    float s = dA / (dA - dB);
                    temp_vertices.push_back(A + s * (B - A));
                }
            }
            else if (B_in) {
                // Case 3: Entering - output intersection and then current point
                float s = dA / (dA - dB);
                temp_vertices.push_back(A + s * (B - A));
                temp_vertices.push_back(B);
            }
            // Case 4: Both outside - output nothing

            // Update dA for the next edge iteration
            dA = dB;
        }
        // Update vertices for the next clipping plane stage
        vertices = temp_vertices;
    }

    // A polygon is valid only if it has at least 3 vertices remaining
    return vertices.size() >= 3;
}