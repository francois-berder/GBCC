ODIR=obj
SRCDIR=src
CC=clang
CFLAGS=-Weverything -std=gnu11 -Wno-unused-parameter -Wno-padded -Wno-format-nonliteral -O2
LIBS=-lSDL2 -lm

_DEPS = gbcc.h gbcc_apu.h gbcc_audio.h gbcc_constants.h gbcc_cpu.h gbcc_ops.h gbcc_memory.h \
	gbcc_mbc.h gbcc_window.h gbcc_input.h gbcc_bit_utils.h gbcc_debug.h \
	gbcc_ppu.h gbcc_save.h gbcc_colour.h gbcc_hdma.h
DEPS = makefile $(patsubst %,$(SRCDIR)/%,$(_DEPS))

_OBJ = main.o gbcc_apu.o gbcc_audio.o gbcc_init.o gbcc_cpu.o gbcc_ops.o gbcc_memory.o \
       gbcc_mbc.o gbcc_window.o gbcc_input.o gbcc_bit_utils.o gbcc_debug.o \
       gbcc_ppu.o gbcc_save.o gbcc_colour.o gbcc_hdma.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gbcc: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir -p $@


.PHONY: clean

clean:
	rm -rf $(ODIR) $(SRCDIR)/*~ gbcc
