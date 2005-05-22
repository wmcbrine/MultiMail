O = obj

!include ../modules

MODDEFS = $(USE_BW) $(USE_QWK) $(USE_OMEN) $(USE_SOUP) $(USE_OPX)
CPPFLAGS = -DMM_MAJOR=$(MM_MAJOR) -DMM_MINOR=$(MM_MINOR) $(MODDEFS)

.cc.obj:	.autodepend
	$(COMPILER) $(CPPFLAGS) $<

OBJS = misc.$(O) resource.$(O) mmail.$(O) driverl.$(O) filelist.$(O) &
area.$(O) letter.$(O) read.$(O) compress.$(O) pktbase.$(O)

mm-main:	$(OBJS) $(MODULES)

modclean
	del driverl.$(O) $(MODULES)
