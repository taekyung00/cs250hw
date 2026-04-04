/**
 * @file Affine.cpp
 * @author Jean Ho (Taekyung Ho)
 * @date April 4, 2026
 * @brief Implementation of affine transformations and mathematical operations
 * using homogeneous coordinates for 3D graphics.
 * Course: CS250
 */

#include "Affine.h"

namespace {
	/**
	 * @brief Internal helper to calculate the dot product of 4D homogeneous coordinates.
	 * @param u First coordinate.
	 * @param v Second coordinate.
	 * @return The scalar dot product result.
	 */
	float dot(const Hcoord& u, const Hcoord& v) {
		return u.x * v.x + u.y * v.y + u.z * v.z + u.w * v.w;
	}
}

/**
 * @brief General constructor for Homogeneous Coordinates (Hcoord).
 */
Hcoord::Hcoord(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W)
{
}

/**
 * @brief Constructor for a Point in 3D space.
 * Sets the homogeneous component w to 1.0f to allow translation.
 */
Point::Point(float X, float Y, float Z) : Hcoord(X, Y, Z, 1.0f)
{
}

/**
 * @brief Constructor for a Vector in 3D space.
 * Sets the homogeneous component w to 0.0f to ensure it is translation-invariant.
 */
Vector::Vector(float X, float Y, float Z) : Hcoord(X, Y, Z, 0.0f)
{
}

/**
 * @brief Default constructor for the Affine class.
 * Initializes the linear transformation part to zero and sets the
 * fourth row to standard homogeneous form (0, 0, 0, 1).
 */
Affine::Affine(void)
{
	row[0] = Hcoord(0.0f, 0.0f, 0.0f, 0.0f);
	row[1] = Hcoord(0.0f, 0.0f, 0.0f, 0.0f);
	row[2] = Hcoord(0.0f, 0.0f, 0.0f, 0.0f);
	row[3] = Hcoord(0.0f, 0.0f, 0.0f, 1.0f);
}

/**
 * @brief Constructs an Affine matrix using basis vectors and a translation point.
 * @param Lx Local X-axis vector.
 * @param Ly Local Y-axis vector.
 * @param Lz Local Z-axis vector.
 * @param D  Translation point (origin of the local frame).
 */
Affine::Affine(const Vector& Lx, const Vector& Ly, const Vector& Lz, const Point& D)
{
	row[0] = Hcoord(Lx.x, Ly.x, Lz.x, D.x);
	row[1] = Hcoord(Lx.y, Ly.y, Lz.y, D.y);
	row[2] = Hcoord(Lx.z, Ly.z, Lz.z, D.z);
	row[3] = Hcoord(0.0f, 0.0f, 0.0f, 1.0f);
}

/**
 * @brief Addition of two homogeneous coordinates.
 */
Hcoord operator+(const Hcoord& u, const Hcoord& v)
{
	return Hcoord(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w);
}

/**
 * @brief Subtraction of two homogeneous coordinates.
 */
Hcoord operator-(const Hcoord& u, const Hcoord& v)
{
	return Hcoord(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w);
}

/**
 * @brief Unary negation of a homogeneous coordinate.
 */
Hcoord operator-(const Hcoord& v)
{
	return Hcoord(-v.x, -v.y, -v.z, -v.w);
}

/**
 * @brief Scalar multiplication of a homogeneous coordinate.
 */
Hcoord operator*(float r, const Hcoord& v)
{
	return Hcoord(r * v.x, r * v.y, r * v.z, r * v.w);
}

/**
 * @brief Multiplies a matrix by a vector (A * v).
 * Each component of the resulting Hcoord is the dot product of a matrix row and the vector.
 */
Hcoord operator*(const Matrix& A, const Hcoord& v)
{
	return Hcoord(dot(A[0], v), dot(A[1], v), dot(A[2], v), dot(A[3], v));
}

/**
 * @brief Multiplies two 4x4 matrices (A * B).
 * Uses column-major extraction of matrix B to perform dot products with rows of A.
 */
Matrix operator*(const Matrix& A, const Matrix& B)
{
	const Hcoord BCol[4] = {
		Hcoord{B[0][0], B[1][0], B[2][0], B[3][0]},
		Hcoord{B[0][1], B[1][1], B[2][1], B[3][1]},
		Hcoord{B[0][2], B[1][2], B[2][2], B[3][2]},
		Hcoord{B[0][3], B[1][3], B[2][3], B[3][3] } };
	return Matrix(
		{ Hcoord{ dot(A[0], BCol[0]), dot(A[0], BCol[1]), dot(A[0], BCol[2]), dot(A[0], BCol[3]) },
		Hcoord{ dot(A[1], BCol[0]), dot(A[1], BCol[1]), dot(A[1], BCol[2]), dot(A[1], BCol[3]) },
		Hcoord{ dot(A[2], BCol[0]), dot(A[2], BCol[1]), dot(A[2], BCol[2]), dot(A[2], BCol[3]) },
		Hcoord{ dot(A[3], BCol[0]), dot(A[3], BCol[1]), dot(A[3], BCol[2]), dot(A[3], BCol[3]) } }
	);
}

/**
 * @brief Calculates the 3D dot product of two vectors.
 */
float dot(const Vector& u, const Vector& v)
{
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

/**
 * @brief Calculates the magnitude (Euclidean length) of a vector.
 */
float abs(const Vector& v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

/**
 * @brief Normalizes a vector to unit length.
 * If the vector is near zero, returns a zero vector to avoid division by zero.
 */
Vector normalize(const Vector& v)
{
	const float r = abs(v);
	if (Hcoord::near(r, 0))
	{
		return Vector(0.0f, 0.0f, 0.0f);
	}
	else
	{
		return Vector(v.x / r, v.y / r, v.z / r);
	}
}

/**
 * @brief Calculates the cross product of two 3D vectors.
 */
Vector cross(const Vector& u, const Vector& v)
{
	return Vector(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}

/**
 * @brief Creates a rotation matrix about an arbitrary axis using Rodrigues' Rotation Formula.
 * @param t Rotation angle in radians.
 * @param v Axis of rotation.
 */
Affine rotate(float t, const Vector& v)
{
	const float c = cos(t), s = sin(t);
	const float L = abs(v);
	Affine R;
	R[0] = Hcoord(c + (1 - c) * v.x * v.x / (L * L), (1 - c) * v.x * v.y / (L * L) - s * v.z / L, (1 - c) * v.x * v.z / (L * L) + s * v.y / L, 0.0f);
	R[1] = Hcoord((1 - c) * v.y * v.x / (L * L) + s * v.z / L, c + (1 - c) * v.y * v.y / (L * L), (1 - c) * v.y * v.z / (L * L) - s * v.x / L, 0.0f);
	R[2] = Hcoord((1 - c) * v.z * v.x / (L * L) - s * v.y / L, (1 - c) * v.z * v.y / (L * L) + s * v.x / L, c + (1 - c) * v.z * v.z / (L * L), 0.0f);
	return R;
}

/**
 * @brief Creates a translation matrix from a given vector.
 */
Affine translate(const Vector& v)
{
	Affine R;
	R[0] = Hcoord(1.0f, 0.0f, 0.0f, v.x);
	R[1] = Hcoord(0.0f, 1.0f, 0.0f, v.y);
	R[2] = Hcoord(0.0f, 0.0f, 1.0f, v.z);
	return R;
}

/**
 * @brief Creates a uniform scale matrix.
 */
Affine scale(float r)
{
	Affine S;
	S[0].x = S[1].y = S[2].z = r;
	return S;
}

/**
 * @brief Creates a non-uniform scale matrix.
 */
Affine scale(float rx, float ry, float rz)
{
	Affine S;
	S[0].x = rx;
	S[1].y = ry;
	S[2].z = rz;
	return S;
}

/**
 * @brief Calculates the inverse of an Affine matrix using the cofactor method.
 * * Logic:
 * 1. An Affine matrix M can be decomposed into a linear part L and translation T.
 * 2. Since the matrix may contain Scale or Shear, orthogonality is not assumed.
 * 3. We find the inverse of the 3x3 linear part using Minors, Cofactors, and the Determinant.
 * 4. The total inverse is calculated as L^-1 * T^-1.
 */
Affine inverse(const Affine& A)
{
	// 1. Calculate the Minors (M_ij) of the 3x3 linear part (L)
	float M00 = A[1].y * A[2].z - A[1].z * A[2].y;
	float M01 = A[1].x * A[2].z - A[1].z * A[2].x;
	float M02 = A[1].x * A[2].y - A[1].y * A[2].x;

	float M10 = A[0].y * A[2].z - A[0].z * A[2].y;
	float M11 = A[0].x * A[2].z - A[0].z * A[2].x;
	float M12 = A[0].x * A[2].y - A[0].y * A[2].x;

	float M20 = A[0].y * A[1].z - A[0].z * A[1].y;
	float M21 = A[0].x * A[1].z - A[0].z * A[1].x;
	float M22 = A[0].x * A[1].y - A[0].y * A[1].x;

	// 2. Calculate the Cofactors (C_ij) by applying alternating signs: (-1)^(i+j)
	float C00 = M00;
	float C01 = -M01;
	float C02 = M02;

	float C10 = -M10;
	float C11 = M11;
	float C12 = -M12;

	float C20 = M20;
	float C21 = -M21;
	float C22 = M22;

	// 3. Calculate the determinant of the linear part
	float det = A[0].x * C00 + A[0].y * C01 + A[0].z * C02;

	// Ensure matrix is non-singular before proceeding
	assert(std::abs(det) > 1e-5f);

	// 4. Calculate L^-1: (1/det) * Adjugate(L)
	// The Adjugate is the transpose of the cofactor matrix
	float invDet = 1.0f / det;
	Vector invLx(C00 * invDet, C10 * invDet, C20 * invDet);
	Vector invLy(C01 * invDet, C11 * invDet, C21 * invDet);
	Vector invLz(C02 * invDet, C12 * invDet, C22 * invDet);

	// Create inverse linear part with no translation
	Affine invL(invLx, invLy, invLz, Point(0.0f, 0.0f, 0.0f));

	// 5. Create T^-1: Negate the translation vector
	Vector original_translation(A[0].w, A[1].w, A[2].w);
	Affine invT = translate(-original_translation);

	// Result: (T * L)^-1 = L^-1 * T^-1
	return invL * invT;
}

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