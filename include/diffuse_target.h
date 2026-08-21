#ifndef DIFFUSE_TARGET_H
#define DIFFUSE_TARGET_H

class DiffuseTarget
{
public:
    DiffuseTarget(
        double center_x,
        double center_y,
        double center_z,
        double width,
        double height,
        double reflectivity
    );

    bool IsInside(
        double x,
        double y
    ) const;

    bool Reflect(
        double &m,
        double &n,
        double &l
    ) const;

    double GetCenterX() const { return center_x_; }
    double GetCenterY() const { return center_y_; }
    double GetCenterZ() const { return center_z_; }
    double GetWidth() const { return width_; }
    double GetHeight() const { return height_; }
    double GetReflectivity() const { return reflectivity_; }

private:
    double center_x_;
    double center_y_;
    double center_z_;
    double width_;
    double height_;
    double reflectivity_;
};

#endif
