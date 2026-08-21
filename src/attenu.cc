/*********************************************************************
 *                                                                   *
 *     Atmospheric Absorption due to Rayleigh scattering             * 
 *                                                                   *
 *     Created: Dec 2006, A. Moralejo (moralejo@ifae.es)             *
 *     From a fortran code by Aitor Ibarra Ibaibarriaga and          *
 *     Jose Carlos Gonzalez                                          *
 *                                                                   *
 *     Now this accounts only for Rayleigh scattering. Mie and       *
 *     Ozone absorption are now treated separatedly (see atm.c).     *
 *                                                                   *
 *********************************************************************/

/*
 * December 2002, Abelardo Moralejo: checked algorithms, removed 
 * old/unnecessary code, fixed some bugs, added comments.
 *
 * Fixed bugs (of small influence) in Mie absorption implementation: there were
 * errors in the optical depths table, as well as a confusion: height a.s.l.
 * was used as if it was height above the telescope level. The latter error was
 * also present in the Ozone aborption implementation.
 *
 * On the other hand, now we have the tables AE_ABI and OZ_ABI with optical 
 * depths between sea level and a height h (before it was between 2km a.s.l 
 * and a height h). So that we can simulate also in the future a different 
 * observation level. 
 *
 * AM: WARNING: IF VERY LARGE zenith angle simulations are to be done (say 
 * above 85 degrees, for neutrino primaries or any other purpose) this code 
 * will have to be adapted accordingly and checked, since in principle it has 
 * been written and tested to simulate the absorption of Cherenkov photons 
 * arriving at the telescope from above.
 *
 * AM: WARNING 2: not to be used for wavelengths outside the range 250-700 nm
 *
 * January 2003, Abelardo Moralejo: found error in Ozone absorption treatment.
 * At large zenith angles, the air mass used was the one calculated for 
 * Rayleigh scattering, but since the Ozone distribution is rather different
 * from the global distribution of air molecules, this is not a good 
 * approximation. Now I have left in this code only the Rayleigh scattering,
 * and moved to atm.c the Mie scattering and Ozone absorption calculated in
 * a better way.
 *
 * AM, Jan 2003: added obslev as an argument of the function. Changed the 
 * meaning of the argument height: now it is the true height above sea level 
 * at which a photon has been emitted, before it was the height given by
 * Corsika, measured in the vertical of the observer and not in the vertical 
 * of the emitting particle.
 *
 * MH, AM, Dec 2006: introduced two new atmospheric models: MAGIC Winter and 
 * MAGIC summer. parametrized by M. Haffke
 *
 * AM, Dec 2006: translated attenu.f into the current c-code.
 *
 */

#include "attenu.h"
#include <math.h>

void attenu(float wavelength, float height, float obslev, float theta,
	    int atm_model, float *tr_atmos, double *AM)
{
  // AM, Jan 2002: now the argument height is directly the height above 
  // sea level, calculated in atm.c.

  // Set low limit of first atmospheric layer to the observation level
  // so that the traversed atmospheric depth in the Rayleigh scattering 
  // will be calculated correctly.
  
  lahg[0] = obslev;

  double airmass;
  double T_Ray, Rho_Tot, Rho_Fi;
  double Rcos2, Rsin2;
  double x1, x2, x3, x4, e1, e2, e3, e4;
  int    ilayer;

  // For the case of simulating a telescope higher than 4 km...

  if (obslev > lahg[1])
    lahg[1] = obslev;

  T_Ray = 1.0;



  // LARGE ZENITH ANGLE FACTOR (AIR MASS FACTOR): 
  // Air mass factor "airmass" calculated using a one-exponential density
  // profile for the atmosphere, rho = rho_0 exp(-height/hscale) with 
  // hscale = 7.4 km. The air mass factor is defined as I(theta)/I(0), 
  // the ratio of the optical paths I (in g/cm2) traversed between two 
  // given heights, at theta and at 0 deg z.a. 

  Rcos2 = rt * pow(cos(theta), 2.);
  Rsin2 = rt * pow(sin(theta), 2.);

  // AM, Dec 2002: slightly changed the calculation of the air mass factor,
  // for what I think is a better approximation (numerically the results are 
  // almost exactly the same, this change is irrelevant!):

  x1 = sqrt(Rcos2 / (2*hscale));
  x2 = sqrt((2*(height-obslev) + Rcos2) / (2*hscale));
  x3 = sqrt(rt / (2*hscale));
  x4 = sqrt((2*(height-obslev) + rt) / (2*hscale));
    
  // AM Dec 2001, to avoid crash! Sometime a few photons seem to be 
  // "corrupted" (have absurd values) in cer files... Next lines avoid the
  // crash. They will also return a -1 transmittance in the case the photon 
  // comes exactly from the observation level, because in that case the 
  // "air mass factor" would become infinity and the calculation is not valid!

  if (fabs(x3-x4) < 1.e-10)
    {
      *tr_atmos = -1.;
      return;
    }

  e1 = erfc(x1);
  e2 = erfc(x2);
  e3 = erfc(x3);
  e4 = erfc(x4);
  
  airmass = exp(-Rsin2 / (2. * hscale)) * ((e1 - e2) / (e3 - e4));
  *AM = airmass;
  
  // Calculate the traversed "vertical thickness" of air using the
  // US Standard atmosphere:

  Rho_Tot = 0.0;

  for (ilayer = 1; ilayer < 5; ilayer++)
    {
      if (height < lahg[ilayer])
	{
          Rho_Tot = Rho_Tot + batm[atm_model-2][ilayer-1]*
	    (exp(-lahg[ilayer-1]/catm[atm_model-2][ilayer-1]) -
	     exp(-height/catm[atm_model-2][ilayer-1]));
          break;
	}
      else
	Rho_Tot = Rho_Tot + batm[atm_model-2][ilayer-1]*
	  (exp(-lahg[ilayer-1]/catm[atm_model-2][ilayer-1]) -
	   exp(-lahg[ilayer]/catm[atm_model-2][ilayer-1]));
    }
 
  // We now convert from "vertical thickness" to "slanted thickness"
  // traversed by the photon on its way to the telescope, simply
  // multiplying by the air mass factor m:

  Rho_Fi = airmass * Rho_Tot;

  // Finally we calculate the transmission coefficient for the Rayleigh
  // scattering:
  // AM Dec 2002, introduced ABS below to account (in the future) for 
  // possible photons coming from below the observation level.

  T_Ray = exp(-fabs(Rho_Fi/xr)*pow(400./wavelength,4));
   
  *tr_atmos = T_Ray;

  return;
}
