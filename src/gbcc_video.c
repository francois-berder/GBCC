#include "gbcc.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_video.h"
#include <stdio.h>

#define BACKGROUND_MAP_BANK_1 0x9800u
#define BACKGROUND_MAP_BANK_2 0x9C00u
#define NUM_SPRITES 40

enum palette_flag { bk, sp1, sp2 };

static void gbcc_draw_line(struct gbc *gbc);
static void gbcc_draw_background_line(struct gbc *gbc);
static void gbcc_draw_window_line(struct gbc *gbc);
static void gbcc_draw_sprite_line(struct gbc *gbc);
static uint8_t gbcc_get_video_mode(struct gbc *gbc);
static void gbcc_set_video_mode(struct gbc *gbc, uint8_t mode);
static uint32_t get_palette_colour(uint8_t palette, uint8_t n, enum palette_flag pf);


void gbcc_video_update(struct gbc *gbc)
{
	uint8_t mode = gbcc_get_video_mode(gbc);
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t stat = gbcc_memory_read(gbc, STAT, true);
	uint16_t clock = gbc->clock % 456;

	if (clock == 0) {
		gbcc_memory_increment(gbc, LY, true);
	}
	/* Line-drawing timings */
	if (mode != GBC_LCD_MODE_VBLANK) {
		if (clock == 0) {
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_READ);
		} else if (clock == GBC_LCD_MODE_PERIOD) {
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_VRAM_READ);
		} else if (clock == (3 * GBC_LCD_MODE_PERIOD)) {
			gbcc_draw_line(gbc);
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_HBLANK);
		}
	}
	/* LCD STAT Interrupt */
	if (ly != 0 && ly == gbcc_memory_read(gbc, LYC, true)) {
		if (clock == 4) {
			gbcc_memory_set_bit(gbc, STAT, 2, true);
		} else if (clock == 8) {
			gbcc_memory_clear_bit(gbc, STAT, 2, true);
		}
	}
	if ((check_bit(stat, 2) && check_bit(stat, 6))
			|| (mode == GBC_LCD_MODE_HBLANK && check_bit(stat, 3))
			|| (mode == GBC_LCD_MODE_OAM_READ && check_bit(stat, 5))
			|| (mode == GBC_LCD_MODE_VBLANK && check_bit(stat, 4))) {
		gbcc_memory_set_bit(gbc, IF, 1, true);
	}
	/* VBLANK interrupt flag */
	if (ly == 144) {
		if (clock == 4) {
			gbcc_memory_set_bit(gbc, IF, 0, true);
			gbcc_set_video_mode(gbc, GBC_LCD_MODE_VBLANK);
			uint32_t *tmp = gbc->memory.gbc_screen;
			gbc->memory.gbc_screen = gbc->memory.sdl_screen;
			gbc->memory.sdl_screen = tmp;
		} else if (clock == 8) {
			gbcc_memory_clear_bit(gbc, IF, 0, true);
		}
	} else if (ly == 154 ) {
		gbcc_memory_write(gbc, LY, 0, true);
		gbcc_set_video_mode(gbc, GBC_LCD_MODE_OAM_READ);
	}
}

void gbcc_draw_line(struct gbc *gbc)
{
	if (gbcc_memory_read(gbc, LY, true) > GBC_SCREEN_HEIGHT) {
		return;
	}
	gbcc_draw_background_line(gbc);
	gbcc_draw_window_line(gbc);
	gbcc_draw_sprite_line(gbc);
}

void gbcc_draw_background_line(struct gbc *gbc)
{
	uint8_t scy = gbcc_memory_read(gbc, SCY, true);
	uint8_t scx = gbcc_memory_read(gbc, SCX, true);
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t palette = gbcc_memory_read(gbc, BGP, true);
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);

	uint8_t ty = ((scy + ly) / 8u) % 32u;
	uint16_t line_offset = 2 * ((scy + ly) % 8u); /* 2 bytes per line */
	uint16_t map;
	if (check_bit(lcdc, 3)) {
		map = BACKGROUND_MAP_BANK_2;
	} else {
		map = BACKGROUND_MAP_BANK_1;
	}

	if (!check_bit(lcdc, 0)) {
		for (size_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
			gbc->memory.gbc_screen[GBC_SCREEN_WIDTH * ly + x] = 0xc4cfa1u;
		}
		return;
	}

	for (size_t x = 0; x < GBC_SCREEN_WIDTH; x++) {
		uint8_t tx = ((scx + x) / 8u) % 32u;
		uint8_t xoff = (scx + x) % 8u;
		uint8_t tile = gbcc_memory_read(gbc, map + 32 * ty + tx, true);
		uint16_t tile_addr;
		/* TODO: Put this somewhere better */
		if (check_bit(lcdc, 4)) {
			tile_addr = VRAM_START + 16 * tile;
		} else {
			tile_addr = (uint16_t)(0x9000 + 16 * (int8_t)tile);
		}
		uint8_t lo = gbcc_memory_read(gbc, tile_addr + line_offset, true);
		uint8_t hi = gbcc_memory_read(gbc, tile_addr + line_offset + 1, true);
		uint8_t colour = (uint8_t)(check_bit(hi, 7 - xoff) << 1u) | check_bit(lo, 7 - xoff);
		gbc->memory.gbc_screen[GBC_SCREEN_WIDTH * ly + x] = get_palette_colour(palette, colour, bk);
	}
}

void gbcc_draw_window_line(struct gbc *gbc)
{
	uint8_t wy = gbcc_memory_read(gbc, WY, true);
	uint8_t wx = gbcc_memory_read(gbc, WX, true);
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t palette = gbcc_memory_read(gbc, BGP, true);
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);

	if (ly < wy || !check_bit(lcdc, 5)) {
		return;
	}

	uint8_t ty = ((ly - wy) / 8u) % 32u;
	uint16_t line_offset = 2 * ((ly - wy) % 8);
	uint16_t map;
	if (check_bit(lcdc, 6)) {
		map = BACKGROUND_MAP_BANK_2;
	} else {
		map = BACKGROUND_MAP_BANK_1;
	}

	for (int x = 0; x < GBC_SCREEN_WIDTH; x++) {
		if (x < wx - 7) {
			continue;
		}
		uint8_t tx = ((x - wx + 7) / 8) % 32;
		uint8_t xoff = (x - wx + 7) % 8;
		uint8_t tile = gbcc_memory_read(gbc, map + 32 * ty + tx, true);
		uint16_t tile_addr;
		/* TODO: Put this somewhere better */
		if (check_bit(lcdc, 4)) {
			tile_addr = VRAM_START + 16 * tile;
		} else {
			tile_addr = (uint16_t)(0x9000 + 16 * (int8_t)tile);
		}
		uint8_t lo = gbcc_memory_read(gbc, tile_addr + line_offset, true);
		uint8_t hi = gbcc_memory_read(gbc, tile_addr + line_offset + 1, true);
		uint8_t colour = (uint8_t)(check_bit(hi, 7 - xoff) << 1u) | check_bit(lo, 7 - xoff);
		gbc->memory.gbc_screen[GBC_SCREEN_WIDTH * ly + x] = get_palette_colour(palette, colour, bk);
	}
}

void gbcc_draw_sprite_line(struct gbc *gbc)
{
	uint8_t ly = gbcc_memory_read(gbc, LY, true);
	uint8_t lcdc = gbcc_memory_read(gbc, LCDC, true);
	uint32_t bg0 = get_palette_colour(gbcc_memory_read(gbc, BGP, true), 0, bk);
	enum palette_flag pf;
	if (!check_bit(lcdc, 1)) {
		return;
	}
	uint8_t size = 1 + check_bit(lcdc, 2); /* 1 or 2 8x8 tiles */
	for (size_t s = NUM_SPRITES-1; s < NUM_SPRITES; s--) {
		/* 
		 * Together SY & SX define the bottom right corner of the
		 * sprite. X=1, Y=1 is the top left corner, so each of these
		 * are offset by 1 for array values.
		 * TODO: Is this off-by-one true?
		 */
		uint8_t sy = gbcc_memory_read(gbc, (uint16_t)(OAM_START + 4 * s), true);
		uint8_t sx = gbcc_memory_read(gbc, (uint16_t)(OAM_START + 4 * s + 1), true);
		uint8_t tile = gbcc_memory_read(gbc, (uint16_t)(OAM_START + 4 * s + 2), true);
		uint8_t attr = gbcc_memory_read(gbc, (uint16_t)(OAM_START + 4 * s + 3), true);
		if (ly < sy - 16 || ly >= sy) {
			//if (!(sy < 16u && ly < sy)) {
			continue;
			//}
		}
		if (size > 1) {
			/* Check for Y-flip, and swap top & bottom tiles correspondingly */
			if (ly < sy - 8u) {
				/* We are drawing the top tile */
				if (check_bit(attr, 6)) {
					tile |= 0x01u;
				} else {
					tile &= 0xFEu;
				}
				sy -= 8u;
			} else {
				/* We are drawing the bottom tile */
				if (check_bit(attr, 6)) {
					tile &= 0xFEu;
				} else {
					tile |= 0x01u;
				}
			}
		} else {
			/* 
			 * 8x8 sprites draw as if they were the top tile of an
			 * 8x16 sprite.
			 */
			if (ly < sy - 8u) {
				sy -= 8u;
			} else {
				continue;
			}
		}
		uint16_t tile_offset = VRAM_START + 16 * tile;
		uint8_t palette;
		if (check_bit(attr, 4)) {
			palette = gbcc_memory_read(gbc, OBP1, true);
			pf = sp1;
		} else {
			palette = gbcc_memory_read(gbc, OBP0, true);
			pf = sp2;
		}
		uint8_t lo;
		uint8_t hi;
		/* Check for Y-flip */
		if (check_bit(attr, 6)) {
			lo = gbcc_memory_read(gbc, tile_offset + 2 * (sy - ly - 1), true);
			hi = gbcc_memory_read(gbc, tile_offset + 2 * (sy - ly - 1) + 1, true);
		} else {
			lo = gbcc_memory_read(gbc, tile_offset + 2 * (8 - (sy - ly)), true);
			hi = gbcc_memory_read(gbc, tile_offset + 2 * (8 - (sy - ly)) + 1, true);
		}
		for (uint8_t x = 0; x < 8; x++) {
			uint8_t colour = (uint8_t)(check_bit(hi, 7 - x) << 1u) | check_bit(lo, 7 - x);
			/* Colour 0 is transparent */
			if (!colour) {
				continue;
			}
			/* Check for X-flip */
			uint8_t screen_x;
			if (check_bit(attr, 5)) {
				screen_x = sx - x - 1;
			} else {
				screen_x = sx + x - 8;
			}
			if (screen_x < GBC_SCREEN_WIDTH) {
				if (check_bit(attr, 7) && gbc->memory.gbc_screen[GBC_SCREEN_WIDTH * ly + screen_x] != bg0) {
					continue;
				}
				gbc->memory.gbc_screen[GBC_SCREEN_WIDTH * ly + screen_x] = get_palette_colour(palette, colour, pf);
			}
		}
	}
}

uint8_t gbcc_get_video_mode(struct gbc *gbc)
{
	return gbcc_memory_read(gbc, STAT, true) & 0x03u;
}

void gbcc_set_video_mode(struct gbc *gbc, uint8_t mode)
{
	uint8_t stat = gbcc_memory_read(gbc, STAT, true);
	stat &= 0xFCu;
	stat |= mode;
	gbcc_memory_write(gbc, STAT, stat, true);
}

uint32_t get_palette_colour(uint8_t palette, uint8_t n, enum palette_flag pf)
{
	uint8_t colours[4] = {
		(palette & 0x03u) >> 0u,
		(palette & 0x0Cu) >> 2u,
		(palette & 0x30u) >> 4u,
		(palette & 0xC0u) >> 6u
	};
	switch (pf) {
		case bk:
		switch (colours[n]) {
			case 3:
				return 0x000000u;
			case 2:
				return 0x7f3848u;
			case 1:
				return 0xff8096u;
			case 0:
				return 0xf8f8f8u;
			default:
				return 0;
		}
		case sp1:
		switch (colours[n]) {
			case 3:
				return 0x093609u;
			case 2:
				return 0x376019u;
			case 1:
				return 0x1fba1fu;
			case 0:
				return 0xf8f8f8u;
			default:
				return 0;
		}
		case sp2:
		switch (colours[n]) {
			case 3:
				return 0x000000u;
			case 2:
				return 0x0f3eaau;
			case 1:
				return 0x71b6d0u;
			case 0:
				return 0xf8f8f8u;
			default:
				return 0;
		}
	}
}
