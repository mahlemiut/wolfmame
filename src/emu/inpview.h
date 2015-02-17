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

// uncomment if you plan to use MAME's built-in font
//#define OLD_UI_FONT 1

#ifndef OLD_UI_FONT
#define DIRECTION_LEFT   0x2190
#define DIRECTION_UP     0x2191
#define DIRECTION_RIGHT  0x2192
#define DIRECTION_DOWN   0x2193
#else
#define DIRECTION_LEFT   'L'
#define DIRECTION_UP     'U'
#define DIRECTION_RIGHT  'R'
#define DIRECTION_DOWN   'D'
#endif

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

static const struct _input_type_definition
{
	char name[13];  // NULL-terminated string to identify different type using a possible -inplayout option
	int lines;  // number of lines to use for this type
	struct inputs inp[64];  // list of displayed buttons, and the inputs they correspond to (64 max)
} inptype[INPUT_TYPES] = 
{
	{
		"standard",
		1,
		{
			{"1P", IPT_START1, 1, 50, 0, 0, COL_WHITE},
			{"2P", IPT_START2, 1, 53, 0, 0, COL_WHITE},
			{"3P", IPT_START3, 1, 56, 0, 0, COL_WHITE},
			{"4P", IPT_START4, 1, 59, 0, 0, COL_WHITE},
			{"5P", IPT_START5, 1, 62, 0, 0, COL_WHITE},
			{"6P", IPT_START6, 1, 65, 0, 0, COL_WHITE},
			{"7P", IPT_START7, 1, 68, 0, 0, COL_WHITE},
			{"8P", IPT_START8, 1, 71, 0, 0, COL_WHITE},
			{"1",  IPT_BUTTON1, 1, 20, 0, 1, COL_WHITE},
			{"2",  IPT_BUTTON2, 1, 22, 0, 1, COL_WHITE},
			{"3",  IPT_BUTTON3, 1, 24, 0, 1, COL_WHITE},
			{"4",  IPT_BUTTON4, 1, 26, 0, 1, COL_WHITE},
			{"5",  IPT_BUTTON5, 1, 28, 0, 1, COL_WHITE},
			{"6",  IPT_BUTTON6, 1, 30, 0, 1, COL_WHITE},
			{"7",  IPT_BUTTON7, 1, 32, 0, 1, COL_WHITE},
			{"8",  IPT_BUTTON8, 1, 34, 0, 1, COL_WHITE},
			{"9",  IPT_BUTTON9, 1, 36, 0, 1, COL_WHITE},
			{"0",  IPT_BUTTON10, 1, 38, 0, 1, COL_WHITE},
			{"_L",  IPT_JOYSTICK_LEFT, 1, 10, 0, 1, COL_WHITE},
			{"_R",  IPT_JOYSTICK_RIGHT, 1, 12, 0, 1, COL_WHITE},
			{"_U",  IPT_JOYSTICK_UP, 1, 14, 0, 1, COL_WHITE},
			{"_D",  IPT_JOYSTICK_DOWN, 1, 16, 0, 1, COL_WHITE},
			{"Tilt",  IPT_TILT, 1, 75, 0, 0, COL_RED},
			{"Test",  IPT_SERVICE, 1, 75, 0, 1, COL_GREEN},
			{"NULL", -1, 0,0,0,0,0}
		}
	},
	{
		"mahjong",
		2,
		{
			{"1P", IPT_START1, 1, 50, 0, 0, COL_WHITE},
			{"2P", IPT_START2, 1, 53, 0, 0, COL_WHITE},
			{"3P", IPT_START3, 1, 56, 0, 0, COL_WHITE},
			{"4P", IPT_START4, 1, 59, 0, 0, COL_WHITE},
			{"5P", IPT_START5, 1, 62, 0, 0, COL_WHITE},
			{"6P", IPT_START6, 1, 65, 0, 0, COL_WHITE},
			{"7P", IPT_START7, 1, 68, 0, 0, COL_WHITE},
			{"8P", IPT_START8, 1, 71, 0, 0, COL_WHITE},
			{"A",  IPT_MAHJONG_A, 1, 10, 0, 1, COL_WHITE},
			{"B",  IPT_MAHJONG_B, 1, 12, 0, 1, COL_WHITE},
			{"C",  IPT_MAHJONG_C, 1, 14, 0, 1, COL_WHITE},
			{"D",  IPT_MAHJONG_D, 1, 16, 0, 1, COL_WHITE},
			{"E",  IPT_MAHJONG_E, 1, 18, 0, 1, COL_WHITE},
			{"F",  IPT_MAHJONG_F, 1, 20, 0, 1, COL_WHITE},
			{"G",  IPT_MAHJONG_G, 1, 22, 0, 1, COL_WHITE},
			{"H",  IPT_MAHJONG_H, 1, 24, 0, 1, COL_WHITE},
			{"I",  IPT_MAHJONG_I, 1, 26, 0, 1, COL_WHITE},
			{"J",  IPT_MAHJONG_J, 1, 28, 0, 1, COL_WHITE},
			{"K",  IPT_MAHJONG_K, 1, 30, 0, 1, COL_WHITE},
			{"L",  IPT_MAHJONG_L, 1, 32, 0, 1, COL_WHITE},
			{"M",  IPT_MAHJONG_M, 1, 34, 0, 1, COL_WHITE},
			{"N",  IPT_MAHJONG_N, 1, 36, 0, 1, COL_WHITE},
			{"O",  IPT_MAHJONG_O, 1, 38, 0, 1, COL_WHITE},
			{"P",  IPT_MAHJONG_P, 1, 40, 0, 1, COL_WHITE},
			{"Q",  IPT_MAHJONG_Q, 1, 42, 0, 1, COL_WHITE},
			{"REACH",  IPT_MAHJONG_REACH, 2, 14, 0, 1, COL_WHITE},
			{"CHI",  IPT_MAHJONG_CHI, 2, 26, 0, 1, COL_WHITE},
			{"PON",  IPT_MAHJONG_PON, 2, 34, 0, 1, COL_WHITE},
			{"KAN",  IPT_MAHJONG_KAN, 2, 42, 0, 1, COL_WHITE},
			{"RON",  IPT_MAHJONG_RON, 2, 50, 0, 1, COL_WHITE},
			{"BET",  IPT_MAHJONG_BET, 2, 58, 0, 1, COL_WHITE},
			{"Tilt",  IPT_TILT, 2, 75, 0, 0, COL_RED},
			{"Test",  IPT_SERVICE, 1, 75, 0, 1, COL_GREEN},
			{"NULL", -1, 0,0,0,0,0}
		}
	},
	{
		"dualstick",
		1,
		{
			{"1P", IPT_START1, 1, 50, 0, 0, COL_WHITE},
			{"2P", IPT_START2, 1, 53, 0, 0, COL_WHITE},
			{"3P", IPT_START3, 1, 56, 0, 0, COL_WHITE},
			{"4P", IPT_START4, 1, 59, 0, 0, COL_WHITE},
			{"5P", IPT_START5, 1, 62, 0, 0, COL_WHITE},
			{"6P", IPT_START6, 1, 65, 0, 0, COL_WHITE},
			{"7P", IPT_START7, 1, 68, 0, 0, COL_WHITE},
			{"8P", IPT_START8, 1, 71, 0, 0, COL_WHITE},
			{"1",  IPT_BUTTON1, 1, 30, 0, 1, COL_WHITE},
			{"2",  IPT_BUTTON2, 1, 32, 0, 1, COL_WHITE},
			{"3",  IPT_BUTTON3, 1, 34, 0, 1, COL_WHITE},
			{"4",  IPT_BUTTON4, 1, 36, 0, 1, COL_WHITE},
			{"5",  IPT_BUTTON5, 1, 38, 0, 1, COL_WHITE},
			{"6",  IPT_BUTTON6, 1, 40, 0, 1, COL_WHITE},
			{"7",  IPT_BUTTON7, 1, 42, 0, 1, COL_WHITE},
			{"8",  IPT_BUTTON8, 1, 44, 0, 1, COL_WHITE},
			{"9",  IPT_BUTTON9, 1, 46, 0, 1, COL_WHITE},
			{"0",  IPT_BUTTON10, 1, 48, 0, 1, COL_WHITE},
			{"_L",  IPT_JOYSTICKLEFT_LEFT,   1, 6, 0, 1, COL_WHITE},
			{"_R",  IPT_JOYSTICKLEFT_RIGHT,  1, 8, 0, 1, COL_WHITE},
			{"_U",  IPT_JOYSTICKLEFT_UP,     1, 10, 0, 1, COL_WHITE},
			{"_D",  IPT_JOYSTICKLEFT_DOWN,   1, 12, 0, 1, COL_WHITE},
			{"_L",  IPT_JOYSTICKRIGHT_LEFT,  1, 18, 0, 1, COL_WHITE},
			{"_R",  IPT_JOYSTICKRIGHT_RIGHT, 1, 20, 0, 1, COL_WHITE},
			{"_U",  IPT_JOYSTICKRIGHT_UP,    1, 22, 0, 1, COL_WHITE},
			{"_D",  IPT_JOYSTICKRIGHT_DOWN,  1, 24, 0, 1, COL_WHITE},
			{"Tilt",  IPT_TILT, 1, 75, 0, 0, COL_RED},
			{"Test",  IPT_SERVICE, 1, 75, 0, 1, COL_GREEN},
			{"NULL", -1, 0,0,0,0,0}
		}
	},
	{
		"neogeo",
		2,
		{
			{"1P", IPT_START1, 2, 53, 0, 0, COL_YELLOW},
			{"2P", IPT_START2, 1, 53, 0, 0, COL_YELLOW},
			{"A",  IPT_BUTTON1, 1, 26, 0, 1, COL_RED},
			{"B",  IPT_BUTTON2, 2, 29, 0, 1, COL_YELLOW},
			{"C",  IPT_BUTTON3, 2, 32, 0, 1, COL_GREEN},
			{"D",  IPT_BUTTON4, 1, 35, 0, 1, COL_BLUE},
			{"_L",  IPT_JOYSTICK_LEFT, 1, 15, 0, 1, COL_WHITE},
			{"_R",  IPT_JOYSTICK_RIGHT, 1, 19, 0, 1, COL_WHITE},
			{"_U",  IPT_JOYSTICK_UP, 2, 17, 0, 1, COL_WHITE},
			{"_D",  IPT_JOYSTICK_DOWN, 1, 17, 0, 1, COL_WHITE},
			{"Tilt",  IPT_TILT, 2, 75, 0, 0, COL_RED},
			{"Test",  IPT_SERVICE, 1, 75, 0, 1, COL_GREEN},
			{"NULL", -1, 0,0,0,0,0}
		}
	},
	{
		"6button",
		2,
		{
			{"1P", IPT_START1, 2, 53, 0, 0, COL_YELLOW},
			{"2P", IPT_START2, 1, 53, 0, 0, COL_YELLOW},
			{"1",  IPT_BUTTON1, 2, 30, 0, 1, COL_WHITE},
			{"2",  IPT_BUTTON2, 2, 33, 0, 1, COL_WHITE},
			{"3",  IPT_BUTTON3, 2, 36, 0, 1, COL_WHITE},
			{"4",  IPT_BUTTON4, 1, 30, 0, 1, COL_WHITE},
			{"5",  IPT_BUTTON5, 1, 33, 0, 1, COL_WHITE},
			{"6",  IPT_BUTTON6, 1, 36, 0, 1, COL_WHITE},
			{"_L",  IPT_JOYSTICK_LEFT, 1, 15, 0, 1, COL_WHITE},
			{"_R",  IPT_JOYSTICK_RIGHT, 1, 19, 0, 1, COL_WHITE},
			{"_U",  IPT_JOYSTICK_UP, 2, 17, 0, 1, COL_WHITE},
			{"_D",  IPT_JOYSTICK_DOWN, 1, 17, 0, 1, COL_WHITE},
			{"Tilt",  IPT_TILT, 2, 75, 0, 0, COL_RED},
			{"Test",  IPT_SERVICE, 1, 75, 0, 1, COL_GREEN},
			{"NULL", -1, 0,0,0,0,0}
		}
	}
};


