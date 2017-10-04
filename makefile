ODIR=obj
SRCDIR=src
CC=gcc
CFLAGS=-Wall -Wextra -pedantic -g -Wno-unused-parameter
LIBS=-lSDL2 -lm

_DEPS = gbcc.h gbcc_constants.h gbcc_cpu.h gbcc_ops.h
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = main.o gbcc_init.o gbcc_cpu.o gbcc_ops.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gbcc: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir -p $@


.PHONY: clean

clean:
	rm -rf $(ODIR) $(SRCDIR)/*~ gbcc