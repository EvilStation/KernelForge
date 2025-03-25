.PHONY: all install clean

INC_DIR	   	:= ../include/
BUILD_DIR	:= ../build/

SRC			:= $(wildcard *.c)
TRGT     	:= $(patsubst %.c, %, $(SRC))

CFLAGS		:= -I$(INC_DIR) -I/usr/include/bpf/uapi
LDFLAGS		:= -lbpf -lelf -lz

all: install

%: %.c
	$(LINK.c) $^ -o $@

install: $(TRGT)
	@for target in $^; do mv $$target ../$$target; done

clean:
	rm -rf $(TRGT)