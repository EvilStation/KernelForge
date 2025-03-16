.PHONY: all install clean

BUILD_DIR := ../build/
INC_DIR	  := ../include/

SRC 	  := $(wildcard *.c)
OBJ		  := $(patsubst %.c, $(BUILD_DIR)%.o, $(SRC))
TRGT	  := kernel_compiler

CFLAGS	  := `pkg-config --cflags glib-2.0`
LDFLAGS   := `pkg-config --libs glib-2.0`

all: install

$(TRGT): $(OBJ)
	$(LINK.o) $^ -o $@

$(BUILD_DIR)%.o: %.c
	$(COMPILE.c) $^ -o $@

install: $(TRGT)
	mv $^ ../$^

clean:
	rm -rf $(TRGT)
