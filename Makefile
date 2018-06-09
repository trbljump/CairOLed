OBJECTS = doll1.o hooman.o
CONFIG_LIBS=cairo box2d
ifeq ($(shell hostname),vmpi)
	DEFINES += -DUSE_X11
	CONFIG_LIBS += x11
	OBJECTS += cairo_x.o
else
	OBJECTS += cairo_fb.o
endif

LIBS = $(shell pkg-config --libs $(CONFIG_LIBS))
CFLAGS = -O4 -Wall $(DEFINES) $(shell pkg-config --cflags $(CONFIG_LIBS))

.cc.o: $<
	g++ ${CFLAGS} -o $@ -c $<

default: doll1

doll1: ${OBJECTS}
	g++ -o doll1 ${LIBS} ${CFLAGS} ${OBJECTS}

doll1.o: doll1.cc hooman.h cairo_x.h cairo_fb.h

cairo_x.o: cairo_x.cc cairo_x.h

cairo_fb.o: cairo_fb.cc cairo_fb.h

hooman.o: hooman.cc hooman.h
