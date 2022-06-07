CC = gcc

BINDIR = bin
INCLUDE = include
LIBDIR = lib
SRCDIR = src

CFLAGS = -Wall -I$(INCLUDE) 
LDFLAGS = -L$(LIBDIR)
LDLIBS = -lfft

SRC = $(SRCDIR)/discorb.c
OBJ = $(SRCDIR)/fft.o
LIB = $(LIBDIR)/libfft.a

TARGET = $(BINDIR)/discorb.out

$(TARGET): $(SRC) $(LIB)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) -lm

$(LIB): $(OBJ)
	$(AR) rsv $@ $^

.PHONY: tmpclean clean

tmpclean:
	rm -f $(SRCDIR)/*~
clean: tmpclean
	rm -f $(TARGET) $(LIB) $(OBJ)