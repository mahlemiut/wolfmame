// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

    ui/recinp.h

    UI main options menu manager.

***************************************************************************/

#pragma once

#ifndef __UI_RECINP_H__
#define __UI_RECINP_H__

#define TOTAL_WARNINGS 3

class ui_menu_record_inp : public ui_menu
{
public:
	ui_menu_record_inp(running_machine &machine, render_container *container, const game_driver *driver = nullptr);
	virtual ~ui_menu_record_inp();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
protected:
	char m_filename_entry[100];
	const game_driver *m_driver;
private:
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
	ui_menu_playback_inp(running_machine &machine, render_container *container, const game_driver *driver = nullptr);
	virtual ~ui_menu_playback_inp();
	virtual void populate() override;
	virtual void handle() override;
private:
	virtual void start_inp() override;
};


#endif /* __UI_RECINP_H__ */
