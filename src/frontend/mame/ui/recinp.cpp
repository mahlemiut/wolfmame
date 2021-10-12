// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
	ui/recinp.cpp - UI code for INP recording function
*/

#include "emu.h"
#include "mame.h"
#include "drivenum.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/recinp.h"
#include "ui/selector.h"
#include "ui/filesel.h"
#include "ui/selsoft.h"
#include "ui/utils.h"
#include "emuopts.h"
#include "mameopts.h"
#include "audit.h"
#include "softlist_dev.h"

namespace ui {

// INP recording class

ui_menu_record_inp::ui_menu_record_inp(mame_ui_manager &mui, render_container &container, const game_driver *driver) : menu(mui, container)
{
	std::string path;
	m_driver = (driver == nullptr) ? mame_options::system(mui.machine().options()) : driver;
	m_warning_count = 0;

	// check if setup is correct for MARP use
	// first, NVRAM
	path = mui.machine().options().nvram_directory();
	path += "/";
	path += m_driver->name;
	m_warning[0] = false;
	if(strcmp(mui.machine().options().nvram_directory(),"NUL") != 0 && strcmp(mui.machine().options().nvram_directory(),"/dev/null") != 0)
	{
		// silence warning if nvram folder doesn't exist
		auto e = osd_stat(path);
		if (e != nullptr)
		{
			m_warning_count++;
			m_warning[0] = true;
		}
	}

	// DIFF file
	m_warning[1] = false;
	path = mui.machine().options().diff_directory();
	path += "/";
	path += m_driver->name;
	path += ".dif";
	auto e = osd_stat(path);
	if (e != nullptr)
	{
		m_warning_count++;
		m_warning[1] = true;
	}

	// Lua console
	m_warning[2] = false;
	if(mui.machine().options().console())
	{
		m_warning_count++;
		m_warning[2] = true;
	}
}

ui_menu_record_inp::~ui_menu_record_inp()
{
//	menu::menu_stack->reset(reset_options::SELECT_FIRST);
//	save_ui_options(machine());
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_record_inp::handle()
{
	bool changed = false;

	// process the menu
	const event *menu_event = process(0);

	if (menu_event != nullptr)
	{
		switch (menu_event->iptkey)
		{
			case IPT_SPECIAL:
				int buflen = strlen(m_filename_entry);

				// if it's a backspace and we can handle it, do so
				if (((menu_event->unichar == 8 || menu_event->unichar == 0x7f) && buflen > 0))
				{
					*(char *)utf8_previous_char(&m_filename_entry[buflen]) = 0;
					reset(reset_options::SELECT_FIRST);
				}

				// if it's any other key and we're not maxed out, update
				else if ((menu_event->unichar >= ' ' && menu_event->unichar < 0x7f))
				{
					buflen += utf8_from_uchar(&m_filename_entry[buflen], std::size(m_filename_entry) - buflen, menu_event->unichar);
					m_filename_entry[buflen] = 0;
					reset(reset_options::SELECT_FIRST);
				}
				break;
		}
		if(menu_event->itemref != nullptr)
		{
			switch((uintptr_t)menu_event->itemref)
			{
			case 1:
				if(menu_event->iptkey == IPT_UI_SELECT)
				{
					// if filename doesn't end in ".inp", then add it
					if(strcmp(&m_filename_entry[strlen(m_filename_entry)-4],".inp"))
						strcat(m_filename_entry,".inp");
					start_inp();
				}
				break;
			}
		}
	}

	if (changed)
		reset(reset_options::REMEMBER_REF);

}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_record_inp::populate(float &customtop, float &custombottom)
{
	// add options items
	item_append(_("Start recording"), "", 0 , (void*)(uintptr_t)1);

	customtop = mame_machine_manager::instance()->ui().get_line_height() + (3.0f * mame_machine_manager::instance()->ui().box_tb_border());
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_record_inp::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	mame_ui_manager &mui = mame_machine_manager::instance()->ui();
	float height = mame_machine_manager::instance()->ui().get_line_height();
	std::string str;

	// filename entry
	str = "Filename: ";
	str += m_filename_entry;
	str += "_";

	mui.draw_outlined_box(container(), 0.1f,origy1 - (height*2),0.9f,origy1, mui.colors().background_color());
	mui.draw_text_full(container(),_("Please enter a filename for the INP..."),0.1f,origy1 - (height*2),0.8f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, mui.colors().text_color(), mui.colors().text_bg_color(), nullptr, nullptr);
	mui.draw_text_full(container(),str.c_str(),0.1f,origy1 - height,0.8f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, mui.colors().text_color(), mui.colors().text_bg_color(), nullptr, nullptr);

	// warning display
	if(m_warning_count > 0)
	{
		float line = 3;
		int x;
		mui.draw_outlined_box(container(), 0.1f,1.0f - (height*3*m_warning_count),0.9f,1.0f, UI_YELLOW_COLOR);
		for(x=0;x<TOTAL_WARNINGS;x++)
		{
			if(m_warning[x])
			{
				mui.draw_text_full(container(),m_warning_text[x].c_str(),0.1f,1.0f - (height*line),0.8f, ui::text_layout::LEFT, ui::text_layout::WORD, mame_ui_manager::NORMAL, mui.colors().text_color(), mui.colors().text_bg_color(), nullptr, nullptr);
				line += 3;
			}
		}
	}
}

void ui_menu_record_inp::start_inp()
{
	// audit the game first to see if we're going to work
//	driver_enumerator enumerator(machine().options(), *m_driver);
//	enumerator.next();
//	media_auditor auditor(enumerator);
//	media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

	// if everything looks good, schedule the new driver
/*	if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
	{
		if ((m_driver->flags & MACHINE_TYPE_ARCADE) == 0)
		{
			for (software_list_device &swlistdev : software_list_device_iterator(enumerator.config()->root_device()))
				if (!swlistdev.get_info().empty())
				{
					menu::stack_push<menu_select_software>(ui(), container(), m_driver);
					return;
				}
		}

		s_bios biosname;
		machine().options().set_value(OPTION_RECORD,m_filename_entry,OPTION_PRIORITY_HIGH);
		if (!mame_machine_manager::instance()->ui().options().skip_bios_menu()) && has_multiple_bios(m_driver, biosname))
			menu::stack_push<bios_selection>(ui(), container(), biosname, (void *)m_driver, false, false);
		else
		{
			reselect_last::reset();
			reselect_last::set_driver(m_driver);
			mame_machine_manager::instance()->schedule_new_driver(*m_driver);
			machine().schedule_hard_reset();
			menu::stack_reset(machine());
		}
	}
	// otherwise, display an error
	else
	{
		machine().popmessage(_("ROM audit failed.  Cannot start system.  Please check your ROMset is correct and up to date."));
	}*/
}
/*
void ui_menu_record_inp::inkey_export()
{
}

float ui_menu_record_inp::draw_left_panel(float x1, float y1, float x2, float y2)
{
	return 0.0f;
}

void ui_menu_record_inp::get_selection(ui_software_info const *&software, ui_system_info const *&system) const
{
	software = nullptr;
	system = nullptr;
}

render_texture *ui_menu_record_inp::get_icon_texture(int linenum, void *selectedref)
{
	return nullptr;
}

void ui_menu_record_inp::make_topbox_text(std::string &line0,std::string &line1,std::string &line2) const
{
}

std::string ui_menu_record_inp::make_software_description(ui_software_info const &software) const
{
	return std::string("what");
}

void ui_menu_record_inp::filter_selected()
{
}
*/

// INP playback class
ui_menu_playback_inp::ui_menu_playback_inp(mame_ui_manager &mui, render_container &container, const game_driver *driver)
	: ui_menu_record_inp(mui, container, driver),
	  browse_done(false)
{
	inp_file = "";
	inp_dir += mui.machine().options().input_directory();
}

ui_menu_playback_inp::~ui_menu_playback_inp()
{
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_playback_inp::populate(float &customtop, float &custombottom)
{
	// add options items
	item_append(_("Start playback"), "", 0 , (void*)(uintptr_t)1);
	item_append(_("Browse..."), "", 0 , (void*)(uintptr_t)2);
	customtop = mame_machine_manager::instance()->ui().get_line_height() + (3.0f * mame_machine_manager::instance()->ui().box_tb_border());
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_playback_inp::handle()
{
	bool changed = false;

	// process the menu
	const event *menu_event = process(PROCESS_LR_REPEAT);

	if(browse_done)
	{
		if(browse_result == menu_file_selector::result::FILE)
		{
			int pos = inp_file.find_last_of("/\\");
			strcpy(m_filename_entry,inp_file.substr(pos+1).c_str());
		}
		browse_done = false;
	}

	if (menu_event != nullptr)
	{
		switch (menu_event->iptkey)
		{
			case IPT_SPECIAL:
				int buflen = strlen(m_filename_entry);

				// if it's a backspace and we can handle it, do so
				if (((menu_event->unichar == 8 || menu_event->unichar == 0x7f) && buflen > 0))
				{
					*(char *)utf8_previous_char(&m_filename_entry[buflen]) = 0;
					reset(reset_options::SELECT_FIRST);
				}

				// if it's any other key and we're not maxed out, update
				else if ((menu_event->unichar >= ' ' && menu_event->unichar < 0x7f))
				{
					buflen += utf8_from_uchar(&m_filename_entry[buflen], std::size(m_filename_entry) - buflen, menu_event->unichar);
					m_filename_entry[buflen] = 0;
					reset(reset_options::SELECT_FIRST);
				}
				break;
		}
		if(menu_event->itemref != nullptr)
		{
			switch((uintptr_t)menu_event->itemref)
			{
			case 1:
				if(menu_event->iptkey == IPT_UI_SELECT)
				{
					// if filename doesn't end in ".inp", then add it
					if(strcmp(&m_filename_entry[strlen(m_filename_entry)-4],".inp"))
						strcat(m_filename_entry,".inp");
					start_inp();
				}
				break;
			case 2:
				if(menu_event->iptkey == IPT_UI_SELECT)
				{
					// browse for INP file
					menu::stack_push<menu_file_selector>(ui(), container(), nullptr, inp_dir, inp_file, false, false, false, browse_result);
					browse_done = true;
				}
				break;
			}
		}
	}

	if (changed)
		reset(reset_options::REMEMBER_REF);
}


void ui_menu_playback_inp::start_inp()
{
	// audit the game first to see if we're going to work
	std::string fname;
	inp_header hdr;
	emu_file f(OPEN_FLAG_READ);
//	driver_enumerator enumerator(machine().options(), *m_driver);
//	enumerator.next();
//	media_auditor auditor(enumerator);
//	media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

	// check if INP file exists
	fname = machine().options().input_directory();
	fname += "/";
	fname += m_filename_entry;
	std::error_condition const filerr = f.open(fname.c_str());
	if (filerr)
	{
		machine().popmessage(_("Cannot find or open INP file."));
		f.close();
		return;
	}

	// check if the correct game is selected (at this stage, auto-selecting the game from the INP header would be awkward from here)
	hdr.read(f);
	if(strcmp(m_driver->name,hdr.get_sysname().c_str()) != 0)
	{
		machine().popmessage(_("INP is not recorded from the same game as you have selected."));
		f.close();
		return;
	}

	f.close();

	// if everything looks good, schedule the new driver
/*	if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
	{
		if ((m_driver->flags & MACHINE_TYPE_ARCADE) == 0)
		{
			for (software_list_device &swlistdev : software_list_device_iterator(enumerator.config()->root_device()))
				if (!swlistdev.get_info().empty())
				{
					menu::stack_push<menu_select_software>(ui(), container(), m_driver);
					return;
				}
		}

		s_bios biosname;
		machine().options().set_value(OPTION_PLAYBACK,m_filename_entry,OPTION_PRIORITY_HIGH);
		if (!mame_machine_manager::instance()->ui().options().skip_bios_menu() && has_multiple_bios(m_driver, biosname))
			menu::stack_push<bios_selection>(ui(), container(), biosname, (void *)m_driver, false, false);
		else
		{
			reselect_last::driver = m_driver->name;
			reselect_last::software.clear();
			reselect_last::swlist.clear();
			mame_machine_manager::instance()->schedule_new_driver(*m_driver);
			machine().schedule_hard_reset();
			menu::stack_reset(machine());
		}
	}
	// otherwise, display an error
	else
	{
		machine().popmessage(_("ROM audit failed.  Cannot start system.  Please check your ROMset is correct and up to date."));
	}*/
}

} // namespace ui
