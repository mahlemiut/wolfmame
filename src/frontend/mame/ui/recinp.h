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

namespace ui {

class ui_menu_record_inp : public menu
{
public:
	ui_menu_record_inp(mame_ui_manager &mui, render_container &container, const game_driver *driver = nullptr);
	virtual ~ui_menu_record_inp();
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
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
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;
private:
	virtual void start_inp() override;
	menu_file_selector::result browse_result;
	std::string inp_file;
	std::string inp_dir;
	bool browse_done;
};

} // namespace ui

#endif /* __UI_RECINP_H__ */
