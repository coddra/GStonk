CC:=../gstonk
CFLAGS:=
GSTFILES:=$(wildcard *.gst)
define nl


endef
COMPCMDS:=$(patsubst %.gst,$(CC) $(CFLAGS) %.gst$(nl),$(GSTFILES))

all:
	$(COMPCMDS)
