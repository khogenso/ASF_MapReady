#include "asf.h"
#include "asf_convert.h"
#include "asf_sar.h"
#include <ctype.h>

static int strindex(char s[], char t[])
{
  int i, j, k;

  for (i=0; s[i]!='\0'; i++) {
    for (j=i, k=0; t[k]!='\0' && s[j]==t[k]; j++, k++)
      ;
    if (k>0 && t[k]=='\0')
      return i;
  }
  return -1;
}

static void apply_default_dirs(convert_config *cfg)
{ 
  // apply default dir to input name if there isn't a dir there already
  char file_dir[4096], file[4096];
  split_dir_and_file(cfg->general->in_name, file_dir, file);
  int file_len = strlen(file);
  int file_dir_len = strlen(file_dir);
  int def_dir_len = strlen(cfg->general->default_in_dir);
  if (file_dir_len==0 && def_dir_len!=0 && file_len>0) {
    char *tmpstr = (char*)MALLOC(sizeof(char)*(file_len+def_dir_len+2));
    strcpy(tmpstr, cfg->general->default_in_dir);
    strcat(tmpstr, DIR_SEPARATOR_STR);
    strcat(tmpstr, file);
    cfg->general->in_name = (char*)realloc(cfg->general->in_name,
                                           sizeof(char)*(strlen(tmpstr)+2));
    strcpy(cfg->general->in_name, tmpstr);
    FREE(tmpstr);
  }

  // apply default dir to output name if there isn't a dir there already
  split_dir_and_file(cfg->general->out_name, file_dir, file);
  file_len = strlen(file);
  file_dir_len = strlen(file_dir);
  def_dir_len = strlen(cfg->general->default_out_dir);
  if (file_dir_len==0 && def_dir_len!=0 && file_len>0) {
    char *tmpstr = (char*)MALLOC(sizeof(char)*(file_len+def_dir_len+2));
    strcpy(tmpstr, cfg->general->default_out_dir);
    strcat(tmpstr, DIR_SEPARATOR_STR);
    strcat(tmpstr, file);
    cfg->general->out_name = (char*)realloc(cfg->general->out_name,
                                            sizeof(char)*(strlen(tmpstr)+2));
    strcpy(cfg->general->out_name, tmpstr);
    FREE(tmpstr);
  }
}

static char *read_param(char *line)
{
  int i, k;
  char *value=(char *)MALLOC(sizeof(char)*255);

  strcpy(value, "");
  i=strindex(line, "]");
  k=strindex(line, "=");
  if (i>0) strncpy(value, line, i+1);
  if (k>0) strncpy(value, line, k);
  return value;
}

static char *read_str(char *line, char *param)
{
  static char value[255];
  char *p = strchr(line, '=');

  // skip past the '=' sign, and eat up any whitespace
  ++p;
  while (isspace(*p))
      ++p;

  strcpy(value, p);

  // eat up trailing whitespace, too
  p = value + strlen(value) - 1;
  while (isspace(*p))
      *p-- = '\0';

  return value;
}

static int read_int(char *line, char *param)
{
  char *tmp;
  int value;

  tmp = read_str(line, param);
  sscanf(tmp, "%i", &value);

  return value;
}

static double read_double(char *line, char *param)
{
  char *tmp;
  double value;

  tmp = read_str(line, param);
  sscanf(tmp, "%lf", &value);

  return value;
}

int init_convert_config(char *configFile)
{
  FILE *fConfig;

  if ( fileExists(configFile) ) {
    asfPrintError("Cannot create file, %s, because it already exists.\n",
                  configFile);
  }
  fConfig = FOPEN(configFile, "w");

  fprintf(fConfig, "asf_convert configuration file\n\n");

  fprintf(fConfig, "[General]\n\n");
  // input file
  fprintf(fConfig, "# This parameter looks for the basename of the input file\n\n");
  fprintf(fConfig, "input file = \n\n");
  // default input directory
  fprintf(fConfig, "# Directory where to look for the input file(s). If there is a\n"
                   "# directory already specified in the input file parameter, this\n"
		   "# value will be ignored.\n\n");
  fprintf(fConfig, "default input dir = \n\n");
  // output file
  fprintf(fConfig, "# This parameter looks for the basename of the output file\n\n");
  fprintf(fConfig, "output file = \n\n");
  // default output directory
  fprintf(fConfig, "# Directory where to put the output file(s). If there is a\n"
                   "# directory already specified in the output file parameter, this\n"
		   "# value will be ignored.\n\n");
  fprintf(fConfig, "default output dir = \n\n");
  // import flag
  fprintf(fConfig, "# The import flag indicates whether the data needs to be run through\n"
          "# 'asf_import' (1 for running it, 0 for leaving out the import step).\n"
          "# For example, setting the import switch to zero assumes that all the data \n"
          "# is already in the ASF internal format.\n");
  fprintf(fConfig, "# Running asf_convert with the -create option and the import flag\n"
          "# switched on will generate an [Import] section where you can define further\n"
          "# parameters.\n\n");
  fprintf(fConfig, "import = 1\n\n");

  // SAR processing flag -- removed for 3.0!
//  fprintf(fConfig, "# The SAR processing flag indicates whether the data needs to be run\n"
//        "# through 'ardop' (1 for running it, 0 for leaving out the SAR processing step).\n");
//  fprintf(fConfig, "# Running asf_convert with the -create option and the SAR processing \n"
//        "# flag switched on will generate a [SAR processing] section where you can define\n"
//        "# further parameters.\n\n");
//  fprintf(fConfig, "sar processing = 0\n\n");

  // c2p flag -- removed for 3.x!
  //fprintf(fConfig, "# For SLC (single-look complex) data, asf_convert can convert the data\n"
  //        "# to polar (amplitude and phase), before further processing.  In fact,\n"
  //        "# if you wish to do any further processing, this step is required.\n\n");
  //fprintf(fConfig, "c2p = 0\n\n");

  // terrain correction flag
  fprintf(fConfig, "# The terrain correction flag indicates whether the data needs to be run\n"
          "# through 'asf_terrcorr' (1 for running it, 0 for leaving out the terrain\n"
          "# correction step).\n");
  fprintf(fConfig, "# Running asf_convert with the -create option and the terrain correction\n"
          "# flag switched on will generate an [Terrain correction] section where you\n"
          "# can define further parameters.\n\n");
  fprintf(fConfig, "terrain correction = 0\n\n");
  // geocoding flag
  fprintf(fConfig, "# The geocoding flag indicates whether the data needs to be run through\n"
          "# 'asf_geocode' (1 for running it, 0 for leaving out the geocoding step).\n");
  fprintf(fConfig, "# Running asf_convert with the -create option and the geocoding flag\n"
          "# switched on will generate a [Geocoding] section where you can define further\n"
          "# parameters.\n\n");
  fprintf(fConfig, "geocoding = 1\n\n");
  // export flag
  fprintf(fConfig, "# The export flag indicates whether the data needs to be run through\n"
          "# 'asf_export' (1 for running it, 0 for leaving out the export step).\n");
  fprintf(fConfig, "# Running asf_convert with the -create option and the export flag\n"
          "# switched on will generate an [Export] section where you can define further\n"
          "# parameters.\n\n");
  fprintf(fConfig, "export = 1\n\n");
  // default values file
  fprintf(fConfig, "# The default values file is used to define the user's preferred parameter\n"
          "# settings. In most cases, you will work on a study where your area of interest is\n"
          "# geographically well defined. You want the data for the entire project in the same\n"
          "# projection, with the same pixel spacing and the same output format.\n");
  fprintf(fConfig, "# A sample of a default values file can be located in\n");
  fprintf(fConfig, "# %s/asf_convert.\n\n", get_asf_share_dir());
  fprintf(fConfig, "default values = \n\n");
  // intermediates flag
  fprintf(fConfig, "# The intermediates flag indicates whether the intermediate processing\n"
          "# results are kept (1 for keeping them, 0 for deleting them at the end of the\n"
          "# processing).\n\n");
  fprintf(fConfig, "intermediates = 0\n\n");
  // quiet flag
  fprintf(fConfig, "# The quiet flag determines how much information is reported by the\n"
          "# individual tools (1 for keeping reporting to a minimum, 0 for maximum reporting\n\n");
  fprintf(fConfig, "quiet = 1\n\n");
  // short configuration file flag
  fprintf(fConfig, "# The short configuration file flag allows the experienced user to generate\n"
          "# configuration files without the verbose comments that explain all entries for\n"
          "# the parameters in the configuration file (1 for a configuration without comments,\n"
          "# 0 for a configuration file with verbose comments)\n\n");
  fprintf(fConfig, "short configuration file = 0\n\n");
  // dump envi header flag
  fprintf(fConfig, "# If you would like to view intermediate imagery, you may wish to turn this\n"
          "# option on -- it dumps an ENVI-compatible .hdr file, that will allow ENVI to view\n"
          "# ASF Internal format .img files.  These files are not used by the ASF Tools.\n\n");
  fprintf(fConfig, "dump envi header = 1\n\n");
  // batch file
  fprintf(fConfig, "# This parameter looks for the location of the batch file\n");
  fprintf(fConfig, "# asf_convert can be used in a batch mode to run a large number of data\n"
          "# sets through the processing flow with the same processing parameters.\n\n");
  fprintf(fConfig, "batch file = \n\n");
  // prefix
  fprintf(fConfig, "# A prefix can be added to the outfile name to avoid overwriting\n"
          "# files (e.g. when running the same data sets through the processing flow\n"
          "# with different map projection parameters\n\n");
  fprintf(fConfig, "prefix = \n\n");
  // suffix
  fprintf(fConfig, "# A suffix can be added to the outfile name to avoid overwriting\n"
          "# files (e.g when running the same data sets through the processing flow\n"
          "# with different map projection parameters\n\n");
  fprintf(fConfig, "suffix = \n\n");

  FCLOSE(fConfig);

  asfPrintStatus("   Initialized basic configuration file\n\n");

  return(0);
}

void free_convert_config(convert_config *cfg)
{
    if (cfg) {
        if (cfg->general) {
            FREE(cfg->general->in_name);
            FREE(cfg->general->out_name);
            FREE(cfg->general->default_in_dir);
            FREE(cfg->general->default_out_dir);
            FREE(cfg->general->batchFile);
            FREE(cfg->general->defaults);
            FREE(cfg->general->prefix);
            FREE(cfg->general->suffix);
            FREE(cfg->general->status_file);
            FREE(cfg->general->tmp_dir);
            FREE(cfg->general);
        }
        if (cfg->import) {
            FREE(cfg->import->format);
            FREE(cfg->import->radiometry);
            FREE(cfg->import->lut);
            FREE(cfg->import->prc);
            FREE(cfg->import);
        }
        if (cfg->airsar) {
            FREE(cfg->airsar);
        }
        if (cfg->sar_processing) {
            FREE(cfg->sar_processing->radiometry);
            FREE(cfg->sar_processing);
        }
        if (cfg->c2p) {
            FREE(cfg->c2p);
        }
        if (cfg->image_stats) {
            FREE(cfg->image_stats->values);
            FREE(cfg->image_stats);
        }
        if (cfg->detect_cr) {
            FREE(cfg->detect_cr->cr_location);
            FREE(cfg->detect_cr);
        }
        if (cfg->terrain_correct) {
            FREE(cfg->terrain_correct->dem);
            FREE(cfg->terrain_correct->mask);
            FREE(cfg->terrain_correct);
        }
        if (cfg->geocoding) {
            FREE(cfg->geocoding->projection);
            FREE(cfg->geocoding->datum);
            FREE(cfg->geocoding->resampling);
            FREE(cfg->geocoding);
        }
        if (cfg->export) {
            FREE(cfg->export->band);
            FREE(cfg->export->format);
            FREE(cfg->export->byte);
            FREE(cfg->export->rgb);
            FREE(cfg->export);
        }
        FREE(cfg);
    }
}

convert_config *init_fill_convert_config(char *configFile)
{
#define newStruct(type) (type *)MALLOC(sizeof(type))
#define LINE_LEN 8196

  FILE *fConfig, *fDefaults;
  char line[LINE_LEN], params[25];
  char *test;
  int i;

  // Create structure
  strcpy(params, "");
  convert_config *cfg = newStruct(convert_config);
  cfg->general = newStruct(s_general);
  cfg->import = newStruct(s_import);
  cfg->airsar = newStruct(s_airsar);
  cfg->sar_processing = newStruct(s_sar_processing);
  cfg->c2p = newStruct(s_c2p);
  cfg->image_stats = newStruct(s_image_stats);
  cfg->detect_cr = newStruct(s_detect_cr);
  cfg->terrain_correct = newStruct(s_terrain_correct);
  cfg->geocoding = newStruct(s_geocoding);
  cfg->export = newStruct(s_export);

  // Initialize structure
  strcpy(cfg->comment, "asf_convert configuration file");

  cfg->general->in_name = (char *)MALLOC(sizeof(char)*1200);
  strcpy(cfg->general->in_name, "");
  cfg->general->out_name = (char *)MALLOC(sizeof(char)*1200);
  strcpy(cfg->general->out_name, "");
  cfg->general->default_in_dir = (char *)MALLOC(sizeof(char)*1024);
  strcpy(cfg->general->default_in_dir, "");
  cfg->general->default_out_dir = (char *)MALLOC(sizeof(char)*1024);
  strcpy(cfg->general->default_out_dir, "");
  cfg->general->import = 0;
  cfg->general->sar_processing = 0;
  cfg->general->c2p = 0;
  cfg->general->image_stats = 0;
  cfg->general->detect_cr = 0;
  cfg->general->terrain_correct = 0;
  cfg->general->geocoding = 0;
  cfg->general->export = 0;
  cfg->general->batchFile = (char *)MALLOC(sizeof(char)*255);
  strcpy(cfg->general->batchFile, "");
  cfg->general->defaults = (char *)MALLOC(sizeof(char)*255);
  strcpy(cfg->general->defaults, "");
  cfg->general->status_file = (char *)MALLOC(sizeof(char)*1024);
  strcpy(cfg->general->status_file, "");
  cfg->general->prefix = (char *)MALLOC(sizeof(char)*255);
  strcpy(cfg->general->prefix, "");
  cfg->general->suffix = (char *)MALLOC(sizeof(char)*255);
  strcpy(cfg->general->suffix, "");
  cfg->general->intermediates = 0;
  cfg->general->quiet = 1;
  cfg->general->short_config = 0;
  cfg->general->dump_envi = 1;
  cfg->general->tmp_dir = (char *)MALLOC(sizeof(char)*255);
  strcpy(cfg->general->tmp_dir, "");
  cfg->general->thumbnail = 0;

  cfg->import->format = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->import->format, "CEOS");
  cfg->import->radiometry = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->import->radiometry, "AMPLITUDE_IMAGE");
  cfg->import->lut = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->import->lut, "");
  cfg->import->lat_begin = -99.0;
  cfg->import->lat_end = -99.0;
  cfg->import->prc = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->import->prc, "");
  cfg->import->output_db = 0;
  cfg->import->complex_slc = 0;
  cfg->import->multilook_slc = 0;

  cfg->airsar->c_vv = 0;
  cfg->airsar->l_vv = 0;
  cfg->airsar->c_pol = 0;
  cfg->airsar->l_pol = 0;
  cfg->airsar->p_pol = 0;

  cfg->sar_processing->radiometry = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->sar_processing->radiometry, "AMPLITUDE_IMAGE");

  cfg->c2p->multilook = FALSE;

  cfg->image_stats->values = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->image_stats->values, "LOOK");
  cfg->image_stats->bins = -99;
  cfg->image_stats->interval = -99.9;

  cfg->detect_cr->cr_location = (char *)MALLOC(sizeof(char)*255);
  strcpy(cfg->detect_cr->cr_location, "");
  cfg->detect_cr->chips = 0;
  cfg->detect_cr->text = 0;

  cfg->terrain_correct->pixel = -99;
  cfg->terrain_correct->dem = (char *)MALLOC(sizeof(char)*255);
  strcpy(cfg->terrain_correct->dem, "");
  cfg->terrain_correct->mask = (char *)MALLOC(sizeof(char)*255);
  cfg->terrain_correct->auto_mask_water = 0;
  cfg->terrain_correct->water_height_cutoff = 1.0;
  cfg->terrain_correct->fill_value = 0;
  cfg->terrain_correct->do_radiometric = 0;
  cfg->terrain_correct->smooth_dem_holes = 0;
  cfg->terrain_correct->save_terrcorr_dem = 0;
  cfg->terrain_correct->save_terrcorr_layover_mask = 0;
  strcpy(cfg->terrain_correct->mask, "");
  cfg->terrain_correct->refine_geolocation_only = 0;
  cfg->terrain_correct->interp = 1;

  cfg->geocoding->projection = (char *)MALLOC(sizeof(char)*255);
  sprintf(cfg->geocoding->projection, "%s/projections/utm/utm.proj",
          get_asf_share_dir());
  cfg->geocoding->pixel = -99;
  cfg->geocoding->height = 0.0;
  cfg->geocoding->datum = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->geocoding->datum, "WGS84");
  cfg->geocoding->resampling = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->geocoding->resampling, "BILINEAR");
  cfg->geocoding->force = 0;
  cfg->geocoding->background = DEFAULT_NO_DATA_VALUE;

  cfg->export->format = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->export->format, "GEOTIFF");
  cfg->export->byte = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->export->byte, "SIGMA");
  cfg->export->rgb = (char *)MALLOC(sizeof(char)*255);
  strcpy(cfg->export->rgb, "");
  cfg->export->band = (char *)MALLOC(sizeof(char)*25);
  strcpy(cfg->export->band, "");
  cfg->export->truecolor = 0;
  cfg->export->falsecolor = 0;
  cfg->export->pauli = 0;
  cfg->export->sinclair = 0;

  // Check for a default values file
  fConfig = fopen(configFile, "r");
  if (fConfig) {
    i=0;
    while (fgets(line, LINE_LEN, fConfig) != NULL) {
        if (i==0) strcpy(cfg->comment, line);
        i++;

        if (strncmp(line, "[General]", 9)==0) strcpy(params, "general");
        if (strcmp(params, "general")==0) {
        test = read_param(line);
        if (strncmp(test, "default values", 14)==0)
            strcpy(cfg->general->defaults, read_str(line, "default values"));
        FREE(test);
        }
    }
    FCLOSE(fConfig);
  }

  // Read default values file if there is one
  if (strcmp(cfg->general->defaults, "") != 0) {
    if (!fileExists(cfg->general->defaults))
      check_return(1, "Default values file does not exist\n");
    fDefaults = FOPEN(cfg->general->defaults, "r");
    while (fgets(line, LINE_LEN, fDefaults) != NULL) {
      test = read_param(line);
      // General
      if (strncmp(test, "default input dir", 17)==0)
        strcpy(cfg->general->default_in_dir, read_str(line, "default input dir"));
      if (strncmp(test, "default output dir", 18)==0)
        strcpy(cfg->general->default_out_dir, read_str(line, "default output dir"));
      if (strncmp(test, "import", 6)==0)
        cfg->general->import = read_int(line, "import");
      if (strncmp(test, "sar processing", 14)==0)
        cfg->general->sar_processing = read_int(line, "sar processing");
      if (strncmp(test, "c2p", 3)==0)
        cfg->general->c2p = read_int(line, "c2p");
      if (strncmp(test, "image stats", 11)==0)
        cfg->general->image_stats = read_int(line, "image stats");
      if (strncmp(test, "detect corner reflectors", 24)==0)
        cfg->general->detect_cr = read_int(line, "detect corner reflectors");
      if (strncmp(test, "terrain correction", 18)==0)
        cfg->general->terrain_correct = read_int(line, "terrain correction");
      if (strncmp(test, "geocoding", 9)==0)
        cfg->general->geocoding = read_int(line, "geocoding");
      if (strncmp(test, "export", 6)==0)
        cfg->general->export = read_int(line, "export");
      if (strncmp(test, "intermediates", 13)==0)
        cfg->general->intermediates = read_int(line, "intermediates");
      if (strncmp(test, "quiet", 5)==0)
        cfg->general->quiet = read_int(line, "quiet");
      if (strncmp(test, "short configuration file", 24)==0)
        cfg->general->short_config = read_int(line, "short configuration file");
      if (strncmp(test, "dump envi header", 16)==0)
        cfg->general->dump_envi = read_int(line, "dump envi header");
      if (strncmp(test, "tmp dir", 7)==0)
        strcpy(cfg->general->tmp_dir, read_str(line, "tmp dir"));
      if (strncmp(test, "status file", 11)==0)
        strcpy(cfg->general->status_file, read_str(line, "status file"));
      if (strncmp(test, "prefix", 6)==0)
        strcpy(cfg->general->prefix, read_str(line, "prefix"));
      if (strncmp(test, "suffix", 6)==0)
        strcpy(cfg->general->suffix, read_str(line, "suffix"));
      if (strncmp(test, "thumbnail", 9)==0)
        cfg->general->thumbnail = read_int(line, "thumbnail");
      // Import
      if (strncmp(test, "input format", 12)==0)
        strcpy(cfg->import->format, read_str(line, "input format"));
      if (strncmp(test, "radiometry", 10)==0)
        strcpy(cfg->import->radiometry, read_str(line, "radiometry"));
      if (strncmp(test, "look up table", 13)==0)
        strcpy(cfg->import->lut, read_str(line, "look up table"));
      if (strncmp(test, "output db", 9)==0)
        cfg->import->output_db = read_int(line, "output db");
      if (strncmp(test, "complex SLC", 11)==0)
        cfg->import->complex_slc = read_int(line, "complex SLC");
      if (strncmp(test, "multilook SLC", 13)==0)
        cfg->import->multilook_slc = read_int(line, "multilook SLC");
      // AirSAR
      if (strncmp(test, "airsar c interferometric", 24)==0)
        cfg->airsar->c_vv = read_int(line, "airsar c interferometric");
      if (strncmp(test, "airsar l interferometric", 24)==0)
        cfg->airsar->l_vv = read_int(line, "airsar l interferometric");
      if (strncmp(test, "airsar c polarimetric", 21)==0)
        cfg->airsar->c_pol = read_int(line, "airsar c polarimetric");
      if (strncmp(test, "airsar l polarimetric", 21)==0)
        cfg->airsar->l_pol = read_int(line, "airsar l polarimetric");
      if (strncmp(test, "airsar p polarimetric", 21)==0)
        cfg->airsar->p_pol = read_int(line, "airsar p polarimetric");
      // SAR processing
      if (strncmp(test, "radiometry", 10)==0)
        strcpy(cfg->sar_processing->radiometry, read_str(line, "radiometry"));
      // C2P
      if (strncmp(test, "multilook", 9)==0)
        cfg->c2p->multilook = read_int(line, "multilook");
      // Image stats
      if (strncmp(test, "stats values", 12)==0)
        strcpy(cfg->image_stats->values, read_str(line, "stats values"));
      // Detect corner reflectors
      if (strncmp(test, "detect corner reflectors", 24)==0)
        cfg->general->detect_cr = read_int(line, "detect corner reflectors");
      if (strncmp(test, "corner reflector locations", 26)==0)
        strcpy(cfg->detect_cr->cr_location,
               read_str(line, "corner reflector locations"));
      // Terrain correction
      if (strncmp(test, "pixel spacing", 13)==0)
        cfg->terrain_correct->pixel = read_double(line, "pixel spacing");
      if (strncmp(test, "digital elevation model", 23)==0)
        strcpy(cfg->terrain_correct->dem,
               read_str(line, "digital elevation model"));
      if (strncmp(test, "mask", 4)==0)
        strcpy(cfg->terrain_correct->mask, read_str(line, "mask"));
      if (strncmp(test, "auto mask water", 15)==0)
        cfg->terrain_correct->auto_mask_water = read_int(line, "auto mask water");
      if (strncmp(test, "water height cutoff", 19)==0)
        cfg->terrain_correct->water_height_cutoff = read_double(line, "water height cutoff");
      if (strncmp(test, "fill value", 8)==0)
        cfg->terrain_correct->fill_value = read_int(line, "fill value");
      if (strncmp(test, "do radiometric", 12)==0)
        cfg->terrain_correct->do_radiometric = read_int(line, "do radiometric");
      if (strncmp(test, "smooth dem holes", 16)==0)
        cfg->terrain_correct->smooth_dem_holes = read_int(line, "smooth dem holes");
      if (strncmp(test, "save terrcorr dem", 17)==0)
        cfg->terrain_correct->save_terrcorr_dem =
            read_int(line, "save terrcorr dem");
      if (strncmp(test, "save terrcorr layover mask", 26)==0)
        cfg->terrain_correct->save_terrcorr_layover_mask =
            read_int(line, "save terrcorr layover mask");
      if (strncmp(test, "refine geolocation only", 23)==0)
        cfg->terrain_correct->refine_geolocation_only =
          read_int(line, "refine_geolocation_only");
      if (strncmp(test, "interpolate", 11)==0)
        cfg->terrain_correct->interp = read_int(line, "interpolate");

      // Geocoding
      if (strncmp(test, "projection", 10)==0)
        strcpy(cfg->geocoding->projection, read_str(line, "projection"));
      if (strncmp(test, "pixel spacing", 13)==0)
        cfg->geocoding->pixel = read_double(line, "pixel spacing");
      if (strncmp(test, "height", 6)==0)
        cfg->geocoding->height = read_double(line, "height");
      if (strncmp(test, "datum", 5)==0)
        strcpy(cfg->geocoding->datum, read_str(line, "datum"));
      if (strncmp(test, "resampling", 10)==0)
        strcpy(cfg->geocoding->resampling, read_str(line, "resampling"));
      if (strncmp(test, "background", 10)==0)
        cfg->geocoding->background = read_int(line, "background");
      if (strncmp(test, "force", 5)==0)
        cfg->geocoding->force = read_int(line, "force");

      // Export
      if (strncmp(test, "output format", 13)==0)
        strcpy(cfg->export->format, read_str(line, "output format"));
      if (strncmp(test, "byte conversion", 15)==0)
        strcpy(cfg->export->byte, read_str(line, "byte conversion"));
      if (strncmp(test, "rgb banding", 11)==0)
        strcpy(cfg->export->rgb, read_str(line, "rgb banding"));
      if (strncmp(test, "truecolor", 9)==0)
        cfg->export->truecolor = read_int(line, "truecolor");
      if (strncmp(test, "falsecolor", 9)==0)
        cfg->export->falsecolor = read_int(line, "falsecolor");
      if (strncmp(test, "band", 4)==0)
        strcpy(cfg->export->band, read_str(line, "band"));
      if (strncmp(test, "pauli", 5)==0)
        cfg->export->pauli = read_int(line, "pauli");
      if (strncmp(test, "sinclair", 8)==0)
        cfg->export->sinclair = read_int(line, "sinclair");
      FREE(test);
    }
  }

  // Read in parameters
  fConfig = fopen(configFile, "r");
  if (fConfig) {
    i=0;
    while (fgets(line, LINE_LEN, fConfig) != NULL) {
        if (i==0) strcpy(cfg->comment, line);
        i++;

        if (strncmp(line, "[General]", 9)==0) strcpy(params, "general");
        if (strcmp(params, "general")==0) {
        test = read_param(line);
        if (strncmp(test, "input file", 10)==0)
            strcpy(cfg->general->in_name, read_str(line, "input file"));
        if (strncmp(test, "output file", 11)==0)
            strcpy(cfg->general->out_name, read_str(line, "output file"));
        if (strncmp(test, "default input dir", 17)==0)
            strcpy(cfg->general->default_in_dir, read_str(line, "default input dir"));
        if (strncmp(test, "default output dir", 18)==0)
            strcpy(cfg->general->default_out_dir, read_str(line, "default output dir"));
        if (strncmp(test, "import", 6)==0)
            cfg->general->import = read_int(line, "import");
        if (strncmp(test, "sar processing", 14)==0)
            cfg->general->sar_processing = read_int(line, "sar processing");
        if (strncmp(test, "c2p", 3)==0)
            cfg->general->c2p = read_int(line, "c2p");
        if (strncmp(test, "image stats", 11)==0)
            cfg->general->image_stats = read_int(line, "image stats");
        if (strncmp(test, "detect corner reflectors", 24)==0)
            cfg->general->detect_cr = read_int(line, "detect corner reflectors");
        if (strncmp(test, "terrain correction", 18)==0)
            cfg->general->terrain_correct = read_int(line, "terrain correction");
        if (strncmp(test, "geocoding", 9)==0)
            cfg->general->geocoding = read_int(line, "geocoding");
        if (strncmp(test, "export", 6)==0)
            cfg->general->export = read_int(line, "export");
        if (strncmp(test, "default values", 14)==0)
            strcpy(cfg->general->defaults, read_str(line, "default values"));
        if (strncmp(test, "intermediates", 13)==0)
            cfg->general->intermediates = read_int(line, "intermediates");
        if (strncmp(test, "quiet", 13)==0)
            cfg->general->quiet = read_int(line, "quiet");
        if (strncmp(test, "short configuration file", 24)==0)
            cfg->general->short_config = read_int(line, "short configuration file");
        if (strncmp(test, "dump envi header", 16)==0)
            cfg->general->dump_envi = read_int(line, "dump envi header");
        if (strncmp(test, "tmp dir", 7)==0)
            strcpy(cfg->general->tmp_dir, read_str(line, "tmp dir"));
        if (strncmp(test, "status file", 11)==0)
            strcpy(cfg->general->status_file, read_str(line, "status file"));
        if (strncmp(test, "batch file", 10)==0)
            strcpy(cfg->general->batchFile, read_str(line, "batch file"));
        if (strncmp(test, "prefix", 6)==0)
            strcpy(cfg->general->prefix, read_str(line, "prefix"));
        if (strncmp(test, "suffix", 6)==0)
            strcpy(cfg->general->suffix, read_str(line, "suffix"));
        if (strncmp(test, "thumbnail", 9)==0)
            cfg->general->thumbnail = read_int(line, "thumbnail");
        FREE(test);
        }
    }
    FCLOSE(fConfig);
  }

  apply_default_dirs(cfg);
    
  return cfg;
}

convert_config *read_convert_config(char *configFile)
{
  FILE *fConfig;
  convert_config *cfg=NULL;
  char line[255], params[50];
  char *test;

  strcpy(params, "");
  cfg = init_fill_convert_config(configFile);
  if (cfg == NULL) check_return(1, "Creating configuration structure.\n");
  fConfig = fopen(configFile, "r");
  if (!fConfig) return NULL;
  while (fgets(line, 255, fConfig) != NULL) {

    if (strncmp(line, "[General]", 9)==0) strcpy(params, "General");
    if (strncmp(params, "General", 7)==0) {
      test = read_param(line);
      if (strncmp(test, "input file", 10)==0)
        strcpy(cfg->general->in_name, read_str(line, "input file"));
      if (strncmp(test, "output file", 11)==0)
        strcpy(cfg->general->out_name, read_str(line, "output file"));
      if (strncmp(test, "default input dir", 17)==0)
          strcpy(cfg->general->default_in_dir, read_str(line, "default input dir"));
      if (strncmp(test, "default output dir", 18)==0)
          strcpy(cfg->general->default_out_dir, read_str(line, "default output dir"));
      if (strncmp(test, "import", 6)==0)
        cfg->general->import = read_int(line, "import");
      if (strncmp(test, "sar processing", 14)==0)
        cfg->general->sar_processing = read_int(line, "sar processing");
      if (strncmp(test, "c2p", 3)==0)
        cfg->general->c2p = read_int(line, "c2p");
      if (strncmp(test, "image stats", 11)==0)
        cfg->general->image_stats = read_int(line, "image stats");
      if (strncmp(test, "detect corner reflectors", 24)==0)
        cfg->general->detect_cr = read_int(line, "detect corner reflectors");
      if (strncmp(test, "terrain correction", 18)==0)
        cfg->general->terrain_correct = read_int(line, "terrain correction");
      if (strncmp(test, "geocoding", 9)==0)
        cfg->general->geocoding = read_int(line, "geocoding");
      if (strncmp(test, "export", 6)==0)
        cfg->general->export = read_int(line, "export");
      if (strncmp(test, "default values", 14)==0)
        strcpy(cfg->general->defaults, read_str(line, "default values"));
      if (strncmp(test, "intermediates", 13)==0)
        cfg->general->intermediates = read_int(line, "intermediates");
      if (strncmp(test, "quiet", 5)==0)
        cfg->general->quiet = read_int(line, "quiet");
      if (strncmp(test, "short configuration file", 24)==0)
        cfg->general->short_config = read_int(line, "short configuration file");
      if (strncmp(test, "dump envi header", 16)==0)
        cfg->general->dump_envi = read_int(line, "dump envi header");
      if (strncmp(test, "tmp dir", 7)==0)
        strcpy(cfg->general->tmp_dir, read_str(line, "tmp dir"));
      if (strncmp(test, "status file", 11)==0)
        strcpy(cfg->general->status_file, read_str(line, "status file"));
      if (strncmp(test, "batch file", 10)==0)
        strcpy(cfg->general->batchFile, read_str(line, "batch file"));
      if (strncmp(test, "prefix", 6)==0)
        strcpy(cfg->general->prefix, read_str(line, "prefix"));
      if (strncmp(test, "suffix", 6)==0)
        strcpy(cfg->general->suffix, read_str(line, "suffix"));
      if (strncmp(test, "thumbnail", 9)==0)
        cfg->general->thumbnail = read_int(line, "thumbnail");
      FREE(test);
    }

    if (strncmp(line, "[Import]", 8)==0) strcpy(params, "Import");
    if (strncmp(params, "Import", 6)==0) {
      test = read_param(line);
      if (strncmp(test, "format", 6)==0)
        strcpy(cfg->import->format, read_str(line, "format"));
      if (strncmp(test, "radiometry", 10)==0)
        strcpy(cfg->import->radiometry, read_str(line, "radiometry"));
      if (strncmp(test, "look up table", 13)==0)
        strcpy(cfg->import->lut, read_str(line, "look up table"));
      if (strncmp(test, "lat begin", 9)==0)
        cfg->import->lat_begin = read_double(line, "lat begin");
      if (strncmp(test, "lat end", 7)==0)
        cfg->import->lat_end = read_double(line, "lat end");
      if (strncmp(test, "precise", 7)==0)
        strcpy(cfg->import->prc, read_str(line, "precise"));
      if (strncmp(test, "output db", 9)==0)
        cfg->import->output_db = read_int(line, "output db");
      if (strncmp(test, "complex SLC", 11)==0)
        cfg->import->complex_slc = read_int(line, "complex SLC");
      if (strncmp(test, "multilook SLC", 13)==0)
        cfg->import->multilook_slc = read_int(line, "multilook SLC");
      FREE(test);
    }

    if (strncmp(line, "[AirSAR]", 8)==0) strcpy(params, "AirSAR");
    if (strncmp(params, "AirSAR", 6)==0) {
      test = read_param(line);
      if (strncmp(test, "airsar c interferometric", 24)==0)
        cfg->airsar->c_vv = read_int(line, "airsar c interferometric");
      if (strncmp(test, "airsar l interferometric", 24)==0)
        cfg->airsar->l_vv = read_int(line, "airsar l interferometric");
      if (strncmp(test, "airsar c polarimetric", 21)==0)
        cfg->airsar->c_pol = read_int(line, "airsar c polarimetric");
      if (strncmp(test, "airsar l polarimetric", 21)==0)
        cfg->airsar->l_pol = read_int(line, "airsar l polarimetric");
      if (strncmp(test, "airsar p polarimetric", 21)==0)
        cfg->airsar->p_pol = read_int(line, "airsar p polarimetric");
      FREE(test);
    }

    if (strncmp(line, "[SAR processing]", 16)==0) strcpy(params, "SAR processing");
    if (strncmp(params, "SAR processing", 14)==0) {
      test = read_param(line);
      if (strncmp(test, "radiometry", 10)==0)
        strcpy(cfg->sar_processing->radiometry, read_str(line, "radiometry"));
      FREE(test);
    }

    if (strncmp(line, "[C2P]", 5)==0) strcpy(params, "C2P");
    if (strncmp(params, "C2P", 3)==0) {
        test = read_param(line);
        if (strncmp(test, "multilook", 9)==0)
            cfg->c2p->multilook = read_int(line, "multilook");
        FREE(test);
    }

    if (strncmp(line, "[Image stats]", 13)==0) strcpy(params, "Image stats");
    if (strncmp(params, "Image stats", 11)==0) {
      test = read_param(line);
      if (strncmp(test, "values", 6)==0)
        strcpy(cfg->image_stats->values, read_str(line, "values"));
      if (strncmp(test, "bins", 4)==0)
        cfg->image_stats->bins = read_int(line, "bins");
      if (strncmp(test, "interval", 8)==0)
        cfg->image_stats->interval = read_double(line, "interval");
      FREE(test);
    }

    if (strncmp(line, "[Detect corner reflectors]", 26)==0)
      strcpy(params, "Detect corner reflectors");
    if (strncmp(params, "Detect corner reflectors", 24)==0) {
      test = read_param(line);
      if (strncmp(test, "corner reflector locations", 26)==0)
        strcpy(cfg->detect_cr->cr_location, read_str(line, "corner reflector locations"));
      if (strncmp(test, "chips", 5)==0)
        cfg->detect_cr->chips = read_int(line, "chips");
      if (strncmp(test, "text", 4)==0)
        cfg->detect_cr->text = read_int(line, "text");
      FREE(test);
    }

    if (strncmp(line, "[Terrain correction]", 20)==0)
      strcpy(params, "Terrain correction");
    if (strncmp(params, "Terrain correction", 18)==0) {
      test = read_param(line);
      if (strncmp(test, "pixel spacing", 13)==0)
        cfg->terrain_correct->pixel = read_double(line, "pixel spacing");
      if (strncmp(test, "digital elevation model", 23)==0)
        strcpy(cfg->terrain_correct->dem,
               read_str(line, "digital elevation model"));
      if (strncmp(test, "mask", 4)==0)
        strcpy(cfg->terrain_correct->mask, read_str(line, "mask"));
      if (strncmp(test, "auto mask water", 15)==0)
        cfg->terrain_correct->auto_mask_water = read_int(line, "auto mask water");
      if (strncmp(test, "water height cutoff", 19)==0)
        cfg->terrain_correct->water_height_cutoff = read_double(line, "water height cutoff");
      if (strncmp(test, "fill value", 8)==0)
        cfg->terrain_correct->fill_value = read_int(line, "fill value");
      if (strncmp(test, "do radiometric", 12)==0)
        cfg->terrain_correct->do_radiometric = read_int(line, "do radiometric");
      if (strncmp(test, "smooth dem holes", 16)==0)
        cfg->terrain_correct->smooth_dem_holes = read_int(line, "smooth dem holes");
      if (strncmp(test, "save terrcorr dem", 17)==0)
        cfg->terrain_correct->save_terrcorr_dem =
            read_int(line, "save terrcorr dem");
      if (strncmp(test, "save terrcorr layover mask", 26)==0)
        cfg->terrain_correct->save_terrcorr_layover_mask =
            read_int(line, "save terrcorr layover mask");
      if (strncmp(test, "refine geolocation only", 23)==0)
        cfg->terrain_correct->refine_geolocation_only =
            read_int(line, "refine_geolocation_only");
      if (strncmp(test, "interpolate", 11)==0)
        cfg->terrain_correct->interp = read_int(line, "interpolate");
      FREE(test);
    }

    if (strncmp(line, "[Geocoding]", 11)==0) strcpy(params, "Geocoding");
    if (strncmp(params, "Geocoding", 9)==0) {
      test = read_param(line);
      if (strncmp(test, "projection", 10)==0)
        strcpy(cfg->geocoding->projection, read_str(line, "projection"));
      if (strncmp(test, "pixel spacing", 13)==0)
        cfg->geocoding->pixel = read_double(line, "pixel spacing");
      if (strncmp(test, "height", 6)==0)
        cfg->geocoding->height = read_double(line, "height");
      if (strncmp(test, "datum", 5)==0)
        strcpy(cfg->geocoding->datum, read_str(line, "datum"));
      if (strncmp(test, "resampling", 10)==0)
        strcpy(cfg->geocoding->resampling, read_str(line, "resampling"));
      if (strncmp(test, "background", 10)==0)
        cfg->geocoding->background = read_int(line, "background");
      if (strncmp(test, "force", 5)==0)
        cfg->geocoding->force = read_int(line, "force");
      FREE(test);
    }

    if (strncmp(line, "[Export]", 8)==0) strcpy(params, "Export");
    if (strncmp(params, "Export", 6)==0) {
      test = read_param(line);
      if (strncmp(test, "format", 6)==0)
        strcpy(cfg->export->format, read_str(line, "format"));
      if (strncmp(test, "byte conversion", 15)==0)
        strcpy(cfg->export->byte, read_str(line, "byte conversion"));
      if (strncmp(test, "rgb banding", 11)==0)
        strcpy(cfg->export->rgb, read_str(line, "rgb banding"));
      if (strncmp(test, "truecolor", 9)==0)
        cfg->export->truecolor = read_int(line, "truecolor");
      if (strncmp(test, "falsecolor", 10)==0)
        cfg->export->falsecolor = read_int(line, "falsecolor");
      if (strncmp(test, "band", 4)==0)
        strcpy(cfg->export->band, read_str(line, "band"));
      if (strncmp(test, "pauli", 5)==0)
        cfg->export->pauli = read_int(line, "pauli");
      if (strncmp(test, "sinclair", 8)==0)
        cfg->export->sinclair = read_int(line, "sinclair");
      FREE(test);
    }
  }

  apply_default_dirs(cfg);
    
  FCLOSE(fConfig);

  return cfg;
}

int write_convert_config(char *configFile, convert_config *cfg)
{
  FILE *fConfig;
  int shortFlag=FALSE;

  if (cfg == NULL)
    check_return(1, "No configuration structure to write.\n");
  if (cfg->general->short_config)
    shortFlag = TRUE;

  fConfig = FOPEN(configFile, "w");

  if (strcmp(cfg->general->batchFile, "") == 0) {
    fprintf(fConfig, "%s\n", cfg->comment);

    // General
    fprintf(fConfig, "[General]\n");
    if (!shortFlag)
      fprintf(fConfig, "\n# This parameter looks for the basename of the input file\n\n");
    fprintf(fConfig, "input file = %s\n", cfg->general->in_name);
    if (!shortFlag)
      fprintf(fConfig, "\n# This parameter looks for the basename of the output file\n\n");
    fprintf(fConfig, "output file = %s\n", cfg->general->out_name);
    if (!shortFlag)
      fprintf(fConfig, "\n# Default directory to find files in. If there is no directory in\n"
                       "# the input file value, this directory will be appended to it.\n\n");
    fprintf(fConfig, "default input dir = %s\n", cfg->general->default_in_dir);
    if (!shortFlag)
      fprintf(fConfig, "\n# Default directory to put files in. If there is no directory in\n"
                       "# the output file value, this directory will be appended to it.\n\n");
    fprintf(fConfig, "default output dir = %s\n", cfg->general->default_out_dir);
    if (!shortFlag) {
      fprintf(fConfig, "\n# The import flag indicates whether the data needs to be run through\n"
              "# 'asf_import' (1 for running it, 0 for leaving out the import step).\n"
              "# For example, setting the import switch to zero assumes that all the data \n"
              "# is already in the ASF internal format.\n");
      fprintf(fConfig, "# Running asf_convert with the -create option and the import flag\n"
              "# switched on will generate an [Import] section where you can define further\n"
              "# parameters.\n\n");
    }
    fprintf(fConfig, "import = %i\n", cfg->general->import);
    // General - SAR processing
    // In 3.0, only write this out if the flag is actually set.
    // For 3.5 or 4.0 or ?, we should be able to remove the outer if block here.
    if (cfg->general->sar_processing) {
        if (!shortFlag) {
            fprintf(fConfig, "\n# The SAR processing flag indicates whether the data needs to be run\n"
                    "# through 'ardop' (1 for running it, 0 for leaving out the SAR processing step).\n");
            fprintf(fConfig, "# Running asf_convert with the -create option and the SAR processing \n"
                    "# flag switched on will generate a [SAR processing] section where you can define\n"
                    "# further parameters.\n\n");
        }
        fprintf(fConfig, "sar processing = %i\n", cfg->general->sar_processing);
    }
    // General - c2p
    // For 3.2, only write this out if the flag is actually set.
    if (cfg->general->c2p) {
        if (!shortFlag) {
            fprintf(fConfig, "\n# For SLC (single-look complex) data, asf_convert can convert the data\n"
                    "# to polar (amplitude and phase), before further processing.  In fact,\n"
                    "# if you wish to do any further processing, this step is required.\n\n");
            fprintf(fConfig, "# Running asf_convert with the -create option and the c2p\n"
                    "# flag switched on will generate a [C2P] section where you can define\n"
                    "# further parameters.\n\n");
        }
        fprintf(fConfig, "c2p = %i\n", cfg->general->c2p);
    }
    // General - Image stats
    if (cfg->general->image_stats) {
      if (!shortFlag)
        fprintf(fConfig, "\n# The image stats flag indicates whether the data needs to be run\n"
                "# through 'image_stats' (1 for running it, 0 for leave out the image stats\n"
                "# step).\n\n");
      fprintf(fConfig, "image stats = %i\n", cfg->general->image_stats);
    }
    // General - Detect corner reflectors
    if (cfg->general->detect_cr) {
      if (!shortFlag)
        fprintf(fConfig, "\n# The corner reflector detection flag indicates whether the data\n"
                "# needs to be run through 'detect_cr' (1 for running it, 0 for leave out\n"
                "# the detect corner reflector step).\n\n");
      fprintf(fConfig, "detect corner reflectors = %i\n", cfg->general->detect_cr);
    }
    // General - Terrain correction
    if (!shortFlag) {
      fprintf(fConfig, "\n# The terrain correction flag indicates whether the data needs be run\n"
              "# through 'asf_terrcorr' (1 for running it, 0 for leaving out the terrain\n"
              "# correction step).\n");
      fprintf(fConfig, "# Running asf_convert with the -create option and the terrain correction\n"
              "# flag switched on will generate an [Terrain correction] section where you\n"
              "# can define further parameters.\n\n");
    }
    fprintf(fConfig, "terrain correction = %i\n", cfg->general->terrain_correct);
    // General - Geocoding
    if (!shortFlag) {
      fprintf(fConfig, "\n# The geocoding flag indicates whether the data needs to be run through\n"
              "# 'asf_geocode' (1 for running it, 0 for leaving out the geocoding step).\n");
      fprintf(fConfig, "# Running asf_convert with the -create option and the geocoding flag\n"
              "# switched on will generate an [Geocoding] section where you can define further\n"
              "# parameters.\n\n");
    }
    fprintf(fConfig, "geocoding = %i\n", cfg->general->geocoding);
    // General - Export
    if (!shortFlag) {
      fprintf(fConfig, "\n# The export flag indicates whether the data needs to be run through\n"
              "# 'asf_export' (1 for running it, 0 for leaving out the export step).\n");
      fprintf(fConfig, "# Running asf_convert with the -create option and the export flag\n"
              "# switched on will generate an [Export] section where you can define further\n"
              "# parameters.\n\n");
    }
    fprintf(fConfig, "export = %i\n", cfg->general->export);
    // General - Default values
    if (!shortFlag) {
      fprintf(fConfig, "\n# The default values file is used to define the user's preferred parameter\n"
              "# settings. In most cases, you will work on a study where your area of interest is\n"
              "# geographically well defined. You want the data for the entire project in the same\n"
              "# projection, with the same pixel spacing and the same output format.\n\n");
      fprintf(fConfig, "# A sample of a default values file can be located in\n");
      fprintf(fConfig, "# %s/asf_convert.\n\n", get_asf_share_dir());
    }
    fprintf(fConfig, "default values = %s\n", cfg->general->defaults);
    // General - Intermediates
    if (!shortFlag)
      fprintf(fConfig, "\n# The intermediates flag indicates whether the intermediate processing\n"
              "# results are kept (1 for keeping them, 0 for deleting them at the end of the\n"
              "# processing).\n\n");
    fprintf(fConfig, "intermediates = %i\n", cfg->general->intermediates);
    if (!shortFlag)
      fprintf(fConfig, "\n# The short configuration file flag allows the experienced user to\n"
              "# generate configuration files without the verbose comments that explain all\n"
              "# entries for the parameters in the configuration file (1 for a configuration\n"
              "# without comments, 0 for a configuration file with verbose comments)\n\n");
    fprintf(fConfig, "short configuration file = %i\n", cfg->general->short_config);
    if (!shortFlag)
      fprintf(fConfig, "\n# If you would like to view intermediate imagery, you may wish to turn this\n"
              "# option on -- it dumps an ENVI-compatible .hdr file, that will allow ENVI to view\n"
              "# ASF Internal format .img files.  These files are not used by the ASF Tools.\n\n");
    fprintf(fConfig, "dump envi header = %i\n", cfg->general->dump_envi);
    if (!shortFlag)
      fprintf(fConfig, "\n# The tmp dir is where temporary files used during processing will\n"
              "# be kept until processing is completed. Then the entire directory and its\n"
              "# contents will be deleted.\n\n");
    fprintf(fConfig, "tmp dir = %s\n", cfg->general->tmp_dir);

    // Import
    if (cfg->general->import) {
      fprintf(fConfig, "\n\n[Import]\n");
      if (!shortFlag)
        fprintf(fConfig, "\n# The recognized import formats are: ASF, CEOS, AirSAR, and\n"
                "# STF.  Defining ASF, being the internal format, as the import format is\n"
                "# just another way of actually skipping the import step.  For AirSAR,\n"
                "# you can configure which products are processed with the AirSAR block.\n\n");
      fprintf(fConfig, "format = %s\n", cfg->import->format);
      if (!shortFlag)
        fprintf(fConfig, "\n# The radiometry can be one of the following: AMPLITUDE_IMAGE,\n"
                "# POWER_IMAGE, SIGMA_IMAGE, GAMMA_IMAGE and BETA_IMAGE.\n"
                "# The amplitude image is the regularly processed SAR image. The power image\n"
                "# represents the magnitude (square of the amplitude) of the SAR image.\n"
                "# The sigma, gamma and beta image are different representations of calibrated\n"
                "# SAR images. Their values are in power scale.\n\n");
      fprintf(fConfig, "radiometry = %s\n", cfg->import->radiometry);
      if (!shortFlag)
        fprintf(fConfig, "\n# The look up table option is primarily used by the Canadian Ice\n"
                "# Service (CIS) and scales the amplitude values in range direction. The file\n"
                "# parsed in to the import tool is expected to have two columns, the first one\n"
                "# indicating the look angle with the corresponding scale factor as the second\n"
                "# column.\n\n");
      fprintf(fConfig, "look up table = %s\n", cfg->import->lut);
      if (!shortFlag)
        fprintf(fConfig, "\n# The latitude constraints (lat begin and lat end) can only be used\n"
                "# when importing level zero swath data (STF). This is the most convenient way\n"
                "# to cut a subset out of a long image swath.\n\n");
      fprintf(fConfig, "lat begin = %.2f\n", cfg->import->lat_begin);
      fprintf(fConfig, "lat end = %.2f\n", cfg->import->lat_end);
      if (!shortFlag)
        fprintf(fConfig, "\n# The precise option, currently under development, will allow the use\n"
                "# of ERS precision state vector from DLR as a replacement of the restituted\n"
                "# state vectors that are provided from the European Space Agency. The parameter\n"
                "# required here defines the location of the precision state vectors.\n\n");
      fprintf(fConfig, "precise = %s\n", cfg->import->prc);
      if (!shortFlag)
          fprintf(fConfig, "\n# When the output db flag is non-zero, the calibrated image\n"
                "# is output in decibels.  It only applies when the radiometry is sigma,\n"
                "# gamma or beta.\n\n");
      fprintf(fConfig, "output db = %d\n", cfg->import->output_db);
    }
    if (!shortFlag)
      fprintf(fConfig, "\n# When the complex SLC flag in non-zero, single "
	      "look complex data is stored in I/Q values. Otherwise SLC data\n"
	      "# will be stored as amplitude/phase.\n\n");
    fprintf(fConfig, "complex SLC = %d\n", cfg->import->complex_slc);
    if (!shortFlag)
      fprintf(fConfig, "\n# When the multilook SLC flag in non-zero, single "
	      "look complex data that is stored as amplitude/phase is being\n"
	      "# multilooked.\n\n");
    fprintf(fConfig, "multilook SLC = %d\n\n", cfg->import->multilook_slc);

    // AirSAR -- only write out if the import format is AirSAR
    if (cfg->general->import && strncmp_case(cfg->import->format, "airsar", 6)==0) {
      fprintf(fConfig, "\n\n[AirSAR]\n");
      if (!shortFlag)
        fprintf(fConfig, "\n# Flag indicating if the AirSAR C-band cross-track interferometric data\n"
                "# (containing a VV image, a DEM, and a coherence image) should be processed.\n"
                "# Not all AirSAR data sets will contain this type of data.\n\n");
      fprintf(fConfig, "airsar c interferometric = %d\n", cfg->airsar->c_vv);
      if (!shortFlag)
        fprintf(fConfig, "\n# Flag indicating if the AirSAR L-band cross-track interferometric data\n"
                "# (containing a VV image, a DEM, and a coherence image) should be processed.\n"
                "# Not all AirSAR data sets will contain this type of data.\n\n");
      fprintf(fConfig, "airsar l interferometric = %d\n", cfg->airsar->l_vv);
      if (!shortFlag)
        fprintf(fConfig, "\n# Flag indicating if the AirSAR C-band polarimetric data (containing\n"
                "# 9 bands: power, and amplitude & phase for each of VV, HV, VH, and VV)\n"
                "# should be processed.  Not all AirSAR data sets will contain this type\n"
                "# of data.\n\n");
      fprintf(fConfig, "airsar c polarimetric = %d\n", cfg->airsar->c_pol);
      if (!shortFlag)
        fprintf(fConfig, "\n# Flag indicating if the AirSAR L-band polarimetric data (containing\n"
                "# 9 bands: power, and amplitude & phase for each of VV, HV, VH, and VV)\n"
                "# should be processed.  Not all AirSAR data sets will contain this type\n"
                "# of data.\n\n");
      fprintf(fConfig, "airsar l polarimetric = %d\n", cfg->airsar->l_pol);
      if (!shortFlag)
        fprintf(fConfig, "\n# Flag indicating if the AirSAR P-band polarimetric data (containing\n"
                "# 9 bands: power, and amplitude & phase for each of VV, HV, VH, and VV)\n"
                "# should be processed.  Not all AirSAR data sets will contain this type\n"
                "# of data.\n\n");
      fprintf(fConfig, "airsar p polarimetric = %d\n", cfg->airsar->p_pol);
    }

    // SAR processing
    if (cfg->general->sar_processing) {
      fprintf(fConfig, "\n[SAR processing]\n");
      if (!shortFlag)
        fprintf(fConfig, "\n# The radiometry can be one of the following: AMPLITUDE_IMAGE,\n"
                "# POWER_IMAGE, SIGMA_IMAGE, GAMMA_IMAGE and BETA_IMAGE.\n"
                "# The amplitude image is the regularly processed SAR image. The power image\n"
                "# represents the magnitude (square of the amplitude) of the SAR image.\n"
                "# The sigma, gamma and beta image are different representations of calibrated\n"
                "# SAR images. Their values are in power scale.\n\n");
        fprintf(fConfig, "radiometry = %s\n\n", cfg->sar_processing->radiometry);
    }
    if (cfg->general->c2p) {
        fprintf(fConfig, "\n[C2P]\n");
        if (!shortFlag)
            fprintf(fConfig, "\n# SLC data is not multilooked, but will be if this flag\n"
                    "is set.\n\n");
        fprintf(fConfig, "multilook = %d\n\n", cfg->c2p->multilook);
    }
    // Image stats
    if (cfg->general->image_stats) {
      fprintf(fConfig, "\n[Image stats]\n\n");
      fprintf(fConfig, "values = %s\n", cfg->image_stats->values);
      fprintf(fConfig, "bins = %i\n", cfg->image_stats->bins);
      fprintf(fConfig, "interval = %.2lf\n", cfg->image_stats->interval);
    }
    // Detect corner reflectors
    if (cfg->general->detect_cr) {
      fprintf(fConfig, "\n[Detect corner reflectors]\n\n");
      fprintf(fConfig, "corner reflector locations = %s\n", cfg->detect_cr->cr_location);
      fprintf(fConfig, "chips = %i\n", cfg->detect_cr->chips);
      fprintf(fConfig, "text = %i\n", cfg->detect_cr->text);
    }
    // Terrain correction
    if (cfg->general->terrain_correct) {
      fprintf(fConfig, "\n[Terrain correction]\n");
      if (!shortFlag)
        fprintf(fConfig, "\n# This parameter defines the output size of the terrain corrected\n"
                "# image. If set to -99 this parameter will be ignored and the 'asf_terrcorr' will\n"
                "# deal with the issues that might occur when using different pixel spacings in\n"
                "# the SAR image and the reference DEM\n\n");
      fprintf(fConfig, "pixel spacing = %.2lf\n", cfg->terrain_correct->pixel);
      if (!shortFlag)
        fprintf(fConfig, "\n# The heights of the reference DEM are used to correct the SAR image\n"
                "# for terrain effects. The quality and resolution of the reference DEM determines\n"
                "# the quality of the resulting terrain corrected product\n\n");
      fprintf(fConfig, "digital elevation model = %s\n", cfg->terrain_correct->dem);
      if (!shortFlag)
        fprintf(fConfig, "\n# In some case parts of the images are known to be moving (e.g. water,\n"
                "# glaciers etc.). This can cause severe problems in matching the SAR image with\n"
                "# the simulated SAR image derived from the reference. Providing a mask defines the\n"
                "# areas that are stable and can be used for the matching process.\n\n");
      fprintf(fConfig, "mask = %s\n", cfg->terrain_correct->mask);
      if (!shortFlag)
        fprintf(fConfig, "\n# Instead of creating a mask, you can have terrain correction\n"
                "# automatically generate a mask for you, based on the DEM, which attempts to mask\n"
                "# the regions of your scene that are water (these regions provide a poor match).\n"
                "# Specifically, all DEM values <1m are masked, unless a different height cutoff\n"
                "# is specified with the 'water height cutoff' option, described next.\n\n");
      fprintf(fConfig, "auto mask water = %d\n", cfg->terrain_correct->auto_mask_water);
      if (!shortFlag)
        fprintf(fConfig, "\n# When creating a mask automatically with the previous flag,\n"
                "# you may specify use a value other than 1m as the height cutoff.\n"
                "# This value is ignored when 'auto mask water' is 0.\n\n");
      fprintf(fConfig, "water height cutoff = %f\n", cfg->terrain_correct->water_height_cutoff);
      if (!shortFlag)
        fprintf(fConfig, "\n# When applying a mask during terrain correction, you can choose\n"
                "# how the regions covered by the mask are filled in the final terrain corrected\n"
                "# result.  You can either specify a (non-negative) value of your choosing,or\n"
                "# if you'd like the SAR data to be kept then use %d as the fill value.\n\n", LEAVE_MASK);
      fprintf(fConfig, "fill value = %d\n", cfg->terrain_correct->fill_value);
      if (!shortFlag)
          fprintf(fConfig, "\n# Normally during terrain correction, only geometric terrain is\n"
                  "# applied.  This option will also turn on radiometric terrain correction.\n"
                  "# This option is still experimental.  Currently, terrain corrected pixel\n"
                  "# values are scaled using the formula: 1 - .7*pow(cos(li),7), where li is\n"
                  "# is the local incidence angle.\n\n");
      fprintf(fConfig, "do radiometric = %d\n", cfg->terrain_correct->do_radiometric);
      if (!shortFlag)
          fprintf(fConfig, "\n# If your DEM has a number of \"holes\" in it, this can cause streaking\n"
                  "# in the terrain corrected product.  This option will attempt to replace DEM holes\n"
                  "# with interpolated values.\n\n");
      fprintf(fConfig, "smooth dem holes = %d\n", cfg->terrain_correct->smooth_dem_holes);
      if (!shortFlag)
        fprintf(fConfig, "\n# Even if you don't want to change the image via terrain correction,\n"
                "# you may still wish to use the DEM to refine the geolocation of the SAR image.\n"
                "# If this flag is set, terrain correction is NOT performed.\n\n");
      fprintf(fConfig, "refine geolocation only = %d\n", cfg->terrain_correct->refine_geolocation_only);
      if (!shortFlag)
        fprintf(fConfig, "\n# Layover/shadow regions can either be left black (resulting in better\n"
                "# image statistics in the remainder of the image), or they may be interpolated over\n"
                "# (resulting in a nicer-looking image).  Setting this parameter to 1 indicates that\n"
                "# these regions should be interpolated over.\n\n");
      fprintf(fConfig, "interpolate = %d\n", cfg->terrain_correct->interp);
      if (!shortFlag)
        fprintf(fConfig, "\n# The DEM that is provided is clipped to match the scene.  Normally this\n"
                "# clipped DEM is removed along with the other temporary files, however if you are \n"
                "# interested you can turn this option on (set it to 1), which will keep the clipped \n"
                "# DEM, as well as geocode (if you've elected to geocode) and export it (if you've \n"
                "# elected to export, though the DEM is always exported as floating point data even \n"
                "# when you exporting your SAR data as bytes).  The clipped DEM will be slightly\n"
                "# larger than the SAR image, usually, since a larger region must be clipped to\n"
                "# allow for the height variations.\n\n");
      fprintf(fConfig, "save terrcorr dem = %d\n", cfg->terrain_correct->save_terrcorr_dem);
      if (!shortFlag)
        fprintf(fConfig, "\n# This option determines if a file marking the regions of layover and \n"
                "# shadow should be created along with the output image.  It is geocoded using the\n"
                "# the same parameters as your SAR image, and exported as byte data.\n\n");
      fprintf(fConfig, "save terrcorr layover mask = %d\n\n", cfg->terrain_correct->save_terrcorr_layover_mask);
    }

    // Geocoding
    if (cfg->general->geocoding) {
      fprintf(fConfig, "\n[Geocoding]\n");
      if (!shortFlag) {
        fprintf(fConfig, "\n# The geocoding tool currently supports five different map projections:\n"
                "# Universal Transverse Mercator (UTM), Polar Stereographic, Albers Equal Area\n"
                "# Conic, Lambert Conformal Conic and Lambert Azimuthal Equal Area.\n"
                "# For all these map projections a large number of projection parameter files\n"
                "# have been predefined for various parts of the world.\n");
        fprintf(fConfig, "# The projection parameter files are located in:\n");
	fprintf(fConfig, "#    %s/projections\n\n", get_asf_share_dir());
      }
      fprintf(fConfig, "projection = %s\n", cfg->geocoding->projection);
      if (!shortFlag)
        fprintf(fConfig, "\n# The pixel spacing determines the pixel size used for the resulting\n"
                "# geocoded image and, therefore, the size of the output image.\n\n");
      fprintf(fConfig, "pixel spacing = %.2f\n", cfg->geocoding->pixel);
      if  (!shortFlag)
        fprintf(fConfig, "\n# An average height can be defined for the image that is taken into\n"
                "#  account and adjusted for during the geocoding process.\n\n");
      fprintf(fConfig, "height = %.1f\n", cfg->geocoding->height);
      if (!shortFlag)
        fprintf(fConfig, "\n# A vertical datum can be defined for geocoded image. WGS84 is the\n"
                "# only currently supported datum. However, NAD27 and NAD83 are planned to be\n"
                "# appropriate alternatives.\n\n");
      fprintf(fConfig, "datum = %s\n", cfg->geocoding->datum);
      if (!shortFlag)
        fprintf(fConfig, "\n# Three different resampling methods have been implemented as part\n"
                "# of the geocoding: NEAREST NEIGHBOR, BILINEAR and BICUBIC. The bilinear\n"
                "# resampling method is the default.\n\n");
      fprintf(fConfig, "resampling = %s\n", cfg->geocoding->resampling);
      if (!shortFlag)
        fprintf(fConfig, "\n# After geocoding, a fill value is required for the regions outside\n"
                "# of the geocoded image.  By default this value is 0, but may be set to a\n"
                "# different value here.\n\n");
      fprintf(fConfig, "background = %.2f\n", cfg->geocoding->background);
      if (!shortFlag)
        fprintf(fConfig, "\n# In order to ensure the proper use of projection parameter files,\n"
                "# we have implemented a number of checks that verify whether the map\n"
                "# projection parameters are reasonable for the area that is covered by the\n"
                "# data. For example, applying a projection parameter file that is defined for\n"
                "# South America for a data set that is covering Alaska would lead to huge\n"
                "# distortions. These checks can be overwritten by setting the force option.\n\n");
      fprintf(fConfig, "force = %i\n\n", cfg->geocoding->force);
    }
    // Export
    if (cfg->general->export) {
      fprintf(fConfig, "\n[Export]\n");
      if (!shortFlag)
        fprintf(fConfig, "\n# The following format are considered valid format: ASF, TIFF, GEOTIFF\n"
                "# JPEG, PNG and PGM.\n"
                "# In the same way as for the import block, ASF as an export option results in\n"
                "# skipping the export step entirely. All other formats, with the exception of\n"
                "# GeoTIFF, require the scaling of the internal ASF format from floating point\n"
                "# to byte. The GeoTIFF supports byte as well as floating point data.\n\n");
      fprintf(fConfig, "format = %s\n", cfg->export->format);
      if (!shortFlag)
        fprintf(fConfig, "\n# The byte conversion options are SIGMA, MINMAX, TRUNCATE or\n"
                "# HISTOGRAM_EQUALIZE. They scale the floating point values to byte values.\n\n");
      fprintf(fConfig, "byte conversion = %s\n", cfg->export->byte);
      if (!shortFlag)
        fprintf(fConfig, "\n# If you have more than one band available in your data, you can\n"
                "# create the exported file using the different bands for the R, G, and B\n"
                "# channels in the output image.  List the R, G, and B channels in that order\n"
                "# separated by commas.  E.g.  HH,HV,VV.  Note that no contrast expansion or\n"
                "# other modification of the data will be applied.\n\n");
      fprintf(fConfig, "rgb banding = %s\n", cfg->export->rgb);
      if (!shortFlag)
        fprintf(fConfig, "\n# If you have 3+ bands available in your optical data,\n"
                "# you can create the exported file using a standard selection of bands for\n"
                "# the R, G, and B channels in the output image.  By setting the truecolor\n"
                "# flag, band assignments will be R = 3, G = 2, and B = 1 and each band\n"
                "# will individually contrast-expanded using a 2-sigma remapping for improved\n"
                "# visualization (use rgb banding if you desire raw data assignments.)\n\n");
      fprintf(fConfig, "truecolor = %i\n", cfg->export->truecolor);
      if (!shortFlag)
        fprintf(fConfig, "\n# If you have 4 bands available in your optical data,\n"
                "# you can create the exported file using a standard selection of bands for\n"
                "# the R, G, and B channels in the output image.  By setting the falsecolor\n"
                "# flag, band assignments will be R = 4, G = 3, and B = 2 and each band\n"
                "# will individually contrast-expanded using a 2-sigma remapping for improved\n"
                "# visualization (use rgb banding if you desire raw data assignments.)\n\n");
      fprintf(fConfig, "falsecolor = %i\n", cfg->export->falsecolor);
      if (!shortFlag)
        fprintf(fConfig, "\n# If you wish to export a single band from the list of\n"
            "# available bands, e.g. HH, HV, VH, VV ...enter VV to export just\n"
                "# the VV band (alone.)\n\n");
      fprintf(fConfig, "band = %s\n", cfg->export->band);
      if (!shortFlag)
        fprintf(fConfig, "\n# If you have quad-pole data available (HH, VV, VH and VH),\n"
            "# you can use the standard Pauli decomposition to map the 4 bands to\n"
                "# the R, G, and B channels in the output image.  In this decomposition,\n"
                "# red is HH-VV, green is HV, and blue is HH+VV.  In addition, each channel\n"
                "# will individually contrast-expanded using a 2-sigma remapping for improved\n"
                "# visualization.\n\n");
      fprintf(fConfig, "pauli = %i\n", cfg->export->pauli);
      if (!shortFlag)
        fprintf(fConfig, "\n# If you have quad-pole data available (HH, VV, VH and VH),\n"
                "# you can use the standard Sinclair decomposition to map the 4 bands to\n"
                "# the R, G, and B channels in the output image.  In this decomposition,\n"
                "# red is VV, green is (HV+VH)/2, and blue is HH.  In addition, each channel\n"
                "# will individually contrast-expanded using a 2-sigma remapping for improved\n"
                "# visualization.\n\n");
      fprintf(fConfig, "sinclair = %i\n\n", cfg->export->sinclair);
    }
  }
  else {
    fprintf(fConfig, "%s\n", cfg->comment);

    fprintf(fConfig, "[General]\n\n");
    if (!shortFlag)
      fprintf(fConfig, "# The default values file is used to define the user's preferred\n"
              "# parameter settings.\n\n");
    fprintf(fConfig, "default values = %s\n\n", cfg->general->defaults);
    if (!shortFlag)
      fprintf(fConfig, "# asf_convert has a batch mode to run a large number of data sets\n"
              "# through the processing flow with the same processing parameters\n\n");
    fprintf(fConfig, "batch file = %s\n\n", cfg->general->batchFile);
    if (!shortFlag)
      fprintf(fConfig, "# A prefix can be added to the outfile name to avoid overwriting\n"
              "# files (e.g. when running the same data sets through the processing flow\n"
              "# with different map projection parameters\n\n");
    fprintf(fConfig, "prefix = %s\n\n", cfg->general->prefix);
    if (!shortFlag)
      fprintf(fConfig, "# A suffix can be added to the outfile name to avoid overwriting\n"
              "# files (e.g when running the same data sets through the processing flow\n"
              "# with different map projection parameters\n\n");
    fprintf(fConfig, "suffix = %s\n\n", cfg->general->suffix);
  }

  FCLOSE(fConfig);

  return(0);
}
