# Makefile for raster_diff
# by Orion Lawlor, ASF, 1997
# by Patrick Denny, 2/2004

include ../../make_support/system_rules

LIBS  = \
	$(LIBDIR)/asf_meta.a \
	$(GSL_LIBS) \
	$(LIBDIR)/libasf_proj.a \
	$(LIBDIR)/asf.a \
	$(PROJ_LIBS) \
	$(XML_LIBS) \
	-lm

OBJS  = raster_diff.o

all: raster_diff clean

raster_diff: $(OBJS)
	$(CC) $(CFLAGS) -o raster_diff $(OBJS) $(LIBS) $(LDFLAGS)
	mv raster_diff$(BIN_POSTFIX) $(BINDIR)
	cp raster_diff.1 ../../man/cat1

clean:
	rm -f core $(OBJS)
