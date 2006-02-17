O = o

CPPFLAGS = $(OPTS) -I$(CURS_DIR) -DMM_MAJOR=$(MM_MAJOR) -DMM_MINOR=$(MM_MINOR)

OBJS = mmcolor.$(O) mysystem.$(O) isoconv.$(O) basic.$(O) interfac.$(O) \
packet.$(O) arealist.$(O) letterl.$(O) letterw.$(O) lettpost.$(O) \
ansiview.$(O) addrbook.$(O) tagline.$(O) help.$(O) main.$(O)

include depend

dep:
	$(CXX) -I$(CURS_DIR) -MM *.cc | sed s/"\.o"/"\.\$$(O)"/ > depend

clean:
	$(RM) *.$(O)

intrfc: $(OBJS)
