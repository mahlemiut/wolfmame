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
#include "softlist_dev.h"

namespace ui {

// INP recording class

ui_menu_record_inp::ui_menu_record_inp(mame_ui_manager &mui, render_container &container, const game_driver *driver) : menu(mui, container)
{
	std::string path;
	m_driver = (driver == nullptr) ? mame_options::system(mui.machine().options()) : driver;
	m_warning_count = 0;

	strcpy(m_filename_entry,"");
	set_process_flags(PROCESS_LR_REPEAT);

	// check if setup is correct for MARP use
	// first, NVRAM
	path = mui.machine().options().nvram_directory();
	path += "/";
	path += std::string(m_driver->name);
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
	path += std::string(m_driver->name);
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

bool ui_menu_record_inp::handle(event const *ev)
{
	bool changed = false;

	if (ev != nullptr)
	{
		switch (ev->iptkey)
		{
			case IPT_SPECIAL:
				int buflen = strlen(m_filename_entry);

				// if it's a backspace and we can handle it, do so
				if (((ev->unichar == 8 || ev->unichar == 0x7f) && buflen > 0))
				{
					*(char *)utf8_previous_char(&m_filename_entry[buflen]) = 0;
					reset(reset_options::SELECT_FIRST);
				}

				// if it's any other key and we're not maxed out, update
				else if ((ev->unichar >= ' ' && ev->unichar < 0x7f))
				{
					buflen += utf8_from_uchar(&m_filename_entry[buflen], std::size(m_filename_entry) - buflen, ev->unichar);
					m_filename_entry[buflen] = 0;
					reset(reset_options::SELECT_FIRST);
				}
				break;
		}
		if(ev->itemref != nullptr)
		{
			switch((uintptr_t)ev->itemref)
			{
			case 1:
				if(ev->iptkey == IPT_UI_SELECT)
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

	return changed;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_record_inp::populate()
{
	// add options items
	item_append(_("Start recording"), "", 0 , (void*)(uintptr_t)1);

//	customtop = mame_machine_manager::instance()->ui().get_line_height() + (3.0f * mame_machine_manager::instance()->ui().box_tb_border());
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_record_inp::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	mame_ui_manager &mui = mame_machine_manager::instance()->ui();
	float height = mame_machine_manager::instance()->ui().get_line_height();
	std::string str;

	// filename entry
	str = "Filename: ";
	str += std::string(m_filename_entry);
	str += "_";

	mui.draw_outlined_box(container(), 0.1f,origy1 - (height*2),0.9f,origy1, mui.colors().background_color());
	mui.draw_text_full(container(),_("Please enter a filename for the INP..."),0.1f,origy1 - (height*2),0.8f, ui::text_layout::text_justify::CENTER, ui::text_layout::word_wrapping::TRUNCATE, mame_ui_manager::NORMAL, mui.colors().text_color(), mui.colors().text_bg_color(), nullptr, nullptr);
	mui.draw_text_full(container(),str.c_str(),0.1f,origy1 - height,0.8f, ui::text_layout::text_justify::CENTER, ui::text_layout::word_wrapping::TRUNCATE, mame_ui_manager::NORMAL, mui.colors().text_color(), mui.colors().text_bg_color(), nullptr, nullptr);

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
				mui.draw_text_full(container(),m_warning_text[x].c_str(),0.1f,1.0f - (height*line),0.8f, ui::text_layout::text_justify::LEFT, ui::text_layout::word_wrapping::WORD, mame_ui_manager::NORMAL, mui.colors().text_color(), 				mui.colors().text_bg_color(), nullptr, nullptr);
				line += 3;
			}
		}
	}
}

void ui_menu_record_inp::launch_system(mame_ui_manager &mui, game_driver const &driver, bool rec, ui_software_info const *swinfo, std::string const *part, int const *bios)
{
	emu_options &moptions(mui.machine().options());
	moptions.set_system_name(driver.name);

	if (swinfo)
	{
		if (!swinfo->startempty)
		{
			if (part)
				moptions.set_value(swinfo->instance, util::string_format("%s:%s:%s", swinfo->listname, swinfo->shortname, *part), OPTION_PRIORITY_CMDLINE);
			else
				moptions.set_value(OPTION_SOFTWARENAME, util::string_format("%s:%s", swinfo->listname, swinfo->shortname), OPTION_PRIORITY_CMDLINE);

			moptions.set_value(OPTION_SNAPNAME, util::path_concat(swinfo->listname, swinfo->shortname), OPTION_PRIORITY_CMDLINE);
		}
	}

	if (bios)
		moptions.set_value(OPTION_BIOS, *bios, OPTION_PRIORITY_CMDLINE);

	// set input file
	if(rec)
		moptions.set_value(OPTION_RECORD, m_filename_entry, OPTION_PRIORITY_CMDLINE);
	else
		moptions.set_value(OPTION_PLAYBACK, m_filename_entry, OPTION_PRIORITY_CMDLINE);

	mame_machine_manager::instance()->schedule_new_driver(driver);
	mui.machine().schedule_hard_reset();
	stack_reset(mui);

}

void ui_menu_record_inp::start_inp()
{
	std::string fname;

	// anything else is a driver
	driver_enumerator enumerator(machine().options(), *m_driver);
	enumerator.next();

	// if there are software entries, show a software selection menu
	for (software_list_device &swlistdev : software_list_device_enumerator(enumerator.config()->root_device()))
	{
		if (!swlistdev.get_info().empty())
		{
//			menu::stack_push<menu_select_software>(ui(), container(), *m_driver);
			return;
		}
	}

	// audit the system ROMs first to see if we're going to work
	media_auditor auditor(enumerator);
	media_auditor::summary const summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

	// if everything looks good, schedule the new driver
	if (audit_passed(summary))
	{
		launch_system(mame_machine_manager::instance()->ui(), *m_driver, true,nullptr,nullptr,nullptr);
	}
	else
	{
		// otherwise, display an error
		machine().popmessage("Cannot start system!");
	}
}

// INP playback class
ui_menu_playback_inp::ui_menu_playback_inp(mame_ui_manager &mui, render_container &container, const game_driver *driver)
	: ui_menu_record_inp(mui, container, driver),
	  browse_done(false)
{
	set_process_flags(PROCESS_LR_REPEAT);
	inp_file = "";
	inp_dir += mui.machine().options().input_directory();
}

ui_menu_playback_inp::~ui_menu_playback_inp()
{
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_playback_inp::populate()
{
	// add options items
	item_append(_("Start playback"), "", 0 , (void*)(uintptr_t)1);
	item_append(_("Browse..."), "", 0 , (void*)(uintptr_t)2);
	//customtop = mame_machine_manager::instance()->ui().get_line_height() + (3.0f * mame_machine_manager::instance()->ui().box_tb_border());
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

bool ui_menu_playback_inp::handle(event const *ev)
{
	bool changed = false;

	if (ev != nullptr)
	{
		switch (ev->iptkey)
		{
			case IPT_SPECIAL:
				int buflen = strlen(m_filename_entry);

				// if it's a backspace and we can handle it, do so
				if (((ev->unichar == 8 || ev->unichar == 0x7f) && buflen > 0))
				{
					*(char *)utf8_previous_char(&m_filename_entry[buflen]) = 0;
					reset(reset_options::SELECT_FIRST);
				}

				// if it's any other key and we're not maxed out, update
				else if ((ev->unichar >= ' ' && ev->unichar < 0x7f))
				{
					buflen += utf8_from_uchar(&m_filename_entry[buflen], std::size(m_filename_entry) - buflen, ev->unichar);
					m_filename_entry[buflen] = 0;
					reset(reset_options::SELECT_FIRST);
				}
				break;
		}
		if(ev->itemref != nullptr)
		{
			switch((uintptr_t)ev->itemref)
			{
			case 1:
				if(ev->iptkey == IPT_UI_SELECT)
				{
					// if filename doesn't end in ".inp", then add it
					if(strcmp(&m_filename_entry[strlen(m_filename_entry)-4],".inp"))
						strcat(m_filename_entry,".inp");
					start_inp();
				}
				break;
			case 2:
				if(ev->iptkey == IPT_UI_SELECT)
				{
					// browse for INP file
					menu::stack_push<menu_file_selector>(ui(), container(), nullptr, inp_dir, inp_file, false, false, false,
						[this](menu_file_selector::result result, std::string &&directory, std::string &&inp_file)
						{
							if(result == menu_file_selector::result::FILE)
							{
								int pos = inp_file.find_last_of("/\\");
								strcpy(m_filename_entry,inp_file.substr(pos+1).c_str());
							}
							browse_done = false;
						});
					browse_done = true;
				}
				break;
			}
		}
	}

	return changed;
}


void ui_menu_playback_inp::start_inp()
{
	std::string fname;
	inp_header hdr;
	emu_file f(OPEN_FLAG_READ);

	// check if INP file exists
	fname = machine().options().input_directory();
	fname += "/";
	fname += std::string(m_filename_entry);
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
	// anything else is a driver
	driver_enumerator enumerator(machine().options(), *m_driver);
	enumerator.next();

	// if there are software entries, show a software selection menu
	for (software_list_device &swlistdev : software_list_device_enumerator(enumerator.config()->root_device()))
	{
		if (!swlistdev.get_info().empty())
		{
//			menu::stack_push<menu_select_software>(ui(), container(), *m_driver);
			return;
		}
	}

	// audit the system ROMs first to see if we're going to work
	media_auditor auditor(enumerator);
	media_auditor::summary const summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

	// if everything looks good, schedule the new driver
	if (audit_passed(summary))
	{
		launch_system(mame_machine_manager::instance()->ui(), *m_driver, false,nullptr,nullptr,nullptr);
	}
	else
	{
		// otherwise, display an error
		machine().popmessage("Cannot start system!");
	}
}

} // namespace ui
