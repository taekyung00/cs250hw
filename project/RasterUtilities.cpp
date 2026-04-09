#include "RasterUtilities.h"
#include "Affine.h"
#include <cmath>

/******************************************************************************
 * File: RasterUtilities.cpp
 * Author: Taekyung Ho
 * Course: CS250
 * Assignment: Z Buffer Programming Assignment
 * Description: Implements frame/z-buffer clearing and the Edge Equations
 * Triangle Rasterization Method with Z-buffering and planar
 * depth interpolation.
 *****************************************************************************/

namespace
{
	/**
	 * Helper operator to divide all components of an Hcoord by a scalar.
	 * Used for explicit perspective divide.
	 */
	template <typename DIVISOR>
	Hcoord operator/(const Hcoord& A, DIVISOR r) {
		Hcoord B(A);
		B.x /= r;
		B.y /= r;
		B.z /= r;
		B.w /= r;
		return B;
	}
	
	/**
	 * Represents the implicit line equation Ax + By + C = 0 for an edge.
	 */
	struct Edge {
		float A{ 0 };
		float B{ 0 };
		float C{ 0 };
		constexpr bool tl() const noexcept {
			return A > 0.f || (A == 0.f && B < 0.f);
		}
	};
	struct vec2 {
		float x{ 0.f };
		float y{ 0.f };
	};

	/**
	 * Evaluates the edge equation f(x,y) = Ax + By + C at a given 2D point.
	 */
	[[nodiscard]] constexpr float EquationValue(const Edge& edge, vec2 point) noexcept {
		return edge.A * point.x + edge.B * point.y + edge.C;
	}

	/**
	 * Checks if a point is inside the edge, applying the Top-Left rule
	 * to prevent drawing pixels shared between adjacent triangles.
	 */
	[[nodiscard]] constexpr bool PointInEdgeTopLeft(const Edge& edge, float Eval) noexcept {
		return Eval > 0.f || (Eval == 0.f && edge.tl());
	}

	/**
	 * Calculates the A, B, C coefficients for the edge from pos0 to pos1.
	 */
	[[nodiscard]] constexpr Edge calc_edge(vec2 pos0, vec2 pos1) noexcept {
		Edge edge;
		edge.A = pos0.y - pos1.y;
		edge.B = pos1.x - pos0.x;
		edge.C = pos1.y * pos0.x - pos1.x * pos0.y;
		return edge;
	}

	Vector PointToVector(const Point& p)
	{
		return Vector(p.x, p.y, p.z);
	}
	
	/**
	 * Determines if the triangle is back-facing using a 2D cross product.
	 */
	[[nodiscard]] constexpr bool back_facing(vec2 pos0, vec2 pos1, vec2 pos2) noexcept {
		return (pos1.x - pos0.x) * (pos2.y - pos0.y) - (pos2.x - pos0.x) * (pos1.y - pos0.y) < 0;
	}

	void swap_hcoord(Hcoord& a, Hcoord& b)
	{
		Hcoord temp = a;
		a = b;
		b = temp;
	}
	
	/**
	 * Reorders the vertices to guarantee a front-facing (counter-clockwise)
	 * winding order, ensuring consistent edge equation signs.
	 */
	void make_front_face(Hcoord(&P)[3])
	{		
		if (!back_facing({ P[0].x, P[0].y }, { P[2].x, P[2].y }, { P[1].x, P[1].y }))
		{
			swap_hcoord(P[1], P[2]);
			return;
		}
		if (!back_facing({ P[1].x, P[1].y }, { P[0].x, P[0].y }, { P[2].x, P[2].y }))
		{
			swap_hcoord(P[0], P[1]);
			return;
		}
		if (!back_facing({ P[1].x, P[1].y }, { P[2].x, P[2].y }, { P[0].x, P[0].y }))
		{
			swap_hcoord(P[0], P[2]);
			swap_hcoord(P[0], P[1]);
			return;
		}
		if (!back_facing({ P[2].x, P[2].y }, { P[0].x, P[0].y }, { P[1].x, P[1].y }))
		{
			swap_hcoord(P[0], P[1]);
			swap_hcoord(P[0], P[2]);
			return;
		}
		if (!back_facing({ P[2].x, P[2].y }, { P[1].x, P[1].y }, { P[0].x, P[0].y }))
		{
			swap_hcoord(P[0], P[2]);
			return;
		}
	}
}

/**
 * Clears the frame buffer and resets the Z-buffer to the specified depth.
 */
void ClearBuffers(Raster& r, float z)
{
	int width = r.Width(),
		height = r.Height();
	for (int i = 0; i < height; ++i) {
		int j = 0;
		r.GotoPoint(j, i);
		for (; j < width; ++j)
		{
			r.WriteZ(z);
			r.WritePixel();
			r.IncrementX();
		}
	}
}
/**
 * Rasterizes a 3D triangle using the Edge Equations method, applying
 * Z-buffering and planar depth interpolation for hidden surface removal.
 */
void FillTriangle(Raster& r, const Hcoord& P, const Hcoord& Q, const Hcoord& R)
{
	// divide by w 
	Hcoord P_[3] = {
		P / P.w,
		Q / Q.w,
		R / R.w };
	if (back_facing({ P_[0].x, P_[0].y }, { P_[1].x, P_[1].y }, { P_[2].x, P_[2].y }))
		make_front_face(P_);
	if (std::abs(P_[2].y - P_[0].y) < 1e-5f) return;
	//NOTE: The edge equations have to be calculated before sorting the vertices,because  but since they only depend on the vertex positions, the order of the vertices does not affect the edge equations.
	Edge E0 = calc_edge(vec2{ P_[0].x,P_[0].y }, vec2{ P_[1].x,P_[1].y });
	Edge E1 = calc_edge(vec2{ P_[1].x,P_[1].y }, vec2{ P_[2].x,P_[2].y });
	Edge E2 = calc_edge(vec2{ P_[2].x,P_[2].y }, vec2{ P_[0].x,P_[0].y });

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
	//clamping
	int int_xmin = static_cast<int>(std::ceil(xmin)) ;
	if (int_xmin < 0) int_xmin = 0;
	int int_xmax = static_cast<int>(std::floor(xmax));
	if (int_xmax >= r.Width()) int_xmax = r.Width() - 1;
	int int_ymin = static_cast<int>(std::ceil(ymin));
	if (int_ymin < 0) int_ymin = 0;
	int int_ymax = static_cast<int>(std::floor(ymax));
	if (int_ymax >= r.Height()) int_ymax = r.Height() - 1;

	float Eval0 = EquationValue(E0, { static_cast<float>(int_xmin) , static_cast<float>(int_ymin) });
	float Eval1 = EquationValue(E1, { static_cast<float>(int_xmin) , static_cast<float>(int_ymin) });
	float Eval2 = EquationValue(E2, { static_cast<float>(int_xmin) , static_cast<float>(int_ymin) });


	Vector Normal = cross(Vector(P_[1] - P_[0]), Vector(P_[2] - P_[0]));
	if (std::abs(Normal.z) < 1e-5f) return;
	float d = dot(Normal, PointToVector(P_[0]));

	float dz_dx = -Normal.x / Normal.z;
	
	for (int y = int_ymin; y <= int_ymax; ++y)
	{
		// start values for horizontal spans
		float hEval0 = Eval0;
		float hEval1 = Eval1;
		float hEval2 = Eval2;
		float hZ = (d - Normal.x * int_xmin - Normal.y * y) / Normal.z;
		r.GotoPoint(int_xmin, y);
		for (int x = int_xmin; x <= int_xmax; ++x) {
				if (PointInEdgeTopLeft(E0, hEval0) && PointInEdgeTopLeft(E1, hEval1) && PointInEdgeTopLeft(E2, hEval2)) 
				{
					if (hZ < r.GetZ())
					{
						r.WritePixel();
						r.WriteZ(hZ);
					}
				}
				// incrementally update hEval0, hEval1, hEval2
				hEval0 += E0.A;
				hEval1 += E1.A;
				hEval2 += E2.A;
				hZ += dz_dx;
				r.IncrementX();
		}
		// incrementally update Eval0, Eval1, Eval2
		Eval0 += E0.B;
		Eval1 += E1.B;
		Eval2 += E2.B;
	}
}

/*
* edge walking algorithm
* void FillTriangle(Raster& r, const Hcoord& P, const Hcoord& Q, const Hcoord& R)
{
	// divide by w 
	Hcoord P_[3] = {
		P / P.w,
		Q / Q.w,
		R / R.w };
	//NOTE: The edge equations have to be calculated before sorting the vertices,because  but since they only depend on the vertex positions, the order of the vertices does not affect the edge equations.
	Edge E0 = calc_edge(vec2{ P_[0].x,P_[0].y }, vec2{ P_[1].x,P_[1].y });
	Edge E1 = calc_edge(vec2{ P_[1].x,P_[1].y }, vec2{ P_[2].x,P_[2].y });
	Edge E2 = calc_edge(vec2{ P_[2].x,P_[2].y }, vec2{ P_[0].x,P_[0].y });

	// Sort vertices by y-coordinate ascending (P0.y <= P1.y <= P2.y)
	Hcoord temp;
	if (P_[0].y > P_[1].y)
	{
		temp = P_[0];
		P_[0] = P_[1];
		P_[1] = temp;
	}
	if (P_[0].y > P_[2].y)
	{
		temp = P_[0];
		P_[0] = P_[2];
		P_[2] = temp;
	}
	if (P_[1].y > P_[2].y)
	{
		temp = P_[1];
		P_[1] = P_[2];
		P_[2] = temp;
	}

	

	// Assuming vertices are sorted: p0.y <= p1.y <= p2.y
	// Compute inverse slopes for the edges starting from p0
	float invSlope01 = (P_[1].x - P_[0].x) / (P_[1].y - P_[0].y); // Edge P0-P1
	float invSlope02 = (P_[2].x - P_[0].x) / (P_[2].y - P_[0].y); // Edge P0-P2

	// Determine configuration: 
	// If invSlope01 < invSlope02, p1 is on the left.
	// If invSlope01 > invSlope02, p1 is on the right.
	bool isMiddleVertexLeft = (invSlope01 < invSlope02);

	Vector Normal = cross(Vector(P_[1] - P_[0]), Vector(P_[2] - P_[0]));
	float d = dot(Normal, PointToVector(P_[0]));

	//LOWER PART
	int j_min = static_cast<int>(std::ceil(P_[0].y));
	int j_max = static_cast<int>(std::floor(P_[1].y));

	for (int j = j_min; j <= j_max; ++j)
	{
		// Calculate x coordinates on both edges at current scanline y = j
		float x_edge1 = P_[0].x + invSlope01 * (j - P_[0].y);
		float x_edge2 = P_[0].x + invSlope02 * (j - P_[0].y);

		float x_min, x_max;

		if (isMiddleVertexLeft) {
			// p1 is on the left: P0-P1 is left edge, P0-P2 is right edge
			x_min = x_edge1;
			x_max = x_edge2;
		}
		else {
			// p1 is on the right: P0-P2 is left edge, P0-P1 is right edge
			x_min = x_edge2;
			x_max = x_edge1;
		}

		int i_min = static_cast<int>(std::ceil(x_min));
		int i_max = static_cast<int>(std::floor(x_max));
		// Move to the starting pixel of the span
		r.GotoPoint(i_min, j);
		for (int i = i_min; i <= i_max; ++i)
		{
			vec2 point{ static_cast<float>(i) + 0.5f, static_cast<float>(j) + 0.5f };
			[[maybe_unused]]float Eval0 = EquationValue(E0, point);
			[[maybe_unused]] float Eval1 = EquationValue(E1, point);
			[[maybe_unused]] float Eval2 = EquationValue(E2, point);
			//if (PointInEdgeTopLeft(E0, Eval0) && PointInEdgeTopLeft(E1, Eval1) && PointInEdgeTopLeft(E2, Eval2))
			//{
				float z = (d - Normal.x * i - Normal.y * j) / Normal.z;
				if (z < r.GetZ())
				{
					r.WritePixel();
					r.WriteZ(z);
				}
			//}
			r.IncrementX();
		}
		r.IncrementY();
	}

	//UPPER PART
	float invSlope12 = (P_[2].x - P_[1].x) / (P_[2].y - P_[1].y); // Edge P1-P2
	j_min = static_cast<int>(std::ceil(P_[1].y));
	j_max = static_cast<int>(std::floor(P_[2].y));

	for (int j = j_min; j <= j_max; ++j)
	{
		float x_min, x_max;

		if (isMiddleVertexLeft) {
			// Calculate x coordinates on both edges at current scanline y = j
			// p1 is on the left: P1-P2 is left edge, P0-P2 is right edge
			x_min = P_[1].x + invSlope12 * (j - P_[1].y);
			x_max = P_[2].x - invSlope02 * (P_[2].y - j);
		}
		else {
			// Calculate x coordinates on both edges at current scanline y = j
			// p1 is on the right: P0-P2 is left edge, P1-P2 is right edge
			x_min = P_[2].x - invSlope02 * (P_[2].y - j);
			x_max = P_[1].x + invSlope12 * (j - P_[1].y);
		}

		int i_min = static_cast<int>(std::ceil(x_min));
		int i_max = static_cast<int>(std::floor(x_max));

		// Move to the starting pixel of the span
		r.GotoPoint(i_min, j);
		for (int i = i_min; i <= i_max; ++i)
		{
			vec2 point{ static_cast<float>(i) + 0.5f, static_cast<float>(j) + 0.5f };
			[[maybe_unused]] float Eval0 = EquationValue(E0, point);
			[[maybe_unused]] float Eval1 = EquationValue(E1, point);
			[[maybe_unused]] float Eval2 = EquationValue(E2, point);
			//if (PointInEdgeTopLeft(E0, Eval0) && PointInEdgeTopLeft(E1, Eval1) && PointInEdgeTopLeft(E2, Eval2))
			//{
				float z = (d - Normal.x * i - Normal.y * j) / Normal.z;
				if (z < r.GetZ())
				{
					r.WritePixel();
					r.WriteZ(z);
				}
			//}
			r.IncrementX();
		}
		r.IncrementY();
	}
}
*/