.PHONY: all install clean

UI_C	:= blueprint-compiler
SRC		:= kf.blp
TRGT 	:= kf.ui

all: install

%.ui : %.blp
	$(UI_C) compile $^ > $@

install: $(TRGT)
	mv $^ ../$^

clean:
	rm -f $(TRGT)