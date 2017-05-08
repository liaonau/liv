NAME=liv

PKGS = gtk+-3.0 lua

INCS    := $(shell pkg-config --cflags $(PKGS)) -I./
CFLAGS  := -std=gnu11 -ggdb -W -Wall -Wextra $(INCS) $(CFLAGS) -Wno-unused-parameter
LIBS    += $(shell pkg-config --libs $(PKGS))
LDFLAGS := $(LIBS) $(LDFLAGS) -Wl,--export-dynamic

SRCS = $(wildcard *.c)
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

all: $(NAME)

