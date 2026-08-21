#ifndef __RFL_ATM__
#define __RFL_ATM__

#define ITEM_LIST	/* LIST OF POSIBLE MODELS OF ATMOSPHERE */	 \
T(ATM_NOATMOSPHERE),	/* no atmosphere at all: transmittance = 100% */ \
T(ATM_90PERCENT),	/* atmosphere with transmittance = 90% */	 \
T(ATM_CORSIKA),		/* atmosphere as defined in CORSIKA */           \
T(ATM_MagicWinter),     /* atmosphere of LaPalma in winter*/             \
T(ATM_MagicSummer)      /* atmosphere of LaPalma in summer*/


#define T(x)  x		/* define T() as the name as it is  */
    enum { ITEM_LIST };
#undef T

#undef ITEM_LIST

#define NWAVELENGTHS 15 /* Number of wavelengths in some of the arrays below */

/* A. Moralejo, January 2003: added some parameters for Mie scattering and 
 * Ozone absorption derived from the clear standard atmosphere model in 
 * "Atmospheric Optics", by L. Elterman and R.B. Toolin, chapter 7 of 
 * the "Handbook of geophysics and Space environments". S.L. Valley, 
 * editor. McGraw-Hill, NY 1965.
 */

/* Aerosol number density for 31 heights a.s.l., from 0 to 30 km, 
 * in 1 km steps (units: cm^-3)
 */

float aero_n[31] = {2.0e2, 8.7e1, 3.8e1, 1.6e1, 7.2, 3.1, 1.1, 4.0e-1, 1.4e-1, 5.0e-2, 2.6e-2, 2.3e-2, 2.1e-2, 2.3e-2, 2.5e-2, 4.1e-2, 6.7e-2, 7.3e-2, 8.0e-2, 9.0e-2, 8.6e-2, 8.2e-2, 8.0e-2, 7.6e-2, 5.2e-2, 3.6e-2, 2.5e-2, 2.4e-2, 2.2e-2, 2.0e-2, 1.9e-2};

/* Aerosol scattering coefficient at sea level for the following wavelengths:
 * 280, 300, 320, 340, 360, 380, 400, 450, 500, 550, 600, 650, 700, 800 and 900 nm
 * (units: km^-1)
 */

float aero_betap[NWAVELENGTHS] = {0.27, 0.26, 0.25, 0.24, 0.24, 0.23, 0.20, 0.180, 0.167, 0.158, 0.150, 0.142, 0.135, 0.127, 0.120};

float wl[NWAVELENGTHS] = {280, 300, 320, 340, 360, 380, 400, 450, 500, 550, 600, 650, 700, 800, 900};

/* Ozone concentration for 51 heights a.s.l., from 0 to 50 km, 
 * in 1 km steps (units: cm/km)
 */

float oz_conc[51]={0.3556603E-02, 0.3264150E-02, 0.2933961E-02, 0.2499999E-02, 0.2264150E-02, 0.2207546E-02, 0.2160377E-02, 0.2226414E-02, 0.2283018E-02, 0.2811320E-02, 0.3499999E-02, 0.4603772E-02, 0.6207545E-02, 0.8452828E-02, 0.9528299E-02, 0.9905657E-02, 0.1028302E-01, 0.1113207E-01, 0.1216981E-01, 0.1424528E-01, 0.1641509E-01, 0.1839622E-01, 0.1971697E-01, 0.1981131E-01, 0.1933962E-01, 0.1801886E-01, 0.1632075E-01, 0.1405660E-01, 0.1226415E-01, 0.1066037E-01, 0.9028300E-02, 0.7933960E-02, 0.6830187E-02, 0.5820753E-02, 0.4830188E-02, 0.4311319E-02, 0.3613206E-02, 0.3018867E-02, 0.2528301E-02, 0.2169811E-02, 0.1858490E-02, 0.1518867E-02, 0.1188679E-02, 0.9301884E-03, 0.7443394E-03, 0.5764149E-03, 0.4462263E-03, 0.3528301E-03, 0.2792452E-03, 0.2226415E-03, 0.1858490E-03};

/* Vigroux ozone absorption coefficients (cm-1) */

float oz_vigroux[NWAVELENGTHS]= {1.06e2, 1.01e1, 8.98e-1, 6.40e-2, 1.80e-3, 0, 0, 3.50e-3, 3.45e-2, 
				 9.20e-2, 1.32e-1, 6.20e-2, 2.30e-2, 1.00e-2, 0.00};


#endif
