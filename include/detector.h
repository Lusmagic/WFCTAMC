#ifndef DETECTOR_H
#define DETECTOR_H

// ============================================================
// Detector ray-tracing status
// ============================================================

enum DetectorStatus
{
    DETECTOR_ACCEPTED = 1,
    DETECTOR_OUTSIDE_BODY = 0,
    DETECTOR_BLOCKED_BODY = -1,
    DETECTOR_MISS_APERTURE = -2,
    DETECTOR_OUTSIDE_FOV = -3,
    DETECTOR_INVALID_GEOMETRY = -4,
    DETECTOR_WRONG_DIRECTION = -5
};

// ============================================================
// Detector
// ============================================================

class Detector
{
public:
    Detector(
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
    );

    // Detector pointing
    bool LookAt(
        double target_x,
        double target_y,
        double target_z
    );

    // Ray-detector interaction
    int CheckRay(
        double ray_x,
        double ray_y,
        double ray_z,
        double ray_m,
        double ray_n,
        double ray_l,
        double &hit_x,
        double &hit_y,
        double &hit_z
    ) const;

    // Position
    double GetCenterX() const;
    double GetCenterY() const;
    double GetCenterZ() const;

    // LookAt target
    double GetLookAtX() const;
    double GetLookAtY() const;
    double GetLookAtZ() const;

    // Optical parameters
    double GetFocalLength() const;
    double GetFNumber() const;
    double GetHorizontalFOV() const;
    double GetVerticalFOV() const;
    double GetEffectiveApertureDiameter() const;
    double GetEffectiveApertureRadius() const;

    // Circular housing
    double GetDiskDiameter() const;
    double GetDiskRadius() const;

    // Base/support geometry
    double GetBaseWidth() const;
    double GetBaseHeight() const;
    double GetBaseLeftOffset() const;
    double GetBaseLeft() const;
    double GetBaseRight() const;
    double GetBaseTop() const;
    double GetBaseBottom() const;

    // Physical receiving hole
    double GetHoleDiameter() const;
    double GetHoleRadius() const;
    double GetHoleEdgeDistance() const;
    double GetHoleCenterX() const;
    double GetHoleCenterY() const;

    // Geometry state
    bool IsGeometryValid() const;

private:
    // Geometry initialization
    bool ValidateParameters() const;
    bool BuildLocalBasis();

    // Mechanical geometry
    bool IsInsideDisk(
        double local_x,
        double local_y
    ) const;

    bool IsInsideBase(
        double local_x,
        double local_y
    ) const;

    bool IsInsideBody(
        double local_x,
        double local_y
    ) const;

    // Optical opening
    bool IsInsidePhysicalHole(
        double local_x,
        double local_y
    ) const;

    bool IsInsideEffectiveAperture(
        double local_x,
        double local_y
    ) const;

    // Position
    double center_x_;
    double center_y_;
    double center_z_;

    // LookAt target
    double lookat_x_;
    double lookat_y_;
    double lookat_z_;

    // Optical axis
    double axis_x_;
    double axis_y_;
    double axis_z_;

    // Detector local X direction
    double ux_x_;
    double ux_y_;
    double ux_z_;

    // Detector local Y direction
    double uy_x_;
    double uy_y_;
    double uy_z_;

    // Optical parameters
    double focal_length_;
    double f_number_;
    double horizontal_fov_deg_;
    double vertical_fov_deg_;
    double horizontal_half_fov_rad_;
    double vertical_half_fov_rad_;
    double effective_aperture_diameter_;
    double effective_aperture_radius_;

    // Circular housing
    double disk_diameter_;
    double disk_radius_;

    // Base/support
    double base_width_;
    double base_height_;
    double base_left_offset_;
    double base_left_;
    double base_right_;
    double base_top_;
    double base_bottom_;

    // Physical receiving hole
    double hole_diameter_;
    double hole_radius_;
    double hole_edge_distance_;
    double hole_center_x_;
    double hole_center_y_;

    // Geometry state
    bool geometry_valid_;
};

#endif
