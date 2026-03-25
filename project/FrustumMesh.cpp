#include "FrustumMesh.h"
namespace {
	const Point CenterOfProjection = Point(0.0f, 0.0f, 0.0f);
	const Vector Right = Vector(1.0f, 0.0f, 0.0f);
	const Vector Up = Vector(0.0f, 1.0f, 0.0f);
	const Vector Back = Vector(0.0f, 0.0f, 1.0f);
}

const FrustumMesh::Face FrustumMesh::faces[12] =
{
	{3,2,4}, {2,1,4}, //near
	{7,3,4}, {7,4,8}, //top
	{7,3,2}, {2,6,7}, //right
	{7,6,2}, {7,2,3}, //left
	{2,6,1}, {6,5,1}, //bottom
	{8,5,7}, {5,6,7}  //far
};
const FrustumMesh::Edge FrustumMesh::edges[16] =
{
	{0,1}, {0,2}, {0,3}, {0,4},
	//near
	{1,2}, // BR to BL
	{2,3}, // BL to TL
	{3,4}, // TL to TR
	{4,1}, // TR to BR
	//far
	{5,6}, // BR to BL
	{6,7}, // BL to TL
	{7,8}, // TL to TR
	{8,5}, // TR to BR
	{1,5}, // Near BR to Far BR
	{2,6}, // Near BL to Far BL
	{3,7}, // Near TL to Far TL
	{4,8}  // Near TR to Far TR
};


FrustumMesh::FrustumMesh(float fov, float a, float n, float f)
{
	vertices[0] = CenterOfProjection;
	float fov_tan_value = tan(fov * 0.5f);
	float near_width_half =  n * fov_tan_value;
	float near_height_half = near_width_half / a;
	float far_width_half = f * fov_tan_value;
	float far_height_half = far_width_half / a;
	
	vertices[1] = Point(near_width_half, -near_height_half, -n);
	vertices[2] = Point(-near_width_half, -near_height_half, -n);
	vertices[3] = Point(-near_width_half, near_height_half, -n);
	vertices[4] = Point(near_width_half, near_height_half, -n);
	vertices[5] = Point(far_width_half, -far_height_half, -f);
	vertices[6] = Point(-far_width_half, -far_height_half, -f);
	vertices[7] = Point(-far_width_half, far_height_half, -f);
	vertices[8] = Point(far_width_half, far_height_half, -f);

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
	dimensions = Vector(max_x - min_x, max_y - min_y, max_z - min_z);

	float sum_x = 0.0f;
	float sum_y = 0.0f;
	float sum_z = 0.0f;
	for (Point v : vertices)
	{
		sum_x += v.x;
		sum_y += v.y;
		sum_z += v.z;
	}
	center = Point(sum_x / VertexCount(), sum_y / VertexCount(), sum_z / VertexCount());
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
	
	return dimensions;
}

Point FrustumMesh::Center(void)
{
	
	return center;
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
