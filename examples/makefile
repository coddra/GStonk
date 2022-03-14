CC:=../gstonk
CFLAGS:=
NAMES:=$(patsubst %.gst,%,$(wildcard *.gst))

all: clean $(NAMES)

clean:
	rm -rf $(NAMES)

$(NAMES):
	$(CC) $(CFLAGS) $@.gst
