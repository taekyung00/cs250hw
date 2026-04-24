#include "Clip.h"

bool Clip::operator()(std::vector<Hcoord>& vertices) {
    for (size_t p = 0; p < half_spaces.size(); ++p) {
        const HalfSpace& plane = half_spaces[p];
        if (vertices.empty()) return false;

        temp_vertices.clear();
        size_t num_vertices = vertices.size();

        for (size_t i = 0; i < num_vertices; ++i) {
            const Hcoord& A = (i == 0) ? vertices.back() : vertices[i - 1];
            const Hcoord& B = vertices[i];

            float dA = dot(plane, A);
            float dB = dot(plane, B);

            bool A_in = (dA <= 0.0f);
            bool B_in = (dB <= 0.0f);

            if (A_in && B_in) {
                temp_vertices.push_back(B);
            }
            else if (A_in && !B_in) {
                float s = dA / (dA - dB);
                Hcoord I;
                I.x = A.x + s * (B.x - A.x);
                I.y = A.y + s * (B.y - A.y);
                I.z = A.z + s * (B.z - A.z);
                I.w = A.w + s * (B.w - A.w);
                temp_vertices.push_back(I);
            }
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
        }
        vertices = temp_vertices;
    }
    return vertices.size() >= 3;
}