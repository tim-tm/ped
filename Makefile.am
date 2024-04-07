AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = ped
ped_SOURCES = \
			src/buffer.h \
			src/buffer.c \
			src/defs.h \
			src/main.c
ped_CPPFLAGS = @NCURSES_CFLAGS@
ped_LDFLAGS = @NCURSES_LIBS@ -lm
