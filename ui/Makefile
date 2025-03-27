.PHONY: all install clean

UI_C	:= blueprint-compiler
SRC		:= $(wildcard *.blp)
TRGT    := $(patsubst %.blp, %.ui, $(SRC))

all: install

%.ui : %.blp
	$(UI_C) compile $^ > $@

install: $(TRGT)
	@for target in $^; do mv $$target ../$$target; done

clean:
	rm -f $(TRGT)