O = o

include ../modules

MODDEFS = $(USE_BW) $(USE_QWK) $(USE_OMEN) $(USE_SOUP) $(USE_OPX)
CPPFLAGS = $(OPTS) -DMM_MAJOR=$(MM_MAJOR) -DMM_MINOR=$(MM_MINOR) $(MODDEFS)

OBJS = misc.$(O) resource.$(O) mmail.$(O) driverl.$(O) filelist.$(O) \
area.$(O) letter.$(O) read.$(O) compress.$(O) pktbase.$(O)

include depend

dep:
	$(CXX) $(MODDEFS) -MM *.cc | sed s/"\.o"/"\.\$$(O)"/ > depend

modclean:
	$(RM) driverl.$(O) $(MODULES)

clean:
	$(RM) *.$(O)

mm-main: $(OBJS) $(MODULES)
