# Makefile for          : concatm
# Module Author         : Tom Logan & Mike Shindle
# Module Version        : 1.0

include ../../make_support/system_rules

LIBS  = $(LIBDIR)/asf_concat.a \
	$(LIBDIR)/asf_meta.a \
	$(GSL_LIBS) \
	$(LIBDIR)/libasf_proj.a \
	$(PROJ_LIBS) \
	$(LIBDIR)/asf.a \
	$(XML_LIBS) \
	-lm

OBJS  = concatm.o \
	averopt.o \
	overlay_img.o \
	pad.o \
	set_dtype.o

concatm: $(OBJS)
	$(CC) $(CFLAGS) -o concatm $(OBJS) $(LIBS) $(LDFLAGS)
	rm *.o
	mv concatm$(BIN_POSTFIX) $(BINDIR)/concatm$(BIN_POSTFIX)
	cp concatm.man ../../man/cat1/concatm.1
