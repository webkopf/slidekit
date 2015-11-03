
all: 
	mkdir -p bin	
	gcc -Wall -O src/slidekit.c src/utils.c -o bin/slidekit `pkg-config --cflags gtk+-3.0 --libs webkitgtk-3.0` -g

	
clean:
	rm bin/slidekit