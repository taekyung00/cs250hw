#include "Affine.h"
namespace {
	float dot(const Hcoord& u, const Hcoord& v) {
		return u.x * v.x + u.y * v.y + u.z * v.z + u.w * v.w;
	}
}

Hcoord::Hcoord(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W)
{
}

Point::Point(float X, float Y, float Z) : Hcoord(X, Y, Z, 1.0f)
{
}

Vector::Vector(float X, float Y, float Z) : Hcoord(X, Y, Z, 0.0f)
{
}

Affine::Affine(void)
{
	row[0] = Hcoord(1.0f, 0.0f, 0.0f, 0.0f);
	row[1] = Hcoord(0.0f, 1.0f, 0.0f, 0.0f);
	row[2] = Hcoord(0.0f, 0.0f, 1.0f, 0.0f);
	row[3] = Hcoord(0.0f, 0.0f, 0.0f, 1.0f);
}

Affine::Affine(const Vector& Lx, const Vector& Ly, const Vector& Lz, const Point& D)
{
	row[0] = Hcoord(Lx.x, Ly.x, Lz.x, D.x);
	row[1] = Hcoord(Lx.y, Ly.y, Lz.y, D.y);
	row[2] = Hcoord(Lx.z, Ly.z, Lz.z, D.z);
	row[3] = Hcoord(0.0f, 0.0f, 0.0f, 1.0f);
}

Hcoord operator+(const Hcoord& u, const Hcoord& v)
{
	return Hcoord(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w);
}

Hcoord operator-(const Hcoord& u, const Hcoord& v)
{
	return Hcoord(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w);
}

Hcoord operator-(const Hcoord& v)
{
	return Hcoord(-v.x, -v.y, -v.z, -v.w);
}

Hcoord operator*(float r, const Hcoord& v)
{
	return Hcoord(r * v.x, r * v.y, r * v.z, r * v.w);
}

Hcoord operator*(const Matrix& A, const Hcoord& v)
{
	return Hcoord(dot(A[0], v), dot(A[1], v), dot(A[2], v), dot(A[3], v));
}

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

float dot(const Vector& u, const Vector& v)
{
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

float abs(const Vector& v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

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

Vector cross(const Vector& u, const Vector& v)
{
	return Vector(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}

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

Affine translate(const Vector& v)
{
	Affine R;
	R[0].w = v.x;
	R[1].w = v.y;
	R[2].w = v.z;
	return R;
}

Affine scale(float r)
{
	Affine S;
	S[0].x = S[1].y = S[2].z = r;
	return S;
}

Affine scale(float rx, float ry, float rz)
{
	Affine S;
	S[0].x = rx;
	S[1].y = ry;
	S[2].z = rz;
	return S;
}
/**
* Orthogonality
* 1. Lx, Ly, Lz are mutually perpendicular to each other.
* 2. each basis vector has unit length.
* => Rotation/Reflection matrix is orthogonal matrix, and its inverse is equal to its transpose.
* But, Scale/Shear matrix is not orthogonal, and its inverse is not equal to its transpose.
* So we should not use Orthogonality to calculate the inverse of an Affine matrix, because it may contain Scale/Shear component.
*/
Affine inverse(const Affine& A)
{
	// 1. Calculate the Minors (M_ij) of the 3x3 linear part (L)
	// M_ij is the determinant of the 2x2 matrix obtained by crossing out row i and column j
	float M00 = A[1].y * A[2].z - A[1].z * A[2].y;
	float M01 = A[1].x * A[2].z - A[1].z * A[2].x;
	float M02 = A[1].x * A[2].y - A[1].y * A[2].x;

	float M10 = A[0].y * A[2].z - A[0].z * A[2].y;
	float M11 = A[0].x * A[2].z - A[0].z * A[2].x;
	float M12 = A[0].x * A[2].y - A[0].y * A[2].x;

	float M20 = A[0].y * A[1].z - A[0].z * A[1].y;
	float M21 = A[0].x * A[1].z - A[0].z * A[1].x;
	float M22 = A[0].x * A[1].y - A[0].y * A[1].x;

	// 2. Calculate the Cofactors (C_ij) by applying the alternating signs: (-1)^(i+j)
	float C00 = M00;
	float C01 = -M01;
	float C02 = M02;

	float C10 = -M10;
	float C11 = M11;
	float C12 = -M12;

	float C20 = M20;
	float C21 = -M21;
	float C22 = M22;

	// 3. Calculate the determinant using the first row of L and the first row of C
	// det(L) = L_11 * C_11 + L_12 * C_12 + L_13 * C_13
	float det = A[0].x * C00 + A[0].y * C01 + A[0].z * C02;

	// Check if the matrix is invertible
	assert(std::abs(det) > 1e-5f);

	// 4. Calculate the inverse of the 3x3 matrix: (1/det) * C^T
	// Transpose the cofactor matrix (C^T) by swapping row and column indices here
	float invDet = 1.0f / det;
	Vector invLx(C00 * invDet, C10 * invDet, C20 * invDet);
	Vector invLy(C01 * invDet, C11 * invDet, C21 * invDet);
	Vector invLz(C02 * invDet, C12 * invDet, C22 * invDet);

	// Create the inverse linear transformation matrix (L^-1)
	Affine invL(invLx, invLy, invLz, Point(0.0f, 0.0f, 0.0f));

	// Create the inverse translation matrix (T^-1)
	// The inverse of translating by vector v is translating by vector -v
	Vector original_translation(A[0].w, A[1].w, A[2].w);
	Affine invT = translate(-original_translation);

	// Return the combined inverse transformation: L^-1 * T^-1
	return invL * invT;
}



Point VectorToPoint(const Vector& v)
{
	return Point(v.x, v.y, v.z);
}

Vector PointToVector(const Point& p)
{
	return Vector(p.x, p.y, p.z);
}
