# Makefile for deskew_dem
# by Orion Lawlor 1997; Patrick Denny 2004

include ../../make_support/system_rules

LIBS  = \
	$(LIBDIR)/libasf_sar.a \
	$(LIBDIR)/libasf_raster.a \
	$(LIBDIR)/asf_meta.a \
	$(LIBDIR)/libasf_proj.a \
	$(LIBDIR)/asf.a \
	$(GLIB_LIBS) \
	$(GSL_LIBS) \
	$(PROJ_LIBS) \
	$(XML_LIBS) \
	-lm

CFLAGS += \
	$(GLIB_CFLAGS) \
	$(GSL_CFLAGS) \
	$(PROJ_CFLAGS)

OBJS  = deskew_dem.o

all: deskew_dem clean

deskew_dem: $(OBJS)
	$(CC) $(CFLAGS) -o deskew_dem $(OBJS) $(LIBS) $(LDFLAGS)
	mv deskew_dem$(BIN_POSTFIX) $(BINDIR)
	cp deskew_dem.1 ../../man/cat1

clean:
	rm -f core *.o *~
