// Input viewer module for MAME
// C++ re-write started May 23, 2012

#include <stdio.h>
#include "emu.h"
#include "ui/ui.h"
#include "render.h"

input_viewer::input_viewer(running_machine& machine) :
	m_machine(machine)
{
}

void input_viewer::render_input()
{
	int port = 0;
	char txt[6];
	float height = machine().ui().get_line_height();

	if(m_player < 1 || m_player > 8)
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
				int ch = convert_txt(txt);
				int col = inptype[m_layout].inp[port].colour;
				if(ch == 0)
					machine().ui().draw_text_full(ui,txt,(float)(inptype[m_layout].inp[port].x * CHARACTER_WIDTH),1.0f - (float)(height * inptype[m_layout].inp[port].line),1.0f,JUSTIFY_LEFT,WRAP_NEVER,DRAW_OPAQUE,col,0,NULL,NULL);
				else
					machine().render().ui_container().add_char((float)(inptype[m_layout].inp[port].x * CHARACTER_WIDTH),1.0f - (float)(height * inptype[m_layout].inp[port].line),height,machine().render().ui_aspect(),col,*(machine().ui().get_font()),ch);
			}
		}
		else
		{
			if(input_port_used(inptype[m_layout].inp[port].port,m_player-1) != 0)
			{
				int ch = convert_txt(txt);
				int col = inptype[m_layout].inp[port].colour;
				if(ch == 0)
					machine().ui().draw_text_full(ui,txt,(float)(inptype[m_layout].inp[port].x * CHARACTER_WIDTH),1.0f - (float)(height * inptype[m_layout].inp[port].line),1.0f,JUSTIFY_LEFT,WRAP_NEVER,DRAW_OPAQUE,col,0,NULL,NULL);
				else
					machine().render().ui_container().add_char((float)(inptype[m_layout].inp[port].x * CHARACTER_WIDTH),1.0f - (float)(height * inptype[m_layout].inp[port].line),height,machine().render().ui_aspect(),col,*(machine().ui().get_font()),ch);
			}
		}
		port++;
	}
}

void input_viewer::render_dips()
{
	render_container* ui = &(machine().render().ui_container());
	ioport_port* port;
	ioport_field* bit;
	int dip_num = 0;
	float height = machine().ui().get_line_height();
	int x;

	// determine number of DIP switches to display
	/* loop over all input ports */
	for (port = machine().ioport().first_port(); port != NULL; port = port->next())
	{
		/* loop over all bitfields for this port */
		for (bit = port->first_field(); bit != NULL; bit = bit->next())
		{
			if (bit->type() == IPT_DIPSWITCH)
			{
				dip_num++;
			}
		}
	}

	ui->add_quad(0.0f,1.0f-(float)(dip_num*height),1.0f,1.0f,BGCOL,NULL,PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	x = dip_num;

	/* loop over all input ports */
	for (port = machine().ioport().first_port(); port != NULL; port = port->next())
	{
		/* loop over all bitfields for this port */
		for (bit = port->first_field(); bit != NULL; bit = bit->next())
		{
			if (bit->type() == IPT_DIPSWITCH)
			{
				char txt[512];
				const char* dip;
				const char* value = "INVALID";
				const char* def = "INVALID";
				ioport_value portdata;

				// scan the list of settings looking for a match on the current value
				if(bit->first_setting() != NULL)
				{
					dip = bit->name();
					portdata = machine().ioport().get_defvalue(port);
					for (ioport_setting *setting = bit->first_setting(); setting != NULL; setting = setting->next())
						if (setting->enabled())
						{
							if (setting->value() == (portdata & bit->mask()))
								value = setting->name();
							if (setting->value() == (bit->defvalue() & bit->mask()))
								def = setting->name();
						}
					sprintf(txt,"%s : %s [%s]",dip,value,def);
					machine().ui().draw_text_full(ui,txt,0.0f,1.0f - (float)(height * x),1.0f,JUSTIFY_LEFT,WRAP_NEVER,DRAW_OPAQUE,COL_WHITE,0,NULL,NULL);
					x--;
				}
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
	ioport_port* port;
	ioport_field* bit;

	/* loop over all input ports */
	for (port = machine().ioport().first_port(); port != NULL; port = port->next())
	{
		UINT32 portvalue;

		/* loop over all bitfields for this port */
		for (bit = port->first_field(); bit != NULL; bit = bit->next())
		{
			if((bit->defvalue() & bit->mask()) != 0)  // default value for the bit should be 0 if active high.
				portvalue = ~machine().ioport().get_digital(port);
			else
				portvalue = machine().ioport().get_digital(port);
			if(bit->type() == type && bit->player() == player)
			{
				if((bit->type() == type) && (portvalue & bit->mask()) != (bit->defvalue() & bit->mask()))
					return 1;
				else
					return 0;
			}
		}
	}
	return 0;
}


unsigned int input_viewer::convert_txt(char* txt)
{
		if(strcmp(txt,"_L") == 0)
			return DIRECTION_LEFT;
		if(strcmp(txt,"_R") == 0)
			return DIRECTION_RIGHT;
		if(strcmp(txt,"_U") == 0)
			return DIRECTION_UP;
		if(strcmp(txt,"_D") == 0)
			return DIRECTION_DOWN;
	return 0;
}


