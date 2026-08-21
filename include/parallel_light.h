#ifndef PARALLEL_LIGHT_H
#define PARALLEL_LIGHT_H

class ParallelLight
{
public:
    ParallelLight(
        double center_x,
        double center_y,
        double center_z,
        double radius,
        double wavelength
    );

    void GenerateRay(
        double &x,
        double &y,
        double &z,
        double &m,
        double &n,
        double &l
    ) const;

    double GetCenterX() const { return center_x_; }
    double GetCenterY() const { return center_y_; }
    double GetCenterZ() const { return center_z_; }
    double GetRadius() const { return radius_; }
    double GetWavelength() const { return wavelength_; }

private:
    double center_x_;
    double center_y_;
    double center_z_;
    double radius_;
    double wavelength_;
};

#endif
