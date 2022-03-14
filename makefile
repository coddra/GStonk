CC:=gcc
CFLAGS:=-ggdb -g3 -w

OBJS:=$(patsubst %.c,%.o,$(wildcard *.c))
DFILES=$(patsubst %.o,%.d,$(OBJS))

all: gstonk clean

gstonk: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	find -name "*~" -delete
	rm -rf $(OBJS) $(DFILES)

examples:
	make -C examples all

$(OBJS):
-include $(DFILES)
