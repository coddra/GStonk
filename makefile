CC:=gcc
CFLAGS:=-ggdb -g3 -w

OBJS:=$(patsubst %.c,%.o,$(wildcard *.c))
DFILES:=$(patsubst %.o,%.d,$(OBJS))

XMPLS:=$(patsubst examples/%.gst,%,$(wildcard examples/*.gst))

all: gstonk clean

gstonk: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(OBJS) $(DFILES)
	make -C examples clean

examples: $(XMPLS)

$(OBJS):
-include $(DFILES)

$(XMPLS):
	make -C examples $@
