#include "CameraRender2D.h"
#include "Projection.h"
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

	bool is_front_of_camera(const Point& camera_vertices)
	{
		return camera_vertices.z < 0;
	}
}

CameraRender2D::CameraRender2D(Render& r) : render(r)
{
}

CameraRender2D::~CameraRender2D(void)
{
}

void CameraRender2D::SetCamera(const Camera& cam)
{
	cam_vertices.clear();
	world2camera = WorldToCamera(cam);
	camera2ndc = CameraToNDC(cam);
	cam_vertices.push_back(cam.Eye());//center of proj
	cam_vertices.push_back(VectorToPoint(cam.Back()));//line of sight

}
//Now camera is moving!!!!
void CameraRender2D::DisplayEdges(Mesh& m, const Affine& ModelToWorld, const Vector& color)
{
	render.SetColor(color);
	int vertexCount = m.VertexCount();
	std::vector<Point> vertices;
	vertices.reserve(vertexCount);

	// Combined Model-to-World and World-to-Camera and Camera-to-NDC
	Matrix M = camera2ndc * world2camera * ModelToWorld;

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
		const Point camera_vertices[2] = { world2camera * ModelToWorld * m.GetVertex(index1),world2camera * ModelToWorld * m.GetVertex(index2) };
		if (is_front_of_camera(camera_vertices[0]) && is_front_of_camera(camera_vertices[1]))
		{
			render.DrawLine(vertices[index1], vertices[index2]);
		}
	}
}

/**
 * Renders the mesh as a solid with diffuse shading.
 * Uses backface culling to only draw visible faces and calculates
 * shading based on the light direction L=<0,0,1>
 */
void CameraRender2D::DisplayFaces(Mesh& m, const Affine& ModelToWorld, const Vector& color)
{
	int vertexCount = m.VertexCount();
	std::vector<Point> vertices;
	vertices.reserve(vertexCount);

	// Combined Model-to-World and World-to-Camera and Camera-to-NDC
	Matrix M = camera2ndc * world2camera * ModelToWorld;
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
	const Vector line_shines = PointToVector(cam_vertices[1]); // Light direction **now this is changing every time!!

	for (int i = 0; i < faceCount; i++)
	{
		int index1 = m.GetFace(i).index1;
		int index2 = m.GetFace(i).index2;
		int index3 = m.GetFace(i).index3;

		// Calculate face normal in world space for backface culling and shading
		const Point world_vertices[3] = { ModelToWorld * m.GetVertex(index1), ModelToWorld * m.GetVertex(index2), ModelToWorld * m.GetVertex(index3) };
		const Point camera_vertices[3] = { world2camera * ModelToWorld * m.GetVertex(index1),world2camera * ModelToWorld * m.GetVertex(index2),world2camera * ModelToWorld * m.GetVertex(index3) };
		Vector normal = cross(world_vertices[1] - world_vertices[0], world_vertices[2] - world_vertices[0]);

		// Vector from a point on the face to the Center of Projection 
		Vector PE = PointToVector(cam_vertices[0]) - PointToVector(world_vertices[0]);

		// Backface culling check 
		if (is_frontface(normal, PE) && is_front_of_camera(camera_vertices[0]) && is_front_of_camera(camera_vertices[1]) && is_front_of_camera(camera_vertices[2]))
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
