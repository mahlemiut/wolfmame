// license:BSD-3-Clause
// copyright-holders:R. Belmont, Vas Crabb
#ifndef MAME_BUS_NUBUS_NUBUS_SPEC8_H
#define MAME_BUS_NUBUS_NUBUS_SPEC8_H

#pragma once

#include "nubus.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nubus_spec8s3_device :
		public device_t,
		public device_nubus_card_interface,
		public device_video_interface,
		public device_palette_interface
{
public:
	// construction/destruction
	nubus_spec8s3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nubus_spec8s3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	// palette implementation
	virtual uint32_t palette_entries() const override;

	TIMER_CALLBACK_MEMBER(vbl_tick);

private:
	uint32_t spec8s3_r(offs_t offset, uint32_t mem_mask = ~0);
	void spec8s3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t vram_r(offs_t offset, uint32_t mem_mask = ~0);
	void vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void update_crtc();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	static void crtc_w(int16_t &param, uint32_t offset, uint32_t data);

	required_ioport m_userosc;
	emu_timer *m_timer;

	std::vector<uint32_t> m_vram;
	uint32_t m_mode, m_vbl_disable;
	uint32_t m_colors[3], m_count, m_clutoffs;

	uint8_t m_osc;
	int16_t m_hsync, m_hstart, m_hend, m_htotal;
	int16_t m_vsync, m_vstart, m_vend, m_vtotal;
	bool m_interlace;

	uint16_t m_hpan, m_vpan;
	uint8_t m_zoom;

	uint8_t m_param_bit, m_param_sel, m_param_val;

	bool m_vbl_pending;
};


// device type definition
DECLARE_DEVICE_TYPE(NUBUS_SPEC8S3, nubus_spec8s3_device)

#endif // MAME_BUS_NUBUS_NUBUS_SPEC8_H
