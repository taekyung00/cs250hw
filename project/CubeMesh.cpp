#include "CubeMesh.h"

const Point CubeMesh::vertices[8] = 
{
	Point(-1,-1,-1), //0
	Point(1,-1,-1),  //1
	Point(1,-1,1),   //2
	Point(-1,-1,1),  //3
	Point(-1,1,-1),  //4
	Point(1,1,-1),   //5
	Point(1,1,1),    //6
	Point(-1,1,1)    //7
};
const CubeMesh::Face CubeMesh::faces[12]= 
{
	{0,1,2}, {0,2,3}, //bottom
	{4,7,6}, {6,5,4}, //top
	{7,3,2}, {2,6,7}, //front
	{6,2,1}, {1,5,6}, //right
	{5,1,0}, {0,4,5}, //back
	{4,0,3}, {3,7,4}  //left
};
const CubeMesh::Edge CubeMesh::edges[12]=
{
	{0,1}, {1,2}, {2,3}, {3,0}, //bottom
	{4,5}, {5,6}, {6,7}, {7,4}, //top
	{0,4}, {1,5}, {2,6}, {3,7}  //verticals
};


int CubeMesh::VertexCount(void)
{
	return 8;
}

Point CubeMesh::GetVertex(int i)
{
	return vertices[i];
}

Vector CubeMesh::Dimensions(void)
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
	return Vector(delta_x,delta_y,delta_z);
}

Point CubeMesh::Center(void)
{
	float sum_x = 0.0f;
	float sum_y = 0.0f;
	float sum_z = 0.0f;
	for(Point v : vertices)
	{
		sum_x += v.x;
		sum_y += v.y;
		sum_z += v.z;
	}
	return Point(sum_x/VertexCount(), sum_y/VertexCount(), sum_z/VertexCount());
}

int CubeMesh::FaceCount(void)
{
	return 12;
}

CubeMesh::Face CubeMesh::GetFace(int i)
{
	return faces[i];
}

int CubeMesh::EdgeCount(void)
{
	return 12;
}

CubeMesh::Edge CubeMesh::GetEdge(int i)
{
	return edges[i];
}
