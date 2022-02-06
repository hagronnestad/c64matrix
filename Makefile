# Based on https://github.com/KimJorgensen/KungFuFlash/blob/master/launcher/Makefile

# do not remove intermediate targets
.SECONDARY:

name := c64matrix

ld_config := ld.crt.cfg

obj :=
obj += build/c64matrix.o

inc      := .

INCLUDE  := $(addprefix -I,$(inc))

.PHONY: all
all: build/$(name)

# Poor men's dependencies: Let all files depend from all header files
headers := $(wildcard *.h)

build/%.s: %.c $(headers) | build
	cc65 -Oisr -t c64 -T -O --static-locals $(INCLUDE) $(DEFINE) -o $@ $<

build/%.o: build/%.s | build
	ca65 -t c64 $(INCLUDE) -o $@ $<

build/%.o: %.s | build
	ca65 -t c64 $(INCLUDE) -o $@ $<

build:
	mkdir -p $@

build/$(name): build/crt0.crt.o $(obj) $(ld_config)
	ld65 -o $@.bin -m $@.map -C $(ld_config) build/crt0.crt.o $(obj) \
		-L /usr/local/lib/cc65/lib --lib c64.lib
	cl65 -Oisr -o $@.prg c64matrix.c
	cat $@.map | grep -e "^Name\|^CRTSIG\|^CODE\|^DATA\|^BSS\|^RODATA\|^ONCE\|^INIT\|^EAPI"

.PHONY: clean
clean:
	rm -rf build
