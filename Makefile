all: gtk-nodeco.c
	gcc -g `pkg-config --cflags --libs gtk+-3.0` gtk-nodeco.c -o gtk-nodeco
