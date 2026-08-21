#include "parallel_light.h"

#include <cmath>

#include "TRandom.h"
#include "TMath.h"


ParallelLight::ParallelLight(
    double center_x,
    double center_y,
    double center_z,
    double radius,
    double wavelength
)
:
    center_x_(center_x),
    center_y_(center_y),
    center_z_(center_z),
    radius_(radius),
    wavelength_(wavelength)
{
}


void ParallelLight::GenerateRay(
    double &x,
    double &y,
    double &z,
    double &m,
    double &n,
    double &l
) const
{
    // -------------------------------------------------
    // 1. 在半径为 radius_ 的圆形光源上均匀随机取一点
    // -------------------------------------------------

    double u1 = gRandom->Rndm();
    double u2 = gRandom->Rndm();

    double r =
        radius_ * std::sqrt(u1);

    double phi =
        2.0 * TMath::Pi() * u2;


    // -------------------------------------------------
    // 2. 光子起始位置
    // -------------------------------------------------

    x =
        center_x_ + r * std::cos(phi);

    y =
        center_y_ + r * std::sin(phi);

    z =
        center_z_;


    // -------------------------------------------------
    // 3. 平行光传播方向
    // -------------------------------------------------

    m = 0.0;
    n = 0.0;
    l = -1.0;
}
