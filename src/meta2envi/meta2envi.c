/******************************************************************************
*                                                                             *
* Copyright (c) 2004, Geophysical Institute, University of Alaska Fairbanks   *
* All rights reserved.                                                        *
*                                                                             *
* Redistribution and use in source and binary forms, with or without          *
* modification, are permitted provided that the following conditions are met: *
*                                                                             *
*    * Redistributions of source code must retain the above copyright notice, *
*      this list of conditions and the following disclaimer.                  *
*    * Redistributions in binary form must reproduce the above copyright      *
*      notice, this list of conditions and the following disclaimer in the    *
*      documentation and/or other materials provided with the distribution.   *
*    * Neither the name of the Geophysical Institute nor the names of its     *
*      contributors may be used to endorse or promote products derived from   *
*      this software without specific prior written permission.               *
*                                                                             *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  *
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE    *
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        *
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    *
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     *
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     *
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  *
* POSSIBILITY OF SUCH DAMAGE.                                                 *
*                                                                             *
*       For more information contact us at:                                   *
*                                                                             *
*       Alaska Satellite Facility                                             *
*       Geophysical Institute                   http://www.asf.alaska.edu     *
*       University of Alaska Fairbanks          uso@asf.alaska.edu            *
*       P.O. Box 757320                                                       *
*       Fairbanks, AK 99775-7320                                              *
*                                                                             *
******************************************************************************/

#include "asf.h"
#include "asf_nan.h"
#include "envi.h"
#include "asf_meta.h"
#include "time.h"
#include <string.h>

#define VERSION 1.0


void usage(char *name)
{
  printf("\n"
	 "USAGE:\n"
	 "   %s [ -log <logFile> ] <meta_name> <envi_name>\n",name);
  printf("\n"
	 "REQUIRED ARGUMENTS:\n"
	 "   meta_name   Base name of the new style meta data.\n"
	 "   envi_name   Base name of the ENVI style header file.");
  printf("\n"
	 "DESCRIPTION:\n"
	 "   %s converts new ASF style metadata to an ENVI header file.\n",
	 name);
  printf("\n"
	 "Version %.2f, ASF SAR Tools\n"
	 "\n",VERSION);
  exit(EXIT_FAILURE);
}


int main(int argc, char **argv)
{
  char meta_name[255];
  char envi_name[255];
  meta_parameters *meta=NULL;
  envi_header *envi=NULL;
  FILE *fp;
  time_t t;
  char t_stamp[15];
  extern int currArg; /* from cla.h in asf.h... initialized to 1 */
  logflag = 0;

  /* Parse command line args */
  while (currArg < (argc-2))
    {
      char *key=argv[currArg++];
      if (strmatch(key,"-log")) {
	sprintf(logFile, "%s", argv[currArg]);
	logflag = 1;
      }
      else {
	printf("\n   ***Invalid option:  %s\n\n",
	       argv[currArg-1]);
	usage(argv[0]);
      }
    }
  if ((argc-currArg) < 2) {
    printf("Insufficient arguments.\n"); 
    usage(argv[0]);
  }

  create_name(meta_name, argv[currArg], ".meta");
  create_name(envi_name, argv[currArg+1], ".hdr");

  asfSplashScreen(argc, argv);
  t = time(NULL);
  strftime(t_stamp, 12, "%d-%b-%Y", localtime(&t));
  
  /* Read .meta and fill meta structures */ 
  meta = meta_read(meta_name);

  /* Fill ENVI header struct with valid data */
  envi = meta2envi(meta);
  
  /* Write ENVI header file */
  fp = FOPEN(envi_name, "w");
  fprintf(fp, "ENVI\n");
  fprintf(fp, "description = {\n"
	      "  Created by meta2envi (%s)}\n", t_stamp);
  fprintf(fp, "samples = %i\n", envi->samples);
  fprintf(fp, "lines = %i\n", envi->lines);
  fprintf(fp, "bands = %i\n", envi->bands);
  fprintf(fp, "header offset = %i\n", envi->header_offset);
  fprintf(fp, "file type = %s\n", envi->file_type);
  fprintf(fp, "data type = %i\n", envi->data_type);
  fprintf(fp, "interleave = %s\n", envi->interleave);
  fprintf(fp, "sensor type = %s\n", envi->sensor_type);
  fprintf(fp, "byte order = %i\n", envi->byte_order);
  if (meta->projection) {
    switch (meta->projection->type)
      {
      case UNIVERSAL_TRANSVERSE_MERCATOR:
	fprintf(fp, 
		"map info = {%s, %i, %i, %.4f, %.4f, %.4f, %.4f, %i, %s}\n", 
		envi->projection, envi->ref_pixel_x, envi->ref_pixel_y, 
		envi->pixel_easting, envi->pixel_northing, envi->proj_dist_x,
		envi->proj_dist_y, envi->projection_zone, envi->hemisphere);
	fprintf(fp, 
		"projection info = {3, %.4f, %.4f, %.4f, %.4f, " 
                "0.0, 0.0, 0.99996, %s}\n", 
		envi->semimajor_axis, envi->semiminor_axis, envi->center_lat,
		envi->center_lon, envi->projection);
	break;
      case POLAR_STEREOGRAPHIC:
	fprintf(fp, 
		"map info = {%s, %i, %i, %.4f, %.4f, %.4f, %.4f, %s}\n", 
		envi->projection, envi->ref_pixel_x, envi->ref_pixel_y, 
		envi->pixel_easting, envi->pixel_northing, envi->proj_dist_x,
		envi->proj_dist_y, envi->hemisphere);
	fprintf(fp, 
		"projection info = {31, %.4f, %.4f, %.4f, %.4f, " 
                "0.0, 0.0, %s}\n", 
		envi->semimajor_axis, envi->semiminor_axis, envi->center_lat,
		envi->center_lon, envi->projection);
	break;
      case ALBERS_EQUAL_AREA:
	fprintf(fp, 
		"map info = {%s, %i, %i  , %.4f, %.4f, %.4f, %.4f, %s}\n", 
		envi->projection, envi->ref_pixel_x, envi->ref_pixel_y, 
		envi->pixel_easting, envi->pixel_northing, envi->proj_dist_x,
		envi->proj_dist_y, envi->hemisphere);
	fprintf(fp, 
		"projection info = {9, %.4f, %.4f, %.4f, %.4f, " 
                "0.0, 0.0, %.4f, %.4f, %s}\n", 
		envi->semimajor_axis, envi->semiminor_axis, envi->center_lat,
		envi->center_lon, envi->standard_parallel1, 
		envi->standard_parallel2, envi->projection);
	break;
      case LAMBERT_CONFORMAL_CONIC:
	fprintf(fp, 
		"map info = {%s, %i, %i, %.4f, %.4f, %.4f, %.4f, %s}\n", 
		envi->projection, envi->ref_pixel_x, envi->ref_pixel_y, 
		envi->pixel_easting, envi->pixel_northing, envi->proj_dist_x,
		envi->proj_dist_y, envi->hemisphere);
	fprintf(fp, 
		"projection info = {4, %.4f, %.4f, %.4f, %.4f, " 
                "0.0, 0.0, %.4f, %.4f, %s}\n", 
		envi->semimajor_axis, envi->semiminor_axis, envi->center_lat,
		envi->center_lon, envi->standard_parallel1,
		envi->standard_parallel2, envi->projection);
	break;
      case LAMBERT_AZIMUTHAL_EQUAL_AREA:
	fprintf(fp, 
		"map info = {%s, %i, %i, %.4f, %.4f, %.4f, %.4f, %s}\n", 
		envi->projection, envi->ref_pixel_x, envi->ref_pixel_y, 
		envi->pixel_easting, envi->pixel_northing, envi->proj_dist_x,
		envi->proj_dist_y, envi->hemisphere);
	fprintf(fp, 
		"projection info = {11, %.4f, %.4f, %.4f, %.4f, " 
                "0.0, 0.0, %s}\n", 
		envi->semimajor_axis, envi->semiminor_axis, envi->center_lat,
		envi->center_lon, envi->projection);
	break;
      case STATE_PLANE:
      case SCANSAR_PROJECTION: 
      case LAT_LONG_PSEUDO_PROJECTION:
	break;
      }
  }
  fprintf(fp, "wavelength units = %s\n", envi->wavelength_units);
  /*** wavelength, data ignore and default stretch currently not used ***/
  FCLOSE(fp);

  /* Clean and report */
  meta_free(meta);
  asfPrintStatus("   Converted metadata file (%s) to ENVI header (%s)\n\n",
		 meta_name, envi_name);
  
  return 0;
}

