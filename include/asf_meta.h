/*************************************************************************** 
   NAME:  asf_meta.h

   Header file for the asf_meta.a library's meta_get* routines.
   
   These routines are intended as a higher-level SAR image metadata
   extraction layer.  By using these routines, we can add support for
   new CEOS formats, CEOS bug fixes, and other data types in *one*
   place, instead of being scattered through the software.

   It would be even better to have a single small clean interface to
   our own metadata format, and a seperate package for import/export
   from other formats.  So the CEOS/other format reading/writing code
   should be refactored.
   
   This library is intended to completely replace the asf_sar.a and
   geolocate.a libraries.

   Orion Lawlor, 9/10/98

***************************************************************************/

#ifndef __ASF_META_H__
#define __ASF_META_H__

#include "geolocate.h"		/* For stateVector.  */
#include "ddr.h"

/******************Baseline Utilities******************/
typedef struct {
	double Bn;	 /* Normal Baseline: perpendicular to look direction.*/
	double dBn;	 /* Change in normal baseline per scene.*/
	double Bp;	 /* Parallel Baseline: parallel to look direction.*/
	double dBp;	 /* Change in parallel baseline per scene.*/
	double temporal; /* Temporal baseline, in fractional days.*/
} baseline;

/* Reads baseline from given baseline file, or exits.*/
baseline read_baseline(char *fName);

/******************** Metadata Utilities ***********************/
/*  These structures are used by the meta_get* routines.
    Try to use the functions (meta_*) where possible; don't just
    read values directly.
---------------------------------------------------------------*/

/* Maximum length of most string fields, including trailing null.  */
#define FIELD_STRING_MAX 256

/* Maximum length of mode field, including trailing null.  In case its
   so short for some good reason.  */
#define MODE_FIELD_STRING_MAX 5

/********************************************************************
 * meta_general: General RAdio Detection And Ranging parameters
 */
typedef struct {
  char sensor[FIELD_STRING_MAX];     /* Name of imaging sensor.            */
  char mode[MODE_FIELD_STRING_MAX];  /* Mode of imaging sensor.            */
  char processor[FIELD_STRING_MAX];  /* Name and version of SAR processor. */
  char data_type[FIELD_STRING_MAX];  /* Type of samples (e.g. "REAL*4").   */
  char system[FIELD_STRING_MAX];     /* System of samples (e.g. "iee-std") */
  int orbit;		     /* Orbit number of satellite.                 */
  int frame;		     /* Frame for this image or -1 if inapplicable.*/
  int band_number;           /* Band number; first band is 0               */
  char orbit_direction;	     /* Ascending 'A', or descending 'D'.          */
  int line_count;            /* Number of lines in image.                  */
  int sample_count;          /* Number of samples in image.                */
  int start_line;	     /* First line relative to original image.     */
  int start_sample;          /* First sample relative to original image.   */
  double x_pixel_size;       /* Range pixel size, in meters                */
  double y_pixel_size;       /* Azimuth pixel size, in meters              */
  double center_latitude;    /* Approximate image center latitude.         */
  double center_longitude;   /* Approximage image center longitude.        */
  double re_major;           /* Semimajor axis length (equator) (meters).  */
  double re_minor;           /* Semiminor axis length (poles) (meters).    */
  double bit_error_rate;     /* Fraction of bits which are in error.       */
  int missing_lines;         /* Number of missing lines in data take       */
} meta_general;


/********************************************************************
 * meta_sar: SAR specific parameters
 */
typedef struct {
  /* 'S'-> Slant Range; 'G'-> Ground Range; 'P'-> Map Projected.  */
  char image_type; 
  char look_direction;            /* 'L'-> Left Looking; 'R'-> Right Looking.*/
  int look_count;                 /* Number of looks to take from SLC.       */
  double look_angle;              /* Angle SAR looks at earth [radians]      */
  int deskewed;                   /* True if image moved to zero doppler.    */
  double range_time_per_pixel;    /* Time per pixel in range.                */
  double azimuth_time_per_pixel;  /* Time per pixel in azimuth.              */
  double slant_shift;             /* Error correction factor, in slant range */
  double time_shift;                 /* Error correction factor, in time.    */
  double slant_range_first_pixel;    /* Slant range to first pixel.          */
  double wavelength;		     /* SAR carrier wavelength, in meters.   */
  double prf;                        /* Pulse Repition Frequency.            */
  /* Doppler centroid, doppler per pixel, and doppler per pixel squared.  */
  double range_doppler_coefficients[3];
  /* Doppler centroid, doppler per pixel, and doppler per pixel squared.  */
  double azimuth_doppler_coefficients[3]; 
  /* Satellite binary time.  */
  char satellite_binary_time[FIELD_STRING_MAX];
  /* Satellite UTC clock time.  */
  char satellite_clock_time[FIELD_STRING_MAX];
} meta_sar;


/********************************************************************
 * meta_optical: paramenters specific to optical images
 * NOT YET IN USE
 */
typedef struct {
  double cloud_pct;                  /* Cloud cover percentage */
  double sun_az_angle;               /* Sun azimuth angle      */
  double sun_elev_angle;             /* Sun elevation angle    */
} meta_optical;


/********************************************************************
 * meta_thermal: paramenters specific to thermal images
 * NOT YET IN USE
 */
typedef struct {
  double band_gain;                  /* band gain                   */
  double band_gain_change;           /* band gain change            */
  int day;                           /* Day/Night flag 1=day 0=night*/
} meta_thermal;


/********************************************************************
 * meta_projection / proj_parameters: These describe a map projection.
 * Projection parameter components: one for each projection.
 */
/* Along-track/cross-track.*/
typedef struct {
  double rlocal;              /* Radius of earth at scene center (meters)*/
  double alpha1,alpha2,alpha3;/* Rotation angles, in degrees             */
} proj_atct;
 /* Lambert Conformal projection.*/
typedef struct {
  double plat1;     /* First standard parallel for Lambert */
  double plat2;     /* Second standard parallel for Lambert*/
  double lat0;      /* Original lat for Lambert            */
  double lon0;      /* Original lon for Lambert            */
} proj_lambert;
 /* Polar Sterographic.  */
typedef struct {
  double slat;      /* Reference latitude for polar stereographic */
  double slon;      /* Reference longitude for polar stereographic*/
} proj_ps;
 /* Universal Transverse Mercator.*/
typedef struct {
  int zone;
} proj_utm;
 /* Projection parameters for the projection in use.  */
typedef union {		     
  proj_atct     atct;     /* Along-track/cross-track      */
  proj_lambert  lambert;  /* Lambert Conformal projection */
  proj_ps       ps;       /* Polar Sterographic           */
  proj_utm      utm;      /* Universal Transverse Mercator*/
} param_t;
typedef struct {
  char type;   /* 'A'->Along Track/Cross Track; 'P'->Polar Stereographic;
	          'L'->Lambert Conformal; 'U'->Universal Transverse Mercator.*/
  double startX,startY;  /* Projection coordinates of top, lefthand corner.*/
  double perX,perY;      /* Projection coordinates per X and Y pixel.      */
  char units[12];        /* Units of projection (meters, arcsec)           */
  char hem;              /* Hemisphere Code: 'S'->southern; other northern.*/
  double re_major;       /* Semimajor axis length (equator) (meters).      */
  double re_minor;       /* Semiminor axis length (poles) (meters).        */
  /* Note: we compute ecc=sqrt(1-re_major^2/re_minor^2).  This field
     is therefore redundant and should be eliminated.  DEPRECATED.         */
  double ecc;            /* First eccentricity of earth ellipsoid.         */
    /* Projection parameters for each projection.                          */
  union {		     
    proj_atct     atct;	    /* Along-track/cross-track.                    */
    proj_lambert  lambert;  /* Lambert Conformal projection.               */
    proj_ps       ps;       /* Polar Sterographic.                         */
    proj_utm      utm;      /* Universal Transverse Mercator.              */
  } param;
} meta_projection;
 /* Compatibility alias.  proj_parameters is DEPRECATED.  */
typedef meta_projection proj_parameters;

/********************************************************************
 * meta_stats: statistical info about the image
 */
typedef struct {
  double max, min;           /* Maximum and minimum image values */
  double mean;               /* Mean                             */
  double rms;                /* Root mean squared                */
  double std_deviation;      /* Standard deviation               */
} meta_stats;  


/********************************************************************
 * State_vectors: Some collection of fixed-earth state vectors around
 * the image.  These are always increasing in time; but beyond that,
 * have no assumptions.
 */
typedef struct {
  double time;     /* Time of state vector, in seconds from the
			   start of the image.  */
  stateVector vec; /* Fixed-earth state vector.  */
} state_loc;
typedef struct {
  int year;           /* Year for first state vector                  */
  int julDay;         /* Julian day of year for first state vector.   */
  double second;      /* Seconds of day for first state vector.       */
  int vector_count;   /* Number of state vectors.                     */
  int num;	      /* Same as vector_count.  For backward compat.  */
  state_loc *vecs;    /* Array sized at run-time.                     */
} meta_state_vectors;


/* DEPRECATED */
/*Geo_parameters: These are used in geolocating the image.*/
typedef struct {
	char type;		/* 'S'-> Slant Range; 'G'-> Ground Range; 
				   'P'-> Map Projected.*/
  	proj_parameters *proj;	/* Projection parameters, for map-projected images.*/
	char lookDir;		/* 'L'-> Left Looking; 'R'-> Right Looking.*/
	int deskew;		/* Image moved to zero-doppler? (1-> yes; 0->no)*/
	double xPix,yPix;	/* Range, azimuth pixel size, in m*/
	double rngPixTime,azPixTime;  /* Range, Azimuth pixel time, in s.*/
	double timeShift, slantShift; /* Image correction (fudge)
					 factors in azimuth and range, in s and m.*/
	double slantFirst;      /* Slant range to first pixel, in m.*/
	double wavelen;         /* Satellite wavelength, in meters.*/
	double dopRange[3], dopAz[3]; /* Doppler centroid constant, linear, and
                                         quadratic terms, in azimuth and range (Hz)*/
} geo_parameters;

/* DEPRECATED */
/*Ifm_parameters: These are used only for interferometry.*/
typedef struct {
	double er;  /* Earth radius at scene center.*/
	double ht;  /* Satellite height from earth's center.*/
	int nLooks; /* Number of looks to take on SLC data to make square pixels*/
	int orig_nLines,orig_nSamples;
	double lookCenter;/*Look angle to image center (CALCULATED).*/
} ifm_parameters;

/* DEPRECATED */
/*extra_info: extra information needed to re-create CEOS files.*/
typedef struct {
	char	sensor[256];	   /* Name of imaging sensor.  */
	char	mode[5];	   /* Mode of imaging sensor.  */
	char    processor[256];    /* Name and version of SAR processor.  */
	int     orbit;		   /* Orbit number of satellite.  */
	double  bitErrorRate;      /* Exactly what it says.  */
	char    satBinTime[256];   /* Satellite binary clock time.  */
	char    satClkTime[256];   /* Satellite UTC time.  */
	double  prf;		   /* Pulse Repition Frequency.  */
} extra_info;

/********************************************************************
 * General ASF metadta structure.  Collection of all above.
 */
typedef struct {
  double meta_version;     /* Version of metadata format conformed to.  */

  meta_general       *general;
  meta_sar           *sar;             /* Can be NULL (check!).  */
  meta_optical       *optical;         /* Can be NULL (check!).  */
  meta_thermal       *thermal;         /* Can be NULL (check!).  */
  meta_projection    *projection;      /* Can be NULL (check!).  */
  meta_stats         *stats;
  meta_state_vectors *state_vectors;   /* Can be NULL (check!).  */
    /* Deprecated elements from old metadata format.  */
  meta_state_vectors *stVec;	       /* Can be NULL (check!).  */
  geo_parameters  *geo;
  ifm_parameters  *ifm;
  proj_parameters *proj;               /* Can be NULL (check!).  */
  extra_info      *info;               /* Can be NULL (check!).  */
} meta_parameters;


/****************** Creation/IO *********************
 * meta_init: in meta_init.c
 *	Extracts and returns SAR parameters from CEOS metadata.
*/
/*In meta_init.c.  
 * These are the routines to use, generally.*/
meta_parameters *meta_init(const char *fName);
void meta_free(meta_parameters *meta);

/*In meta_coni.c*//*
void meta_write_old(meta_parameters *meta,const char *outName);
meta_parameters *meta_read_old(const char *inName);
void meta_write(meta_parameters *meta,const char *outName);
meta_parameters *meta_read(const char *inName);
*/
/*In meta_read*/
meta_parameters *meta_read(const char *inName);

/* TO BE IN META_WRITE.C */
void meta_write(meta_parameters *meta,const char *outName);

/*Internal creation routines:*/
meta_parameters *raw_init(void);
meta_parameters *meta_create(const char *fName);

/*************************************************************
These routines all return various parameters from the
relevant metadata.  Values are always in m, m/s, s, and radians.
All arrays and coordinates are zero-based.
*/

/******************** General ***********************
General Calls: in meta_get.c*/
/* Convert a line, sample pair to a time, slant-range pair.
These only use the geo_parameters structure, and work
for all images.  They apply the time and slant range 
correction fudge factors. Returns seconds and meters.
*/
double meta_get_time(meta_parameters *sar,double yLine,double xSample);
double meta_get_slant(meta_parameters *sar,double yLine, double xSample);

/*Converts a line, sample pair to the doppler value
at that location. Returns Hz.  Only works for SR & GR.
*/
double meta_get_dop(meta_parameters *sar,double yLine, double xSample);

/*Return fixed-earth state vector for the given time.*/
stateVector meta_get_stVec(meta_parameters *sar,double time);

/*Return the incidence angle: this is the angle measured
	by the target between straight up and the satellite.
	Returns radians.*/
double meta_incid(meta_parameters *sar,double y,double x);

/*Return the look angle: this is the angle measured
	by the satellite between earth's center and the target point.
	Returns radians.*/
double meta_look(meta_parameters *sar,double y,double x);

/************* Geolocation ***********************
Geolocation Calls: in meta_get_geo.c.
Here, latitude and longitude are always in degrees.*/

/* Converts given line and sample to that for
the non-multilooked, non-windowed, equivalent image.*/
void meta_get_orig(void *ddr,
	int y, int x,int *yOrig,int *xOrig);

/* Converts given line and sample to geodetic
latitude and longitude.  Works with all image types.
*/
void meta_get_latLon(meta_parameters *sar,
	double yLine, double xSample,double elev,double *lat,double *lon);

/* Finds line and sample corresponding to given
latitude and longitude. */
void meta_get_lineSamp(meta_parameters *meta,
	double lat,double lon,double elev,double *yLine,double *xSample);

/* Converts a given line and sample in image into time,
slant-range, and doppler.  Works with all image types.
*/  
void meta_get_timeSlantDop(meta_parameters *sar,
	double yLine,double xSample,
	double *time,double *slant,double *dop);

/*Converts the given time, slant range, doppler, and elevation
off earth's surface into a latitude and longitude.*/
void meta_timeSlantDop2latLon(meta_parameters *sar,
	double time, double slant,double dop,double elev,
	double *lat,double *lon);

/*Convert given latitude and longitude to time, slant
range, and doppler, using state vectors.*/
void latLon2timeSlant(meta_parameters *sar,
	double lat,double lon,
	double *time,double *slant,double *dop);

void getLatLongMeta(const stateVector stVec,meta_parameters *meta,
	double range,double doppler,double elev,
	double *targLat, double *targLon, double *targRadius);

/* This low-level routine is used internally by asf_meta.  */
GEOLOCATE_REC *init_geolocate_meta(const stateVector *stVec,meta_parameters *meta);

/************* Interferometry *******************
Interferometry Calls: in meta_get_ifm.c*/

/* Get original image dimensions.  */
void meta_get_orig_img_dimensions(meta_parameters *sar, long *lines, 
				  long *samples);

/*Return satellite height.*/
double meta_get_sat_height(meta_parameters *meta, long line, long sample);

/*Return earth radius at scene center.*/
double meta_get_earth_radius(meta_parameters *meta, long line, long sample);

/*Return slant range to first (leftmost) pixel, and slant range per pixel.*/
void meta_get_slants(meta_parameters *sar,double *slantFirst, double *slantPer);

/*return satellite wavenumber k=2*PI/wavelength.*/
double meta_get_k(meta_parameters *sar);

/*Return the fraction of the scene that is after line y*/
double meta_scene_frac(meta_parameters *sar,int y);

/*Interpolate the given baseline for the (original image) line y.*/
void meta_interp_baseline(meta_parameters *sar,const baseline base,
	int y,double *Bn_y,double *Bp_y);

/*Return the "flat earth" look deviation--
	this is the x look angle minus the center look angle.*/
double meta_flat(meta_parameters *sar,double y,double x);

/*Return the expected phase of the given point for a bald earth-- elevation 0.*/
double meta_flat_phase(meta_parameters *sar,const baseline base,int y,int x);

/*Return the "phase rate"-- the number of meters of elevation per radian of phase.*/
double meta_phase_rate(meta_parameters *sar,const baseline base,int y,int x);

#endif
