all: src/slidekit.c
	mkdir -p bin
	gcc -O src/slidekit.c -o bin/slidekit `pkg-config --cflags gtk+-3.0 --libs webkitgtk-3.0` -g
	
clean:
	rm bin/slidekit