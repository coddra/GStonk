CC:=gcc
CFLAGS:=-O3

OBJS:=$(patsubst %.c,%.o,$(wildcard *.c))
OBJS+= $(patsubst %.c,%.o,$(wildcard mcx/c/*.c))
OBJS+= $(patsubst %.c,%.o,$(wildcard ccom/c/*.c))
DFILES:=$(patsubst %.o,%.d,$(OBJS))

XMPLS:=$(patsubst examples/%.gst,%,$(wildcard examples/*.gst))

all: gstonk clean

gstonk: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(OBJS) $(DFILES)
	make -C examples clean

examples:
	make -C examples all

$(OBJS):
-include $(DFILES)

$(XMPLS):
	make -C examples $@
