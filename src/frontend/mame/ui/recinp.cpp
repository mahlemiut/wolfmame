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

#include "imagedev/floppy.h"

#include "uiinput.h"

#include "util/corestr.h"
#include "util/zippath.h"

#include <cstring>
#include <locale>

namespace ui {

// custom file browser (basically the file selector without using images)

// conditional compilation to enable chosing of image formats - this is not
// yet fully implemented
#define ENABLE_FORMATS          0

// time (in seconds) to display errors
#define ERROR_MESSAGE_TIME      5

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_browse::menu_browse(
		mame_ui_manager &mui,
		render_target &target,
		std::string_view directory,
		std::string_view file,
		bool has_empty,
		bool has_softlist,
		bool has_create,
		handler_function &&handler)
	: menu(mui, target)
	, m_handler(std::move(handler))
	, m_current_directory(directory)
	, m_current_file(file)
	, m_has_empty(has_empty)
	, m_has_softlist(has_softlist)
	, m_has_create(has_create)
	, m_is_midi(false)
	, m_clicked_directory(std::string::npos, std::string::npos)
{
	set_process_flags(PROCESS_IGNOREPAUSE);
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_browse::~menu_browse()
{
}


//-------------------------------------------------
//  recompute_metrics - recompute metrics
//-------------------------------------------------

void menu_browse::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	m_path_layout.reset();
	m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);

	set_custom_space(line_height() + 3.0F * tb_border(), 0.0F);
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_browse::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// lay out extra text
	if (!m_path_layout)
	{
		m_path_layout.emplace(create_layout());
		m_path_layout->add_text(m_current_directory);
	}
	else
	{
		rgb_t const fgcolor = ui().colors().text_color();
		rgb_t const bgcolor = rgb_t::transparent();
		m_path_layout->restyle(0, m_current_directory.length(), &fgcolor, &bgcolor);
	}

	// position this extra text
	float x2, y2;
	extra_text_position(origx1, origx2, origy1, top, *m_path_layout, -1, m_path_position.first, m_path_position.second, x2, y2);

	// draw a box
	ui().draw_outlined_box(container(), m_path_position.first, m_path_position.second, x2, y2, ui().colors().background_color());

	// take off the borders
	m_path_position.first += lr_border();
	m_path_position.second += tb_border();

	if (m_clicked_directory.second > m_clicked_directory.first)
	{
		// see if it's still over the clicked path component
		auto const [x, y] = pointer_location();
		size_t start = 0, span = 0;
		if (m_path_layout->hit_test(x - m_path_position.first, y - m_path_position.second, start, span))
		{
			if ((start >= m_clicked_directory.first) && ((start + span) <= m_clicked_directory.second))
			{
				rgb_t const fgcolor = ui().colors().selected_color();
				rgb_t const bgcolor = ui().colors().selected_bg_color();
				m_path_layout->restyle(m_clicked_directory.first, m_clicked_directory.second - m_clicked_directory.first, &fgcolor, &bgcolor);
			}
		}
	}
	else if (pointer_idle())
	{
		// see if it's hovering over a path component
		auto const [x, y] = pointer_location();
		auto const [target_dir_start, target_dir_end] = get_directory_range(x, y);
		if (target_dir_end > target_dir_start)
		{
			rgb_t const fgcolor = ui().colors().mouseover_color();
			rgb_t const bgcolor = ui().colors().mouseover_bg_color();
			m_path_layout->restyle(target_dir_start, target_dir_end - target_dir_start, &fgcolor, &bgcolor);
		}
	}

	// draw the text within it
	m_path_layout->emit(container(), m_path_position.first, m_path_position.second);
}


//-------------------------------------------------
//  custom_pointer_updated - perform our special
//  pointer handling
//-------------------------------------------------

std::tuple<int, bool, bool> menu_browse::custom_pointer_updated(bool changed, ui_event const &uievt)
{
	// track pointer after clicking a path component
	if (m_clicked_directory.second > m_clicked_directory.first)
	{
		if (ui_event::type::POINTER_ABORT == uievt.event_type)
		{
			// abort always cancels
			m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
			return std::make_tuple(IPT_INVALID, false, true);
		}
		else if (uievt.pointer_released & 0x01)
		{
			// releasing the primary button - check for dragging out
			auto const [x, y] = pointer_location();
			size_t start = 0, span = 0;
			if (m_path_layout->hit_test(x - m_path_position.first, y - m_path_position.second, start, span))
			{
				// abuse IPT_CUSTOM to change to the clicked directory
				if ((start >= m_clicked_directory.first) && ((start + span) <= m_clicked_directory.second))
					return std::make_tuple(IPT_CUSTOM, false, true);
			}
			m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
			return std::make_tuple(IPT_INVALID, false, true);
		}
		else if (uievt.pointer_buttons & ~u32(1))
		{
			// pressing more buttons cancels
			m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
			return std::make_tuple(IPT_INVALID, false, true);
		}
		else
		{
			// keep tracking the pointer
			return std::make_tuple(IPT_INVALID, true, false);
		}
	}

	// check for clicks if we have up-to-date content on-screen
	if (m_path_layout && pointer_idle() && (uievt.pointer_buttons & 0x01) && !(uievt.pointer_buttons & ~u32(0x01)))
	{
		auto const [x, y] = pointer_location();
		auto const [target_dir_start, target_dir_end] = get_directory_range(x, y);
		if (target_dir_end > target_dir_start)
		{
			m_clicked_directory = std::make_pair(target_dir_start, target_dir_end);
			return std::make_tuple(IPT_INVALID, true, true);
		}
	}

	return std::make_tuple(IPT_INVALID, false, false);
}


//-------------------------------------------------
//  menu_activated - menu has gained focus
//-------------------------------------------------

void menu_browse::menu_activated()
{
	m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
}


//-------------------------------------------------
//  append_entry - appends a new
//  file selector entry to an entry list
//-------------------------------------------------

menu_browse::file_selector_entry &menu_browse::append_entry(
		file_selector_entry_type entry_type,
		const std::string &entry_basename,
		const std::string &entry_fullpath)
{
	return append_entry(entry_type, std::string(entry_basename), std::string(entry_fullpath));
}


//-------------------------------------------------
//  append_entry - appends a new
//  file selector entry to an entry list
//-------------------------------------------------

menu_browse::file_selector_entry &menu_browse::append_entry(
		file_selector_entry_type entry_type,
		std::string &&entry_basename,
		std::string &&entry_fullpath)
{
	// allocate a new entry
	file_selector_entry entry;
	entry.type = entry_type;
	entry.basename = std::move(entry_basename);
	entry.fullpath = std::move(entry_fullpath);

	// find the end of the list
	return m_entrylist.emplace_back(std::move(entry));
}


//-------------------------------------------------
//  append_dirent_entry - appends
//  a menu item for a file selector entry
//-------------------------------------------------

menu_browse::file_selector_entry *menu_browse::append_dirent_entry(const osd::directory::entry *dirent)
{
	file_selector_entry_type entry_type;
	switch (dirent->type)
	{
	case osd::directory::entry::entry_type::FILE:
		entry_type = SELECTOR_ENTRY_TYPE_FILE;
		break;

	case osd::directory::entry::entry_type::DIR:
		entry_type = SELECTOR_ENTRY_TYPE_DIRECTORY;
		break;

	default:
		// exceptional case; do not add a menu item
		return nullptr;
	}

	// determine the full path
	std::string buffer = util::zippath_combine(m_current_directory, dirent->name);

	// create the file selector entry
	return &append_entry(
			entry_type,
			dirent->name,
			std::move(buffer));
}


//-------------------------------------------------
//  append_entry_menu_item - appends
//  a menu item for a file selector entry
//-------------------------------------------------

void menu_browse::append_entry_menu_item(const file_selector_entry *entry)
{
	std::string text;
	std::string subtext;

	switch(entry->type)
	{
		case SELECTOR_ENTRY_TYPE_EMPTY:
			text = _("[empty slot]");
			break;

		case SELECTOR_ENTRY_TYPE_MIDI:
			text = _("[MIDI port]");
			break;

		case SELECTOR_ENTRY_TYPE_CREATE:
			text = _("[create]");
			break;

		case SELECTOR_ENTRY_TYPE_SOFTWARE_LIST:
			text = _("[software list]");
			break;

		case SELECTOR_ENTRY_TYPE_DRIVE:
			text = entry->basename;
			subtext = "[DRIVE]";
			break;

		case SELECTOR_ENTRY_TYPE_DIRECTORY:
			text = entry->basename;
			subtext = "[DIR]";
			break;

		case SELECTOR_ENTRY_TYPE_FILE:
			text = entry->basename;
			subtext = "[FILE]";
			break;
	}
	item_append(std::move(text), std::move(subtext), 0, (void *) entry);
}


//-------------------------------------------------
//  select_item
//-------------------------------------------------

void menu_browse::select_item(const file_selector_entry &entry)
{
	switch (entry.type)
	{
	case SELECTOR_ENTRY_TYPE_EMPTY:
		m_handler(result::EMPTY, m_current_directory, m_current_file);
		break;

	case SELECTOR_ENTRY_TYPE_MIDI:
		m_handler(result::MIDI, m_current_directory, m_current_file);
		break;

	case SELECTOR_ENTRY_TYPE_CREATE:
		m_handler(result::CREATE, m_current_directory, m_current_file);
		break;

	case SELECTOR_ENTRY_TYPE_SOFTWARE_LIST:
		m_handler(result::SOFTLIST, m_current_directory, m_current_file);
		break;

	case SELECTOR_ENTRY_TYPE_DRIVE:
	case SELECTOR_ENTRY_TYPE_DIRECTORY:
		{
			// drive/directory - first check the path
			util::zippath_directory::ptr dir;
			std::error_condition const err = util::zippath_directory::open(entry.fullpath, dir);
			if (err)
			{
				// this path is problematic; present the user with an error and bail
				ui().popup_time(1, _("Error accessing %s"), entry.fullpath);
				break;
			}
		}
		m_current_directory = entry.fullpath;
		m_path_layout.reset();
		m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
		reset(reset_options::SELECT_FIRST);
		break;

	case SELECTOR_ENTRY_TYPE_FILE:
		// file
		m_current_file.assign(entry.fullpath);
		m_handler(result::FILE, m_current_directory, m_current_file);
		break;
	}
}


//-------------------------------------------------
//  update_search
//-------------------------------------------------

void menu_browse::update_search()
{
	ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename);

	file_selector_entry const *const cur_selected(reinterpret_cast<file_selector_entry const *>(get_selection_ref()));

	// if it's a perfect match for the current selection, don't move it
	if (!cur_selected || core_strnicmp(cur_selected->basename.c_str(), m_filename.c_str(), m_filename.size()))
	{
		std::string::size_type bestmatch(0);
		file_selector_entry const *selected_entry(cur_selected);
		for (auto &entry : m_entrylist)
		{
			// TODO: more efficient "common prefix" code
			std::string::size_type match(0);
			for (std::string::size_type i = 1; m_filename.size() >= i; ++i)
			{
				if (!core_strnicmp(entry.basename.c_str(), m_filename.c_str(), i))
					match = i;
				else
					break;
			}

			if (match > bestmatch)
			{
				bestmatch = match;
				selected_entry = &entry;
			}
		}

		if (selected_entry && (selected_entry != cur_selected))
		{
			set_selection((void *)selected_entry);
			centre_selection();
		}
	}
}


//-------------------------------------------------
//  get_directory_range
//-------------------------------------------------

std::pair<size_t, size_t> menu_browse::get_directory_range(float x, float y)
{
	size_t start = 0, span = 0;
	if (m_path_layout->hit_test(x - m_path_position.first, y - m_path_position.second, start, span))
	{
		if (std::string_view(m_current_directory).substr(start, span) != PATH_SEPARATOR)
		{
			auto target_start = m_current_directory.rfind(PATH_SEPARATOR, start);
			if (std::string::npos == target_start)
				target_start = 0;
			else
				target_start += 1;

			auto target_end = m_current_directory.find(PATH_SEPARATOR, start + span);
			if (std::string::npos == target_end)
				target_end = m_current_directory.length();

			return std::make_pair(target_start, target_end);
		}
	}

	return std::make_pair(std::string::npos, std::string::npos);
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_browse::populate()
{
	const file_selector_entry *selected_entry = nullptr;

	// clear out the menu entries
	m_entrylist.clear();

	// open the directory
	util::zippath_directory::ptr directory;
	std::error_condition const err = util::zippath_directory::open(m_current_directory, directory);

	// add the "[empty slot]" entry if available
	if (m_has_empty)
		append_entry(SELECTOR_ENTRY_TYPE_EMPTY, "", "");

	// add the "[midi port]" entry if available
	if (m_is_midi)
		append_entry(SELECTOR_ENTRY_TYPE_MIDI, "", "");

	// add the "[create]" entry
	if (m_has_create && directory && !directory->is_archive())
		append_entry(SELECTOR_ENTRY_TYPE_CREATE, "", "");

	// add and select the "[software list]" entry if available
	if (m_has_softlist)
		selected_entry = &append_entry(SELECTOR_ENTRY_TYPE_SOFTWARE_LIST, "", "");

	// add the drives
	for (std::string const &volume_name : osd_get_volume_names())
		append_entry(SELECTOR_ENTRY_TYPE_DRIVE, volume_name, volume_name);

	// mark first filename entry
	std::size_t const first = m_entrylist.size() + 1;

	// build the menu for each item
	if (err)
	{
		osd_printf_verbose(
				"menu_browse::populate: error opening directory '%s' (%s:%d %s)\n",
				m_current_directory, err.category().name(), err.value(), err.message());
	}
	else
	{
		for (osd::directory::entry const *dirent = directory->readdir(); dirent; dirent = directory->readdir())
		{
			// append a dirent entry
			file_selector_entry const *entry = append_dirent_entry(dirent);
			if (entry)
			{
				// set the selected item to be the first non-parent directory or file
				if (!selected_entry && strcmp(dirent->name, ".."))
					selected_entry = entry;

				// do we have to select this file?
				if (!core_stricmp(m_current_file, dirent->name))
					selected_entry = entry;
			}
		}
	}
	directory.reset();

	if (m_entrylist.size() > first)
	{
		// sort the menu entries
		std::locale const lcl;
		std::collate<wchar_t> const &coll = std::use_facet<std::collate<wchar_t> >(lcl);
		std::sort(
				m_entrylist.begin() + first,
				m_entrylist.end(),
				[&coll] (file_selector_entry const &x, file_selector_entry const &y)
				{
					std::wstring const xstr = wstring_from_utf8(x.basename);
					std::wstring const ystr = wstring_from_utf8(y.basename);
					return coll.compare(xstr.data(), xstr.data() + xstr.size(), ystr.data(), ystr.data() + ystr.size()) < 0;
				});
	}

	// append all of the menu entries
	for (file_selector_entry const &entry : m_entrylist)
		append_entry_menu_item(&entry);
	item_append(menu_item_type::SEPARATOR);

	// set the selection (if we have one)
	if (selected_entry)
		set_selection((void *)selected_entry);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_browse::handle(event const *ev)
{
	if (!ev)
		return false;

	if (ev->iptkey == IPT_SPECIAL)
	{
		// if it's any other key and we're not maxed out, update
		if (input_character(m_filename, ev->unichar, uchar_is_printable))
		{
			update_search();
			return true;
		}
	}
	else if (ev->iptkey == IPT_UI_PASTE)
	{
		if (paste_text(m_filename, uchar_is_printable))
		{
			update_search();
			return true;
		}
	}
	else if (ev->iptkey == IPT_UI_CANCEL)
	{
		// reset the char buffer also in this case
		if (!m_filename.empty())
		{
			m_filename.clear();
			ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename);
			return true;
		}
	}
	else if (ev->iptkey == IPT_CUSTOM)
	{
		// clicked a path component
		if (m_clicked_directory.second > m_clicked_directory.first)
		{
			m_current_directory.resize(m_clicked_directory.second + strlen(PATH_SEPARATOR));
			m_path_layout.reset();
			m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
			reset(reset_options::SELECT_FIRST);
			return true;
		}
	}
	else if (ev->itemref && (ev->iptkey == IPT_UI_SELECT))
	{
		// reset search when selecting an item
		m_filename.clear();

		select_item(*reinterpret_cast<file_selector_entry const *>(ev->itemref));
		return true;
	}

	return false;
}


// INP recording class

ui_menu_record_inp::ui_menu_record_inp(mame_ui_manager &mui, render_target &target, const game_driver *driver) : menu(mui, target)
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
	float height = mame_machine_manager::instance()->ui().get_line_height(target());
	std::string str;

	// filename entry
	str = "Filename: ";
	str += std::string(m_filename_entry);
	str += "_";

	mui.draw_outlined_box(container(), 0.1f,origy1 - (height*2),0.9f,origy1, mui.colors().background_color());
	mui.draw_text_full(target(),_("Please enter a filename for the INP..."),0.1f,origy1 - (height*2),0.8f, ui::text_layout::text_justify::CENTER, ui::text_layout::word_wrapping::TRUNCATE, mame_ui_manager::NORMAL, mui.colors().text_color(), mui.colors().text_bg_color(), nullptr, nullptr);
	mui.draw_text_full(target(),str.c_str(),0.1f,origy1 - height,0.8f, ui::text_layout::text_justify::CENTER, ui::text_layout::word_wrapping::TRUNCATE, mame_ui_manager::NORMAL, mui.colors().text_color(), mui.colors().text_bg_color(), nullptr, nullptr);

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
				mui.draw_text_full(target(),m_warning_text[x].c_str(),0.1f,1.0f - (height*line),0.8f, ui::text_layout::text_justify::LEFT, ui::text_layout::word_wrapping::WORD, mame_ui_manager::NORMAL, mui.colors().text_color(), mui.colors().text_bg_color(), nullptr, nullptr);
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
ui_menu_playback_inp::ui_menu_playback_inp(mame_ui_manager &mui, render_target &target, const game_driver *driver)
	: ui_menu_record_inp(mui, target, driver),
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
					menu::stack_push<menu_browse>(ui(), target(), inp_dir, inp_file, false, false, false,
						[this](menu_browse::result result, std::string directory, std::string inp_file)
						{
							if(result == menu_browse::result::FILE)
							{
								int pos = inp_file.find_last_of("/\\");
								strcpy(m_filename_entry,inp_file.substr(pos+1).c_str());
							}
							browse_done = true;
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
