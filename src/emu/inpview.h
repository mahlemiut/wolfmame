// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Input viewer header

#include "emu.h"

#define INPUT_TYPES 5
#define CHARACTER_WIDTH  (1.0f / 80.0f)

#define COL_RED     rgb_t(0xff, 0xff, 0x00, 0x00)
#define COL_BLUE    rgb_t(0xff, 0x00, 0x00, 0xff)
#define COL_GREEN   rgb_t(0xff, 0x00, 0xff, 0x00)
#define COL_YELLOW  rgb_t(0xff, 0xff, 0xff, 0x00)
#define COL_ORANGE  rgb_t(0xff, 0xff, 0x80, 0x00)
#define COL_BLACK   rgb_t(0xff, 0x00, 0x00, 0x00)
#define COL_GRAY    rgb_t(0xff, 0x80, 0x80, 0x80)
#define COL_WHITE   rgb_t(0xff, 0xff, 0xff, 0xff)

#define BGCOL       rgb_t(0x80, 0x80, 0x00, 0x00)

struct inputs
{
	char text[6];  // character(s) to display
	int port;  // port to check
	int line;  // line to display on
	int x;  // location on line to display at
	char isanalogue;  // non-zero if the input is analogue
	char playerspecific;  // is a player specific port (if 0, then player should be 0 too)
	unsigned long colour;
};


