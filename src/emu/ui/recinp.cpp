// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
	ui/recinp.cpp - UI code for INP recording function
*/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/recinp.h"
#include "ui/selector.h"
#include "ui/utils.h"
#include "cliopts.h"

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_record_inp::ui_menu_record_inp(running_machine &machine, render_container *container, const game_driver *driver) : ui_menu(machine, container)
{
	m_driver = (driver == nullptr) ? &machine.system() : driver;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_record_inp::~ui_menu_record_inp()
{
	ui_menu::menu_stack->reset(UI_MENU_RESET_SELECT_FIRST);
//	save_ui_options(machine());
	ui_globals::switch_image = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_record_inp::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *m_event = process(0);

	if (m_event != nullptr)
	{
		switch (m_event->iptkey)
		{
			case IPT_SPECIAL:
				int buflen = strlen(m_filename_entry);

				// if it's a backspace and we can handle it, do so
				if (((m_event->unichar == 8 || m_event->unichar == 0x7f) && buflen > 0))
				{
					*(char *)utf8_previous_char(&m_filename_entry[buflen]) = 0;
					reset(UI_MENU_RESET_SELECT_FIRST);
				}

				// if it's any other key and we're not maxed out, update
				else if ((m_event->unichar >= ' ' && m_event->unichar < 0x7f))
				{
					buflen += utf8_from_uchar(&m_filename_entry[buflen], ARRAY_LENGTH(m_filename_entry) - buflen, m_event->unichar);
					m_filename_entry[buflen] = 0;
					reset(UI_MENU_RESET_SELECT_FIRST);
				}
				break;
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);

}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_record_inp::populate()
{
	// add options items
	item_append("Start recording", nullptr, 0 , nullptr);

	customtop = machine().ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_record_inp::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	ui_manager &mui = machine().ui();
	std::string str;

	str = "Filename: ";
	str += m_filename_entry;
	str += "_";

	mui.draw_outlined_box(container, 0.1f,0.0f,0.9f,machine().ui().get_line_height()*2, UI_BACKGROUND_COLOR);
	mui.draw_text_full(container,"Please enter a filename for the INP...",0.1f,0.0f,0.8f, JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
	mui.draw_text_full(container,str.c_str(),0.1f,machine().ui().get_line_height(),0.8f, JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}
