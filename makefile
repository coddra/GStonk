CC:=gcc
CFLAGS:=-O3

OBJS:=$(patsubst %.c,%.o,$(wildcard *.c))
DFILES:=$(patsubst %.o,%.d,$(OBJS))

XMPLS:=$(patsubst examples/%.gst,%,$(wildcard examples/*.gst))

all: gstonk clean

gstonk: $(OBJS)
	make -C MCX mcx
	$(CC) $(CFLAGS) -o $@ $(OBJS) ./MCX/mcx.o

clean:
	rm -rf $(OBJS) $(DFILES)
	make -C examples clean
	make -C MCX clean

examples:
	make -C examples all

$(OBJS):
-include $(DFILES)

$(XMPLS):
	make -C examples $@
