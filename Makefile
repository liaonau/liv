NAME=liv

PKGS = gtk+-3.0 lua

GCR = /usr/bin/glib-compile-resources
RES = resources.xml

INCS    := $(shell pkg-config --cflags $(PKGS)) -I./
CFLAGS  := -std=c11 -ggdb -W -Wall -Wextra -Wno-unused-parameter -O3 $(INCS) $(CFLAGS)
LIBS    += $(shell pkg-config --libs $(PKGS))
LDFLAGS := -Wl,--export-dynamic $(LIBS) $(LDFLAGS)

INSTALLDIR := /usr
DOCDIR     := /usr/share/doc/liv

SRCS  = $(wildcard *.c)
HEADS = $(wildcard *.h)
OBJS  = $(foreach obj,$(SRCS:.c=.o),$(obj))

.PHONY: all

$(NAME): $(OBJS)
	@echo $(CC) -o $(NAME) $(OBJS)
	@$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)

$(OBJS): $(HEADS)

.c.o:
	@echo $(CC) -c $< -o $@
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -f $(NAME) $(OBJS)

resorces:
	$(GCR) --target=resources.h --sourcedir=./resources --generate-source $(RES)

all: $(NAME)

install:
	install -d $(INSTALLDIR)/bin
	install liv $(INSTALLDIR)/bin/liv
	install -d /etc/xdg/liv/
	install config/rc.lua /etc/xdg/liv/
	install -d $(DOCDIR)
	install -m644 README.md COPYING doc $(DOCDIR)

