#include "detector.h"

#include <algorithm>
#include <cmath>

// ============================================================
// Constants
// ============================================================

namespace
{
    const double PI = 3.14159265358979323846;
    const double EPS = 1.0e-12;

    double Dot(
        double ax,
        double ay,
        double az,
        double bx,
        double by,
        double bz
    )
    {
        return ax * bx + ay * by + az * bz;
    }

    double Norm(
        double x,
        double y,
        double z
    )
    {
        return std::sqrt(
            x * x +
            y * y +
            z * z
        );
    }
}

// ============================================================
// Constructor
// ============================================================

Detector::Detector(
    double center_x,
    double center_y,
    double center_z,
    double focal_length,
    double f_number,
    double horizontal_fov_deg,
    double vertical_fov_deg,
    double disk_diameter,
    double base_width,
    double base_height,
    double base_left_offset,
    double hole_diameter,
    double hole_edge_distance
)
    : center_x_(center_x),
      center_y_(center_y),
      center_z_(center_z),
      lookat_x_(center_x),
      lookat_y_(center_y),
      lookat_z_(center_z + 1.0),
      axis_x_(0.0),
      axis_y_(0.0),
      axis_z_(1.0),
      ux_x_(1.0),
      ux_y_(0.0),
      ux_z_(0.0),
      uy_x_(0.0),
      uy_y_(1.0),
      uy_z_(0.0),
      focal_length_(focal_length),
      f_number_(f_number),
      horizontal_fov_deg_(horizontal_fov_deg),
      vertical_fov_deg_(vertical_fov_deg),
      horizontal_half_fov_rad_(0.0),
      vertical_half_fov_rad_(0.0),
      effective_aperture_diameter_(0.0),
      effective_aperture_radius_(0.0),
      disk_diameter_(disk_diameter),
      disk_radius_(disk_diameter / 2.0),
      base_width_(base_width),
      base_height_(base_height),
      base_left_offset_(base_left_offset),
      base_left_(0.0),
      base_right_(0.0),
      base_top_(0.0),
      base_bottom_(0.0),
      hole_diameter_(hole_diameter),
      hole_radius_(hole_diameter / 2.0),
      hole_edge_distance_(hole_edge_distance),
      hole_center_x_(0.0),
      hole_center_y_(0.0),
      geometry_valid_(false)
{
    horizontal_half_fov_rad_ =
        horizontal_fov_deg_ * PI / 360.0;

    vertical_half_fov_rad_ =
        vertical_fov_deg_ * PI / 360.0;

    if(f_number_ > 0.0)
    {
        effective_aperture_diameter_ =
            focal_length_ / f_number_;

        effective_aperture_radius_ =
            effective_aperture_diameter_ / 2.0;
    }

    // --------------------------------------------------------
    // Base horizontal range
    //
    // Disk radius = 85 mm
    // Base left edge = -85 + 6 = -79 mm
    //
    // For base width = 140 mm:
    // Base right edge = -79 + 140 = +61 mm
    // --------------------------------------------------------

    base_left_ =
        -disk_radius_ +
        base_left_offset_;

    base_right_ =
        base_left_ +
        base_width_;

    // --------------------------------------------------------
    // Base bottom
    //
    // The visible base extends 15 mm below the disk:
    //
    // -85 - 15 = -100 mm
    // --------------------------------------------------------

    base_bottom_ =
        -disk_radius_ -
        base_height_;

    // --------------------------------------------------------
    // Base top
    //
    // The 140 mm wide support extends vertically upward until
    // its entire width is completely covered by the circular
    // housing.
    //
    // The limiting X position is the base edge with the
    // largest absolute X coordinate.
    //
    // y = -sqrt(R^2 - x_limit^2)
    //
    // For R = 85 mm, x_limit = 79 mm:
    // y approximately -31.37 mm
    // --------------------------------------------------------

    const double x_limit =
        std::max(
            std::fabs(base_left_),
            std::fabs(base_right_)
        );

    if(x_limit <= disk_radius_)
    {
        const double inside =
            disk_radius_ * disk_radius_ -
            x_limit * x_limit;

        base_top_ =
            -std::sqrt(
                std::max(0.0, inside)
            );
    }

    // --------------------------------------------------------
    // Receiving-hole center
    //
    // Hole left edge is 13 mm from disk left edge.
    //
    // x_hole =
    // -R + edge_distance + hole_radius
    //
    // For R = 85 mm:
    // -85 + 13 + 10.5 = -61.5 mm
    // --------------------------------------------------------

    hole_center_x_ =
        -disk_radius_ +
        hole_edge_distance_ +
        hole_radius_;

    hole_center_y_ = 0.0;

    geometry_valid_ =
        ValidateParameters() &&
        BuildLocalBasis();
}

// ============================================================
// Validate detector parameters
// ============================================================

bool Detector::ValidateParameters() const
{
    if(focal_length_ <= 0.0)
    {
        return false;
    }

    if(f_number_ <= 0.0)
    {
        return false;
    }

    if(horizontal_fov_deg_ <= 0.0 ||
       horizontal_fov_deg_ >= 180.0)
    {
        return false;
    }

    if(vertical_fov_deg_ <= 0.0 ||
       vertical_fov_deg_ >= 180.0)
    {
        return false;
    }

    if(disk_diameter_ <= 0.0 ||
       disk_radius_ <= 0.0)
    {
        return false;
    }

    if(base_width_ <= 0.0 ||
       base_height_ <= 0.0)
    {
        return false;
    }

    if(base_left_offset_ < 0.0)
    {
        return false;
    }

    if(base_left_ < -disk_radius_ ||
       base_right_ > disk_radius_)
    {
        return false;
    }

    if(base_top_ < base_bottom_)
    {
        return false;
    }

    if(hole_diameter_ <= 0.0 ||
       hole_radius_ <= 0.0)
    {
        return false;
    }

    if(hole_edge_distance_ < 0.0)
    {
        return false;
    }

    const double hole_center_distance =
        std::sqrt(
            hole_center_x_ * hole_center_x_ +
            hole_center_y_ * hole_center_y_
        );

    if(hole_center_distance + hole_radius_ >
       disk_radius_ + EPS)
    {
        return false;
    }

    if(effective_aperture_diameter_ <= 0.0)
    {
        return false;
    }

    if(effective_aperture_diameter_ >
       hole_diameter_ + EPS)
    {
        return false;
    }

    return true;
}

// ============================================================
// Build detector local coordinate system
// ============================================================

bool Detector::BuildLocalBasis()
{
    double ax =
        lookat_x_ -
        center_x_;

    double ay =
        lookat_y_ -
        center_y_;

    double az =
        lookat_z_ -
        center_z_;

    const double axis_norm =
        Norm(ax, ay, az);

    if(axis_norm <= EPS)
    {
        return false;
    }

    ax /= axis_norm;
    ay /= axis_norm;
    az /= axis_norm;

    axis_x_ = ax;
    axis_y_ = ay;
    axis_z_ = az;

    // --------------------------------------------------------
    // Choose a stable reference direction.
    //
    // For an optical axis close to global Z, use global Y as
    // reference. Otherwise use global Z.
    // --------------------------------------------------------

    double ref_x = 0.0;
    double ref_y = 0.0;
    double ref_z = 1.0;

    if(std::fabs(axis_z_) > 0.99)
    {
        ref_x = 0.0;
        ref_y = 1.0;
        ref_z = 0.0;
    }

    // Local X = reference x optical_axis

    double ux_x =
        ref_y * axis_z_ -
        ref_z * axis_y_;

    double ux_y =
        ref_z * axis_x_ -
        ref_x * axis_z_;

    double ux_z =
        ref_x * axis_y_ -
        ref_y * axis_x_;

    const double ux_norm =
        Norm(
            ux_x,
            ux_y,
            ux_z
        );

    if(ux_norm <= EPS)
    {
        return false;
    }

    ux_x /= ux_norm;
    ux_y /= ux_norm;
    ux_z /= ux_norm;

    ux_x_ = ux_x;
    ux_y_ = ux_y;
    ux_z_ = ux_z;

    // Local Y = optical_axis x local_X

    double uy_x =
        axis_y_ * ux_z_ -
        axis_z_ * ux_y_;

    double uy_y =
        axis_z_ * ux_x_ -
        axis_x_ * ux_z_;

    double uy_z =
        axis_x_ * ux_y_ -
        axis_y_ * ux_x_;

    const double uy_norm =
        Norm(
            uy_x,
            uy_y,
            uy_z
        );

    if(uy_norm <= EPS)
    {
        return false;
    }

    uy_x /= uy_norm;
    uy_y /= uy_norm;
    uy_z /= uy_norm;

    uy_x_ = uy_x;
    uy_y_ = uy_y;
    uy_z_ = uy_z;

    return true;
}

// ============================================================
// Set detector pointing
// ============================================================

bool Detector::LookAt(
    double target_x,
    double target_y,
    double target_z
)
{
    lookat_x_ = target_x;
    lookat_y_ = target_y;
    lookat_z_ = target_z;

    geometry_valid_ =
        ValidateParameters() &&
        BuildLocalBasis();

    return geometry_valid_;
}

// ============================================================
// Circular housing
// ============================================================

bool Detector::IsInsideDisk(
    double local_x,
    double local_y
) const
{
    const double r2 =
        local_x * local_x +
        local_y * local_y;

    return
        r2 <=
        disk_radius_ * disk_radius_ + EPS;
}

// ============================================================
// Vertical 140 mm base/support
//
// The support begins 15 mm below the circular housing and
// extends upward until its full width is hidden by the disk.
// ============================================================

bool Detector::IsInsideBase(
    double local_x,
    double local_y
) const
{
    if(local_x < base_left_ - EPS ||
       local_x > base_right_ + EPS)
    {
        return false;
    }

    if(local_y < base_bottom_ - EPS ||
       local_y > base_top_ + EPS)
    {
        return false;
    }

    return true;
}

// ============================================================
// Complete mechanical housing
// ============================================================

bool Detector::IsInsideBody(
    double local_x,
    double local_y
) const
{
    return
        IsInsideDisk(
            local_x,
            local_y
        ) ||
        IsInsideBase(
            local_x,
            local_y
        );
}

// ============================================================
// Physical 21 mm receiving hole
// ============================================================

bool Detector::IsInsidePhysicalHole(
    double local_x,
    double local_y
) const
{
    const double dx =
        local_x -
        hole_center_x_;

    const double dy =
        local_y -
        hole_center_y_;

    return
        dx * dx +
        dy * dy
        <=
        hole_radius_ * hole_radius_ + EPS;
}

// ============================================================
// Effective optical aperture
// ============================================================

bool Detector::IsInsideEffectiveAperture(
    double local_x,
    double local_y
) const
{
    const double dx =
        local_x -
        hole_center_x_;

    const double dy =
        local_y -
        hole_center_y_;

    return
        dx * dx +
        dy * dy
        <=
        effective_aperture_radius_ *
        effective_aperture_radius_ +
        EPS;
}

// ============================================================
// Ray-detector interaction
//
// Important order:
//
// 1. Validate geometry
// 2. Normalize ray
// 3. Intersect detector plane
// 4. Transform intersection to detector-local coordinates
// 5. Test mechanical housing
// 6. Test physical hole
// 7. Test front-side incidence
// 8. Test FOV
// 9. Test effective optical aperture
// 10. Accept
//
// This ordering is important because the CCD housing itself
// can block photons travelling toward the telescope mirror.
// ============================================================

int Detector::CheckRay(
    double ray_x,
    double ray_y,
    double ray_z,
    double ray_m,
    double ray_n,
    double ray_l,
    double &hit_x,
    double &hit_y,
    double &hit_z
) const
{
    hit_x = 0.0;
    hit_y = 0.0;
    hit_z = 0.0;

    if(!geometry_valid_)
    {
        return DETECTOR_INVALID_GEOMETRY;
    }

    // --------------------------------------------------------
    // Normalize ray direction
    // --------------------------------------------------------

    const double direction_norm =
        Norm(
            ray_m,
            ray_n,
            ray_l
        );

    if(direction_norm <= EPS)
    {
        return DETECTOR_INVALID_GEOMETRY;
    }

    const double dm =
        ray_m /
        direction_norm;

    const double dn =
        ray_n /
        direction_norm;

    const double dl =
        ray_l /
        direction_norm;

    // --------------------------------------------------------
    // Ray-plane intersection
    //
    // Detector reference plane:
    //
    // (P - C) . axis = 0
    //
    // P = ray_origin + t * ray_direction
    // --------------------------------------------------------

    const double denominator =
        Dot(
            dm,
            dn,
            dl,
            axis_x_,
            axis_y_,
            axis_z_
        );

    if(std::fabs(denominator) <= EPS)
    {
        return DETECTOR_WRONG_DIRECTION;
    }

    const double numerator =
        Dot(
            center_x_ - ray_x,
            center_y_ - ray_y,
            center_z_ - ray_z,
            axis_x_,
            axis_y_,
            axis_z_
        );

    const double t =
        numerator /
        denominator;

    if(t <= EPS)
    {
        return DETECTOR_WRONG_DIRECTION;
    }

    hit_x =
        ray_x +
        t * dm;

    hit_y =
        ray_y +
        t * dn;

    hit_z =
        ray_z +
        t * dl;

    // --------------------------------------------------------
    // Convert hit point to detector-local coordinates
    // --------------------------------------------------------

    const double dx =
        hit_x -
        center_x_;

    const double dy =
        hit_y -
        center_y_;

    const double dz =
        hit_z -
        center_z_;

    const double local_x =
        Dot(
            dx,
            dy,
            dz,
            ux_x_,
            ux_y_,
            ux_z_
        );

    const double local_y =
        Dot(
            dx,
            dy,
            dz,
            uy_x_,
            uy_y_,
            uy_z_
        );

    // --------------------------------------------------------
    // Mechanical housing
    //
    // Outside the detector silhouette:
    // photon does not interact with CCD and may continue.
    // --------------------------------------------------------

    if(!IsInsideBody(
        local_x,
        local_y
    ))
    {
        return DETECTOR_OUTSIDE_BODY;
    }

    // --------------------------------------------------------
    // Photon hits housing rather than the physical hole.
    //
    // This photon is mechanically blocked.
    // --------------------------------------------------------

    if(!IsInsidePhysicalHole(
        local_x,
        local_y
    ))
    {
        return DETECTOR_BLOCKED_BODY;
    }

    // --------------------------------------------------------
    // Front-side incidence
    //
    // axis points from detector toward the object being viewed.
    // Therefore incoming photons accepted by the camera travel
    // approximately opposite to axis.
    // --------------------------------------------------------

    const double forward_component =
        -denominator;

    if(forward_component <= EPS)
    {
        return DETECTOR_WRONG_DIRECTION;
    }

    // --------------------------------------------------------
    // FOV
    //
    // Viewing direction is opposite to photon propagation.
    // --------------------------------------------------------

    const double view_x =
        -Dot(
            dm,
            dn,
            dl,
            ux_x_,
            ux_y_,
            ux_z_
        );

    const double view_y =
        -Dot(
            dm,
            dn,
            dl,
            uy_x_,
            uy_y_,
            uy_z_
        );

    const double view_z =
        -Dot(
            dm,
            dn,
            dl,
            axis_x_,
            axis_y_,
            axis_z_
        );

    if(view_z <= EPS)
    {
        return DETECTOR_WRONG_DIRECTION;
    }

    const double horizontal_angle =
        std::atan2(
            std::fabs(view_x),
            view_z
        );

    const double vertical_angle =
        std::atan2(
            std::fabs(view_y),
            view_z
        );

    if(horizontal_angle >
       horizontal_half_fov_rad_ + EPS)
    {
        return DETECTOR_OUTSIDE_FOV;
    }

    if(vertical_angle >
       vertical_half_fov_rad_ + EPS)
    {
        return DETECTOR_OUTSIDE_FOV;
    }

    // --------------------------------------------------------
    // Effective optical aperture
    //
    // The physical opening is 21 mm, but the optical aperture
    // is f/N = 35/1.8 = 19.444... mm.
    // --------------------------------------------------------

    if(!IsInsideEffectiveAperture(
        local_x,
        local_y
    ))
    {
        return DETECTOR_MISS_APERTURE;
    }

    return DETECTOR_ACCEPTED;
}

// ============================================================
// Position getters
// ============================================================

double Detector::GetCenterX() const
{
    return center_x_;
}

double Detector::GetCenterY() const
{
    return center_y_;
}

double Detector::GetCenterZ() const
{
    return center_z_;
}

// ============================================================
// LookAt getters
// ============================================================

double Detector::GetLookAtX() const
{
    return lookat_x_;
}

double Detector::GetLookAtY() const
{
    return lookat_y_;
}

double Detector::GetLookAtZ() const
{
    return lookat_z_;
}

// ============================================================
// Optical getters
// ============================================================

double Detector::GetFocalLength() const
{
    return focal_length_;
}

double Detector::GetFNumber() const
{
    return f_number_;
}

double Detector::GetHorizontalFOV() const
{
    return horizontal_fov_deg_;
}

double Detector::GetVerticalFOV() const
{
    return vertical_fov_deg_;
}

double Detector::GetEffectiveApertureDiameter() const
{
    return effective_aperture_diameter_;
}

double Detector::GetEffectiveApertureRadius() const
{
    return effective_aperture_radius_;
}

// ============================================================
// Disk getters
// ============================================================

double Detector::GetDiskDiameter() const
{
    return disk_diameter_;
}

double Detector::GetDiskRadius() const
{
    return disk_radius_;
}

// ============================================================
// Base getters
// ============================================================

double Detector::GetBaseWidth() const
{
    return base_width_;
}

double Detector::GetBaseHeight() const
{
    return base_height_;
}

double Detector::GetBaseLeftOffset() const
{
    return base_left_offset_;
}

double Detector::GetBaseLeft() const
{
    return base_left_;
}

double Detector::GetBaseRight() const
{
    return base_right_;
}

double Detector::GetBaseTop() const
{
    return base_top_;
}

double Detector::GetBaseBottom() const
{
    return base_bottom_;
}

// ============================================================
// Hole getters
// ============================================================

double Detector::GetHoleDiameter() const
{
    return hole_diameter_;
}

double Detector::GetHoleRadius() const
{
    return hole_radius_;
}

double Detector::GetHoleEdgeDistance() const
{
    return hole_edge_distance_;
}

double Detector::GetHoleCenterX() const
{
    return hole_center_x_;
}

double Detector::GetHoleCenterY() const
{
    return hole_center_y_;
}

// ============================================================
// Geometry state
// ============================================================

bool Detector::IsGeometryValid() const
{
    return geometry_valid_;
}
