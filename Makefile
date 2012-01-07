all: gtk-nodeco.c
	gcc -g `pkg-config --cflags --libs gtk+-3.0` gtk-nodeco.c -o grfkill
	strip grfkill

# [~] % for i (*svg) gdk-pixbuf-csource $i --struct --name `echo $i | cut -d '.' -f 1`_inline >| `echo $i | cut -d '.' -f 1`.h
