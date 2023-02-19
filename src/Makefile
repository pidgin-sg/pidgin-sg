INCLUDE_DIRS = /usr/include/libpurple \
			   /usr/lib/x86_64-linux-gnu/glib-2.0/include \
			   /usr/include/glib-2.0

CFLAGS += -O2 -fpic ${INCLUDE_DIRS:%=-I%}
CPPFLAGS += -DPURPLE_PLUGINS
LDFLAGS += -shared
CC = gcc

OBJECTS = callbacks.o crc32.o misc.o pending-list.o privacy.o string.o \
		  config.o frame.o plugin.o qa.o


spamguard.so: ${OBJECTS}
	${LD} ${LDFLAGS} -o $@ ${OBJECTS} ${LDLIBS}

install:
	install spamguard.so ~/.purple/plugins

clean:
	rm -f ${OBJECTS}
	rm -f spamguard.so
