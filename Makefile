# disable implicit rules
.SUFFIXES:

HDRDIR := include
SRCDIR := src
BLDDIR := build
OBJDIR := $(BLDDIR)/obj

CC := clang
CFLAGS := -Wextra -Wall -pedantic -std=c11
LFLAGS := 

ifeq ($(DEV), 1)
	CFLAGS += -O0 -g3 -fstack-protector-all -Wshadow -Wunreachable-code \
            -Wstack-protector -W -Werror -pedantic-errors -Wundef \
            -Wfatal-errors -Wstrict-prototypes -Wmissing-prototypes \
            -Wwrite-strings -Wunknown-pragmas -Wstrict-aliasing \
            -Wold-style-definition -Wmissing-field-initializers \
            -Wfloat-equal -Wpointer-arith -Wnested-externs \
            -Wstrict-overflow=5 -Wswitch-default -Wswitch-enum \
            -Wbad-function-cast -Wredundant-decls -Winline \
            -fno-omit-frame-pointer -fstrict-aliasing \
            -Wincompatible-pointer-types
else
	CC := gcc
	CFLAGS += -O3
endif

SRCS := $(shell find $(SRCDIR) -name *.c | grep -v "$(SRCDIR)/bin")
OBJS := $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: blddirs $(BLDDIR)/sandmac

$(BLDDIR)/sandmac: $(SRCDIR)/bin/sandmac.c $(OBJS)
	@echo "building"
	@$(CC) $(CFLAGS) $(LFLAGS) $^ -I$(HDRDIR) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HDRDIR)/%.h
	@echo "compiling $*"
	@$(CC) -c $(CFLAGS) $< -I$(HDRDIR) -o $@

SRCDIRS := $(shell find $(SRCDIR) -type d | grep -v "$(SRCDIR)/bin")
BLDDIRS := $(SRCDIRS:$(SRCDIR)%=$(OBJDIR)%)
blddirs:
	mkdir -p $(BLDDIRS)

.PHONY: clean
clean:
	rm -rf $(BLDDIR)
