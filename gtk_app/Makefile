.PHONY: all install clean

BUILD_DIR := ../build/
INC_DIR	  := ../include/

SRC 	  := $(wildcard *.c)
OBJ		  := $(patsubst %.c, $(BUILD_DIR)%.o, $(SRC))
TRGT	  := KernelForge

CFLAGS	  := `pkg-config --cflags gtk4`
CFLAGS	  += `pkg-config --cflags libadwaita-1`
CFLAGS	  += -Wl, -export-dynamic
CFLAGS	  += -I$(INC_DIR) -I/usr/include/bpf/uapi

LDFLAGS   := `pkg-config --libs gtk4` -lbpf
LDFLAGS   += `pkg-config --libs libadwaita-1`

all: install

$(TRGT): $(OBJ)
	$(LINK.o) $^ -o $@

$(BUILD_DIR)%.o: %.c
	$(COMPILE.c) $^ -o $@

install: $(TRGT)
	mv $^ ../$^

clean:
	rm -rf $(TRGT)