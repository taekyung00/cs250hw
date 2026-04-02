#include "Projection.h"

Affine CameraToWorld(const Camera& cam)
{
	return Affine(cam.Right(),cam.Up(), cam.Back(), cam.Eye());
}

Affine WorldToCamera(const Camera& cam)
{
	Affine rotate_t = Affine(cam.Right(), cam.Up(), cam.Back(), Point(0.f, 0.f, 0.f));
	Affine translate_neg = translate(-PointToVector(cam.Eye()));
	float temp;

	temp = rotate_t[0].y;
	rotate_t[0].y = rotate_t[1].x;
	rotate_t[1].x = temp;

	temp = rotate_t[0].z;
	rotate_t[0].z = rotate_t[2].x;
	rotate_t[2].x = temp;

	temp = rotate_t[1].z;
	rotate_t[1].z = rotate_t[2].y;
	rotate_t[2].y = temp;

	return rotate_t*translate_neg;
}

Matrix CameraToNDC(const Camera& cam)
{
	Matrix proj_and_NDC;
	float near = cam.NearDistance();
	float far = cam.FarDistance();
	float width = cam.ViewportGeometry().x;
	float height = cam.ViewportGeometry().y;
	float distance = cam.ViewportGeometry().z;
	proj_and_NDC[0].x = 2 * distance / width;
	proj_and_NDC[1].y = 2 * distance / height;
	proj_and_NDC[2].z = (far + near) / (near - far);
	proj_and_NDC[2].w = 2 * far * near / (near - far);
	proj_and_NDC[3].z = -1;
	return proj_and_NDC;
}
