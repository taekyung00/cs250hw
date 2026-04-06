/******************************************************************************
 * File: SimpleRender.cpp
 * Author: Taekyung Ho
 * Course: CS250
 * Assignment: Camera Frustum Programming Assignment
 * Description: Implements the SimpleRender class to render 3D meshes as
 * wireframes or solid objects with diffuse shading and backface
 * culling using a fixed camera.
 *****************************************************************************/
#include "SimpleRender.h"

namespace {
	/**
 * @brief Casting function: Converts a Vector to a Point.
 */
	Point VectorToPoint(const Vector& v)
	{
		return Point(v.x, v.y, v.z);
	}

	/**
	 * @brief Casting function: Converts a Point to a Vector.
	 */
	Vector PointToVector(const Point& p)
	{
		return Vector(p.x, p.y, p.z);
	}
}

namespace
{
	/**
	 * Determines if a face is facing the camera.
	 * Returns true if the dot product of the face normal and the vector to the
	 * camera is positive.
	 */
	bool is_frontface(const Vector& v1, const Vector& v2)
	{
		return dot(v1, v2) > 0;
	}

	// The fixed center of projection as specified in the assignment 
	const Point CenterOfProjection(0, 0, 5);
}

/**
 * Constructor: Initializes the perspective projection matrix.
 * The matrix is defined based on the center of projection E=(0,0,5)
 * and projection plane z=-1/2.
 */
SimpleRender::SimpleRender(Render& r) : render(r)
{
	PersProj.row[0] = Hcoord(1, 0, 0, 0);
	PersProj.row[1] = Hcoord(0, 1, 0, 0);
	PersProj.row[2] = Hcoord(0, 0, 1 / 11.f, -5 / 11.f);
	PersProj.row[3] = Hcoord(0, 0, -2 / 11.f, 10 / 11.f);
}

SimpleRender::~SimpleRender(void)
{
}

/**
 * Renders the mesh as a wireframe.
 * Transforms vertices to world space, applies perspective projection,
 * performs perspective divide, and draws edges.
 */
void SimpleRender::DisplayEdges(Mesh& m, const Affine& A, const Vector& color)
{
	render.SetColor(color);
	int vertexCount = m.VertexCount();
	std::vector<Point> vertices;
	vertices.reserve(vertexCount);

	// Combined Model-to-World and Projection matrix
	Matrix M = PersProj * A;

	for (int i = 0; i < vertexCount; i++)
	{
		Hcoord result = M * m.GetVertex(i);

		// Explicit Perspective Divide 
		float w = result.w;
		result.x /= w;
		result.y /= w;
		result.z /= w;
		result.w /= w;
		vertices.push_back(Point(result.x, result.y, result.z));
	}

	// Draw lines for each edge defined in the mesh
	int edgeCount = m.EdgeCount();
	for (int i = 0; i < edgeCount; i++)
	{
		int index1 = m.GetEdge(i).index1;
		int index2 = m.GetEdge(i).index2;
		render.DrawLine(vertices[index1], vertices[index2]);
	}
}

/**
 * Renders the mesh as a solid with diffuse shading.
 * Uses backface culling to only draw visible faces and calculates
 * shading based on the light direction L=<0,0,1>
 */
void SimpleRender::DisplayFaces(Mesh& m, const Affine& A, const Vector& color)
{
	int vertexCount = m.VertexCount();
	std::vector<Point> vertices;
	vertices.reserve(vertexCount);

	// Project vertices for rendering
	Matrix M = PersProj * A;
	for (int i = 0; i < vertexCount; i++)
	{
		Hcoord result = M * m.GetVertex(i);
		float w = result.w;
		result.x /= w;
		result.y /= w;
		result.z /= w;
		result.w /= w;
		vertices.push_back(Point(result.x, result.y, result.z));
	}

	int faceCount = m.FaceCount();
	const Vector line_shines{ 0, 0, 1 }; // Light direction 

	for (int i = 0; i < faceCount; i++)
	{
		int index1 = m.GetFace(i).index1;
		int index2 = m.GetFace(i).index2;
		int index3 = m.GetFace(i).index3;

		// Calculate face normal in world space for backface culling and shading
		const Point world_vertices[3] = { A * m.GetVertex(index1), A * m.GetVertex(index2), A * m.GetVertex(index3) };
		Vector normal = cross(world_vertices[1] - world_vertices[0], world_vertices[2] - world_vertices[0]);

		// Vector from a point on the face to the Center of Projection 
		Vector PE = PointToVector(CenterOfProjection) - PointToVector(world_vertices[0]);

		// Backface culling check 
		if (is_frontface(normal, PE))
		{
			// Calculate diffuse shading coefficient mu
			float diffuse = dot(line_shines, normal) / abs(normal);
			diffuse = diffuse < 0 ? -diffuse : diffuse;

			// Set the shaded color and fill the triangle
			render.SetColor(Vector{ diffuse * color.x, diffuse * color.y, diffuse * color.z });
			render.FillTriangle(vertices[index1], vertices[index2], vertices[index3]);
		}
	}
}