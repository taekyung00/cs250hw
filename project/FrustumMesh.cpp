#include "FrustumMesh.h"
namespace {
	const Point CenterOfProjection = Point(0.0f, 0.0f, 0.0f);
	const Vector Right = Vector(1.0f, 0.0f, 0.0f);
	const Vector Up = Vector(0.0f, 1.0f, 0.0f);
	const Vector Back = Vector(0.0f, 0.0f, 1.0f);
}

const FrustumMesh::Face FrustumMesh::faces[12] =
{
	{2,3,1}, {3,4,1}, //near
	{7,3,4}, {7,4,8}, //top
	{7,3,2}, {2,6,7}, //right
	{7,6,2}, {7,2,3}, //left
	{2,6,1}, {6,5,1}, //bottom
	{8,5,7}, {5,6,7}  //far
};
const FrustumMesh::Edge FrustumMesh::edges[16] =
{
	{0,1}, {0,2}, {0,3}, {0,4},
	{1,2}, {2,3}, {3,4}, {4,1}, //near
	{5,6}, {6,7}, {7,8}, {8,5}, //far
	{8,4}, {1,5}, {3,7}, {6,2}  //verticals
};


FrustumMesh::FrustumMesh(float fov, float a, float n, float f)
{
}

int FrustumMesh::VertexCount(void)
{
	return 9;
}

Point FrustumMesh::GetVertex(int i)
{
	return vertices[i];
}

Vector FrustumMesh::Dimensions(void)
{
	float max_x = vertices[0].x;
	float min_x = vertices[0].x;
	float max_y = vertices[0].y;
	float min_y = vertices[0].y;
	float max_z = vertices[0].z;
	float min_z = vertices[0].z;
	for (Point v : vertices)
	{
		if (max_x < v.x) max_x = v.x;
		if (min_x > v.x) min_x = v.x;
		if (max_y < v.y) max_y = v.y;
		if (min_y > v.y) min_y = v.y;
		if (max_z < v.z) max_z = v.z;
		if (min_z > v.z) min_z = v.z;
	}
	float delta_x = max_x - min_x;
	float delta_y = max_y - min_y;
	float delta_z = max_z - min_z;
	return Vector(delta_x, delta_y, delta_z);
}

Point FrustumMesh::Center(void)
{
	float sum_x = 0.0f;
	float sum_y = 0.0f;
	float sum_z = 0.0f;
	for (Point v : vertices)
	{
		sum_x += v.x;
		sum_y += v.y;
		sum_z += v.z;
	}
	return Point(sum_x / VertexCount(), sum_y / VertexCount(), sum_z / VertexCount());
}

int FrustumMesh::FaceCount(void)
{
	return 12;
}

FrustumMesh::Face FrustumMesh::GetFace(int i)
{
	return faces[i];
}

int FrustumMesh::EdgeCount(void)
{
	return 16;
}

FrustumMesh::Edge FrustumMesh::GetEdge(int i)
{
	return edges[i];
}
