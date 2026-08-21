
// Limits (height in cm) of the four layers in which atmosphere is parametrized: 
double lahg[5] = {0., 4.0e5, 1.0e6, 4.0e6, 1.0e7};

// Take same Earth radius as in CORSIKA (cm)
double rt = 6371315.e2;

// Scale-height (cm) for Exponential density profile
double hscale = 7.4e5; 

// Mean free path for scattering Rayleigh XR (g/cm^2)
double xr = 2.970e3;

// Parameters of the different atmospheres. We use the same parametrization shape as in Corsika
// atmospheric models (see Corsika manual, appendix C). The first index of the arrays refers to 
// the type of atmosphere, in this order: US standard, MAGIC-Winter, MAGIC-Summer. The second 
// index refers to the atmospheric layer (starting from sea level and going upwards)
//
// MAGIC-Winter and MAGIC-Summer by M. Haffke, parametrizing profiles obtained
// with MSIS:
// http://uap-www.nrl.navy.mil/models_web/msis/msis_home.htm
//
// The MAGIC-Winter and MAGIC-Summer parametrisations reproduce the MSIS profiles for the 
// 3 atmospheric layers from 0 up to 40 km height. Beyond that height, it was not possible to achieve a 
// good fit, but the amount of residual atmosphere is so small that light absorption would be negligible 
// anyway. And showers develop well below 40 km. So the parameters in the highest layer are set to 0.
//
// The mass overburden is given by T = AATM + BATM * exp(-h/CATM)
// The parameters "AATM" (in the Corsika manual) are not included in this code because they are not 
// needed. The last layer of the US standard atmosphere (in which T varies linearly with h) is above 100 km 
// and has not been included here for the same reason.
//

double batm[3][4] = {{1222.6562, 1144.9069, 1305.5948, 540.1778},
		     {1220.53, 1179.34, 1435.79, 0.},
		     {1200.84, 1168.35, 1449.73, 0.}};

double catm[3][4] = {{994186.38, 878153.55, 636143.04, 772170.16},
		     {1004320., 932851., 624763., 0.},
		     {1002500, 949750, 629920, 0.}};

