O = o

CPPFLAGS = $(OPTS) -DCURS_INC=$(CURS_INC) -DMM_MAJOR=$(MM_MAJOR) \
-DMM_MINOR=$(MM_MINOR)

OBJS = mmcolor.$(O) mysystem.$(O) isoconv.$(O) basic.$(O) interfac.$(O) \
packet.$(O) arealist.$(O) letterl.$(O) letterw.$(O) lettpost.$(O) \
ansiview.$(O) addrbook.$(O) tagline.$(O) help.$(O) main.$(O)

include depend

dep:
	$(CXX) -DCURS_INC=$(CURS_INC) -MM *.cc | \
	sed s/"\.o"/"\.\$$(O)"/ > depend

clean:
	$(RM) *.$(O)

intrfc: $(OBJS)
