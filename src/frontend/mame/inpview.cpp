// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Input viewer module for MAME
// C++ re-write started May 23, 2012

#include <stdio.h>
#include "emu.h"
#include "mame.h"
#include "ui/uimain.h"
#include "ui/ui.h"
#include "render.h"
#include "inpview.h"

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
			{UTF8_LEFT,  IPT_JOYSTICK_LEFT,  1, 10, 0, 1, COL_WHITE},
			{UTF8_RIGHT, IPT_JOYSTICK_RIGHT, 1, 12, 0, 1, COL_WHITE},
			{UTF8_UP,    IPT_JOYSTICK_UP,    1, 14, 0, 1, COL_WHITE},
			{UTF8_DOWN,  IPT_JOYSTICK_DOWN,  1, 16, 0, 1, COL_WHITE},
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
			{UTF8_LEFT,  IPT_JOYSTICKLEFT_LEFT,   1, 6,  0, 1, COL_WHITE},
			{UTF8_RIGHT, IPT_JOYSTICKLEFT_RIGHT,  1, 8,  0, 1, COL_WHITE},
			{UTF8_UP,    IPT_JOYSTICKLEFT_UP,     1, 10, 0, 1, COL_WHITE},
			{UTF8_DOWN,  IPT_JOYSTICKLEFT_DOWN,   1, 12, 0, 1, COL_WHITE},
			{UTF8_LEFT,  IPT_JOYSTICKRIGHT_LEFT,  1, 18, 0, 1, COL_WHITE},
			{UTF8_RIGHT, IPT_JOYSTICKRIGHT_RIGHT, 1, 20, 0, 1, COL_WHITE},
			{UTF8_UP,    IPT_JOYSTICKRIGHT_UP,    1, 22, 0, 1, COL_WHITE},
			{UTF8_DOWN,  IPT_JOYSTICKRIGHT_DOWN,  1, 24, 0, 1, COL_WHITE},
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
			{UTF8_LEFT,  IPT_JOYSTICK_LEFT,  1, 10, 0, 1, COL_WHITE},
			{UTF8_RIGHT, IPT_JOYSTICK_RIGHT, 1, 12, 0, 1, COL_WHITE},
			{UTF8_UP,    IPT_JOYSTICK_UP,    1, 14, 0, 1, COL_WHITE},
			{UTF8_DOWN,  IPT_JOYSTICK_DOWN,  1, 16, 0, 1, COL_WHITE},
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
			{UTF8_LEFT,  IPT_JOYSTICK_LEFT,  1, 10, 0, 1, COL_WHITE},
			{UTF8_RIGHT, IPT_JOYSTICK_RIGHT, 1, 12, 0, 1, COL_WHITE},
			{UTF8_UP,    IPT_JOYSTICK_UP,    1, 14, 0, 1, COL_WHITE},
			{UTF8_DOWN,  IPT_JOYSTICK_DOWN,  1, 16, 0, 1, COL_WHITE},
			{"Tilt",  IPT_TILT, 2, 75, 0, 0, COL_RED},
			{"Test",  IPT_SERVICE, 1, 75, 0, 1, COL_GREEN},
			{"NULL", -1, 0,0,0,0,0}
		}
	}
};


input_viewer::input_viewer(running_machine& machine) :
	m_machine(machine),
	m_player(0)
{
}

void input_viewer::render_input()
{
	int port = 0;
	char txt[6];
	float height = mame_machine_manager::instance()->ui().get_line_height();

	if(m_player < 1 || m_player > 10)
		return;  // invalid player

	if(machine().input().code_pressed(KEYCODE_DEL))
	{
		render_dips();
		return;
	}

	render_container* ui = &(machine().render().ui_container());
	machine().render().ui_container().add_quad(0.0f,1.0f-(float)(inptype[m_layout].lines*height),1.0f,1.0f,BGCOL,NULL,PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	while(inptype[m_layout].inp[port].port != -1)
	{
		strcpy(txt,inptype[m_layout].inp[port].text);

		if(inptype[m_layout].inp[port].playerspecific == 0)
		{
			if(input_port_used(inptype[m_layout].inp[port].port,0) != 0)
			{
				int col = inptype[m_layout].inp[port].colour;
				mame_machine_manager::instance()->ui().draw_text_full(ui,txt,(float)(inptype[m_layout].inp[port].x * CHARACTER_WIDTH),1.0f - (float)(height * inptype[m_layout].inp[port].line),1.0f,JUSTIFY_LEFT,WRAP_NEVER,DRAW_OPAQUE,col,0,NULL,NULL);
			}
		}
		else
		{
			if(input_port_used(inptype[m_layout].inp[port].port,m_player-1) != 0)
			{
				int col = inptype[m_layout].inp[port].colour;
				mame_machine_manager::instance()->ui().draw_text_full(ui,txt,(float)(inptype[m_layout].inp[port].x * CHARACTER_WIDTH),1.0f - (float)(height * inptype[m_layout].inp[port].line),1.0f,JUSTIFY_LEFT,WRAP_NEVER,DRAW_OPAQUE,col,0,NULL,NULL);
			}
		}
		port++;
	}
}

void input_viewer::render_dips()
{
	render_container* ui = &(machine().render().ui_container());
	int dip_num = 0;
	float height = mame_machine_manager::instance()->ui().get_line_height();
	int x;

	// determine number of DIP switches to display
	/* loop over all input ports */
	for (ioport_port &port : machine().ioport().ports())
	{
		for (ioport_field &field : port.fields())
		{
			if (field.type() == IPT_DIPSWITCH)
				dip_num++;
		}
	}

	ui->add_quad(0.0f,1.0f-(float)(dip_num*height),1.0f,1.0f,BGCOL,NULL,PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	x = dip_num;

	/* loop over all input ports */
	for (ioport_port &port : machine().ioport().ports())
	{
		/* loop over all bitfields for this port */
		for (ioport_field &field : port.fields())
		{
			if (field.type() == IPT_DIPSWITCH)
			{
				char txt[512];
				const char* dip;
				const char* value = "INVALID";
				const char* def = "INVALID";
				ioport_value portdata;
				/* get current settings */
				dip = field.name();
				portdata = port.live().defvalue;
				for (ioport_setting &ptr : field.settings())
				{
					if(field.enabled())
					{
						if (ptr.value() == (portdata & field.mask()))
							value = ptr.name();
						if (ptr.value() == (field.defvalue() & field.mask()))
							def = ptr.name();
					}
				}
				sprintf(txt,"%s : %s [%s]",dip,value,def);
				mame_machine_manager::instance()->ui().draw_text_full(ui,txt,0.0f,1.0f - (float)(height * x),1.0f,JUSTIFY_LEFT,WRAP_NEVER,DRAW_OPAQUE,COL_WHITE,0,NULL,NULL);
				x--;
			}
		}
	}
}

void input_viewer::inpview_set_data(int ply, const char* lay)
{
	m_player = ply;
	m_layout = 0;
	while(m_layout < INPUT_TYPES)
	{
		if(strcmp(inptype[m_layout].name,lay) == 0)
		{
			if(ply != 0)
				printf("INPVIEW: using layout type '%s' for player %i\n",lay,ply);
			return;
		}
		m_layout++;
	}
	printf("INPVIEW: invalid type specified, standard layout in use\n");
	m_layout = 0;
}

int input_viewer::get_player()
{
	return m_player;
}

int input_viewer::input_port_used(int type,int player)
{
	/* loop over all input ports */
	for (ioport_port &port : machine().ioport().ports())
	{
		UINT32 portvalue;

		/* loop over all bitfields for this port */
		for (ioport_field &field : port.fields())
		{
			if((field.defvalue() & field.mask()) != 0)  // default value for the bit should be 0 if active high.
				portvalue = ~port.live().digital;
			else
				portvalue = port.live().digital;
			if(field.type() == type && field.player() == player)
			{
				if((field.type() == type) && (portvalue & field.mask()) != (field.defvalue() & field.mask()))
					return 1;
				else
					return 0;
			}
		}
	}
	return 0;
}


