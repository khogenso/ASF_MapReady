# Makefile for 		:  coregister_coarse
# Module Author 	:  Mike Shindle, Tom Logan, Orion Lawlor

include ../../make_support/system_rules

LIBS  = \
	$(LIBDIR)/libasf_insar.a \
	$(LIBDIR)/asf_meta.a \
	$(GSL_LIBS) \
	$(LIBDIR)/libasf_proj.a \
	$(LIBDIR)/asf.a \
	$(PROJ_LIBS) \
	$(XML_LIBS) \
	-lm

OBJS  = coregister_coarse.o

all: coregister_coarse clean

coregister_coarse: $(OBJS)
	$(CC) $(CFLAGS) -o coregister_coarse $(OBJS) $(LIBS) $(LDFLAGS)
	mv coregister_coarse$(BIN_POSTFIX) $(BINDIR)
	cp coregister_coarse.1 ../../man/man1

clean:
	rm -f core *.o
