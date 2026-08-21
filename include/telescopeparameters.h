#ifndef TELESCOPEPARAMETERS
#define TELESCOPEPARAMETERS
#include <math.h>
#include "TString.h"

//* === COSRIKA CERfile reading parameters === *//
const int PARTICLE_LENGTH_THINNING = 8;
const int PARTICLE_LENGTH_NO_THINNING = 7;
const int N_PARTICLE = 39;
const int N_SUBBLOCK = 21;

const int SUBBLOCK_LENGTH_THINNING = N_PARTICLE * PARTICLE_LENGTH_THINNING;
const int SUBBLOCK_LENGTH_NO_THINNING = N_PARTICLE * PARTICLE_LENGTH_NO_THINNING;
const int RECORD_LENGTH_THINNING = N_SUBBLOCK * SUBBLOCK_LENGTH_THINNING;
const int RECORD_LENGTH_NO_THINNING = N_SUBBLOCK * SUBBLOCK_LENGTH_NO_THINNING;

//* === Telescope geometry and mirror parameters === *//

const double CURVATURE= 5800; //mm // the curvature of the mirrors


const double D_DOOR = 2410. ; //previous value: 2300 ;   from Geng Lisi 20210409
                            //mm , the Width of the door and the maximum size of the mirror
const double Hdoor = 2470. ; //mm, the height of the container;  from Geng Lisi 20210409

const double CLUSTER_X = 922.9; //mm The Size of pmt array

const double CLUSTER_Y = 946.2; //mm

const double RADIUS1_SQUARE = 0.5*D_DOOR*0.5*D_DOOR;

const double RADIUS2_SQUARE = 0.5*CLUSTER_X*0.5*CLUSTER_Y;

const double FOCUS = 2870; //mm, the distance between the mirror and the camera

const double ZCLUSTER0 = FOCUS; //mm

const double ZCLUSTER1 = FOCUS+230; //mm

const double Zcluster1_D = FOCUS+230; //mm  ///< 13102015

const double ZDOOR = ZCLUSTER1 + 80; //mm

const double Zdoor_D = Zcluster1_D + 80; //mm

const double ZMIRROR = CURVATURE - sqrt(CURVATURE*CURVATURE-D_DOOR*D_DOOR/4.);//the mirror, door and cluster

//* === Mirror geometry parameters === *// ///< Provided by MA Lingling 13102015

const double length = 300.; //mm, the length of the mirror facet of spherical design;  20140926 liujl

//* === Camera parameters === *//

const double D_ConeOut = 25.8; //mm, the outer Diameter of the WistonCone

const double D_ConeIn = 24.4; //mm, the inner Diameter of the WistonCone

const double D_SiPM = 15.0; //mm, the side length of squre sipm 

const double D_Cell = 2.5e-2; //mm, the side of the cell  

const int NCell = 600; // the number of cells in x or y directions 

const int  PIX =  32;  ///< number of pmt in x,y direction

const int NSIPM = PIX*PIX;  ///< number of pmt in each camera


const double CONE_HEIGHT = 25.27; //mm

const double CONE_ENTRANCE_CIRCLE = 12.2;//mm

const double CONE_EXIT_CIRCLE = 7.5;//mm

const double CUTOFF_ANGLE = 0.66; //radius  37.9*180/PI

const double CONE_REFLECTIVE_EFFICIENCY=0.9;

extern double mirrorx[7][6];
extern double mirrory[7][6];
extern double mirrorz[7][6];
extern double nalfa[7][6];
extern double nbeta[7][6];
extern double spherecenterx[7][6];
extern double spherecentery[7][6];
extern double spherecenterz[7][6];

///< related value
const double sqrt3 = 1.73205080757;
const double sqrt2 = 1.41421356237;
const double pi = 3.14159265359;
const double twopi = 6.28318530718;

//* === Physical constant === *//
const double Qe=-1.6e-4;      ///< Charge value of an electron, unit in muonA*ns
const double C_LIGHT = 299.7; //light speed in the air  mm/ns


const double Frame_Width=5;//mm, the 1./2 Width of the Frame in the camera, // 20190806

const double Duct_Diameter = 80;// the Diameter of the ventilation duct.

const double Duct_Width = Duct_Diameter; // The Width of the ventilation duct. //20190806 

const double Duct_Length = 1290; // The Length of the ventilation duct. 

const double Duct_Height1 = 1050-Hdoor/2; // The Height of the ventilation duct.

const double Duct_Height2 = 1480-Hdoor/2; // The Height of the ventilation duct.

const double DuctX1 = -645; // The position of ventilation duct on the left side of the door;

const double DuctX2 = 645;  // The position of ventilation duct on the right side of the door;

const double DuctStair_Height = 50-Hdoor/2;// the height of ventilation duct stair; 

const double DuctStair_Length = 160;// the height of ventilation duct stair;

const double DuctZ1 = FOCUS; // The z position of Duct1;

const double DuctZ2 = FOCUS-130; // The z position of Duct2 (mm); 

const double DuctZ3 = FOCUS-250; // The z position of Duct3(mm);

const double DuctZ4 = FOCUS-380; // The z position of Duct4(mm);

const double LedFrameZ = 370; //The z position  of LED Frame(mm)

const double LedFrameWidth = 50; //The Width of LED Frame(mm)

const double LedFrameHeight = -Hdoor/2+1000;//The Height of LED Frame(mm)

const double LedFrameRadius=50; //The Radius of LED Frame(mm)

const double DoorFrameWidth = 100;// mm


#endif // TELESCOPEPARAMETERS

