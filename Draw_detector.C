#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "TBox.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TGraph.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMarker.h"
#include "TPad.h"
#include "TPaveText.h"
#include "TStyle.h"

#include "detector.h"
#include "telescopeparameters.h"

// ============================================================
// User adjustable parameters
// ============================================================

// Detector center
static const double CCD_X = -200.0;  // mm
static const double CCD_Y = -500.0;  // mm
static const double CCD_Z =   20.0;  // mm

// Detector pointing
static const double CCD_LOOKAT_X = -200.0;  // mm
static const double CCD_LOOKAT_Y = -500.0;  // mm
static const double CCD_LOOKAT_Z = 2870.0;  // mm

// Detector optics
static const double CCD_FOCAL_LENGTH = 35.0;    // mm
static const double CCD_F_NUMBER = 1.8;
static const double CCD_HORIZONTAL_FOV = 14.33; // deg
static const double CCD_VERTICAL_FOV = 10.77;   // deg

// Detector mechanical geometry
static const double CCD_DISK_DIAMETER = 170.0;     // mm
static const double CCD_BASE_WIDTH = 140.0;        // mm
static const double CCD_BASE_HEIGHT = 15.0;        // mm
static const double CCD_BASE_LEFT_OFFSET = 6.0;    // mm
static const double CCD_HOLE_DIAMETER = 21.0;      // mm
static const double CCD_HOLE_EDGE_DISTANCE = 13.0; // mm

// Parallel-light source
static const double SOURCE_X = 0.0;        // mm
static const double SOURCE_Y = 0.0;        // mm
static const double SOURCE_Z = 5800.0;     // mm
static const double SOURCE_RADIUS = 2000.0;// mm

// Side-view drawing ranges
static const double DRAW_Z_MIN = -200.0;  // mm
static const double DRAW_Z_MAX = 6200.0;  // mm
static const double DRAW_X_MIN = -2600.0; // mm
static const double DRAW_X_MAX =  2600.0; // mm
static const double DRAW_Y_MIN = -2600.0; // mm
static const double DRAW_Y_MAX =  2600.0; // mm

// Output
static const char *OUTPUT_PNG = "detector_fov_geometry.png";
static const char *OUTPUT_PDF = "detector_fov_geometry.pdf";

// ============================================================
// Vector mathematics
// ============================================================

struct Vec3
{
    double x;
    double y;
    double z;
};

static const double PI = 3.14159265358979323846;
static const double RAD_TO_DEG = 180.0 / PI;

static double Dot(const Vec3 &a, const Vec3 &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static double Norm(const Vec3 &v)
{
    return std::sqrt(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z
    );
}

static Vec3 Normalize(const Vec3 &v)
{
    const double n = Norm(v);

    if(n <= 1.0e-12)
    {
        return {0.0, 0.0, 0.0};
    }

    return {
        v.x / n,
        v.y / n,
        v.z / n
    };
}

static Vec3 Cross(const Vec3 &a, const Vec3 &b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static Vec3 Add(const Vec3 &a, const Vec3 &b)
{
    return {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

static Vec3 Scale(const Vec3 &v, double s)
{
    return {
        v.x * s,
        v.y * s,
        v.z * s
    };
}

// ============================================================
// Detector local basis
//
// axis : optical axis
// ux   : detector horizontal axis
// uy   : detector vertical axis
//
// This follows the same convention used by Detector.
// ============================================================

static bool BuildDetectorBasis(
    Vec3 &axis,
    Vec3 &ux,
    Vec3 &uy)
{
    axis = Normalize({
        CCD_LOOKAT_X - CCD_X,
        CCD_LOOKAT_Y - CCD_Y,
        CCD_LOOKAT_Z - CCD_Z
    });

    if(Norm(axis) <= 1.0e-12)
    {
        return false;
    }

    Vec3 reference = {0.0, 0.0, 1.0};

    if(std::fabs(axis.z) > 0.99)
    {
        reference = {0.0, 1.0, 0.0};
    }

    ux = Normalize(
        Cross(reference, axis)
    );

    if(Norm(ux) <= 1.0e-12)
    {
        return false;
    }

    uy = Normalize(
        Cross(axis, ux)
    );

    return Norm(uy) > 1.0e-12;
}

// ============================================================
// FOV boundary direction
// ============================================================

static Vec3 MakeFOVRay(
    const Vec3 &axis,
    const Vec3 &ux,
    const Vec3 &uy,
    double sx,
    double sy)
{
    const double tan_h =
        std::tan(
            CCD_HORIZONTAL_FOV *
            PI / 360.0
        );

    const double tan_v =
        std::tan(
            CCD_VERTICAL_FOV *
            PI / 360.0
        );

    Vec3 direction = axis;

    direction = Add(
        direction,
        Scale(
            ux,
            sx * tan_h
        )
    );

    direction = Add(
        direction,
        Scale(
            uy,
            sy * tan_v
        )
    );

    return Normalize(direction);
}

// ============================================================
// Ray position at specified Z
// ============================================================

static bool PositionAtZ(
    const Vec3 &origin,
    const Vec3 &direction,
    double z,
    Vec3 &position)
{
    if(std::fabs(direction.z) <= 1.0e-12)
    {
        return false;
    }

    const double t =
        (z - origin.z) /
        direction.z;

    if(t < 0.0)
    {
        return false;
    }

    position = {
        origin.x + t * direction.x,
        origin.y + t * direction.y,
        z
    };

    return true;
}

// ============================================================
// FOV envelope at one Z plane
// ============================================================

static bool GetFOVEnvelope(
    const Vec3 &origin,
    const Vec3 rays[4],
    double z,
    double &xmin,
    double &xmax,
    double &ymin,
    double &ymax)
{
    xmin =  1.0e100;
    xmax = -1.0e100;
    ymin =  1.0e100;
    ymax = -1.0e100;

    bool valid = false;

    for(int i = 0; i < 4; ++i)
    {
        Vec3 p;

        if(!PositionAtZ(
            origin,
            rays[i],
            z,
            p))
        {
            continue;
        }

        xmin = std::min(xmin, p.x);
        xmax = std::max(xmax, p.x);
        ymin = std::min(ymin, p.y);
        ymax = std::max(ymax, p.y);

        valid = true;
    }

    return valid;
}

// ============================================================
// Convert a 3D point to camera angular coordinates
//
// horizontal_angle:
// angle with respect to detector local X
//
// vertical_angle:
// angle with respect to detector local Y
// ============================================================

static bool ProjectToCamera(
    const Vec3 &point,
    const Vec3 &origin,
    const Vec3 &axis,
    const Vec3 &ux,
    const Vec3 &uy,
    double &horizontal_angle,
    double &vertical_angle)
{
    const Vec3 delta = {
        point.x - origin.x,
        point.y - origin.y,
        point.z - origin.z
    };

    const double local_x =
        Dot(delta, ux);

    const double local_y =
        Dot(delta, uy);

    const double local_z =
        Dot(delta, axis);

    if(local_z <= 0.0)
    {
        return false;
    }

    horizontal_angle =
        std::atan2(
            local_x,
            local_z
        ) * RAD_TO_DEG;

    vertical_angle =
        std::atan2(
            local_y,
            local_z
        ) * RAD_TO_DEG;

    return true;
}

// ============================================================
// Drawing helpers
// ============================================================

static TLine *DrawLine(
    double x1,
    double y1,
    double x2,
    double y2,
    int color,
    int width,
    int style = 1)
{
    TLine *line =
        new TLine(
            x1,
            y1,
            x2,
            y2
        );

    line->SetLineColor(color);
    line->SetLineWidth(width);
    line->SetLineStyle(style);
    line->Draw("same");

    return line;
}

static TBox *DrawBox(
    double x1,
    double y1,
    double x2,
    double y2,
    int color,
    double alpha,
    int line_color)
{
    TBox *box =
        new TBox(
            x1,
            y1,
            x2,
            y2
        );

    box->SetFillColorAlpha(
        color,
        alpha
    );

    box->SetLineColor(
        line_color
    );

    box->SetLineWidth(2);
    box->Draw("same");

    return box;
}

// ============================================================
// X-Z view
// ============================================================

static void DrawXZ(
    TPad *pad,
    const Vec3 &origin,
    const Vec3 &axis,
    const Vec3 rays[4])
{
    pad->cd();
    pad->SetLeftMargin(0.13);
    pad->SetRightMargin(0.04);
    pad->SetBottomMargin(0.12);
    pad->SetTopMargin(0.10);
    pad->SetGrid();

    TH2D *frame =
        new TH2D(
            "frame_xz",
            "Horizontal field of view;Z (mm);X (mm)",
            10,
            DRAW_Z_MIN,
            DRAW_Z_MAX,
            10,
            DRAW_X_MIN,
            DRAW_X_MAX
        );

    frame->SetStats(0);
    frame->GetXaxis()->SetLabelFont(132);
    frame->GetYaxis()->SetLabelFont(132);
    frame->GetXaxis()->SetTitleFont(132);
    frame->GetYaxis()->SetTitleFont(132);
    frame->GetXaxis()->SetLabelSize(0.034);
    frame->GetYaxis()->SetLabelSize(0.034);
    frame->GetXaxis()->SetTitleSize(0.042);
    frame->GetYaxis()->SetTitleSize(0.042);
    frame->GetXaxis()->SetTitleOffset(1.10);
    frame->GetYaxis()->SetTitleOffset(1.25);
    frame->Draw();

    const int detector_color =
        TColor::GetColor("#8E44AD");

    const int fov_color =
        TColor::GetColor("#E74C3C");

    const int axis_color =
        TColor::GetColor("#34495E");

    const int camera_color =
        TColor::GetColor("#3498DB");

    const int source_color =
        TColor::GetColor("#F39C12");

    const int mirror_color =
        TColor::GetColor("#7F8C8D");

    // Mirror reference plane
    DrawLine(
        0.0,
        DRAW_X_MIN,
        0.0,
        DRAW_X_MAX,
        mirror_color,
        2,
        2
    );

    // Focal-plane SiPM camera
    DrawBox(
        FOCUS - 15.0,
        -CLUSTER_X / 2.0,
        FOCUS + 15.0,
         CLUSTER_X / 2.0,
        camera_color,
        0.35,
        camera_color
    );

    // Parallel-light source
    DrawBox(
        SOURCE_Z - 25.0,
        SOURCE_X - SOURCE_RADIUS,
        SOURCE_Z + 25.0,
        SOURCE_X + SOURCE_RADIUS,
        source_color,
        0.20,
        source_color
    );

    // Detector
    DrawBox(
        CCD_Z - 30.0,
        CCD_X - CCD_DISK_DIAMETER / 2.0,
        CCD_Z + 30.0,
        CCD_X + CCD_DISK_DIAMETER / 2.0,
        detector_color,
        0.55,
        detector_color
    );

    // Optical axis
    Vec3 axis_end;

    if(PositionAtZ(
        origin,
        axis,
        DRAW_Z_MAX,
        axis_end))
    {
        DrawLine(
            origin.z,
            origin.x,
            axis_end.z,
            axis_end.x,
            axis_color,
            3
        );
    }

    // FOV envelope
    double xmin = 0.0;
    double xmax = 0.0;
    double ymin = 0.0;
    double ymax = 0.0;

    if(GetFOVEnvelope(
        origin,
        rays,
        DRAW_Z_MAX,
        xmin,
        xmax,
        ymin,
        ymax))
    {
        double z_points[4] = {
            origin.z,
            DRAW_Z_MAX,
            DRAW_Z_MAX,
            origin.z
        };

        double x_points[4] = {
            origin.x,
            xmin,
            xmax,
            origin.x
        };

        TGraph *fill =
            new TGraph(
                4,
                z_points,
                x_points
            );

        fill->SetFillColorAlpha(
            fov_color,
            0.10
        );

        fill->SetLineColor(
            fov_color
        );

        fill->Draw("F same");

        DrawLine(
            origin.z,
            origin.x,
            DRAW_Z_MAX,
            xmin,
            fov_color,
            2,
            2
        );

        DrawLine(
            origin.z,
            origin.x,
            DRAW_Z_MAX,
            xmax,
            fov_color,
            2,
            2
        );
    }

    TMarker *detector =
        new TMarker(
            CCD_Z,
            CCD_X,
            20
        );

    detector->SetMarkerColor(
        detector_color
    );

    detector->SetMarkerSize(1.2);
    detector->Draw("same");

    TLegend *legend =
        new TLegend(
            0.16,
            0.69,
            0.43,
            0.88
        );

    legend->SetBorderSize(1);
    legend->SetFillColor(kWhite);
    legend->SetTextFont(132);
    legend->SetTextSize(0.025);

    TBox *det_leg = new TBox();
    det_leg->SetFillColor(detector_color);

    TLine *axis_leg = new TLine();
    axis_leg->SetLineColor(axis_color);
    axis_leg->SetLineWidth(3);

    TLine *fov_leg = new TLine();
    fov_leg->SetLineColor(fov_color);
    fov_leg->SetLineStyle(2);

    TLine *mirror_leg = new TLine();
    mirror_leg->SetLineColor(mirror_color);
    mirror_leg->SetLineStyle(2);

    TBox *camera_leg = new TBox();
    camera_leg->SetFillColor(camera_color);

    TBox *source_leg = new TBox();
    source_leg->SetFillColor(source_color);

    legend->AddEntry(det_leg, "Detector", "f");
    legend->AddEntry(axis_leg, "Optical axis", "l");
    legend->AddEntry(fov_leg, "FOV boundary", "l");
    legend->AddEntry(mirror_leg, "Mirror plane", "l");
    legend->AddEntry(camera_leg, "SiPM camera", "f");
    legend->AddEntry(source_leg, "Parallel source", "f");
    legend->Draw();
}

// ============================================================
// Y-Z view
// ============================================================

static void DrawYZ(
    TPad *pad,
    const Vec3 &origin,
    const Vec3 &axis,
    const Vec3 rays[4])
{
    pad->cd();
    pad->SetLeftMargin(0.13);
    pad->SetRightMargin(0.04);
    pad->SetBottomMargin(0.12);
    pad->SetTopMargin(0.10);
    pad->SetGrid();

    TH2D *frame =
        new TH2D(
            "frame_yz",
            "Vertical field of view;Z (mm);Y (mm)",
            10,
            DRAW_Z_MIN,
            DRAW_Z_MAX,
            10,
            DRAW_Y_MIN,
            DRAW_Y_MAX
        );

    frame->SetStats(0);
    frame->GetXaxis()->SetLabelFont(132);
    frame->GetYaxis()->SetLabelFont(132);
    frame->GetXaxis()->SetTitleFont(132);
    frame->GetYaxis()->SetTitleFont(132);
    frame->GetXaxis()->SetLabelSize(0.034);
    frame->GetYaxis()->SetLabelSize(0.034);
    frame->GetXaxis()->SetTitleSize(0.042);
    frame->GetYaxis()->SetTitleSize(0.042);
    frame->GetXaxis()->SetTitleOffset(1.10);
    frame->GetYaxis()->SetTitleOffset(1.25);
    frame->Draw();

    const int detector_color =
        TColor::GetColor("#8E44AD");

    const int fov_color =
        TColor::GetColor("#E74C3C");

    const int axis_color =
        TColor::GetColor("#34495E");

    const int camera_color =
        TColor::GetColor("#3498DB");

    const int source_color =
        TColor::GetColor("#F39C12");

    const int mirror_color =
        TColor::GetColor("#7F8C8D");

    // Mirror plane
    DrawLine(
        0.0,
        DRAW_Y_MIN,
        0.0,
        DRAW_Y_MAX,
        mirror_color,
        2,
        2
    );

    // Focal-plane SiPM camera
    DrawBox(
        FOCUS - 15.0,
        -CLUSTER_Y / 2.0,
        FOCUS + 15.0,
         CLUSTER_Y / 2.0,
        camera_color,
        0.35,
        camera_color
    );

    // Parallel source
    DrawBox(
        SOURCE_Z - 25.0,
        SOURCE_Y - SOURCE_RADIUS,
        SOURCE_Z + 25.0,
        SOURCE_Y + SOURCE_RADIUS,
        source_color,
        0.20,
        source_color
    );

    // Detector
    DrawBox(
        CCD_Z - 30.0,
        CCD_Y - CCD_DISK_DIAMETER / 2.0,
        CCD_Z + 30.0,
        CCD_Y + CCD_DISK_DIAMETER / 2.0,
        detector_color,
        0.55,
        detector_color
    );

    // Optical axis
    Vec3 axis_end;

    if(PositionAtZ(
        origin,
        axis,
        DRAW_Z_MAX,
        axis_end))
    {
        DrawLine(
            origin.z,
            origin.y,
            axis_end.z,
            axis_end.y,
            axis_color,
            3
        );
    }

    // FOV envelope
    double xmin = 0.0;
    double xmax = 0.0;
    double ymin = 0.0;
    double ymax = 0.0;

    if(GetFOVEnvelope(
        origin,
        rays,
        DRAW_Z_MAX,
        xmin,
        xmax,
        ymin,
        ymax))
    {
        double z_points[4] = {
            origin.z,
            DRAW_Z_MAX,
            DRAW_Z_MAX,
            origin.z
        };

        double y_points[4] = {
            origin.y,
            ymin,
            ymax,
            origin.y
        };

        TGraph *fill =
            new TGraph(
                4,
                z_points,
                y_points
            );

        fill->SetFillColorAlpha(
            fov_color,
            0.10
        );

        fill->SetLineColor(
            fov_color
        );

        fill->Draw("F same");

        DrawLine(
            origin.z,
            origin.y,
            DRAW_Z_MAX,
            ymin,
            fov_color,
            2,
            2
        );

        DrawLine(
            origin.z,
            origin.y,
            DRAW_Z_MAX,
            ymax,
            fov_color,
            2,
            2
        );
    }

    TMarker *detector =
        new TMarker(
            CCD_Z,
            CCD_Y,
            20
        );

    detector->SetMarkerColor(
        detector_color
    );

    detector->SetMarkerSize(1.2);
    detector->Draw("same");
}

// ============================================================
// Camera-view projection
//
// Horizontal axis = detector horizontal viewing angle
// Vertical axis   = detector vertical viewing angle
//
// The plot corresponds directly to what lies inside the
// 14.33 deg x 10.77 deg detector field of view.
// ============================================================

static void DrawCameraView(
    TPad *pad,
    const Vec3 &origin,
    const Vec3 &axis,
    const Vec3 &ux,
    const Vec3 &uy)
{
    pad->cd();
    pad->SetLeftMargin(0.13);
    pad->SetRightMargin(0.05);
    pad->SetBottomMargin(0.13);
    pad->SetTopMargin(0.10);
    pad->SetGrid();

    const double half_h =
        CCD_HORIZONTAL_FOV / 2.0;

    const double half_v =
        CCD_VERTICAL_FOV / 2.0;

    TH2D *frame =
        new TH2D(
            "frame_camera",
            "Detector camera-view projection;Horizontal angle (deg);Vertical angle (deg)",
            10,
            -half_h,
             half_h,
            10,
            -half_v,
             half_v
        );

    frame->SetStats(0);
    frame->GetXaxis()->SetLabelFont(132);
    frame->GetYaxis()->SetLabelFont(132);
    frame->GetXaxis()->SetTitleFont(132);
    frame->GetYaxis()->SetTitleFont(132);
    frame->GetXaxis()->SetLabelSize(0.034);
    frame->GetYaxis()->SetLabelSize(0.034);
    frame->GetXaxis()->SetTitleSize(0.040);
    frame->GetYaxis()->SetTitleSize(0.040);
    frame->GetXaxis()->SetTitleOffset(1.15);
    frame->GetYaxis()->SetTitleOffset(1.30);
    frame->Draw();

    const int camera_color =
        TColor::GetColor("#3498DB");

    const int source_color =
        TColor::GetColor("#F39C12");

    const int axis_color =
        TColor::GetColor("#34495E");

    // --------------------------------------------------------
    // Draw FOV outer boundary
    // --------------------------------------------------------

    TBox *fov_box =
        new TBox(
            -half_h,
            -half_v,
             half_h,
             half_v
        );

    fov_box->SetFillStyle(0);
    fov_box->SetLineColor(kBlack);
    fov_box->SetLineWidth(2);
    fov_box->Draw("same");

    // --------------------------------------------------------
    // Optical-axis center
    // --------------------------------------------------------

    TMarker *axis_marker =
        new TMarker(
            0.0,
            0.0,
            5
        );

    axis_marker->SetMarkerColor(
        axis_color
    );

    axis_marker->SetMarkerSize(1.3);
    axis_marker->Draw("same");

    // ========================================================
    // Project focal-plane SiPM camera
    //
    // Camera physical dimensions:
    // CLUSTER_X x CLUSTER_Y
    //
    // Center:
    // (0, 0, FOCUS)
    // ========================================================

    const Vec3 camera_corners[4] = {
        {
            -CLUSTER_X / 2.0,
            -CLUSTER_Y / 2.0,
            FOCUS
        },
        {
             CLUSTER_X / 2.0,
            -CLUSTER_Y / 2.0,
            FOCUS
        },
        {
             CLUSTER_X / 2.0,
             CLUSTER_Y / 2.0,
            FOCUS
        },
        {
            -CLUSTER_X / 2.0,
             CLUSTER_Y / 2.0,
            FOCUS
        }
    };

    double camera_h[5];
    double camera_v[5];

    bool camera_valid = true;

    for(int i = 0; i < 4; ++i)
    {
        if(!ProjectToCamera(
            camera_corners[i],
            origin,
            axis,
            ux,
            uy,
            camera_h[i],
            camera_v[i]))
        {
            camera_valid = false;
        }
    }

    camera_h[4] = camera_h[0];
    camera_v[4] = camera_v[0];

    if(camera_valid)
    {
        TGraph *camera_projection =
            new TGraph(
                5,
                camera_h,
                camera_v
            );

        camera_projection->SetFillColorAlpha(
            camera_color,
            0.35
        );

        camera_projection->SetLineColor(
            camera_color
        );

        camera_projection->SetLineWidth(2);
        camera_projection->Draw("F same");
        camera_projection->Draw("L same");
    }

    // ========================================================
    // Project parallel-light source disk
    //
    // Source:
    // center = (0,0,5800)
    // radius = 2000 mm
    // ========================================================

    const int source_points = 180;

    std::vector<double> source_h(
        source_points + 1
    );

    std::vector<double> source_v(
        source_points + 1
    );

    bool source_valid = true;

    for(int i = 0; i <= source_points; ++i)
    {
        const double phi =
            2.0 * PI *
            static_cast<double>(i) /
            static_cast<double>(source_points);

        const Vec3 source_point = {
            SOURCE_X +
                SOURCE_RADIUS *
                std::cos(phi),

            SOURCE_Y +
                SOURCE_RADIUS *
                std::sin(phi),

            SOURCE_Z
        };

        if(!ProjectToCamera(
            source_point,
            origin,
            axis,
            ux,
            uy,
            source_h[i],
            source_v[i]))
        {
            source_valid = false;
        }
    }

    if(source_valid)
    {
        TGraph *source_projection =
            new TGraph(
                source_points + 1,
                &source_h[0],
                &source_v[0]
            );

        source_projection->SetFillColorAlpha(
            source_color,
            0.12
        );

        source_projection->SetLineColor(
            source_color
        );

        source_projection->SetLineWidth(2);
        source_projection->SetLineStyle(2);
        source_projection->Draw("F same");
        source_projection->Draw("L same");
    }

    // --------------------------------------------------------
    // Redraw optical-axis center above filled objects
    // --------------------------------------------------------

    axis_marker->Draw("same");

    // ========================================================
    // Legend
    // ========================================================

    TLegend *legend =
        new TLegend(
            0.16,
            0.76,
            0.45,
            0.89
        );

    legend->SetBorderSize(1);
    legend->SetFillColor(kWhite);
    legend->SetFillStyle(1001);
    legend->SetTextFont(132);
    legend->SetTextSize(0.025);

    TBox *camera_leg = new TBox();
    camera_leg->SetFillColor(camera_color);
    camera_leg->SetLineColor(camera_color);

    TBox *source_leg = new TBox();
    source_leg->SetFillColor(source_color);
    source_leg->SetLineColor(source_color);

    TMarker *axis_leg =
        new TMarker(
            0.0,
            0.0,
            5
        );

    axis_leg->SetMarkerColor(axis_color);

    legend->AddEntry(
        camera_leg,
        "SiPM camera",
        "f"
    );

    legend->AddEntry(
        source_leg,
        "Parallel source",
        "f"
    );

    legend->AddEntry(
        axis_leg,
        "Optical axis",
        "p"
    );

    legend->Draw();

    // ========================================================
    // Information box
    // ========================================================

    TPaveText *info =
        new TPaveText(
            0.55,
            0.76,
            0.92,
            0.89,
            "NDC"
        );

    info->SetFillColor(kWhite);
    info->SetBorderSize(1);
    info->SetTextFont(132);
    info->SetTextAlign(12);
    info->SetTextSize(0.022);

    info->AddText(
        Form(
            "FOV: %.2f x %.2f deg",
            CCD_HORIZONTAL_FOV,
            CCD_VERTICAL_FOV
        )
    );

    info->AddText(
        Form(
            "SiPM: %.1f x %.1f mm",
            CLUSTER_X,
            CLUSTER_Y
        )
    );

    info->AddText(
        Form(
            "Source radius: %.0f mm",
            SOURCE_RADIUS
        )
    );

    info->Draw();
}

// ============================================================
// Main
// ============================================================

int main()
{
    gStyle->SetOptStat(0);
    gStyle->SetCanvasColor(kWhite);
    gStyle->SetPadColor(kWhite);
    gStyle->SetTextFont(132);
    gStyle->SetLabelFont(132, "XYZ");
    gStyle->SetTitleFont(132, "XYZ");
    gStyle->SetTitleFont(132, "");

    // ========================================================
    // Instantiate Detector
    // ========================================================

    Detector detector(
        CCD_X,
        CCD_Y,
        CCD_Z,
        CCD_FOCAL_LENGTH,
        CCD_F_NUMBER,
        CCD_HORIZONTAL_FOV,
        CCD_VERTICAL_FOV,
        CCD_DISK_DIAMETER,
        CCD_BASE_WIDTH,
        CCD_BASE_HEIGHT,
        CCD_BASE_LEFT_OFFSET,
        CCD_HOLE_DIAMETER,
        CCD_HOLE_EDGE_DISTANCE
    );

    if(!detector.LookAt(
        CCD_LOOKAT_X,
        CCD_LOOKAT_Y,
        CCD_LOOKAT_Z))
    {
        std::cerr
            << "ERROR: invalid detector geometry or LookAt direction."
            << std::endl;

        return 1;
    }

    // ========================================================
    // Detector basis
    // ========================================================

    Vec3 axis;
    Vec3 ux;
    Vec3 uy;

    if(!BuildDetectorBasis(
        axis,
        ux,
        uy))
    {
        std::cerr
            << "ERROR: failed to build detector local basis."
            << std::endl;

        return 1;
    }

    const Vec3 origin = {
        CCD_X,
        CCD_Y,
        CCD_Z
    };

    Vec3 fov_rays[4];

    fov_rays[0] =
        MakeFOVRay(
            axis,
            ux,
            uy,
            -1.0,
            -1.0
        );

    fov_rays[1] =
        MakeFOVRay(
            axis,
            ux,
            uy,
            -1.0,
            +1.0
        );

    fov_rays[2] =
        MakeFOVRay(
            axis,
            ux,
            uy,
            +1.0,
            -1.0
        );

    fov_rays[3] =
        MakeFOVRay(
            axis,
            ux,
            uy,
            +1.0,
            +1.0
        );

    // ========================================================
    // Numerical FOV footprint
    // ========================================================

    double focal_xmin = 0.0;
    double focal_xmax = 0.0;
    double focal_ymin = 0.0;
    double focal_ymax = 0.0;

    GetFOVEnvelope(
        origin,
        fov_rays,
        FOCUS,
        focal_xmin,
        focal_xmax,
        focal_ymin,
        focal_ymax
    );

    double source_xmin = 0.0;
    double source_xmax = 0.0;
    double source_ymin = 0.0;
    double source_ymax = 0.0;

    GetFOVEnvelope(
        origin,
        fov_rays,
        SOURCE_Z,
        source_xmin,
        source_xmax,
        source_ymin,
        source_ymax
    );

    // ========================================================
    // Canvas: three projections
    // ========================================================

    TCanvas *canvas =
        new TCanvas(
            "canvas",
            "Detector field of view",
            2100,
            720
        );

    canvas->Divide(
        3,
        1,
        0.005,
        0.005
    );

    DrawXZ(
        static_cast<TPad *>(
            canvas->cd(1)
        ),
        origin,
        axis,
        fov_rays
    );

    DrawYZ(
        static_cast<TPad *>(
            canvas->cd(2)
        ),
        origin,
        axis,
        fov_rays
    );

    DrawCameraView(
        static_cast<TPad *>(
            canvas->cd(3)
        ),
        origin,
        axis,
        ux,
        uy
    );

    canvas->Modified();
    canvas->Update();

    canvas->SaveAs(
        OUTPUT_PNG
    );

    canvas->SaveAs(
        OUTPUT_PDF
    );

    // ========================================================
    // Console output
    // ========================================================

    std::cout
        << "============================================================\n";

    std::cout
        << "Detector field-of-view geometry generated\n";

    std::cout
        << "Detector center : ("
        << CCD_X << ", "
        << CCD_Y << ", "
        << CCD_Z << ") mm\n";

    std::cout
        << "Detector LookAt : ("
        << CCD_LOOKAT_X << ", "
        << CCD_LOOKAT_Y << ", "
        << CCD_LOOKAT_Z << ") mm\n";

    std::cout
        << "Optical axis : ("
        << axis.x << ", "
        << axis.y << ", "
        << axis.z << ")\n";

    std::cout
        << "Horizontal FOV : "
        << CCD_HORIZONTAL_FOV
        << " deg\n";

    std::cout
        << "Vertical FOV : "
        << CCD_VERTICAL_FOV
        << " deg\n";

    std::cout
        << "SiPM camera : "
        << CLUSTER_X
        << " x "
        << CLUSTER_Y
        << " mm\n";

    std::cout
        << "SiPM camera Z : "
        << FOCUS
        << " mm\n";

    std::cout
        << "FOV at focal plane X : ["
        << focal_xmin << ", "
        << focal_xmax << "] mm\n";

    std::cout
        << "FOV at focal plane Y : ["
        << focal_ymin << ", "
        << focal_ymax << "] mm\n";

    std::cout
        << "Parallel source center : ("
        << SOURCE_X << ", "
        << SOURCE_Y << ", "
        << SOURCE_Z << ") mm\n";

    std::cout
        << "Parallel source radius : "
        << SOURCE_RADIUS
        << " mm\n";

    std::cout
        << "FOV at source plane X : ["
        << source_xmin << ", "
        << source_xmax << "] mm\n";

    std::cout
        << "FOV at source plane Y : ["
        << source_ymin << ", "
        << source_ymax << "] mm\n";

    std::cout
        << "PNG : "
        << OUTPUT_PNG
        << '\n';

    std::cout
        << "PDF : "
        << OUTPUT_PDF
        << '\n';

    std::cout
        << "============================================================\n";

    return 0;
}
