#include "SimpleRender.h"
namespace
{
	bool is_frontface(const Vector& v1, const Vector& v2)
	{
		return dot(v1,v2) > 0;
	}
	const Point CenterOfProjection(0, 0, 55);
}

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

void SimpleRender::DisplayEdges(Mesh& m, const Affine& A, const Vector& color)
{
	render.SetColor(color);
	int vertexCount = m.VertexCount();
	std::vector<Point> vertices;
	vertices.reserve(vertexCount);
	Matrix M = PersProj * A;
	for (int i = 0; i < vertexCount; i++)
	{
		Hcoord result = M * m.GetVertex(i);
		float w = result.w;
		result.x /= w;
		result.y /= w;
		result.z /= w;
		result.w /= w;
		vertices.push_back(Point(result.x,result.y,result.z));
	}
	int edgeCount = m.EdgeCount();
	for (int i = 0; i < edgeCount; i++)
	{
		int index1 = m.GetEdge(i).index1;
		int index2 = m.GetEdge(i).index2;
		render.DrawLine(vertices[index1], vertices[index2]);
	}

}

void SimpleRender::DisplayFaces(Mesh& m, const Affine& A, const Vector& color)
{
	int vertexCount = m.VertexCount();
	std::vector<Point> vertices;
	vertices.reserve(vertexCount);
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
	const Vector line_sines { 0, 0, 1 };
	for (int i = 0; i < faceCount; i++)
	{
		int index1 = m.GetFace(i).index1;
		int index2 = m.GetFace(i).index2;
		int index3 = m.GetFace(i).index3;
		Vector normal = cross(m.GetVertex(index2) - m.GetVertex(index1), m.GetVertex(index3) - m.GetVertex(index1));
		Vector PE = PointToVector(CenterOfProjection) - PointToVector(m.GetVertex(index1));
		if (is_frontface(normal,PE))
		{
			float diffuse = dot(line_sines, normal) / abs(normal);
			diffuse = diffuse < 0 ? -diffuse : diffuse;
			render.SetColor(Vector{ diffuse * color.x, diffuse * color.y, diffuse * color.z });
			render.FillTriangle(vertices[index1], vertices[index2], vertices[index3]);
		}
	}
}
