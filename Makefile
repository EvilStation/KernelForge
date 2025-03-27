.PHONY: all clean

all:
	@if [ ! -d build ] ; then mkdir build ; fi
	make -C bpf_trace
	make -C daemons
	make -C ui
	make -C compile_kernel
	make -C gtk_app
	make -C banners

clean:
	make clean -C bpf_trace
	make clean -C daemons
	make clean -C ui
	make clean -C compile_kernel
	make clean -C gtk_app
	make clean -C banners
	rm -rf build/*