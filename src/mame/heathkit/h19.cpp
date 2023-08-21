// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H19

    A smart terminal designed and manufactured by Heath Company. This
    is identical to the Zenith Data Systems Z-19.

****************************************************************************/

#include "emu.h"

#include "tlb.h"
#include "bus/rs232/rs232.h"

namespace {

class h19_state : public driver_device
{
public:
	h19_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tlb(*this, "tlb")
	{
	}

	// original h19
	void h19(machine_config &config);

	// replacement ROMs
	void h19_superh19(machine_config &config);
	void h19_watzh19(machine_config &config);
	void h19_ultrah19(machine_config &config);

	// add-on graphics boards
	void h19_gp19(machine_config &config);


private:
	required_device<heath_tlb_device> m_tlb;

};

void h19_state::h19(machine_config &config)
{
	HEATH_TLB(config, m_tlb);
	m_tlb->serial_data_callback().set("dte", FUNC(rs232_port_device::write_txd));

	rs232_port_device &dte(RS232_PORT(config, "dte", default_rs232_devices, "loopback"));
	dte.rxd_handler().set(m_tlb, FUNC(heath_tlb_device::serial_in_w));
}

void h19_state::h19_superh19(machine_config &config)
{
	HEATH_SUPER19(config, m_tlb);
	m_tlb->serial_data_callback().set("dte", FUNC(rs232_port_device::write_txd));

	rs232_port_device &dte(RS232_PORT(config, "dte", default_rs232_devices, "loopback"));
	dte.rxd_handler().set(m_tlb, FUNC(heath_tlb_device::serial_in_w));
}

void h19_state::h19_watzh19(machine_config &config)
{
	HEATH_WATZ(config, m_tlb);
	m_tlb->serial_data_callback().set("dte", FUNC(rs232_port_device::write_txd));

	rs232_port_device &dte(RS232_PORT(config, "dte", default_rs232_devices, "loopback"));
	dte.rxd_handler().set(m_tlb, FUNC(heath_tlb_device::serial_in_w));
}

void h19_state::h19_ultrah19(machine_config &config)
{
	HEATH_ULTRA(config, m_tlb);
	m_tlb->serial_data_callback().set("dte", FUNC(rs232_port_device::write_txd));

	rs232_port_device &dte(RS232_PORT(config, "dte", default_rs232_devices, "loopback"));
	dte.rxd_handler().set(m_tlb, FUNC(heath_tlb_device::serial_in_w));
}

void h19_state::h19_gp19(machine_config &config)
{
	HEATH_GP19(config, m_tlb);
	m_tlb->serial_data_callback().set("dte", FUNC(rs232_port_device::write_txd));

	rs232_port_device &dte(RS232_PORT(config, "dte", default_rs232_devices, "loopback"));
	dte.rxd_handler().set(m_tlb, FUNC(heath_tlb_device::serial_in_w));
}

// ROM definition
ROM_START( h19 )
ROM_END

ROM_START( super19 )
ROM_END

ROM_START( watz19 )
ROM_END

ROM_START( ultra19 )
ROM_END

ROM_START( gp19 )
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE       INPUT   CLASS      INIT        COMPANY          FULLNAME                         FLAGS
COMP( 1979, h19,     0,      0,      h19,             0,   h19_state, empty_init, "Heath Company", "Heathkit H-19",                 MACHINE_SUPPORTS_SAVE )
//Super-19 ROM - ATG Systems, Inc - Adv in Sextant Issue 4, Winter 1983. With the magazine lead-time, likely released late 1982.
COMP( 1982, super19, h19,    0,      h19_superh19,    0,   h19_state, empty_init, "Heath Company", "Heathkit H-19 w/ Super-19 ROM", MACHINE_SUPPORTS_SAVE )
// Watzman ROM - HUG p/n 885-1121, announced in REMark Issue 33, Oct. 1982
COMP( 1982, watz19,  h19,    0,      h19_watzh19,     0,   h19_state, empty_init, "Heath Company", "Heathkit H-19 w/ Watzman ROM",  MACHINE_SUPPORTS_SAVE )
// ULTRA ROM - Software Wizardry, Inc., (c) 1983 William G. Parrott, III
COMP( 1983, ultra19, h19,    0,      h19_ultrah19,    0,   h19_state, empty_init, "Heath Company", "Heathkit H-19 w/ ULTRA ROM",    MACHINE_SUPPORTS_SAVE )
// GP-19 - Northwest Digital Systems, (c) 1983
COMP( 1983, gp19,    h19,    0,      h19_gp19,        0,   h19_state, empty_init, "Heath Company", "Heathkit H-19 w/ GP-19",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
