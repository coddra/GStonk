CC:=../gstonk
CFLAGS:=
NAMES:=$(patsubst %.gst,%,$(wildcard *.gst))
define nl


endef

all: $(NAMES)

clean:
	rm -rf $(NAMES)

$(NAMES):
	$(CC) $(CFLAGS) $@.gst
