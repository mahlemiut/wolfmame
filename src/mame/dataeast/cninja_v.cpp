// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

   Caveman Ninja Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "emu.h"
#include "cninja.h"

/******************************************************************************/

VIDEO_START_MEMBER(cninja_state,cninja)
{
	m_deco_tilegen[0]->set_transmask(1, 0, 0x00ff, 0xff01);
}

VIDEO_START_MEMBER(cninja_state,stoneage)
{
	/* The bootleg has broken scroll registers */
	m_deco_tilegen[0]->set_scrolldx(3, 0, -10, -10); /* pf4 16x16 tilemap */
	m_deco_tilegen[0]->set_scrolldx(1, 0, -10, -10); /* pf2 16x16 tilemap */
	m_deco_tilegen[0]->set_scrolldx(0, 1, 2, 2); /* pf1 8x8 tilemap */
}

/******************************************************************************/


/* The bootleg sprites are in a different format! */
void cninja_state::cninjabl_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint16_t *buffered_spriteram = m_spriteram[0]->buffer();
	int offs;
	int endoffs;

	// bootleg seems to use 0x180 as an end of list marker
	// find it first, so we can use normal list processing
	endoffs = 0x400 - 4;
	for (offs = 0; offs < 0x400 - 4 ; offs += 4)
	{
		int y = buffered_spriteram[offs + 1];

		if (y == 0x180)
		{
			endoffs = offs;
			offs = 0x400 - 4;
		}
	}

	for (offs = endoffs; offs >=0 ; offs -= 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult, pri = 0;

		sprite = buffered_spriteram[offs + 0]; // changed on bootleg!
		y = buffered_spriteram[offs + 1]; // changed on bootleg!

		if (!sprite)
			continue;

		x = buffered_spriteram[offs + 2];

		/* Sprite/playfield priority */
		switch (x & 0xc000)
		{
		case 0x0000: pri = 0; break;
		case 0x4000: pri = 0xf0; break;
		case 0x8000: pri = 0xf0 | 0xcc; break;
		case 0xc000: pri = 0xf0 | 0xcc; break; /* Perhaps 0xf0|0xcc|0xaa (Sprite under bottom layer) */
		}

		flash = y & 0x1000;
		if (flash && (m_screen->frame_number() & 1))
			continue;

		colour = (x >> 9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;

		multi = (1 << ((y & 0x0600) >> 9)) - 1; /* 1x, 2x, 4x, 8x height */

		y -= multi * 16; // changed on bootleg!
		y += 4;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		//sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen())
		{
			y = 240 - y;
			x = 240 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else
			mult = -16;

		while (multi >= 0)
		{
			m_gfxdecode->gfx(3)->prio_transpen(bitmap,cliprect,
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,
					screen.priority(),pri,0);

			multi--;
		}
	}
}

/******************************************************************************/

uint32_t cninja_state::screen_update_cninja(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen[0]->pf_control_r(0);

	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(BIT(flip, 7));
	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	/* Draw playfields */
	screen.priority().fill(0, cliprect);
	bitmap.fill(512, cliprect);
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 2);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 4);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t cninja_state::screen_update_cninjabl2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* force layers to be enabled */
	m_deco_tilegen[0]->set_enable(0, 1 );
	m_deco_tilegen[0]->set_enable(1, 1 );

	screen_update_cninja(screen, bitmap, cliprect);

	return 0;
}

uint32_t cninja_state::screen_update_cninjabl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen[0]->pf_control_r(0);

	/* force layers to be enabled */
	m_deco_tilegen[1]->set_enable(0, 1 );
	m_deco_tilegen[1]->set_enable(1, 1 );

	flip_screen_set(BIT(flip, 7));
	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	/* Draw playfields */
	screen.priority().fill(0, cliprect);
	bitmap.fill(512, cliprect);
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 2);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 4);
	cninjabl_draw_sprites(screen, bitmap, cliprect);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t cninja_state::screen_update_edrandy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen[0]->pf_control_r(0);

	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(BIT(flip, 7));
	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
	m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t cninja_state::screen_update_robocop2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen[0]->pf_control_r(0);
	uint16_t const priority = m_priority;

	/* Update playfields */
	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(BIT(flip, 7));
	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	/* Draw playfields */
	screen.priority().fill(0, cliprect);
	bitmap.fill(0x200, cliprect);

	if ((priority & 4) == 0)
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);

	/* Switchable priority */
	switch (priority & 0x8)
	{
		case 8:
			m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
			if (priority & 4)
				m_deco_tilegen[1]->tilemap_12_combine_draw(screen, bitmap, cliprect, 0, 4);
			else
				m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
			break;
		default:
		case 0:
			if (priority & 4)
				m_deco_tilegen[1]->tilemap_12_combine_draw(screen, bitmap, cliprect, 0, 2);
			else
				m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);

			m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
			break;
	}

	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

VIDEO_START_MEMBER(cninja_state,mutantf)
{
	m_sprgen[0]->alloc_sprite_bitmap();
	m_sprgen[1]->alloc_sprite_bitmap();
}

uint32_t cninja_state::screen_update_mutantf(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const flip = m_deco_tilegen[0]->pf_control_r(0);
	uint16_t const priority = m_priority;

	// sprites are flipped relative to tilemaps
	flip_screen_set(BIT(flip, 7));
	m_sprgen[0]->set_flip_screen(!BIT(flip, 7));
	m_sprgen[1]->set_flip_screen(!BIT(flip, 7));

	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	/* Draw playfields */
	bitmap.fill(0x400, cliprect); /* Confirmed */

	m_sprgen[1]->draw_sprites(bitmap, cliprect, m_spriteram[1]->buffer(), 0x400);
	m_sprgen[0]->draw_sprites(bitmap, cliprect, m_spriteram[0]->buffer(), 0x400);


	/* There is no priority prom on this board, but there is a
	priority control word, the only values used in game appear
	to be 2, 6 & 7 though:

	Bit 0:  If set sprite chip 2 above sprite chip 1 else vice versa
	Bit 1:  Always set?
	Bit 2:  Almost always set  (Sometimes not set on screen transitions)

	The other bits may control alpha blend on the 2nd sprite chip, or
	layer order.
	*/
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);


	if (priority & 1)
	{
		m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0x100, 0x1ff);
		m_sprgen[1]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 1024+768, 0x0ff, 0x80); // fixed alpha of 0x80 for this layer?
	}
	else
	{
		m_sprgen[1]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 1024+768, 0x0ff, 0x80);  // fixed alpha of 0x80 for this layer?
		m_sprgen[0]->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0x100, 0x1ff);
	}
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
