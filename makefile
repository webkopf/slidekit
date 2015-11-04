
all: 
	mkdir -p bin	
	gcc -O0 src/slidekit.c src/utils.c -o bin/slidekit `pkg-config --cflags gtk+-3.0 --libs webkitgtk-3.0` -g
	
clean:
	rm bin/slidekit
	
install: all
	cp bin/slidekit /usr/bin
	mkdir -p /usr/local/share/slidekit
	cp web/slidekit.* /usr/local/share/slidekit
	
	
