CC=gcc
CFLAGS=-Wall -pedantic -std=c99 -g3 -O0 -D_XOPEN_SOURCE
DEPFLAGS=-MMD -MF .deps/$(@:target/%.o=%.d) -MT $@

_objs=main comandos arquivos args shell
OBJECTS=$(_objs:%=target/%.o)
DEPENDS=$(_objs:%=.deps/%.d)

$(shell mkdir -p .deps target)

all: ufms

ufms: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

target/%.o: %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm ufms target/*.o .deps/*.d

-include $(DEPENDS)
