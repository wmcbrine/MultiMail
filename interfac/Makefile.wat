O = obj

CPPFLAGS = -I$(CURS_DIR) -DMM_MAJOR=$(MM_MAJOR) -DMM_MINOR=$(MM_MINOR)

.cc.obj:	.autodepend
	$(COMPILER) $(CPPFLAGS) $<

OBJS = mmcolor.$(O) mysystem.$(O) isoconv.$(O) basic.$(O) interfac.$(O) &
packet.$(O) arealist.$(O) letterl.$(O) letterw.$(O) lettpost.$(O) &
ansiview.$(O) addrbook.$(O) tagline.$(O) help.$(O) main.$(O)

intrfc:	$(OBJS)
