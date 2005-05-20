O = obj

CPPFLAGS = -DCURS_INC=$(CURS_INC) -DMM_MAJOR=$(MM_MAJOR) -DMM_MINOR=$(MM_MINOR)

.cc.obj:	.autodepend
	wpp386 -zw -D__WIN32__ -DWIN32 $(CPPFLAGS) $<

OBJS = mmcolor.$(O) mysystem.$(O) isoconv.$(O) basic.$(O) interfac.$(O) &
packet.$(O) arealist.$(O) letterl.$(O) letterw.$(O) lettpost.$(O) &
ansiview.$(O) addrbook.$(O) tagline.$(O) help.$(O) main.$(O)

intrfc:	$(OBJS)
