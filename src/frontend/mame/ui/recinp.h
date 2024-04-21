// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

    ui/recinp.h

    UI main options menu manager.

***************************************************************************/

#pragma once

#ifndef __UI_RECINP_H__
#define __UI_RECINP_H__

#include "ui/filesel.h"
#include "ui/selgame.h"
#include "audit.h"
#include "util/path.h"

#define TOTAL_WARNINGS 3

// ======================> inp_header

// header at the front of INP files
class inp_header
{
public:
	// parameters
	static constexpr unsigned MAJVERSION = 3;
	static constexpr unsigned MINVERSION = 0;

	bool read(emu_file &f)
	{
		return f.read(m_data, sizeof(m_data)) == sizeof(m_data);
	}
	bool write(emu_file &f) const
	{
		return f.write(m_data, sizeof(m_data)) == sizeof(m_data);
	}

	bool check_magic() const
	{
		return 0 == std::memcmp(MAGIC, m_data + OFFS_MAGIC, OFFS_BASETIME - OFFS_MAGIC);
	}
	u64 get_basetime() const
	{
		return
				(u64(m_data[OFFS_BASETIME + 0]) << (0 * 8)) |
				(u64(m_data[OFFS_BASETIME + 1]) << (1 * 8)) |
				(u64(m_data[OFFS_BASETIME + 2]) << (2 * 8)) |
				(u64(m_data[OFFS_BASETIME + 3]) << (3 * 8)) |
				(u64(m_data[OFFS_BASETIME + 4]) << (4 * 8)) |
				(u64(m_data[OFFS_BASETIME + 5]) << (5 * 8)) |
				(u64(m_data[OFFS_BASETIME + 6]) << (6 * 8)) |
				(u64(m_data[OFFS_BASETIME + 7]) << (7 * 8));
	}
	unsigned get_majversion() const
	{
		return m_data[OFFS_MAJVERSION];
	}
	unsigned get_minversion() const
	{
		return m_data[OFFS_MINVERSION];
	}
	std::string get_sysname() const
	{
		return get_string<OFFS_SYSNAME, OFFS_APPDESC>();
	}
	std::string get_appdesc() const
	{
		return get_string<OFFS_APPDESC, OFFS_END>();
	}

	void set_magic()
	{
		std::memcpy(m_data + OFFS_MAGIC, MAGIC, OFFS_BASETIME - OFFS_MAGIC);
	}
	void set_basetime(u64 time)
	{
		m_data[OFFS_BASETIME + 0] = u8((time >> (0 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 1] = u8((time >> (1 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 2] = u8((time >> (2 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 3] = u8((time >> (3 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 4] = u8((time >> (4 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 5] = u8((time >> (5 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 6] = u8((time >> (6 * 8)) & 0x00ff);
		m_data[OFFS_BASETIME + 7] = u8((time >> (7 * 8)) & 0x00ff);
	}
	void set_version()
	{
		m_data[OFFS_MAJVERSION] = MAJVERSION;
		m_data[OFFS_MINVERSION] = MINVERSION;
	}
	void set_sysname(std::string const &name)
	{
		set_string<OFFS_SYSNAME, OFFS_APPDESC>(name);
	}
	void set_appdesc(std::string const &desc)
	{
		set_string<OFFS_APPDESC, OFFS_END>(desc);
	}

private:
	template <std::size_t BEGIN, std::size_t END> void set_string(std::string const &str)
	{
		std::size_t const used = (std::min<std::size_t>)(str.size() + 1, END - BEGIN);
		std::memcpy(m_data + BEGIN, str.c_str(), used);
		if ((END - BEGIN) > used)
			std::memset(m_data + BEGIN + used, 0, (END - BEGIN) - used);
	}
	template <std::size_t BEGIN, std::size_t END> std::string get_string() const
	{
		char const *const begin = reinterpret_cast<char const *>(m_data + BEGIN);
		return std::string(begin, std::find(begin, reinterpret_cast<char const *>(m_data + END), '\0'));
	}

	static constexpr std::size_t    OFFS_MAGIC       = 0x00;    // 0x08 bytes
	static constexpr std::size_t    OFFS_BASETIME    = 0x08;    // 0x08 bytes (little-endian binary integer)
	static constexpr std::size_t    OFFS_MAJVERSION  = 0x10;    // 0x01 bytes (binary integer)
	static constexpr std::size_t    OFFS_MINVERSION  = 0x11;    // 0x01 bytes (binary integer)
																// 0x02 bytes reserved
	static constexpr std::size_t    OFFS_SYSNAME     = 0x14;    // 0x0c bytes (ASCII)
	static constexpr std::size_t    OFFS_APPDESC     = 0x20;    // 0x20 bytes (ASCII)
	static constexpr std::size_t    OFFS_END         = 0x40;

	static u8 const                 MAGIC[OFFS_BASETIME - OFFS_MAGIC];

	u8                              m_data[OFFS_END];
};


namespace ui {

class ui_menu_record_inp : public menu
{
public:
	ui_menu_record_inp(mame_ui_manager &mui, render_container &container, const game_driver *driver = nullptr);
	virtual ~ui_menu_record_inp();
	virtual void populate() override;
	virtual bool handle(event const *ev) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	static constexpr bool audit_passed(media_auditor::summary summary)
	{
		return (media_auditor::CORRECT == summary) || (media_auditor::BEST_AVAILABLE == summary) || (media_auditor::NONE_NEEDED == summary);
	}
protected:
	char m_filename_entry[100];
	const game_driver *m_driver;
	void launch_system(mame_ui_manager &mui, game_driver const &driver, bool rec, ui_software_info const *swinfo, std::string const *part, int const *bios);
//	virtual void inkey_export() override;
private:
//	virtual float draw_left_panel(float x1, float y1, float x2, float y2) override;
//	virtual void get_selection(ui_software_info const *&software, ui_system_info const *&system) const override;
//	virtual render_texture *get_icon_texture(int linenum, void *selectedref) override;
//	virtual void make_topbox_text(std::string &line0,std::string &line1,std::string &line2) const override;
//	virtual std::string make_software_description(ui_software_info const &software) const override;
//	virtual void filter_selected() override;
	//using s_bios = std::vector<std::pair<std::string, int>>;
	virtual void start_inp();
	int m_warning_count;
	bool m_warning[TOTAL_WARNINGS];
	std::string m_warning_text[TOTAL_WARNINGS] =
	{
		_("NVRAM folder for this system exists, and nvram_directory is not set to NUL or /dev/null.  NVRAM may be present, making playback for others difficult.  MARP requires INPs to be recorded with no NVRAM present."),
		_("CHD DIFF file for this system exists.  Playback may not be possible."),
		_("Lua console is active.  This can affect playbackability if used, and can also be used for cheating.")
	};
};

class ui_menu_playback_inp : public ui_menu_record_inp
{
public:
	ui_menu_playback_inp(mame_ui_manager &mui, render_container &container, const game_driver *driver = nullptr);
	virtual ~ui_menu_playback_inp();
	virtual void populate() override;
	virtual bool handle(event const *ev) override;
private:
	virtual void start_inp() override;
	std::string inp_file;
	std::string inp_dir;
	bool browse_done;
};

} // namespace ui

#endif /* __UI_RECINP_H__ */
