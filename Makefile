CC=gcc
INSTALL=install
RM=rm -f

all: libdriverwmon.so

clean:
	$(RM) driverwmon.o
	$(RM) libdriverwmon.so

install:
	$(INSTALL) -m755 libdriverwmon.so /usr/lib/xfce4/panel/plugins
	$(INSTALL) -m644 driverwmon.desktop /usr/share/xfce4/panel/plugins

uninstall:
	$(RM) /usr/lib/xfce4/panel/plugins/libdriverwmon.so
	$(RM) /usr/share/xfce4/panel/plugins/driverwmon.desktop

libdriverwmon.so: driverwmon.o
	$(CC) -shared -o libdriverwmon.so driverwmon.o

driverwmon.o: driverwmon.c
	$(CC) `pkg-config --cflags libxfce4panel-2.0` `pkg-config --libs libxfce4panel-2.0` -c -fPIC -o driverwmon.o driverwmon.c

