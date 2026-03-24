#include "Camera.h"
namespace 
{
	const float PI = 3.1415926535897932384626433832795f;
	const float DEFAUT_FOV = PI / 2;
	const float DEFAUT_ASPECT = 1.f;
}


Camera::Camera(void) : eye(0.f, 0.f, 0.f), right(1.f, 0.f, 0.f), up(0.f, 1.f, 0.f), back(0.f, 0.f, 1.f),
near(0.1f), far(10.f)
{
}

Camera::Camera(const Point& E, const Vector& look, const Vector& vp, float fov, float aspect, float near, float far)
	: eye(E), near(near), far(far)
{
}
