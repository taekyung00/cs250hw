/******************************************************************************
 * File: Interpolate.cpp
 * Author: [본인 이름 입력]
 * Course: CS250
 * Assignment: Texture Mapping and Clipping
 * Description: Implements perspective-correct texture mapping using screen-space
 * barycentric coordinates, and 3D polygon clipping against a
 * viewing frustum using the Sutherland-Hodgman algorithm.
 *****************************************************************************/

#include "Interpolate.h"

namespace
{
	/**
	 * Helper operator to divide all components of a TexturedCoord by a scalar.
	 * Used to convert original clip-space coordinates into screen-space (NDC) coordinates.
	 */
	template <typename DIVISOR>
	TexturedCoord operator/(const TexturedCoord& A, DIVISOR r) {
		TexturedCoord B(A);
		B.x /= r;
		B.y /= r;
		B.z /= r;
		B.w /= r;
		return B;
	}

	struct barycentric
	{
		float lambda = 0.f, mu = 0.f, nu = 0.f;
	};

	/**
	 * Computes the screen-space barycentric coordinates for a given point.
	 * Used to determine if a pixel is inside the triangle and to compute
	 * linear interpolation weights across the screen.
	 */
	class Barycentric
	{
	public:
		Barycentric(Hcoord v1_, Hcoord v2_, Hcoord v3_) : v1(v1_), v2(v2_), v3(v3_) {}
		Barycentric() = delete;

		barycentric MakeBarycentric(Hcoord i) const {
			barycentric bary;
			Hcoord v1v2 = v2 - v1;
			Hcoord v1v3 = v3 - v1;
			Hcoord v1i = i - v1;

			// Determinant (twice the signed area of the triangle)
			float det = v1v2.x * v1v3.y - v1v2.y * v1v3.x;

			// Prevent division by zero for degenerate (zero-area) triangles
			if (std::abs(det) < 1e-5f) {
				bary.lambda = -1.f; // Forces the inside-triangle test to fail
				return bary;
			}

			float v1i_x_by_det = v1i.x / det;
			float v1i_y_by_det = v1i.y / det;

			// Calculate weights using Cramer's rule equivalents
			bary.mu = v1i_x_by_det * v1v3.y - v1i_y_by_det * v1v3.x;
			bary.nu = v1v2.x * v1i_y_by_det - v1v2.y * v1i_x_by_det;
			bary.lambda = 1 - bary.mu - bary.nu;
			return bary;
		}
	private:
		Hcoord v1, v2, v3;
	};

	// Operator overloads for TexturedCoord to easily compute intersection points during clipping
	TexturedCoord operator +(const TexturedCoord& v1, const TexturedCoord& v2) {
		return TexturedCoord(Hcoord(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w), v1.u + v2.u, v1.v + v2.v);
	}

	TexturedCoord operator -(const TexturedCoord& v1, const TexturedCoord& v2) {
		return TexturedCoord(Hcoord(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w), v1.u - v2.u, v1.v - v2.v);
	}

	TexturedCoord operator *(const float scalar, const TexturedCoord& v) {
		return TexturedCoord(scalar * Hcoord(v), v.u * scalar, v.v * scalar);
	}
}

Texture* TexturedCoord::texture = nullptr;
float TexturedCoord::color_scale = 1.f;

/**
 * Rasterizes a textured triangle with perspective-correct texture mapping.
 */
void FillTriangle(Raster& raster, const TexturedCoord& v0, const TexturedCoord& v1, const TexturedCoord& v2)
{
	// NOTE: we have to use coord divided by w for screen space coordinate, 
	// and we also use original coord for perspective interpolation

	// 1. Perspective divide to get screen space coordinates (P_)
	TexturedCoord P_[3] = {
		v0 / v0.w,
		v1 / v1.w,
		v2 / v2.w };

	// 2. Compute 2D Bounding Box in screen space
	float xmin = P_[0].x;
	float xmax = P_[0].x;
	if (xmin > P_[1].x) xmin = P_[1].x;
	if (xmax < P_[1].x) xmax = P_[1].x;
	if (xmin > P_[2].x) xmin = P_[2].x;
	if (xmax < P_[2].x) xmax = P_[2].x;

	float ymin = P_[0].y;
	float ymax = P_[0].y;
	if (ymin > P_[1].y) ymin = P_[1].y;
	if (ymax < P_[1].y) ymax = P_[1].y;
	if (ymin > P_[2].y) ymin = P_[2].y;
	if (ymax < P_[2].y) ymax = P_[2].y;

	// 3. Clamp bounding box to screen dimensions to prevent out-of-bounds rendering
	int int_xmin = static_cast<int>(std::ceil(xmin));
	if (int_xmin < 0) int_xmin = 0;
	int int_xmax = static_cast<int>(std::floor(xmax));
	if (int_xmax >= raster.Width()) int_xmax = raster.Width() - 1;
	int int_ymin = static_cast<int>(std::ceil(ymin));
	if (int_ymin < 0) int_ymin = 0;
	int int_ymax = static_cast<int>(std::floor(ymax));
	if (int_ymax >= raster.Height()) int_ymax = raster.Height() - 1;

	// Setup barycentric calculator using screen space coordinates
	Barycentric linear_bary(P_[0], P_[1], P_[2]);

	float texture_for_scale = TexturedCoord::color_scale;

	// 4. Rasterization loop
	for (int y = int_ymin; y <= int_ymax; ++y)
	{
		raster.GotoPoint(int_xmin, y);
		for (int x = int_xmin; x <= int_xmax; ++x) {

			barycentric bary_for_z = linear_bary.MakeBarycentric(Hcoord(static_cast<float>(x), static_cast<float>(y), 0, 0));

			// Check if pixel is inside the triangle
			if (bary_for_z.lambda >= 0 && bary_for_z.mu >= 0 && bary_for_z.nu >= 0)
			{
				// Screen-space linear interpolation for Depth (Z)
				float z = P_[0].z * bary_for_z.lambda + P_[1].z * bary_for_z.mu + P_[2].z * bary_for_z.nu;

				// Z-Buffer Depth Test
				if (z < raster.GetZ())
				{
					// --- Perspective-Correct Texture Interpolation ---
					// Calculate 1/W interpolation
					float inv_w = (bary_for_z.lambda / v0.w) + (bary_for_z.mu / v1.w) + (bary_for_z.nu / v2.w);

					// Calculate U/W and V/W interpolations using original vertices' attributes
					float u_over_w = (bary_for_z.lambda * v0.u / v0.w) +
						(bary_for_z.mu * v1.u / v1.w) +
						(bary_for_z.nu * v2.u / v2.w);

					float v_over_w = (bary_for_z.lambda * v0.v / v0.w) +
						(bary_for_z.mu * v1.v / v1.w) +
						(bary_for_z.nu * v2.v / v2.w);

					// Recover actual U, V coordinates
					float final_u = u_over_w / inv_w;
					float final_v = v_over_w / inv_w;

					// Sample texture color
					Vector tex_color = TexturedCoord::texture->uvToRGB(final_u, final_v);

					// Apply shading scale
					tex_color.x *= texture_for_scale;
					tex_color.y *= texture_for_scale;
					tex_color.z *= texture_for_scale;

					// Write to buffers
					raster.SetColor(static_cast<Raster::byte>(tex_color.x),
						static_cast<Raster::byte>(tex_color.y),
						static_cast<Raster::byte>(tex_color.z));
					raster.WritePixel();
					raster.WriteZ(z);
				}
			}
			raster.IncrementX();
		}
	}
}

/**
 * Sutherland-Hodgman Polygon Clipping Algorithm.
 * Clips a textured polygon against all planes in the view frustum.
 */
 //NOTE: use temp!! we have to judge whether the original vertices are in the half space or not based off of "ORIGINAL VERTICES"!!
bool TextureClip::operator()(std::vector<TexturedCoord>& vertices)
{
	// Iteratively clip the polygon against each half-space (plane)
	for (const auto& plane : half_spaces)
	{
		// If polygon is completely outside previous planes, abort early
		if (vertices.empty()) return false;

		temp_vertices.clear();

		size_t num_vertices = vertices.size();
		for (size_t i = 0; i < num_vertices; ++i)
		{
			// Current edge from A to B
			const TexturedCoord& A = vertices[i];
			const TexturedCoord& B = vertices[(i + 1) % num_vertices]; //link last vertex to first vertex

			// Evaluate dot product to determine which side of the plane the vertices lie
			float dA = dot(plane, A);
			float dB = dot(plane, B);

			// According to this implementation's convention, dot < 0 is considered INSIDE the viewing volume
			bool A_in = (dA < 0.f);
			bool B_in = (dB < 0.f);

			// Case 1: Both inside -> Add end point B
			if (A_in && B_in)
			{
				temp_vertices.push_back(B);
			}
			// Case 2: Inside to Outside -> Calculate intersection and add it
			else if (A_in && !B_in)
			{
				float s = dA / (dA - dB);
				temp_vertices.push_back(A + s * (B - A));
			}
			// Case 3: Outside to Inside -> Calculate intersection, add it, then add end point B
			else if (!A_in && B_in)
			{
				float s = dA / (dA - dB);
				temp_vertices.push_back(A + s * (B - A));
				temp_vertices.push_back(B);
			}
			// Case 4: Both outside -> Add nothing
		}
		// Polygon is now the result of this plane's clipping, ready for the next plane
		vertices = temp_vertices;
	}

	// Return true if the resulting polygon has at least 3 vertices (valid for rendering)
	return vertices.size() >= 3;
}