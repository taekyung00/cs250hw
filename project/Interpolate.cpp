#include "Interpolate.h"
namespace
{
	/**
	 * Helper operator to divide all components of an Hcoord by a scalar.
	 * Used for explicit perspective divide.
	 */
	template <typename DIVISOR>
	TexturedCoord operator/(const TexturedCoord& A, DIVISOR r) {
		TexturedCoord B(A);
		B.x /= r;
		B.y /= r;
		B.z /= r;
		B.w /= r;
		return B;
	}

	struct barycentric
	{
		float lambda = 0.f, mu = 0.f, nu = 0.f;
	};

	class Barycentric
	{
	public:
		Barycentric(Hcoord v1_, Hcoord v2_, Hcoord v3_) : v1(v1_), v2(v2_), v3(v3_) {}
		Barycentric() = delete;
		barycentric MakeBarycentric(Hcoord i) const {
			barycentric bary;
			Hcoord v1v2 = v2 - v1;
			Hcoord v1v3 = v3 - v1;
			Hcoord v1i = i - v1;

			float det = v1v2.x * v1v3.y - v1v2.y * v1v3.x;
			if (std::abs(det) < 1e-5f) {
				bary.lambda = -1.f;
				return bary;
			}

			float v1i_x_by_det = v1i.x / det;
			float v1i_y_by_det = v1i.y / det;

			bary.mu = v1i_x_by_det * v1v3.y - v1i_y_by_det * v1v3.x;
			bary.nu = v1v2.x * v1i_y_by_det - v1v2.y * v1i_x_by_det;
			bary.lambda = 1 - bary.mu - bary.nu;
			return bary;
		}
	private:
		Hcoord v1, v2, v3;
	};
}

Texture* TexturedCoord::texture = nullptr;
float TexturedCoord::color_scale = 1.f;
void FillTriangle(Raster& raster, const TexturedCoord& v0, const TexturedCoord& v1, const TexturedCoord& v2)
{
	//dont divide by w since we are doing perspective interpolation, not linear interpolation
	//NOTE: we have to use coord divided by w for screen space coordinate, and we also original coord for perspective interpolation
	
	// divide by w 
	TexturedCoord P_[3] = {
		v0 / v0.w,
		v1 / v1.w,
		v2 / v2.w };

	float xmin = P_[0].x;
	float xmax = P_[0].x;
	if (xmin > P_[1].x) xmin = P_[1].x;
	if (xmax < P_[1].x) xmax = P_[1].x;
	if (xmin > P_[2].x) xmin = P_[2].x;
	if (xmax < P_[2].x) xmax = P_[2].x;
	float ymin = P_[0].y;
	float ymax = P_[0].y;
	if (ymin > P_[1].y) ymin = P_[1].y;
	if (ymax < P_[1].y) ymax = P_[1].y;
	if (ymin > P_[2].y) ymin = P_[2].y;
	if (ymax < P_[2].y) ymax = P_[2].y;

	//clamping
	int int_xmin = static_cast<int>(std::ceil(xmin));
	if (int_xmin < 0) int_xmin = 0;
	int int_xmax = static_cast<int>(std::floor(xmax));
	if (int_xmax >= raster.Width()) int_xmax = raster.Width() - 1;
	int int_ymin = static_cast<int>(std::ceil(ymin));
	if (int_ymin < 0) int_ymin = 0;
	int int_ymax = static_cast<int>(std::floor(ymax));
	if (int_ymax >= raster.Height()) int_ymax = raster.Height() - 1;

	//NOTE: we have to use coord divided by w for screen space coordinate, and we also original coord for perspective interpolation
	Barycentric linear_bary(P_[0], P_[1], P_[2]);

	//NOTE : just interpolate uv coordinate!!
	float texture_for_scale = TexturedCoord::color_scale;

	for (int y = int_ymin; y <= int_ymax; ++y)
	{
		raster.GotoPoint(int_xmin, y);
		for (int x = int_xmin; x <= int_xmax; ++x) {
			barycentric bary_for_z = linear_bary.MakeBarycentric(Hcoord(static_cast<float>(x), static_cast<float>(y), 0, 0));
			if (bary_for_z.lambda >= 0 && bary_for_z.mu >= 0 && bary_for_z.nu >= 0) //linear interpolation
			{
				// get z value in screen space coordinate
				float z = P_[0].z * bary_for_z.lambda + P_[1].z * bary_for_z.mu + P_[2].z * bary_for_z.nu;
				if (z < raster.GetZ())
				{
					float inv_w = (bary_for_z.lambda / v0.w) + (bary_for_z.mu / v1.w) + (bary_for_z.nu / v2.w);

					float u_over_w = (bary_for_z.lambda * v0.u / v0.w) +
						(bary_for_z.mu * v1.u / v1.w) +
						(bary_for_z.nu * v2.u / v2.w);

					float v_over_w = (bary_for_z.lambda * v0.v / v0.w) +
						(bary_for_z.mu * v1.v / v1.w) +
						(bary_for_z.nu * v2.v / v2.w);

					float final_u = u_over_w / inv_w;
					float final_v = v_over_w / inv_w;

					Vector tex_color = TexturedCoord::texture->uvToRGB(final_u, final_v);

					tex_color.x *= texture_for_scale;
					tex_color.y *= texture_for_scale;
					tex_color.z *= texture_for_scale;

					raster.SetColor(static_cast<Raster::byte>(tex_color.x),
						static_cast<Raster::byte>(tex_color.y),
						static_cast<Raster::byte>(tex_color.z));
					raster.WritePixel();
					raster.WriteZ(z);
				}
			}
			raster.IncrementX();
		}
	}
}

bool TextureClip::operator()([[maybe_unused]]std::vector<TexturedCoord>& vertices) 
{
	return true;
}