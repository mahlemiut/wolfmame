// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

    ui/recinp.h.h

    UI main options menu manager.

***************************************************************************/

#pragma once

#ifndef __UI_RECINP_H__
#define __UI_RECINP_H__

class ui_menu_record_inp : public ui_menu
{
public:
	ui_menu_record_inp(running_machine &machine, render_container *container, const game_driver *driver = nullptr);
	virtual ~ui_menu_record_inp();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	const game_driver *m_driver;
	char m_filename_entry[100];
};

#endif /* __UI_RECINP_H__ */
