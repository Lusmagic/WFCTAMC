// ============================================================
// Draw_mirror.C
// WFCTA mirror obstruction projection map
//
// Projection:
// 3D obstruction geometry
//        |
//        v
// along PROJ_DIR
//        |
//        v
// mirror reference plane Z=PROJ_MIRROR_Z
//
// ============================================================

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "TBox.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TEllipse.h"
#include "TGraph.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMarker.h"
#include "TPaveText.h"
#include "TPolyLine.h"
#include "TStyle.h"

#include "wtelescope.h"
#include "telescopeparameters.h"

// ============================================================
// Drawing range
// ============================================================

static const double DRAW_X_MIN = -1500.0;
static const double DRAW_X_MAX =  1500.0;
static const double DRAW_Y_MIN = -1500.0;
static const double DRAW_Y_MAX =  1500.0;

static const int DRAW_NX = 600;
static const int DRAW_NY = 600;

// ============================================================
// Projection configuration
// ============================================================

// Incident parallel light direction
// Default: vertical incidence

static const double PROJ_DIR_X = 0.0;
static const double PROJ_DIR_Y = 0.0;
static const double PROJ_DIR_Z = -1.0;


// Mirror reference plane

static const double PROJ_MIRROR_Z = ZMIRROR;


// Source position for facet calculation

static const double SOURCE_Z_FOR_FACET = 5800.0;


// ============================================================
// Drawing switches
// ============================================================

static const bool DRAW_SIPM_CAMERA = true;
static const bool DRAW_DOOR = true;
static const bool DRAW_DUCT = true;
static const bool DRAW_DUCT_STAIR = true;
static const bool DRAW_LED_FRAME = true;
static const bool DRAW_NEW_CCD = true;


// ============================================================
// CCD geometry
// ============================================================

static const double CCD_X = -200.0;
static const double CCD_Y = -500.0;
static const double CCD_Z = 20.0;

static const double CCD_DISK_DIAMETER = 170.0;

static const double CCD_BASE_WIDTH = 140.0;
static const double CCD_BASE_HEIGHT = 15.0;

static const double CCD_BASE_LEFT_OFFSET = 6.0;

static const double CCD_HOLE_DIAMETER = 21.0;

static const double CCD_HOLE_EDGE_DISTANCE = 13.0;

static const double CCD_FOCAL_LENGTH = 35.0;

static const double CCD_F_NUMBER = 1.8;


// ============================================================
// Output
// ============================================================

static const char *OUTPUT_PNG =
    "mirror_obstruction_map.png";

static const char *OUTPUT_PDF =
    "mirror_obstruction_map.pdf";


// ============================================================
// Geometry structures
// ============================================================

struct Point2D
{
    double x;
    double y;
};


struct Point3D
{
    double x;
    double y;
    double z;
};


// ============================================================
// Normalize projection direction
// ============================================================

static bool NormalizeDirection(
    double &dx,
    double &dy,
    double &dz)
{
    const double norm =
        std::sqrt(
            dx * dx +
            dy * dy +
            dz * dz
        );

    if(norm < 1.0e-12)
    {
        return false;
    }

    dx /= norm;
    dy /= norm;
    dz /= norm;

    return true;
}


// ============================================================
// Project 3D point to mirror plane
// ============================================================

static bool ProjectToMirror(
    double x,
    double y,
    double z,
    double &xm,
    double &ym)
{
    double dx = PROJ_DIR_X;
    double dy = PROJ_DIR_Y;
    double dz = PROJ_DIR_Z;

    if(!NormalizeDirection(dx,dy,dz))
    {
        return false;
    }


    if(std::fabs(dz)<1.0e-12)
    {
        return false;
    }


    const double t =
        (PROJ_MIRROR_Z-z)/dz;


    xm =
        x + dx*t;

    ym =
        y + dy*t;


    return true;
}


// ============================================================
// Graph helper
// ============================================================

static TGraph *MakeClosedGraph(
    const std::vector<Point2D> &points)
{
    if(points.empty())
    {
        return nullptr;
    }


    const int n =
        static_cast<int>(points.size());


    TGraph *graph =
        new TGraph(n+1);


    for(int i=0;i<n;i++)
    {
        graph->SetPoint(
            i,
            points[i].x,
            points[i].y
        );
    }


    graph->SetPoint(
        n,
        points[0].x,
        points[0].y
    );


    return graph;
}


// ============================================================
// Graph style
// ============================================================

static void SetGraphStyle(
    TGraph *graph,
    int fill_color,
    double alpha,
    int line_color,
    int line_width,
    int line_style=1)
{
    graph->SetFillColorAlpha(
        fill_color,
        alpha
    );

    graph->SetLineColor(
        line_color
    );

    graph->SetLineWidth(
        line_width
    );

    graph->SetLineStyle(
        line_style
    );
}


// ============================================================
// Draw projected polygon
// ============================================================

static void DrawProjectedPolygon(
    const std::vector<Point3D> &points3d,
    int fill_color,
    double alpha,
    int line_color,
    int line_width,
    int line_style=1)
{
    std::vector<Point2D> points2d;


    for(size_t i=0;i<points3d.size();i++)
    {
        double x=0.0;
        double y=0.0;


        if(!ProjectToMirror(
            points3d[i].x,
            points3d[i].y,
            points3d[i].z,
            x,
            y))
        {
            return;
        }


        points2d.push_back(
            {x,y}
        );
    }


    TGraph *graph =
        MakeClosedGraph(points2d);


    if(graph==nullptr)
    {
        return;
    }


    SetGraphStyle(
        graph,
        fill_color,
        alpha,
        line_color,
        line_width,
        line_style
    );


    graph->Draw("F same");
    graph->Draw("L same");
}


// ============================================================
// Draw 3D box projection
// ============================================================

static void DrawProjectedBox3D(
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    double zmin,
    double zmax,
    int fill_color,
    double alpha,
    int line_color,
    int line_width,
    int line_style=1)
{
    std::vector<Point3D> box;


    box.push_back(
        {xmin,ymin,zmin}
    );

    box.push_back(
        {xmax,ymin,zmin}
    );

    box.push_back(
        {xmax,ymax,zmin}
    );

    box.push_back(
        {xmin,ymax,zmin}
    );


    box.push_back(
        {xmin,ymin,zmax}
    );

    box.push_back(
        {xmax,ymin,zmax}
    );

    box.push_back(
        {xmax,ymax,zmax}
    );

    box.push_back(
        {xmin,ymax,zmax}
    );


    std::vector<Point3D> outline;


    outline.push_back(box[0]);
    outline.push_back(box[1]);
    outline.push_back(box[2]);
    outline.push_back(box[3]);


    DrawProjectedPolygon(
        outline,
        fill_color,
        alpha,
        line_color,
        line_width,
        line_style
    );
}

// ============================================================
// Recover mirror facet
// ============================================================

static bool GetMirrorFacet(
    WTelescope &telescope,
    double x0,
    double y0,
    int &mirror_i,
    int &mirror_m)
{
    double xmirror = 0.0;
    double ymirror = 0.0;
    double zmirror = 0.0;


    telescope.Sphere(
        ZMIRROR,
        CURVATURE,
        x0,
        y0,
        SOURCE_Z_FOR_FACET,
        0.0,
        0.0,
        -1.0,
        &xmirror,
        &ymirror,
        &zmirror
    );


    double deltax = 0.0;
    double deltay = 0.0;
    double deltaz = 0.0;


    mirror_i = -1;
    mirror_m = -1;


    telescope.WhichMirror(
        xmirror,
        ymirror,
        zmirror,
        &deltax,
        &deltay,
        &deltaz,
        &mirror_i,
        &mirror_m
    );


    if(mirror_i < 0 ||
       mirror_m < 0)
    {
        return false;
    }


    return true;
}


// ============================================================
// Main drawing function
// ============================================================

void Draw_mirror()
{
    gStyle->SetOptStat(0);

    gStyle->SetTextFont(132);
    gStyle->SetLabelFont(132,"XYZ");
    gStyle->SetTitleFont(132,"XYZ");
    gStyle->SetTitleFont(132,"");


    WTelescope telescope;

    telescope.SetMirror();


// ------------------------------------------------------------
// Mirror map
// ------------------------------------------------------------

    TH2D *mirror_map =
        new TH2D(
            "mirror_map",
            "WFCTA mirror obstruction map;X (mm);Y (mm)",
            DRAW_NX,
            DRAW_X_MIN,
            DRAW_X_MAX,
            DRAW_NY,
            DRAW_Y_MIN,
            DRAW_Y_MAX
        );


    std::vector<int> mirror_id(
        DRAW_NX*DRAW_NY,
        -1
    );


    for(int ix=1;ix<=DRAW_NX;ix++)
    {
        double x =
            mirror_map->GetXaxis()
            ->GetBinCenter(ix);


        for(int iy=1;iy<=DRAW_NY;iy++)
        {
            double y =
                mirror_map->GetYaxis()
                ->GetBinCenter(iy);


            int mi=-1;
            int mm=-1;


            if(GetMirrorFacet(
                telescope,
                x,
                y,
                mi,
                mm))
            {
                mirror_map->SetBinContent(
                    ix,
                    iy,
                    1.0
                );


                mirror_id[
                    (iy-1)*DRAW_NX+
                    (ix-1)
                ] =
                    mi*100+mm;
            }
        }
    }


// ------------------------------------------------------------
// Canvas
// ------------------------------------------------------------

    TCanvas *canvas =
        new TCanvas(
            "canvas",
            "Mirror obstruction map",
            1200,
            900
        );


    canvas->SetLeftMargin(0.12);
    canvas->SetRightMargin(0.25);
    canvas->SetBottomMargin(0.12);
    canvas->SetTopMargin(0.08);


    mirror_map->GetXaxis()
        ->SetLabelFont(132);

    mirror_map->GetYaxis()
        ->SetLabelFont(132);

    mirror_map->GetXaxis()
        ->SetTitleFont(132);

    mirror_map->GetYaxis()
        ->SetTitleFont(132);


    mirror_map->Draw("COL");


// ------------------------------------------------------------
// Facet boundary
// ------------------------------------------------------------

    TGraph *facet_boundary =
        new TGraph();


    for(int ix=2;ix<DRAW_NX;ix++)
    {
        for(int iy=2;iy<DRAW_NY;iy++)
        {
            int id =
                mirror_id[
                    (iy-1)*DRAW_NX+
                    ix-1
                ];


            if(id<0)
            {
                continue;
            }


            int left =
                mirror_id[
                    (iy-1)*DRAW_NX+
                    ix-2
                ];

            int right =
                mirror_id[
                    (iy-1)*DRAW_NX+
                    ix
                ];

            int down =
                mirror_id[
                    (iy-2)*DRAW_NX+
                    ix-1
                ];

            int up =
                mirror_id[
                    iy*DRAW_NX+
                    ix-1
                ];


            if(left!=id ||
               right!=id ||
               down!=id ||
               up!=id)
            {
                double x =
                    mirror_map->GetXaxis()
                    ->GetBinCenter(ix);

                double y =
                    mirror_map->GetYaxis()
                    ->GetBinCenter(iy);


                facet_boundary->SetPoint(
                    facet_boundary->GetN(),
                    x,
                    y
                );
            }
        }
    }


    facet_boundary->SetMarkerStyle(1);
    facet_boundary->SetMarkerColor(kGray+2);
    facet_boundary->Draw("P same");


// ------------------------------------------------------------
// Colors
// ------------------------------------------------------------

    int sipm_color =
        TColor::GetColor("#E74C3C");

    int duct_color =
        TColor::GetColor("#3498DB");

    int stair_color =
        TColor::GetColor("#1ABC9C");

    int led_color =
        TColor::GetColor("#F39C12");

    int door_color =
        kBlack;


// ============================================================
// SiPM camera
// ============================================================

    if(DRAW_SIPM_CAMERA)
    {
        DrawProjectedBox3D(
            -CLUSTER_X/2.0,
             CLUSTER_X/2.0,
            -CLUSTER_Y/2.0,
             CLUSTER_Y/2.0,
             ZCLUSTER0,
             ZCLUSTER1,
             sipm_color,
             0.25,
             sipm_color,
             2
        );
    }


// ============================================================
// Ventilation duct
// ============================================================

    if(DRAW_DUCT)
    {

        // Left duct section 1

        DrawProjectedBox3D(
            DuctX1-Duct_Width/2.0,
            DuctX1+Duct_Width/2.0,

            DRAW_Y_MIN,
            Duct_Height1,

            DuctZ2,
            DuctZ1,

            duct_color,
            0.30,
            duct_color,
            2
        );


        // Right duct section 1

        DrawProjectedBox3D(
            DuctX2-Duct_Width/2.0,
            DuctX2+Duct_Width/2.0,

            DRAW_Y_MIN,
            Duct_Height1,

            DuctZ2,
            DuctZ1,

            duct_color,
            0.30,
            duct_color,
            2
        );


        // Left duct section 2

        DrawProjectedBox3D(
            DuctX1-Duct_Width/2.0,
            DuctX1+Duct_Width/2.0,

            DRAW_Y_MIN,
            Duct_Height2,

            DuctZ4,
            DuctZ3,

            duct_color,
            0.30,
            duct_color,
            2
        );


        // Right duct section 2

        DrawProjectedBox3D(
            DuctX2-Duct_Width/2.0,
            DuctX2+Duct_Width/2.0,

            DRAW_Y_MIN,
            Duct_Height2,

            DuctZ4,
            DuctZ3,

            duct_color,
            0.30,
            duct_color,
            2
        );


        // Horizontal connecting duct

        DrawProjectedBox3D(
            -Duct_Length/2.0,
             Duct_Length/2.0,

            Duct_Height1-Duct_Width/2.0,
            Duct_Height1+Duct_Width/2.0,

            DuctZ2,
            DuctZ1,

            duct_color,
            0.30,
            duct_color,
            2
        );
    }


// ============================================================
// Duct stair
// ============================================================

    if(DRAW_DUCT_STAIR)
    {
        DrawProjectedBox3D(
            DuctX1-DuctStair_Length/2.0,
            DuctX1+DuctStair_Length/2.0,

            DRAW_Y_MIN,
            DuctStair_Height,

            DuctZ4,
            DuctZ1,

            stair_color,
            0.25,
            stair_color,
            2
        );


        DrawProjectedBox3D(
            DuctX2-DuctStair_Length/2.0,
            DuctX2+DuctStair_Length/2.0,

            DRAW_Y_MIN,
            DuctStair_Height,

            DuctZ4,
            DuctZ1,

            stair_color,
            0.25,
            stair_color,
            2
        );
    }


// ============================================================
// LED frame
// ============================================================

    if(DRAW_LED_FRAME)
    {
        DrawProjectedBox3D(
            -LedFrameWidth/2.0,
             LedFrameWidth/2.0,

             DRAW_Y_MIN,
             LedFrameHeight,

             LedFrameZ-20.0,
             LedFrameZ,

             led_color,
             0.25,
             led_color,
             2
        );
    }


// ============================================================
// Door
// ============================================================

    if(DRAW_DOOR)
    {
        DrawProjectedBox3D(
            -D_DOOR/2.0,
             D_DOOR/2.0,

            -Hdoor/2.0,
             Hdoor/2.0,

             ZDOOR-20.0,
             ZDOOR,

             door_color,
             0.0,
             door_color,
             3,
             2
        );
    }

// ============================================================
// CCD geometry
// ============================================================

if(DRAW_NEW_CCD)
{
    int ccd_color =
        TColor::GetColor("#8E44AD");

    int aperture_color =
        TColor::GetColor("#2ECC71");


    const double disk_radius =
        CCD_DISK_DIAMETER / 2.0;


    const double hole_radius =
        CCD_HOLE_DIAMETER / 2.0;


    const double aperture_diameter =
        CCD_FOCAL_LENGTH /
        CCD_F_NUMBER;


    const double aperture_radius =
        aperture_diameter / 2.0;



    // ------------------------------
    // CCD base
    // ------------------------------

    TBox *base =
        new TBox(
            CCD_X-CCD_BASE_WIDTH/2.0,
            CCD_Y-disk_radius-CCD_BASE_HEIGHT,
            CCD_X+CCD_BASE_WIDTH/2.0,
            CCD_Y-disk_radius
        );


    base->SetFillColorAlpha(
        ccd_color,
        0.30
    );

    base->SetLineColor(
        ccd_color
    );

    base->SetLineWidth(2);

    base->Draw("same");



    // ------------------------------
    // CCD disk housing
    // ------------------------------

    TEllipse *disk =
        new TEllipse(
            CCD_X,
            CCD_Y,
            disk_radius,
            disk_radius
        );


    disk->SetFillColorAlpha(
        ccd_color,
        0.20
    );

    disk->SetLineColor(
        ccd_color
    );

    disk->SetLineWidth(2);

    disk->Draw("same");



    // ------------------------------
    // physical receiving hole
    // ------------------------------

    double hole_x =
        CCD_X -
        disk_radius +
        CCD_HOLE_EDGE_DISTANCE +
        hole_radius;


    double hole_y =
        CCD_Y;



    TEllipse *hole =
        new TEllipse(
            hole_x,
            hole_y,
            hole_radius,
            hole_radius
        );


    hole->SetFillColor(kWhite);

    hole->SetLineColor(
        ccd_color
    );

    hole->SetLineWidth(2);

    hole->Draw("same");



    // ------------------------------
    // effective aperture
    // ------------------------------

    TEllipse *aperture =
        new TEllipse(
            hole_x,
            hole_y,
            aperture_radius,
            aperture_radius
        );


    aperture->SetFillColorAlpha(
        aperture_color,
        0.50
    );

    aperture->SetLineColor(
        aperture_color
    );

    aperture->SetLineWidth(2);

    aperture->Draw("same");

}



// ============================================================
// Origin marker
// ============================================================

TMarker *origin =
    new TMarker(
        0.0,
        0.0,
        20
    );


origin->SetMarkerColor(
    kBlack
);

origin->SetMarkerSize(
    1.0
);

origin->Draw("same");



// ============================================================
// Legend
// ============================================================

TLegend *legend =
    new TLegend(
        0.78,
        0.62,
        0.97,
        0.90
    );


legend->SetBorderSize(1);

legend->SetFillColor(
    kWhite
);

legend->SetTextFont(
    132
);

legend->SetTextSize(
    0.018
);


TBox *mirror_leg =
    new TBox();


mirror_leg->SetFillColor(
    TColor::GetColor("#D9D9D9")
);


TBox *sipm_leg =
    new TBox();


sipm_leg->SetFillColor(
    TColor::GetColor("#E74C3C")
);


TBox *duct_leg =
    new TBox();


duct_leg->SetFillColor(
    TColor::GetColor("#3498DB")
);


TBox *stair_leg =
    new TBox();


stair_leg->SetFillColor(
    TColor::GetColor("#1ABC9C")
);


TBox *led_leg =
    new TBox();


led_leg->SetFillColor(
    TColor::GetColor("#F39C12")
);


TLine *door_leg =
    new TLine();


door_leg->SetLineColor(
    kBlack
);

door_leg->SetLineWidth(
    2
);

door_leg->SetLineStyle(
    2
);


TBox *ccd_leg =
    new TBox();


ccd_leg->SetFillColor(
    TColor::GetColor("#8E44AD")
);


TBox *aperture_leg =
    new TBox();


aperture_leg->SetFillColor(
    TColor::GetColor("#2ECC71")
);



legend->AddEntry(
    mirror_leg,
    "Mirror facets",
    "f"
);


if(DRAW_SIPM_CAMERA)
{
    legend->AddEntry(
        sipm_leg,
        "SiPM camera",
        "f"
    );
}


if(DRAW_DUCT)
{
    legend->AddEntry(
        duct_leg,
        "Ventilation duct",
        "f"
    );
}


if(DRAW_DUCT_STAIR)
{
    legend->AddEntry(
        stair_leg,
        "Duct stair",
        "f"
    );
}


if(DRAW_LED_FRAME)
{
    legend->AddEntry(
        led_leg,
        "LED frame",
        "f"
    );
}


if(DRAW_DOOR)
{
    legend->AddEntry(
        door_leg,
        "Door opening",
        "l"
    );
}


if(DRAW_NEW_CCD)
{
    legend->AddEntry(
        ccd_leg,
        "CCD housing",
        "f"
    );


    legend->AddEntry(
        aperture_leg,
        "CCD aperture",
        "f"
    );
}


legend->Draw();



// ============================================================
// Information box
// ============================================================

TPaveText *info =
    new TPaveText(
        0.78,
        0.15,
        0.97,
        0.52,
        "NDC"
    );


info->SetFillColor(
    kWhite
);

info->SetBorderSize(
    1
);

info->SetTextFont(
    132
);

info->SetTextSize(
    0.016
);

info->SetTextAlign(
    12
);



info->AddText(
    Form(
        "Projection: (%.2f, %.2f, %.2f)",
        PROJ_DIR_X,
        PROJ_DIR_Y,
        PROJ_DIR_Z
    )
);


info->AddText(
    Form(
        "Mirror Z: %.2f mm",
        PROJ_MIRROR_Z
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
        "Duct width: %.1f mm",
        Duct_Width
    )
);


info->AddText(
    Form(
        "Duct Z1-Z4:"
    )
);


info->AddText(
    Form(
        "%.1f %.1f %.1f %.1f",
        DuctZ1,
        DuctZ2,
        DuctZ3,
        DuctZ4
    )
);


if(DRAW_NEW_CCD)
{
    info->AddText(
        Form(
            "CCD: %.0f x %.0f mm",
            CCD_DISK_DIAMETER,
            CCD_BASE_HEIGHT
        )
    );


    info->AddText(
        Form(
            "Hole: %.1f mm",
            CCD_HOLE_DIAMETER
        )
    );


    info->AddText(
        Form(
            "Aperture: %.4f mm",
            CCD_FOCAL_LENGTH /
            CCD_F_NUMBER
        )
    );
}


info->Draw();



// ============================================================
// Save
// ============================================================

canvas->Modified();

canvas->Update();


canvas->SaveAs(
    OUTPUT_PNG
);


canvas->SaveAs(
    OUTPUT_PDF
);



std::cout
    << "============================================================"
    << std::endl;


std::cout
    << "WFCTA mirror obstruction map generated"
    << std::endl;


std::cout
    << "PNG : "
    << OUTPUT_PNG
    << std::endl;


std::cout
    << "PDF : "
    << OUTPUT_PDF
    << std::endl;


std::cout
    << "Projection direction = ("
    << PROJ_DIR_X << ", "
    << PROJ_DIR_Y << ", "
    << PROJ_DIR_Z
    << ")"
    << std::endl;


std::cout
    << "Mirror reference Z = "
    << PROJ_MIRROR_Z
    << " mm"
    << std::endl;


std::cout
    << "============================================================"
    << std::endl;

}



// ============================================================
// Main
// ============================================================

int main()
{
    Draw_mirror();

    return 0;
}
