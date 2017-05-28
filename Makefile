NAME=liv

PKGS = gtk+-3.0 lua

GCR = /usr/bin/glib-compile-resources
RES = resources.xml

INCS    := $(shell pkg-config --cflags $(PKGS)) -I./
CFLAGS  := -std=gnu11 -ggdb -W -Wall -Wextra -Wno-unused-parameter $(INCS) $(CFLAGS)
LIBS    += $(shell pkg-config --libs $(PKGS))
LDFLAGS := -Wl,--export-dynamic $(LIBS) $(LDFLAGS)

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

