# Makefile for libasf_terrcorr library

include ../../make_support/system_rules

CFLAGS := -Wall $(W_ERROR) $(CFLAGS)

OBJS =  seedsquares.o asf_terrcorr.o build_dem.o rtc.o make_gr_dem.o uavsar_rtc.o

LIBS  = \
	$(LIBDIR)/libasf_raster.a \
	$(LIBDIR)/libasf_sar.a \
	$(LIBDIR)/asf.a \
	$(LIBDIR)/asf_fft.a \
	$(LIBDIR)/asf_meta.a \
	$(LIBDIR)/libasf_proj.a \
	$(GSL_LIBS) \
	$(PROJ_LIBS) \
	-lm

CFLAGS += $(GSL_CFLAGS) $(PROJ_CFLAGS) $(GLIB_CFLAGS) -D_XOPEN_SOURCE=600

all: build_only
	mv libasf_terrcorr.a $(LIBDIR)
	cp asf_terrcorr.h $(ASF_INCLUDE_DIR)

build_only: $(OBJS)
	rm -f libasf_terrcorr.a
	ar r libasf_terrcorr.a $(OBJS)
	$(RANLIB) libasf_terrcorr.a

$(OBJS): Makefile $(wildcard *.h) $(wildcard ../../include/*.h)

clean:
	rm -rf $(OBJS) libasf_terrcorr.a *~
