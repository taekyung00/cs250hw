/******************************************************************************
 * File: Camera.cpp
 * Author: Taekyung Ho
 * Course: CS250
 * Assignment: Camera Frustum Programming Assignment
 * Description: Implements a flexible camera class using a right-handed
 * coordinate system (Right, Up, Back vectors).
 *****************************************************************************/
#include "Camera.h"
#include <cmath>

namespace
{
	const float PI = 3.1415926535897932384626433832795f;
	const float DEFAUT_FOV = PI / 2;
	const float DEFAUT_ASPECT = 1.f;
}

/**
 * Default Constructor: Places camera at origin looking at -Z.
 */
Camera::Camera(void) :
	eye(0.f, 0.f, 0.f), right(1.f, 0.f, 0.f), up(0.f, 1.f, 0.f), back(0.f, 0.f, 1.f),
	near(0.1f), far(10.f)
{
	distance = near;
	width = tan(DEFAUT_FOV / 2) * 2 * distance;
	height = width / DEFAUT_ASPECT;
}

/**
 * Non-default Constructor: Initializes camera with orientation vectors.
 */
Camera::Camera(const Point& E, const Vector& look, const Vector& vp, float fov, float aspect, float near_, float far_)
	: eye(E), near(near_), far(far_)
{
	// Construct the orthonormal basis for the camera
	right = normalize(cross(look, vp));
	back = -normalize(look);
	up = cross(back, right);

	distance = near;
	width = tan(fov / 2) * 2 * distance;
	height = width / aspect;
}

Point Camera::Eye(void) const { return eye; }
Vector Camera::Right(void) const { return right; }
Vector Camera::Up(void) const { return up; }
Vector Camera::Back(void) const { return back; }
Vector Camera::ViewportGeometry(void) const { return Vector(width, height, distance); }
float Camera::NearDistance(void) const { return near; }
float Camera::FarDistance(void) const { return far; }

/**
 * Zoom operation: Multiplies viewport dimensions by a factor.
 */
Camera& Camera::Zoom(float factor)
{
	width *= factor;
	height *= factor;
	return *this;
}

/**
 * Moves camera along its view direction.
 */
Camera& Camera::Forward(float distance_increment)
{
	eye = eye - distance_increment * back;
	return *this;
}

/**
 * Rotates camera around its vertical (Up) axis.
 */
Camera& Camera::Yaw(float angle)
{
	Affine a = rotate(angle, up);
	right = a * right;
	back = a * back;
	return *this;
}

/**
 * Rotates camera around its horizontal (Right) axis.
 */
Camera& Camera::Pitch(float angle)
{
	Affine a = rotate(angle, right);
	up = a * up;
	back = a * back;
	return *this;
}

/**
 * Rotates camera around its viewing (Back) axis.
 */
Camera& Camera::Roll(float angle)
{
	Affine a = rotate(angle, back);
	right = a * right;
	up = a * up;
	return *this;
}