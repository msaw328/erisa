.PHONY: all clear
.DEFAULT_GOAL := all

all: build/liberisa/liberisa.so build/erisa-exec/erisa-exec

build/erisa-exec/erisa-exec: build/liberisa/liberisa.so

build:
	mkdir -p build/liberisa build/erisa-exec/

clear:
	$(MAKE) -C liberisa clear
	$(MAKE) -C erisa-exec clear
	rm build -rf

build/liberisa/liberisa.so: build
	$(MAKE) -C liberisa

build/erisa-exec/erisa-exec: build
	$(MAKE) -C erisa-exec
