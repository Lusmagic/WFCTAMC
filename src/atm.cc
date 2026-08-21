/********************************************************************
*                                                                   *
*  File: atm.c                                                      *
*  Authors: J.C. Gonzalez, A. Moralejo                              *
*                                                                   *
* January 2002, A. Moralejo: lots of changes. Moved the code for    *
*         the Mie scattering and ozone absorption from attenu() to  *
*         here, after some bugs were found. Now the implementation  *
*         is different, we now precalculate the slant paths for the *
*         aerosol and Ozone vertical profiles, and then do an       *
*         interpolation in wavelength for every photon to get the   *
*         optical depths. The parameters used, defined below,       *
*         have been taken from "Atmospheric Optics", by L. Elterman *
*         and R.B. Toolin, chapter 7 of the "Handbook of geophysics *
*         and Space environments". (S.L. Valley, editor).           *
*         McGraw-Hill, NY 1965.                                     *
*                                                                   *
*         WARNING: the Mie scattering and the Ozone absorption are  *
*         implemented to work only with photons produced at a       *
*         height a.s.l larger than the observation level. So this   *
*         is not expected to work well for simulating the telescope *
*         pointing at theta > 90 deg (for instance for neutrino     *
*         studies. Rayleigh scattering works even for light coming  *
*         from below.                                               *
*                                                                   *
*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "atm.h"
//#include "diag.h"
//#include "init.h"

/*  random numbers  */
#define RandomNumber  ranf()
#define STEPTHETA 1.74533e-2 /* aprox. 1 degree */

#define MIN(x,y) ((x)<(y)? (x) : (y))

/*  Function declarations  */
float atm(float wavelength, float height, float theta);
void SetAtmModel(int model, float ol);
//int absorption(float wlen, float height, float theta);
extern void attenu(float, float, float, float, int, float *, double *);
//extern float ranf(void);

/* aerosol_path contains the path integrals for the aerosol number
 * density (relative to the number density at sea level) between the 
 * observation level and a height h for different zenith angles. The 
 * first index indicate height above sea level in units of 100m, the 
 * second is the zenith angle in degrees.
 */
static float aerosol_path[301][90];

/* ozone_path contains the path integrals for the ozone concentration
 * between the observation level and a height h for different zenith 
 * angles. The first index indicate height above sea level in units
 * of 100m, the second is the zenith angle in degrees.
 */
static float ozone_path[501][90];

static float obslev; /* observation level in cm */
static double rt;    /* Earth radius in cm */
static int atmModel;

void SetAtmModel(int model, float ol)
{
  float Rcos2, sin2, rtsq, path_slant, h, dh, theta;
  int j;

  atmModel = model;
  obslev = ol; 
  rt= 6371315.E2; /* Earth radius (same as in Corsika) in cm */

  if (atmModel != ATM_NOATMOSPHERE && atmModel != ATM_90PERCENT)
    {
      /* It follows a precalculation of the slant path integrals we need 
       * for the estimate of the Mie scattering and Ozone absorption:
       */

      rtsq = sqrt(rt);
      dh = 1.e3;

      /* Mie (aerosol): */

      for (j = 0; j < 90; j++)
	{
	  theta = j * STEPTHETA;  /* aprox. steps of 1 deg */

	  path_slant = 0;
	  Rcos2 = rt * cos(theta)*cos(theta);
	  sin2 = sin(theta)*sin(theta);

	  for (h = obslev; h <= 30e5; h += dh)
	    {
	      if (fmod(h,1e4) == 0)
		aerosol_path[(int)(h/1e4)][j] = path_slant;

	      path_slant += 
		(aero_n[(int)floor(h/1.e5)] + (h/1.e5 - floor(h/1.e5))*
		  (aero_n[(int)ceil(h/1.e5)]-aero_n[(int)floor(h/1.e5)]))
		  /aero_n[0] * dh * (rt+h) /
		    sqrt((rt+h)*(rt+h)-(rt+obslev)*(rt+obslev)*sin2);

	    }
	}

      /* Ozone absorption */

      for (j = 0; j < 90; j++)
	{
	  theta = j * STEPTHETA;  /* aprox. steps of 1 deg */
	  path_slant = 0;
	  Rcos2 = rt * cos(theta)*cos(theta);
	  sin2 = sin(theta)*sin(theta);

	  for (h = obslev; h <= 50e5; h += dh)
	    {
	      if (fmod(h,1e4) == 0)
		ozone_path[(int)(h/1e4)][j] = path_slant;

	      path_slant += 
		(oz_conc[(int)floor(h/1.e5)] + (h/1.e5 - floor(h/1.e5))*
		  (oz_conc[(int)ceil(h/1.e5)]-oz_conc[(int)floor(h/1.e5)]))
		  * dh * (rt+h) /
		    sqrt((rt+h)*(rt+h)-(rt+obslev)*(rt+obslev)*sin2);
	    }
	}

    }

}   /*  end of SetAtmModel  */

float atm(float wavelength, float height, float theta )
{   
  float transmittance = 1.0;  /* final atm transmittance (ret. value) */
  float T_Ray, T_Mie, T_Oz;

  float h; /* True height a.s.l. of the photon emission point in cm */
  float tdist;
  float beta0, path;

  int index;

  switch(atmModel)
    {	
    case ATM_NOATMOSPHERE:	/* no atm at all: transmittance = 100%	*/
      break;
    case ATM_90PERCENT:	/* atm. with transmittance = 90%	*/
      transmittance = 0.9;
      break;
    case ATM_CORSIKA:	/* atmosphere as defined in CORSIKA	*/
    case ATM_MagicWinter:
    case ATM_MagicSummer:

      /* Distance to telescope: */
      tdist = (height-obslev)/cos(theta);

      /* Avoid problems if photon is very close to telescope: */
      if (fabs(tdist) < 1.)
	{
	  transmittance = 1.;
	  break;
	}

      /*** We calculate h, the true emission height above sea level: ***/

      h = -rt + sqrt((rt+obslev)*(rt+obslev) + tdist*tdist +
		     (2*(rt+obslev)*(height-obslev)));

      /******* Rayleigh scattering: *******/

      double AM;
      attenu(wavelength, h, obslev, theta, atmModel, &T_Ray, &AM);

      /******* Ozone absorption: *******/

      if (h > 50.e5)
	h = 50.e5;

      /* First we get Vigroux Ozone absorption coefficient for the given 
       * wavelength, through a linear interpolation:
       */

      for (index = 1; index < NWAVELENGTHS-1; index++)
	if (wavelength < wl[index])
	  break;
      if(wavelength<wl[0]) index = 0;
      if(wavelength>wl[NWAVELENGTHS-1]) index = NWAVELENGTHS-1;

      beta0 = oz_vigroux[index-1]+(oz_vigroux[index]-oz_vigroux[index-1])*
	(wavelength-wl[index-1])/(wl[index]-wl[index-1]);

      /* from km^-1 to cm^-1 : */
      beta0 *= 1e-5;
      
      /* Now use the pre-calculated values of the path integral 
       * for h and theta: */

      path = ozone_path[(int)floor(0.5+h/1e4)]
	[(int)MIN(89,floor(0.5+theta/STEPTHETA))];

      T_Oz = exp(-beta0*path);
      //*tr_oz = T_Oz;
      /******* Mie (aerosol): *******/

      if (h > 30.e5)
	h = 30.e5;

      /* First get Mie absorption coefficient at sea level for the given 
       * wavelength, through a linear interpolation:
       */

      for (index = 1; index < NWAVELENGTHS-1; index++)
	if (wavelength < wl[index])
	  break;
      if(wavelength<wl[0]) index = 0;
      if(wavelength>wl[NWAVELENGTHS-1]) index = NWAVELENGTHS-1;

      beta0 = aero_betap[index-1]+(aero_betap[index]-aero_betap[index-1])*
	(wavelength-wl[index-1])/(wl[index]-wl[index-1]);

      /* from km^-1 to cm^-1 : */
      beta0 *= 1e-5;

      /* Now use the pre-calculated values of the path integral 
       * for h and theta: */

      path = aerosol_path[(int)floor(0.5+h/1e4)]
	[(int)MIN(89,floor(0.5+theta/STEPTHETA))];

      T_Mie = exp(-beta0*path);
     // *tr_mie = T_Mie;

      /* Calculate final transmission coefficient: */

      transmittance = T_Ray * T_Mie * T_Oz ;
      //*tr_all = transmittance;

      break;

    }	/*  end of atm switch	*/


  return transmittance;

}   /*  end of atm  */


//int absorption(float wlen, float height, float theta)
//{
//  int ret = 0;	/*  0: passed, 1: absorbed  */
   
//  if (RandomNumber > atm(wlen, height, theta)) ret=1;

//  return ret; 
//}   /*  end of absorption  */



