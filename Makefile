CLANG ?= clang
LLC ?= llc-11
CC ?= gcc

XDP_TARGETS  := xdp_prog_mirror

XDP_C = ${XDP_TARGETS:=.c}
XDP_OBJ = ${XDP_C:.c=.o}

BPF_CFLAGS ?= -I/usr/include/x86_64-linux-gnu/

all: $(XDP_OBJ)
.PHONY: clean $(CLANG) $(LLC)

clean:
	rm -f $(XDP_OBJ)
	rm -f *.ll
	rm -f *~

$(XDP_OBJ): %.o: %.c
	$(CLANG) -S \
            -target bpf \
            -D __BPF_TRACING__ \
            $(BPF_CFLAGS) \
            -Wall \
            -Wno-unused-value \
            -Wno-pointer-sign \
            -Wno-compare-distinct-pointer-types \
            -Werror \
            -O2 -emit-llvm -c -g -o ${@:.o=.ll} $<
	$(LLC) -march=bpf -filetype=obj -o $@ ${@:.o=.ll}
