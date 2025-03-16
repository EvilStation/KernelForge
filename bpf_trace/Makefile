.PHONY: all kernel_header install clean

INC_DIR	   	:= ../include/
BUILD_DIR	:= ../build/

CC		   	:= clang
BPF_SRC		:= $(wildcard *.bpf.c)
BPF_OBJ		:= $(patsubst %.bpf.c, $(BUILD_DIR)%.bpf.o, $(BPF_SRC))
LDR_SRC		:= bpf_loader.c
BPF_TRGT   	:= bpf_loader

BPF_SKEL_H 	:= $(INC_DIR)bpf_trace.skel.h
KRNL_H		:= $(INC_DIR)vmlinux.h

CFLAGS		:= -I$(INC_DIR) -I/usr/include/bpf
CFLAGS		+= -g -Wall -O2

LDFLAGS		:= /usr/lib64/libbpf.so -lelf -lz

all: install

$(BUILD_DIR)%.bpf.o: %.bpf.c
	$(COMPILE.c) $^ -o $@ -target bpf

$(INC_DIR)%.skel.h: $(BUILD_DIR)%.bpf.o
	bpftool gen skeleton $^ > $@

$(BPF_TRGT): $(KRNL_H) $(LDR_SRC) $(BPF_SKEL_H)
	$(LINK.c) $(LDR_SRC) -o $@

$(KRNL_H):
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > $@

install: $(BPF_TRGT)
	mv $^ ../$^

clean:
	rm -f $(BPF_TRGT) $(BPF_SKEL_H) $(KRNL_H)