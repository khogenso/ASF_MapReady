include ../../make_support/system_rules

TARGET = rtc

CFLAGS += -Wall $(W_ERROR) $(GLIBS_CFLAGS) -g

LIBS  = \
	$(LIBDIR)/libasf_terrcorr.a \
	$(LIBDIR)/libasf_export.a \
	$(LIBDIR)/libasf_import.a \
	$(LIBDIR)/libasf_raster.a \
	$(LIBDIR)/asf_meta.a \
	$(LIBDIR)/asf_fft.a \
	$(LIBDIR)/libasf_proj.a \
	$(LIBDIR)/asf.a \
	$(XML_LIBS) \
	$(PROJ_LIBS) \
	$(GSL_LIBS) \
	$(GLIB_LIBS) \
	$(JPEG_LIBS) \
	$(PNG_LIBS) \
	$(GEOTIFF_LIBS) \
	$(TIFF_LIBS) \
	$(ZLIB_LIBS) \
	-lm

CFLAGS += $(GSL_CFLAGS) $(PROJ_CFLAGS) $(GLIB_CFLAGS)

OBJS  = $(TARGET).o

all: prog make_gr_dem uavsar_rtc
	-rm *.o

prog: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS) $(LDFLAGS)
	mv $(TARGET)$(BIN_POSTFIX) $(BINDIR)

make_gr_dem: make_gr_dem.o
	$(CC) $(CFLAGS) -o make_gr_dem make_gr_dem.o $(LIBS) $(LDFLAGS)
	mv make_gr_dem$(BIN_POSTFIX) $(BINDIR)

uavsar_rtc: uavsar_rtc.o
	$(CC) $(CFLAGS) -o uavsar_rtc uavsar_rtc.o $(LIBS) $(LDFLAGS)
	mv uavsar_rtc$(BIN_POSTFIX) $(BINDIR)
	
clean:
	rm -f core $(OBJS) *.o
