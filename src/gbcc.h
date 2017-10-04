#ifndef GBCC_H
#define GBCC_H

#include <stdint.h>
#include <stdbool.h>
#include "gbcc_constants.h"

struct gbc {
	/* Registers */
	struct {
		union {
			struct {
				#ifdef LITTLE_ENDIAN
				union {
					struct {
						uint8_t zf : 1;
						uint8_t nf : 1;
						uint8_t hf : 1;
						uint8_t cf : 1;
						uint8_t    : 4;
					};
					uint8_t f;
				};
				uint8_t a;
				#else
				uint8_t a;
				union {
					struct {
						uint8_t zf : 1;
						uint8_t nf : 1;
						uint8_t hf : 1;
						uint8_t cf : 1;
						uint8_t    : 4;
					};
					uint8_t f;
				};
				#endif
			};
			uint16_t af;
		};
		union {
			struct {
				#ifdef LITTLE_ENDIAN
				uint8_t c;
				uint8_t b;
				#else
				uint8_t b;
				uint8_t c;
				#endif
			};
			uint16_t bc;
		};
		union {
			struct {
				#ifdef LITTLE_ENDIAN
				uint8_t e;
				uint8_t d;
				#else
				uint8_t d;
				uint8_t e;
				#endif
			};
			uint16_t de;
		};
		union {
			struct {
				#ifdef LITTLE_ENDIAN
				uint8_t l;
				uint8_t h;
				#else
				uint8_t h;
				uint8_t l;
				#endif
			};
			uint16_t hl;
		};
		uint16_t sp;
		uint16_t pc;
	} reg;

	/* Non-Register state data */
	uint8_t memory[0x10000];
	enum CART_MODE mode;
	uint8_t opcode;

	/* Cartridge data & flags */
	struct {
		uint8_t *rom;
		size_t rom_size;
		uint8_t *ram;
		size_t ram_size;
		enum MBC mbc;
		bool battery;
		bool timer;
		bool rumble;
	} cart;
};

void gbcc_initialise(struct gbc *gbc, const char *filename);
void gbcc_execute_instruction(struct gbc *gbc);

#endif /* GBCC_H */