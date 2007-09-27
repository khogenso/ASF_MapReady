#include "asf_geocode.h"
#include "asf.h"
#include "cla.h"
#include "asf_nan.h"
#include "asf_meta.h"
#include "libasf_proj.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <glib.h>


static int print_warn = 1;

static void no_arg(char * option)
{
  if (print_warn)
    asfPrintWarning("No argument specified for option: %s\n", option);
}

static void pole_arg(void)
{
  if (print_warn)
    asfPrintWarning("Pole specified twice.\n");
}

static void double_arg(const char * option)
{
  if (print_warn)
    asfPrintWarning("Option occurs twice: %s\n", option);
}

//static void missing_arg(const char * option)
//{
//  if (print_warn)
//    asfPrintWarning(
//    "Projection requires option that was not specified: %s\n", option);
//}

void set_options_testing(int is_testing)
{
  print_warn = !is_testing;
}

static void readline(FILE * f, char * buffer, size_t n)
{
  char * p;
  p = fgets(buffer, n, f);

  if (!p)
  {
    strcpy(buffer, "");
  }
  else
  {
    p = buffer+strlen(buffer)-1;

    while (isspace((int)(*p)))
        *p-- = '\0';
  }
}

static int parse_val(char * inbuf, char * key, double * val)
{
  char * p, * eq, * buf;
  int match = FALSE;

  buf = g_strdup(inbuf);

  p = eq = strchr(buf, '=');
  if (!eq)
    return FALSE;

  *eq = '\0';
  --p;

  while (isspace((int)(*p)))
    *p-- = '\0';

  if (g_ascii_strcasecmp(buf, key) == 0)
  {
    p = eq + 1;
    while (isspace((int)(*p)))
      ++p;

    if (*p)
    {
      double d;
      if (parse_double(p, &d))
      {
        *val = d;
        match = TRUE;
      }
      else
      {
        asfPrintWarning("Illegal value found for key '%s': %s\n",
          key, p);
      }
    }
  }

  g_free(buf);
  return match;
}

static void get_fields(FILE * fp, ...)
{
  va_list ap;
  char * keys[32];
  double * vals[32];
  char buf[256];
  unsigned int nkeys = 0;

  va_start(ap, fp);
  while (nkeys < sizeof(keys))
  {
    keys[nkeys] = va_arg(ap, char *);
    if (!keys[nkeys])
      break;

    vals[nkeys] = va_arg(ap, double *);
    *(vals[nkeys]) = MAGIC_UNSET_DOUBLE;
    ++nkeys;
  }
  va_end(ap);

  while (!feof(fp))
  {
    unsigned int i;
    int found = FALSE;

    readline(fp, buf, sizeof(buf));

    if (strlen(buf) > 0)
    {
      for (i = 0; i < nkeys; ++i)
      {
        if (parse_val(buf, keys[i], vals[i]))
        {
          found = TRUE;
          break;
        }
      }

      ////Changed to ignore fields that aren't understood
      //if (!found)
      //  asfPrintWarning("Unknown key found in projection file: %s\n",
      //  buf);
    }
  }
}

static const char * bracketed_projection_name(projection_type_t proj_type)
{
  switch (proj_type)
  {
  case UNIVERSAL_TRANSVERSE_MERCATOR:
    return "[Universal Transverse Mercator]";

  case POLAR_STEREOGRAPHIC:
    return "[Polar Stereographic]";

  case ALBERS_EQUAL_AREA:
    return "[Albers Conical Equal Area]";

  case LAMBERT_AZIMUTHAL_EQUAL_AREA:
    return "[Lambert Azimuthal Equal Area]";

  case LAMBERT_CONFORMAL_CONIC:
    return "[Lambert Conformal Conic]";

  default:
    asfPrintError("projection_name: illegal projection type!");
    return "";
  }
}

#ifndef TEST_PARSE_OPTIONS
static
#endif
void write_args(projection_type_t proj_type, project_parameters_t *pps,
                char * file, datum_type_t datum)
{
  FILE * fp = FOPEN(file, "wt");

  fprintf(fp, "%s\n", bracketed_projection_name(proj_type));

  switch (proj_type)
  {
  case UNIVERSAL_TRANSVERSE_MERCATOR:
    fprintf(fp, "Scale Factor=%.10f\n", pps->utm.scale_factor);
    fprintf(fp, "Central Meridian=%.10f\n", pps->utm.lon0);
    fprintf(fp, "Latitude of Origin=%.10f\n", pps->utm.lat0);
    fprintf(fp, "False Easting=%.10f\n", pps->utm.false_easting);
    fprintf(fp, "False Northing=%.10f\n", pps->utm.false_northing);
    fprintf(fp, "Zone=%d\n", pps->utm.zone);
    fprintf(fp, "datum=%s\n", datum_toString(datum));
    break;

  case POLAR_STEREOGRAPHIC:
    {
      char spheroid_str[256];
      if (datum == HUGHES_DATUM) {
        strcpy(spheroid_str, "HUGHES");
      }
      else {
        strcpy(spheroid_str, "WGS84");
      }
      fprintf(fp, "First Standard Parallel=%.10f\n", pps->ps.slat);
      fprintf(fp, "Central Meridian=%.10f\n", pps->ps.slon);
      fprintf(fp, "False Easting=%.10f\n", pps->ps.false_easting);
      fprintf(fp, "False Northing=%.10f\n", pps->ps.false_northing);
      fprintf(fp, "Northern Projection=%d\n", pps->ps.is_north_pole);
      fprintf(fp, "spheroid=%s\n", spheroid_str);
    }
    break;

  case ALBERS_EQUAL_AREA:
    fprintf(fp, "First standard parallel=%.10f\n",
      pps->albers.std_parallel1);
    fprintf(fp, "Second standard parallel=%.10f\n",
      pps->albers.std_parallel2);
    fprintf(fp, "Central Meridian=%.10f\n",
      pps->albers.center_meridian);
    fprintf(fp, "Latitude of Origin=%.10f\n",
      pps->albers.orig_latitude);
    fprintf(fp, "False Easting=%.10f\n", pps->albers.false_easting);
    fprintf(fp, "False Northing=%.10f\n", pps->albers.false_northing);
    fprintf(fp, "datum=%s\n", datum_toString(datum));
    break;

  case LAMBERT_AZIMUTHAL_EQUAL_AREA:
    fprintf(fp, "Central Meridian=%.10f\n", pps->lamaz.center_lon);
    fprintf(fp, "Latitude of Origin=%.10f\n", pps->lamaz.center_lat);
    fprintf(fp, "False Easting=%.10f\n", pps->lamaz.false_easting);
    fprintf(fp, "False Northing=%.10f\n", pps->lamaz.false_northing);
    fprintf(fp, "datum=%s\n", datum_toString(datum));
    break;

  case LAMBERT_CONFORMAL_CONIC:
    fprintf(fp, "First standard parallel=%.10f\n", pps->lamcc.plat1);
    fprintf(fp, "Second standard parallel=%.10f\n", pps->lamcc.plat2);
    fprintf(fp, "Central Meridian=%.10f\n", pps->lamcc.lon0);
    fprintf(fp, "Latitude of Origin=%.10f\n", pps->lamcc.lat0);
    fprintf(fp, "False Easting=%.10f\n", pps->lamcc.false_easting);
    fprintf(fp, "False Northing=%.10f\n", pps->lamcc.false_northing);
    fprintf(fp, "datum=%s\n", datum_toString(datum));

    //fprintf(fp, "Scale Factor=%.10f\n",
    //ISNAN(pps->lamcc.scale_factor) ?
    //1.0 :
    //pps->lamcc.scale_factor);

    break;

  default:
    asfPrintError("write_args: illegal projection type!");
    break;
  }

  fclose(fp);
}

void parse_proj_args_file(const char * file, project_parameters_t * pps,
        projection_type_t * proj_type, datum_type_t *datum)
{
  FILE * fp;
  char buf[256];

  fp = fopen_proj_file(file, "r");
  if (!fp)
  {
    asfPrintError("Couldn't open projection file: %s\n", file);
    return;
  }

  readline(fp, buf, sizeof(buf));

  *datum = WGS84_DATUM;
  if (strcmp(buf, bracketed_projection_name(ALBERS_EQUAL_AREA)) == 0 ||
    strcmp(buf, "[Albers Equal Area Conic]") == 0)
  {
    *proj_type = ALBERS_EQUAL_AREA;
    get_fields(fp,
      "First standard parallel", &pps->albers.std_parallel1,
      "Second standard parallel", &pps->albers.std_parallel2,
      "Central Meridian", &pps->albers.center_meridian,
      "Latitude of Origin", &pps->albers.orig_latitude,
      "False Easting", &pps->albers.false_easting,
      "False Northing", &pps->albers.false_northing,
      NULL);
    *datum = get_datum(fp);
  }
  else if (strcmp(buf, bracketed_projection_name(
    LAMBERT_AZIMUTHAL_EQUAL_AREA)) == 0)
  {
    *proj_type = LAMBERT_AZIMUTHAL_EQUAL_AREA;
    get_fields(fp,
      "Central Meridian", &pps->lamaz.center_lon,
      "Latitude of Origin", &pps->lamaz.center_lat,
      "False Easting", &pps->lamaz.false_easting,
      "False Northing", &pps->lamaz.false_northing,
      NULL);
    *datum = get_datum(fp);
  }
  else if (strcmp(buf, bracketed_projection_name(
    LAMBERT_CONFORMAL_CONIC)) == 0)
  {
    *proj_type = LAMBERT_CONFORMAL_CONIC;
    get_fields(fp,
      "First standard parallel", &pps->lamcc.plat1,
      "Second standard parallel", &pps->lamcc.plat2,
      "Central Meridian", &pps->lamcc.lon0,
      "Latitude of Origin", &pps->lamcc.lat0,
      "False Easting", &pps->lamcc.false_easting,
      "False Northing", &pps->lamcc.false_northing,
      /* "Scale Factor", &pps->lamcc.scale_factor, */
      NULL);
    *datum = get_datum(fp);
  }
  else if (strcmp(buf, bracketed_projection_name(POLAR_STEREOGRAPHIC)) == 0)
  {
    spheroid_type_t spheroid;
    //char area[10];
    double is_north_pole;
    *proj_type = POLAR_STEREOGRAPHIC;
    get_fields(fp,
      "Standard parallel", &pps->ps.slat,
      "Standard Parallel", &pps->ps.slat,
      "First Standard Parallel", &pps->ps.slat,
      "Central Meridian", &pps->ps.slon,
      "False Easting", &pps->ps.false_easting,
      "False Northing", &pps->ps.false_northing,
      "Hemisphere", &is_north_pole,
      NULL);
    pps->ps.is_north_pole = pps->ps.slat > 0;
    spheroid = get_spheroid(fp);
    switch(spheroid) {
      case WGS84_SPHEROID:
        *datum = WGS84_DATUM;
        break;
      case HUGHES_SPHEROID:
        *datum = HUGHES_DATUM;
        break;
      case BESSEL_SPHEROID:
      case CLARKE1866_SPHEROID:
      case CLARKE1880_SPHEROID:
      case GEM6_SPHEROID:
      case GEM10C_SPHEROID:
      case GRS1980_SPHEROID:
      case INTERNATIONAL1924_SPHEROID:
      case INTERNATIONAL1967_SPHEROID:
      case WGS72_SPHEROID:
      default:
        asfPrintWarning("Unknown, unrecognized, or unsupported reference ellipsoid found for.\n"
                       "a polar stereographic projection.\n");
        *datum = UNKNOWN_DATUM;
    }
  }
  else if (strcmp(buf, bracketed_projection_name(
    UNIVERSAL_TRANSVERSE_MERCATOR)) == 0)
  {
    double zone;
    *proj_type = UNIVERSAL_TRANSVERSE_MERCATOR;
    get_fields(fp,
      "Scale Factor", &pps->utm.scale_factor,
      "Central Meridian", &pps->utm.lon0,
      "Latitude of Origin", &pps->utm.lat0,
      "False Easting", &pps->utm.false_easting,
      "False Northing", &pps->utm.false_northing,
      "Zone", &zone,
      NULL);
    *datum = WGS84_DATUM;

    pps->utm.zone = (int) zone;

    if (pps->utm.zone == 0 || ISNAN(zone))
      pps->utm.zone = MAGIC_UNSET_INT;
  }
  else
  {
    asfPrintError("Unknown Projection in file '%s': %s\n",
      file, buf);
  }
  if (*datum == UNKNOWN_DATUM) {
    asfPrintWarning("Datum or reference ellipsoid missing in projection file.  Defaulting\n"
                   "to a WGS84 datum\n");
    *datum = WGS84_DATUM;
  }

  fclose(fp);
}

static int matches_write_proj_file(char * param)
{
  return
       strcmp(param, "-wpf") == 0
    || strcmp(param, "-write-proj-file") == 0
    || strcmp(param, "--write-proj-file") == 0;
}

static int matches_read_proj_file(char * param)
{
  return
       strcmp(param, "-rpf") == 0
    || strcmp(param, "-read-proj-file") == 0
    || strcmp(param, "--read-proj-file") == 0;
}

static int parse_zone_option(int *i, int argc,
                             char *argv[], int *specified,
                             int *value, int *ok)
{
  if (strcmp(argv[*i], "--zone") == 0 || strcmp(argv[*i], "-zone") == 0 ||
    strcmp(argv[*i], "-z") == 0)
  {
    *ok = parse_int_option(i, argc, argv, specified, value);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static int parse_first_standard_parallel_option(int *i, int argc,
                                                char *argv[], int *specified,
                                                double *value, int *ok)
{
  if (strcmp(argv[*i], "--first-standard-parallel") == 0 ||
      strcmp(argv[*i], "--plat1") == 0 ||
      strcmp(argv[*i], "-first-standard-parallel") == 0 ||
      strcmp(argv[*i], "-plat1") == 0)
  {
    *ok = parse_double_option(i, argc, argv, specified, value);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static int parse_second_standard_parallel_option(int *i, int argc,
                                                 char *argv[], int *specified,
                                                 double *value, int *ok)
{
  if (strcmp(argv[*i], "--second-standard-parallel") == 0 ||
      strcmp(argv[*i], "-second-standard-parallel") == 0 ||
      strcmp(argv[*i], "--plat2") == 0 ||
      strcmp(argv[*i], "-plat2") == 0)
  {
    *ok = parse_double_option(i, argc, argv, specified, value);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static int parse_center_latitude_option(int *i, int argc,
                                        char *argv[], int *specified,
                                        double *value, int *ok)
{
  if (strcmp(argv[*i], "--center-latitude") == 0 ||
    strcmp(argv[*i], "-center-latitude") == 0 ||
    strcmp(argv[*i], "--latitude-of-origin") == 0 ||
    strcmp(argv[*i], "-latitude-of-origin") == 0 ||
    strcmp(argv[*i], "--lat0") == 0 ||
    strcmp(argv[*i], "-lat0") == 0)
  {
    *ok = parse_double_option(i, argc, argv, specified, value);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static int parse_central_meridian_option(int *i, int argc,
                                         char *argv[], int *specified,
                                         double *value, int *ok)
{
  if (strcmp(argv[*i], "--central-meridian") == 0 ||
      strcmp(argv[*i], "-central-meridian") == 0 ||
      strcmp(argv[*i], "-lon0") == 0 ||
      strcmp(argv[*i], "--lon0") == 0)
  {
    *ok = parse_double_option(i, argc, argv, specified, value);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static int parse_false_easting_option(int *i, int argc,
                                      char *argv[], int *specified,
                                      double *value, int *ok)
{
  if (strcmp(argv[*i], "--false-easting") == 0)
  {
    *ok = parse_double_option(i, argc, argv, specified, value);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static int parse_false_northing_option(int *i, int argc,
                                       char *argv[], int *specified,
                                       double *value, int *ok)
{
  if (strcmp(argv[*i], "--false-northing") == 0)
  {
    *ok = parse_double_option(i, argc, argv, specified, value);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//static int parse_scale_factor_option(int *i, int argc,
//                   char *argv[], int *specified,
//                   double *value, int *ok)
//{
//  if (strcmp(argv[*i], "--scale-factor") == 0)
//  {
//    *ok = parse_double_option(i, argc, argv, specified, value);
//    return TRUE;
//  }
//  else
//  {
//    return FALSE;
//  }
//}

static int parse_write_proj_file_option(int *i, int argc, char *argv[],
                                        char **write_file, int *ok)
{
  if (matches_write_proj_file(argv[*i]))
  {
    ++(*i);

    if (*i == argc)
    {
      no_arg(argv[*i-1]);
      *ok = FALSE;
    }
    else
    {
      *write_file = argv[*i];
      *ok = TRUE;
    }

    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static int parse_read_proj_file_option(int *i, int argc, char *argv[],
                                       projection_type_t *proj_type,
                                       datum_type_t *datum,
                                       project_parameters_t *pps, int *ok)
{
  if (matches_read_proj_file(argv[*i]))
  {
    ++(*i);

    if (*i == argc)
    {
      no_arg(argv[*i-1]);
      *ok = FALSE;
    }
    else
    {
      parse_proj_args_file(argv[*i], pps, proj_type, datum);
      *ok = TRUE;
    }

    ++(*i);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static void extract_flag(int *argc, char **argv[],
                         char *arg, int *found)
{
  int i;

  for (i = 0; i < *argc; ++i)
  {
    if (strcmp((*argv)[i], arg) == 0)
    {
      *found = TRUE;
      remove_args(i, i, argc, argv);
    }
  }
}

static int extract_flags(int *argc, char ***argv, ... )
{
  va_list ap;
  char * arg = NULL;
  int found = FALSE;

  va_start(ap, argv);
  do
  {
    arg = va_arg(ap, char *);

    if (arg)
      extract_flag(argc, argv, arg, &found);
  }
  while (arg);

  return found;
}

static datum_type_t parse_datum_option(int *argc, char **argv[])
{
  char datum[1024];
  datum_type_t the_datum = UNKNOWN_DATUM;

  if (extract_string_options(argc, argv, datum, "--datum", "-datum", NULL))
  {
    if (g_ascii_strcasecmp(datum, "WGS84") == 0)
    {
      the_datum = WGS84_DATUM;
    }
    else if (g_ascii_strcasecmp(datum, "NAD27") == 0 ||
      g_ascii_strcasecmp(datum, "clrk66") == 0)
    {
      the_datum = NAD27_DATUM;
    }
    else if (g_ascii_strcasecmp(datum, "NAD83") == 0)
    {
      the_datum = NAD83_DATUM;
    }
    else if (g_ascii_strcasecmp(datum, "HUGHES") == 0)
    {
      the_datum = HUGHES_DATUM;
    }
    else
    {
      asfPrintWarning("Unknown Datum: %s.  Using WGS84.\n", datum);
      the_datum = WGS84_DATUM;
    }
  }

  return the_datum;
}


static resample_method_t parse_resample_method_option(int *argc, char **argv[])
{
  char resample_method[1024];

  /* To be returned (default to bilinear).  */
  resample_method_t ret = RESAMPLE_BILINEAR;

  if (extract_string_options(argc, argv, resample_method, "--resample-method",
    "-resample-method", NULL))
  {
    if (g_ascii_strcasecmp(resample_method, "nearest_neighbor") == 0)
    {
      ret = RESAMPLE_NEAREST_NEIGHBOR;
    }
    else if (g_ascii_strcasecmp(resample_method, "bilinear") == 0)
    {
      ret = RESAMPLE_BILINEAR;
    }
    else if (g_ascii_strcasecmp(resample_method, "bicubic") == 0)
    {
      ret = RESAMPLE_BICUBIC;
    }
  }
  else
  {
    ret = RESAMPLE_BILINEAR;
  }

  return ret;
}

void parse_other_options(int *argc, char **argv[],
                         double *height, double *pixel_size,
                         datum_type_t *datum,
                         resample_method_t *resample_method,
                         int *override_checks,
                         char *band_id)
{
  *datum = parse_datum_option(argc, argv);
        extract_string_options(argc, argv, band_id, "-band", "--band", NULL);
  extract_double_options(argc, argv, height, "--height", "-height", "-h", NULL);
  extract_double_options(argc, argv, pixel_size,
    "--pixel-size", "-pixel-size", "-pix", "-ps", NULL);
  *resample_method = parse_resample_method_option (argc, argv);
  *override_checks = extract_flags(argc, argv, "--force", "-force", "-f", NULL);

  //debug code for printing out the options after parsing ...
  //printf("argc, argv:\n");
  //int i;
  //for (i=0;i<*argc;++i) { printf(" %d: %s\n", i, (*argv)[i]); }
  //printf("pixel size: %g\n", *pixel_size);
  //exit(1);
}

project_parameters_t * parse_projection_options(int *argc, char **argv[],
                                                projection_type_t * proj_type,
                                                datum_type_t *datum,
                                                int *did_write_proj_file)
{
  int i;
  project_parameters_t *pps = malloc(sizeof(project_parameters_t));
  int start_arg = 0, end_arg = 0;
  char * write_file = NULL;
  int ok = TRUE;

  for (i = 0; i < *argc; ++i)
  {
    if (parse_read_proj_file_option(&i, *argc, *argv, proj_type, datum, pps, &ok))
    {
      if (!ok) {
        FREE(pps);
        return NULL;
      }

      break;
    }

    else if (strcmp((*argv)[i], "--projection") == 0 ||
       strcmp((*argv)[i], "-projection") == 0 ||
       strcmp((*argv)[i], "-p") == 0)
    {
      if (++i == *argc)
      {
        no_arg((*argv)[i-1]);
        FREE(pps);
        return NULL;
      }

      if (strcmp((*argv)[i], "utm") == 0)
      {
        int specified_zone = FALSE;
        int specified_lat0 = FALSE;
        int specified_lon0 = FALSE;

        *proj_type = UNIVERSAL_TRANSVERSE_MERCATOR;

        while (TRUE)
        {
          if (!ok) {
            FREE(pps);
            return NULL;
          }

          if (++i == *argc)
            break;

          if (parse_write_proj_file_option(
            &i, *argc, *argv, &write_file, &ok))
            continue;

          if (parse_zone_option(
            &i, *argc, *argv, &specified_zone,
            &pps->utm.zone, &ok))
            continue;

          if (parse_center_latitude_option(
            &i, *argc, *argv, &specified_lat0,
            &pps->utm.lat0, &ok))
            continue;

          if (parse_central_meridian_option(
            &i, *argc, *argv, &specified_lon0,
            &pps->utm.lon0, &ok))
            continue;

          break;
        }

        if (!specified_lon0)
          pps->utm.lon0 = MAGIC_UNSET_DOUBLE;

        if (!specified_lat0)
          pps->utm.lat0 = MAGIC_UNSET_DOUBLE;

        if (!specified_zone)
          pps->utm.zone = MAGIC_UNSET_INT;

        break;
      }
      else if (strcmp((*argv)[i], "ps") == 0)
      {
        int specified_slat = FALSE;
        int specified_slon = FALSE;
        int specified_pole = FALSE;
        int specified_false_northing = FALSE;
        int specified_false_easting = FALSE;

        pps->ps.is_north_pole = 1;
        *proj_type = POLAR_STEREOGRAPHIC;

        while (TRUE)
        {
          if (!ok) {
            FREE(pps);
            return NULL;
          }

          if (++i == *argc)
            break;

          if (parse_write_proj_file_option(
            &i, *argc, *argv, &write_file, &ok))
            continue;

          if (parse_first_standard_parallel_option(
            &i, *argc, *argv, &specified_slat,
            &pps->ps.slat, &ok))
            continue;

          if (parse_central_meridian_option(
            &i, *argc, *argv, &specified_slon,
            &pps->ps.slon, &ok))
            continue;

          if (parse_false_easting_option(
            &i, *argc, *argv, &specified_false_easting,
            &pps->ps.false_easting, &ok))
            continue;

          if (parse_false_northing_option(
            &i, *argc, *argv, &specified_false_northing,
            &pps->ps.false_northing, &ok))
            continue;

          if (strcmp((*argv)[i], "--north-pole") == 0 ||
            strcmp((*argv)[i], "-n") == 0)
          {
            if (specified_pole)
            {
              double_arg((*argv)[i]);
              FREE(pps);
              return NULL;
            }

            specified_pole = 1;

            pps->ps.is_north_pole = 1;
            continue;
          }

          if (strcmp((*argv)[i], "--south-pole") == 0 ||
            strcmp((*argv)[i], "-s") == 0)
          {
            if (specified_pole)
            {
              pole_arg();
              FREE(pps);
              return NULL;
            }

            specified_pole = 1;

            pps->ps.is_north_pole = 0;
            continue;
          }
          break;
        }

        if (!specified_slat)
          pps->ps.slat = MAGIC_UNSET_DOUBLE;

        if (!specified_slon)
          pps->ps.slon = MAGIC_UNSET_DOUBLE;

        if (!specified_false_easting)
          pps->ps.false_easting = MAGIC_UNSET_DOUBLE;

        if (!specified_false_northing)
          pps->ps.false_northing = MAGIC_UNSET_DOUBLE;

        if (!specified_slat || !specified_slon) {
          FREE(pps);
          return NULL;
        }

        break;
      }
      else if (strcmp((*argv)[i], "lamcc") == 0)
      {
        int specified_plat1 = FALSE;
        int specified_plat2 = FALSE;
        int specified_lat0 = FALSE;
        int specified_lon0 = FALSE;
        int specified_false_easting = FALSE;
        int specified_false_northing = FALSE;
        int specified_scale_factor = FALSE;
        *proj_type = LAMBERT_CONFORMAL_CONIC;

        while (TRUE)
        {
          if (!ok){
            FREE(pps);
            return NULL;
          }

          if (++i == *argc)
            break;

          if (parse_write_proj_file_option(
            &i, *argc, *argv, &write_file, &ok))
            continue;

          if (parse_first_standard_parallel_option(
            &i, *argc, *argv, &specified_plat1,
            &pps->lamcc.plat1, &ok))
            continue;

          if (parse_second_standard_parallel_option(
            &i, *argc, *argv, &specified_plat2,
            &pps->lamcc.plat2, &ok))
            continue;

          if (parse_center_latitude_option(
            &i, *argc, *argv, &specified_lat0,
            &pps->lamcc.lat0, &ok))
            continue;

          if (parse_central_meridian_option(
            &i, *argc, *argv, &specified_lon0,
            &pps->lamcc.lon0, &ok))
            continue;

          if (parse_false_easting_option(
            &i, *argc, *argv, &specified_false_easting,
            &pps->lamcc.false_easting, &ok))
            continue;

          if (parse_false_northing_option(
            &i, *argc, *argv, &specified_false_northing,
            &pps->lamcc.false_northing, &ok))
            continue;

          //if (parse_scale_factor_option(
          //  &i, *argc, *argv, &specified_scale_factor,
          //  &pps->lamcc.scale_factor, &ok))
          //  continue;

          break;
        }

        if (!specified_plat1)
          pps->lamcc.plat1 = MAGIC_UNSET_DOUBLE;

        if (!specified_plat2)
          pps->lamcc.plat2 = MAGIC_UNSET_DOUBLE;

        if (!specified_lat0)
          pps->lamcc.lat0 = MAGIC_UNSET_DOUBLE;

        if (!specified_lon0)
          pps->lamcc.lon0 = MAGIC_UNSET_DOUBLE;

        if (!specified_false_easting)
          pps->lamcc.false_easting = MAGIC_UNSET_DOUBLE;

        if (!specified_false_northing)
          pps->lamcc.false_northing = MAGIC_UNSET_DOUBLE;

        if (!specified_scale_factor)
          pps->lamcc.scale_factor = MAGIC_UNSET_DOUBLE;

        if (!specified_lat0 && !(specified_plat1 && specified_plat2)) {
          asfPrintWarning("Latitude of origin not specified.  If the latitude\n"
              "of origin is not specified, then the image center will be used\n"
              "instead ...but only if the first and second standard parallels\n"
              "WERE specified ...and they were not.\n");
          FREE(pps);
          return NULL;
        }
        else if (!specified_plat1 || !specified_plat2 ||
             !specified_lat0 || !specified_lon0)
        {
          FREE(pps);
          return NULL;
        }

        break;
      }
      else if (strcmp((*argv)[i], "lamaz") == 0)
      {
        int specified_lat0 = FALSE;
        int specified_lon0 = FALSE;
        int specified_false_easting = FALSE;
        int specified_false_northing = FALSE;

        *proj_type = LAMBERT_AZIMUTHAL_EQUAL_AREA;

        while (TRUE)
        {
          if (!ok)
            return NULL;

          if (++i == *argc)
            break;

          if (parse_write_proj_file_option(
            &i, *argc, *argv, &write_file, &ok))
            continue;

          if (parse_center_latitude_option(
            &i, *argc, *argv, &specified_lat0,
            &pps->lamaz.center_lat, &ok))
            continue;

          if (parse_central_meridian_option(
            &i, *argc, *argv, &specified_lon0,
            &pps->lamaz.center_lon, &ok))
            continue;

          if (parse_false_easting_option(
            &i, *argc, *argv, &specified_false_easting,
            &pps->lamaz.false_easting, &ok))
            continue;

          if (parse_false_northing_option(
            &i, *argc, *argv, &specified_false_northing,
            &pps->lamaz.false_northing, &ok))
            continue;

          break;
        }

        if (!specified_lat0)
          pps->lamaz.center_lat = MAGIC_UNSET_DOUBLE;

        if (!specified_lon0)
          pps->lamaz.center_lon = MAGIC_UNSET_DOUBLE;

        if (!specified_false_easting)
          pps->lamaz.false_easting = MAGIC_UNSET_DOUBLE;

        if (!specified_false_northing)
          pps->lamaz.false_northing = MAGIC_UNSET_DOUBLE;

        if (!specified_lon0 ||
            (!specified_lat0 && !specified_lon0))
        {
          FREE(pps);
          return NULL;
        }
        else if (!specified_lat0 && specified_lon0) {
          asfPrintStatus("Latitude of origin (point of tangency) not specified.  Image\n"
                        "center will be used instead.\n");
        }

        break;
      }
      else if (strcmp((*argv)[i], "albers") == 0)
      {
        int specified_plat1 = FALSE;
        int specified_plat2 = FALSE;
        int specified_lat0 = FALSE;
        int specified_lon0 = FALSE;
        int specified_false_easting = FALSE;
        int specified_false_northing = FALSE;

        *proj_type = ALBERS_EQUAL_AREA;

        while (TRUE)
        {
          if (!ok)
            return NULL;

          if (++i == *argc)
            break;

          if (parse_write_proj_file_option(
            &i, *argc, *argv, &write_file, &ok))
            continue;

          if (parse_first_standard_parallel_option(
            &i, *argc, *argv, &specified_plat1,
            &pps->albers.std_parallel1, &ok))
            continue;

          if (parse_second_standard_parallel_option(
            &i, *argc, *argv, &specified_plat2,
            &pps->albers.std_parallel2, &ok))
            continue;

          if (parse_center_latitude_option(
            &i, *argc, *argv, &specified_lat0,
            &pps->albers.orig_latitude, &ok))
            continue;

          if (parse_central_meridian_option(
            &i, *argc, *argv, &specified_lon0,
            &pps->albers.center_meridian, &ok))
            continue;

          if (parse_false_easting_option(
            &i, *argc, *argv, &specified_false_easting,
            &pps->albers.false_easting, &ok))
            continue;

          if (parse_false_northing_option(
            &i, *argc, *argv, &specified_false_northing,
            &pps->albers.false_northing, &ok))
            continue;

          break;
        }

        if (!specified_plat1)
          pps->albers.std_parallel1 = MAGIC_UNSET_DOUBLE;

        if (!specified_plat2)
          pps->albers.std_parallel2 = MAGIC_UNSET_DOUBLE;

        if (!specified_lat0)
          pps->albers.orig_latitude = MAGIC_UNSET_DOUBLE;

        if (!specified_lon0)
          pps->albers.center_meridian = MAGIC_UNSET_DOUBLE;

        if (!specified_false_easting)
          pps->albers.false_easting = MAGIC_UNSET_DOUBLE;

        if (!specified_false_northing)
          pps->albers.false_northing = MAGIC_UNSET_DOUBLE;

        if (!specified_plat1 ||
            !specified_plat2 ||
            !specified_lon0)
        {
          FREE(pps);
          return NULL;
        }
        else if (!specified_lat0 && specified_plat1 && specified_plat2) {
          asfPrintWarning("Latitude of origin not specified.  Image center\n"
                         "will be utilized instead.\n");
        }


        break;
      }
      else
      {
        asfPrintWarning("Unknown projection: %s\n", (*argv)[i]);
        return NULL;
      }
    }

    ++start_arg;
  }

  end_arg = i - 1;

  if (start_arg >= end_arg)
  {
    /* don't need this -- caller detects and prints a better message */
    /* asfPrintWarning("No projection specified\n"); */
    return NULL;
  }
  else
  {
    remove_args(start_arg, end_arg, argc, argv);

    for (i = 0; i < *argc; ++i)
    {
      if (parse_write_proj_file_option(
        &i, *argc, *argv, &write_file, &ok))
      {
        remove_args(i-1, i, argc, argv);
        break;
      }
    }

    if (write_file)
    {
      *did_write_proj_file = TRUE;
      asfPrintStatus("Projection file \"%s\" written.\n", write_file);
      write_args(*proj_type, pps, write_file, *datum);
    }

    return pps;
  }
}

datum_type_t get_datum(FILE *fp)
{
  datum_type_t datum = UNKNOWN_DATUM; // Unknown until found
  char *eq, *s;
  char buf[512];
  if (!fp) asfPrintError("Projection parameter file not open.\n");
  FSEEK(fp, 0, SEEK_SET);
  readline(fp, buf, sizeof(buf));
  while (!feof(fp)) {
    if (strlen(buf)) {
      s = buf;
      while(isspace((int)*s))s++;
      if (strncmp(uc(s), "DATUM", 5) == 0) {
        eq = strchr(buf, '=');
        if (eq) {
          s = eq;
          s++;
          while (isspace((int)*s))s++;
          if (strncmp(uc(s), "EGM96", 5) == 0) {
            datum = EGM96_DATUM;
            break;
          }
          else if (strncmp(uc(s), "ED50", 4)   == 0) {
            datum = ED50_DATUM;
            break;
          }
          else if (strncmp(uc(s), "ETRF89", 6) == 0) {
            datum = ETRF89_DATUM;
            break;
          }
          else if (strncmp(uc(s), "ETRS89", 6) == 0) {
            datum = ETRS89_DATUM;
            break;
          }
          else if (strncmp(uc(s), "ITRF97", 6) == 0) {
            datum = ITRF97_DATUM;
            break;
          }
          else if (strncmp(uc(s), "NAD27", 5)  == 0) {
            datum = NAD27_DATUM;
            break;
          }
          else if (strncmp(uc(s), "NAD83", 5)  == 0) {
            datum = NAD83_DATUM;
            break;
          }
          else if (strncmp(uc(s), "WGS72", 5)  == 0) {
            datum = WGS72_DATUM;
            break;
          }
          else if (strncmp(uc(s), "WGS84", 5)  == 0) {
            datum = WGS84_DATUM;
            break;
          }
          else if (strncmp(uc(s), "HUGHES", 6) == 0) {
            datum = HUGHES_DATUM;
            break;
          }
        }
      }
    }
    readline(fp, buf, sizeof(buf));
  }

  return datum;
}

spheroid_type_t get_spheroid(FILE *fp)
{
  spheroid_type_t spheroid = UNKNOWN_SPHEROID; // Unknown until found
  char *eq, *s;
  char buf[512];
  if (!fp) asfPrintError("Projection parameter file not open.\n");
  FSEEK(fp, 0, SEEK_SET);
  readline(fp, buf, sizeof(buf));

  while (!feof(fp)) {
    if (strlen(buf)) {
      s = buf;
      while(isspace((int)*s))s++;
      if (strncmp(uc(s), "SPHEROID", 5) == 0) {
        eq = strchr(buf, '=');
        if (eq) {
          s = eq;
          s++;
          while (isspace((int)*s))s++;
          if (strncmp(uc(s), "BESSEL", 5) == 0) {
            spheroid = BESSEL_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "CLARKE1866", 4)   == 0) {
            spheroid = CLARKE1866_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "CLARKE1880", 6) == 0) {
            spheroid = CLARKE1880_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "GEM6", 6) == 0) {
            spheroid = GEM6_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "GEM10C", 6) == 0) {
            spheroid = GEM10C_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "GRS1980", 5)  == 0) {
            spheroid = GRS1980_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "INTERNATIONAL1924", 5)  == 0) {
            spheroid = INTERNATIONAL1924_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "INTERNATIONAL1967", 5)  == 0) {
            spheroid = INTERNATIONAL1967_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "WGS72", 5)  == 0) {
            spheroid = WGS72_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "WGS84", 6) == 0) {
            spheroid = WGS84_SPHEROID;
            break;
          }
          else if (strncmp(uc(s), "HUGHES", 6) == 0) {
            spheroid = HUGHES_SPHEROID;
            break;
          }
        }
      }
    }
    readline(fp, buf, sizeof(buf));
  }

  return spheroid;
}

































