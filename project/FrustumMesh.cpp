/******************************************************************************
 * File: FrustumMesh.cpp
 * Author: Taekyung Ho
 * Course: CS250
 * Assignment: Camera Frustum Programming Assignment
 * Description: Defines a mesh representing the viewing frustum of a
 * canonically oriented camera, including COP and near/far planes.
 *****************************************************************************/
#include "FrustumMesh.h"
#include <cmath>

namespace {
	const Point CenterOfProjection = Point(0.0f, 0.0f, 0.0f);
}

/**
 * Indices for frustum faces.
 * Triangles are defined for near, top, left, right, bottom, and far planes.
 */
const FrustumMesh::Face FrustumMesh::faces[12] =
{
	{3,2,4}, {2,1,4}, // near
	{7,3,4}, {7,4,8}, // top
	{7,2,3}, {7,6,2}, // left
	{1,8,4}, {1,5,8}, // right
	{2,6,1}, {6,5,1}, // bottom
	{8,5,7}, {5,6,7}  // far
};

/**
 * Indices for frustum edges.
 * Includes edges connecting to the COP for the wireframe view.
 */
const FrustumMesh::Edge FrustumMesh::edges[16] =
{
	{0,1}, {0,2}, {0,3}, {0,4}, // Edges from COP to near plane
	{1,2}, {2,3}, {3,4}, {4,1}, // Near plane boundary
	{5,6}, {6,7}, {7,8}, {8,5}, // Far plane boundary
	{1,5}, {2,6}, {3,7}, {4,8}  // Connectivity between planes
};

/**
 * Constructor: Computes vertex positions based on frustum parameters.
 */
FrustumMesh::FrustumMesh(float fov, float a, float n, float f)
{
	vertices[0] = CenterOfProjection;

	// Calculate half-width and half-height at near and far distances
	float fov_tan_value = tan(fov * 0.5f);
	float near_width_half = n * fov_tan_value;
	float near_height_half = near_width_half / a;
	float far_width_half = f * fov_tan_value;
	float far_height_half = far_width_half / a;

	// Define 8 vertices forming the near and far clipping planes
	vertices[1] = Point(near_width_half, -near_height_half, -n);
	vertices[2] = Point(-near_width_half, -near_height_half, -n);
	vertices[3] = Point(-near_width_half, near_height_half, -n);
	vertices[4] = Point(near_width_half, near_height_half, -n);
	vertices[5] = Point(far_width_half, -far_height_half, -f);
	vertices[6] = Point(-far_width_half, -far_height_half, -f);
	vertices[7] = Point(-far_width_half, far_height_half, -f);
	vertices[8] = Point(far_width_half, far_height_half, -f);

	// Determine the bounding box dimensions
	float max_x = vertices[0].x, min_x = vertices[0].x;
	float max_y = vertices[0].y, min_y = vertices[0].y;
	float max_z = vertices[0].z, min_z = vertices[0].z;

	for (const Point& v : vertices)
	{
		if (max_x < v.x) max_x = v.x; if (min_x > v.x) min_x = v.x;
		if (max_y < v.y) max_y = v.y; if (min_y > v.y) min_y = v.y;
		if (max_z < v.z) max_z = v.z; if (min_z > v.z) min_z = v.z;
	}
	dimensions = Vector(max_x - min_x, max_y - min_y, max_z - min_z);

	// Compute the center of the vertices
	float sum_x = 0.0f, sum_y = 0.0f, sum_z = 0.0f;
	for (const Point& v : vertices)
	{
		sum_x += v.x; sum_y += v.y; sum_z += v.z;
	}
	center = Point(sum_x / VertexCount(), sum_y / VertexCount(), sum_z / VertexCount());
}

int FrustumMesh::VertexCount(void) { return 9; }
Point FrustumMesh::GetVertex(int i) { return vertices[i]; }
Vector FrustumMesh::Dimensions(void) { return dimensions; }
Point FrustumMesh::Center(void) { return center; }
int FrustumMesh::FaceCount(void) { return 12; }
FrustumMesh::Face FrustumMesh::GetFace(int i) { return faces[i]; }
int FrustumMesh::EdgeCount(void) { return 16; }
FrustumMesh::Edge FrustumMesh::GetEdge(int i) { return edges[i]; }