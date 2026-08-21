#include "diffuse_target.h"

#include <cmath>

#include "TRandom.h"
#include "TMath.h"

DiffuseTarget::DiffuseTarget(
    double center_x,
    double center_y,
    double center_z,
    double width,
    double height,
    double reflectivity
)
    : center_x_(center_x),
      center_y_(center_y),
      center_z_(center_z),
      width_(width),
      height_(height),
      reflectivity_(reflectivity)
{
}

bool DiffuseTarget::IsInside(
    double x,
    double y
) const
{
    if(std::fabs(x - center_x_) > width_ / 2.0)
        return false;

    if(std::fabs(y - center_y_) > height_ / 2.0)
        return false;

    return true;
}

bool DiffuseTarget::Reflect(
    double &m,
    double &n,
    double &l
) const
{
    double u_reflect = gRandom->Rndm();

    if(u_reflect >= reflectivity_)
        return false;

    /*
     * Cosine-weighted Lambertian sampling.
     *
     * Target normal:
     *     (0, 0, -1)
     *
     * All reflected rays propagate into the Z- hemisphere.
     */
    double u1 = gRandom->Rndm();
    double u2 = gRandom->Rndm();

    double r = std::sqrt(u1);
    double phi = 2.0 * TMath::Pi() * u2;

    m = r * std::cos(phi);
    n = r * std::sin(phi);
    l = -std::sqrt(1.0 - u1);

    return true;
}
