#ifndef GBC_CONSTANTS_H
#define GBC_CONSTANTS_H

/* Memory map */
#define ROM0_START 0x0000u
#define ROM0_SIZE 0x4000u
#define ROMX_START 0x4000u
#define ROMX_SIZE 0x4000u
#define VRAM_START 0x8000u
#define VRAM_SIZE 0x2000u
#define SRAM_START 0xA000u
#define SRAM_SIZE 0x2000u
#define WRAM0_START 0xC000u
#define WRAM0_SIZE 0x1000u
#define WRAMX_START 0xD000u
#define WRAMX_SIZE 0x1000u
#define ECHO_START 0xE000u
#define ECHO_SIZE 0x1E00u
#define OAM_START 0xFE00u
#define OAM_SIZE 0x00A0u
#define UNUSED_START 0xFEA0u
#define UNUSED_SIZE 0x0060u
#define IOREG_START 0xFF00u
#define IOREG_SIZE 0x0080u
#define HRAM_START 0xFF80u
#define HRAM_SIZE 0x007Fu
#define IEREG 0xFFFFu

/* Cartridge header map */
#define CART_HEADER_START 0x0100u
#define CART_HEADER_SIZE 0x004Fu
#define CART_LOGO_START 0x0104u
#define CART_LOGO_SIZE 0x0030u
#define CART_GBC_FLAG 0x0143u
#define CART_TYPE 0x0147u
#define CART_ROM_SIZE 0x0148u
#define CART_RAM_SIZE 0x0149u
#define CART_HEADER_CHECKSUM 0x014Du

enum MBC { NONE, MBC1, MBC2, MBC3, MBC4, MBC5, MMM01 };
enum CART_MODE { GBC, DMG };

#endif /* GBC_CONSTANTS_H */
