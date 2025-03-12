.PHONY: all install clean

UI_C	:= blueprint-compiler
SRC		:= kf.ui
TRGT 	:= kf.xml

all: install

%.xml : %.ui
	$(UI_C) compile $^ > $@

install: $(TRGT)
	mv $^ ../$^

clean:
	rm -f $(TRGT)