#include "RasterUtilities.h"
namespace 
{
	template <typename DIVISOR>
	Hcoord operator/(const Hcoord& A, DIVISOR r) {
		Hcoord B(A);
		B.x /= r;
		B.y /= r;
		B.z /= r;
		B.w /= r;
		return B;
	}
}

void ClearBuffers(Raster& r, float z)
{
	int width = r.Width(),
		height = r.Height();
	for (int i = 0; i < height; ++i) {
		int j = 0;
		r.GotoPoint(j, i);
		for (; j < width; ++j)
		{
			r.WriteZ(z);
			r.WritePixel();
			r.IncrementX();
		}
	}
}

void FillTriangle(Raster& r, const Hcoord& P, const Hcoord& Q, const Hcoord& R)
{
	Hcoord P_ = P / P.w,
		Q_ = Q / Q.w,
		R_ = R / R.w;

}
