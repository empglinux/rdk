/*
 * Copyright (c) 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 */

#include "OnScrDsp.h"

gint OnScrDsp::WDT_SCR = 1024;
gint OnScrDsp::HGH_SCR = 600;

const gint OnScrDsp::HGH_BTT = 45;
const gint OnScrDsp::WDT_RGH = 12;

const gint OnScrDsp::WDT_VLM = 250;
const gint OnScrDsp::HGH_VLM = 60;
const gint OnScrDsp::WDT_DSP = 102;
const gint OnScrDsp::HGH_DSP = 49;
const gint OnScrDsp::WDT_LCK = 49;
const gint OnScrDsp::HGH_LCK = 49;
const gint OnScrDsp::WDT_BRG_LVL = 27;
const gint OnScrDsp::HGH_BRG_LVL = 38;
const gint OnScrDsp::WDT_VLM_LVL = 14;
const gint OnScrDsp::HGH_VLM_LVL = 36;

const gint OnScrDsp::BRG_LVL_X = 46;
const gint OnScrDsp::BRG_LVL_Y = 12;
const gint OnScrDsp::VLM_LVL_X = 40;
const gint OnScrDsp::VLM_LVL_Y = 12;

const gint OnScrDsp::TIMEOUT = 5000;

const gint OnScrDsp::BRG_MAX_LVL = 7;
const gint OnScrDsp::VLM_MAX_LVL = 14;

gint OnScrDsp::VLM_X = (WDT_SCR - WDT_VLM) / 2;
gint OnScrDsp::VLM_Y = HGH_SCR - HGH_BTT - HGH_VLM;
gint OnScrDsp::CPS_X = WDT_SCR - WDT_LCK - WDT_RGH;
gint OnScrDsp::CPS_Y = HGH_SCR - HGH_BTT - HGH_LCK;
gint OnScrDsp::DSP_X = (WDT_SCR - WDT_DSP) / 2;
gint OnScrDsp::DSP_Y = HGH_SCR - HGH_BTT - HGH_DSP;
gint OnScrDsp::WRL_X = OnScrDsp::CPS_X;
gint OnScrDsp::WRL_Y = OnScrDsp::CPS_Y;

/**
 * helper to handler timeout
 */
gboolean TmrHlp(gpointer)
{
	OnScrDsp::GetIns().HideWnd();
	return FALSE;
}

/**
 * expose event helper for brightness.
 */
gboolean BrgExp(GtkWidget *, GdkEventExpose *, gpointer)
{
	OnScrDsp::GetIns().DrawBrg();
	return TRUE;
}

/**
 * expose event helper for display.
 */
gboolean DspExp(GtkWidget *, GdkEventExpose *, gpointer)
{
	OnScrDsp::GetIns().DrawDsp();
	return TRUE;
}

/**
 * expose event helper for wireless.
 */
gboolean WrlExp(GtkWidget *, GdkEventExpose *, gpointer)
{
	OnScrDsp::GetIns().DrawWrl();
	return TRUE;
}

/**
 * Constructor of OnScrDsp.
 */
OnScrDsp::OnScrDsp()
	: m_pBrgBckGrn(NULL), m_pBrgLvl(NULL), m_pBrgLvlBln(NULL),
	m_pDspDual(NULL), m_pDspVga(NULL), m_pDspLcd(NULL),
	m_pWrlOff(NULL), m_pWrlOn(NULL),
	m_pCrrWnd(NULL), m_CrrTmr(0), m_CrrLvl(0)
{
	int i = 0;

	m_pBrgBckGrn = gdk_pixbuf_new_from_file(
		"/usr/share/fnkey/images/Brightness_BG.png", NULL);
	m_pBrgLvl = gdk_pixbuf_new_from_file(
		"/usr/share/fnkey/images/Brightness_Level.png", NULL);
	m_pBrgLvlBln = gdk_pixbuf_new_from_file(
		"/usr/share/fnkey/images/Brightness_Level_Blank.png", NULL);
	m_pDspDual = gdk_pixbuf_new_from_file(
		"/usr/share/fnkey/images/Display_Dual.png", NULL);
	m_pDspVga = gdk_pixbuf_new_from_file(
		"/usr/share/fnkey/images/Display_External.png", NULL);
	m_pDspLcd = gdk_pixbuf_new_from_file(
		"/usr/share/fnkey/images/Display_LCD.png", NULL);
	m_pWrlOff = gdk_pixbuf_new_from_file(
		"/usr/share/fnkey/images/Wireless_Off.png", NULL);
	m_pWrlOn = gdk_pixbuf_new_from_file(
		"/usr/share/fnkey/images/Wireless_On.png", NULL);

	assert(m_pBrgBckGrn && m_pBrgLvl && m_pBrgLvlBln &&
		m_pDspDual && m_pDspVga&& m_pDspLcd &&
		m_pWrlOff && m_pWrlOn);

	for (i = 0; i <= WND_WRL; i++) {
		m_pWnd[i] = gtk_window_new(GTK_WINDOW_POPUP);
#if 0
		assert(m_pWnd[i]);
#endif
	}

	SetWnd(&m_pWnd[WND_BRG], GCallback(BrgExp), WDT_VLM, HGH_VLM, VLM_X, VLM_Y);
	SetWnd(&m_pWnd[WND_DSP], GCallback(DspExp), WDT_DSP, HGH_DSP, DSP_X, DSP_Y);
	SetWnd(&m_pWnd[WND_WRL], GCallback(WrlExp), WDT_LCK, HGH_LCK, WRL_X, WRL_Y);
}


/**
 * Destructor of OnScrDsp.
 */
OnScrDsp::~OnScrDsp()
{

}

/**
 * Set style of a window.
 *
 * @param pWnd	GtkWindow to set style
 */
void OnScrDsp::SetWnd(GtkWidget **pWnd, GCallback gcb, gint wdt, gint hgh,
	  gint x, gint y)
{
	gtk_widget_set_app_paintable(*pWnd, TRUE);
	gtk_widget_set_double_buffered(*pWnd, FALSE);
	gtk_widget_set_size_request(*pWnd, wdt, hgh);
	gtk_window_move((GtkWindow *)(*pWnd), x, y);
	gtk_widget_realize(*pWnd);
	gdk_window_set_back_pixmap((*pWnd)->window, NULL, FALSE);


	g_signal_connect(G_OBJECT(*pWnd), "destroy",
		G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(*pWnd), "expose_event", gcb, NULL);

	gtk_widget_add_events(*pWnd, GDK_EXPOSURE_MASK);
}

/**
 * Show brightness window.
 *
 * @param id	window ID
 * @param lvl	brightness level
 */
void OnScrDsp::ShowWnd(WND_ID id, int lvl)
{
	GdkScreen *scr;

	g_usleep(10);
	gdk_threads_enter();

	if (m_CrrTmr) {
		g_source_remove(m_CrrTmr);
		m_CrrTmr = 0;
	}

	/* to avoid flicker, reduce hide and show */
	if (m_pCrrWnd == m_pWnd[id]) {
		switch (id) {
		case WND_BRG:
			m_CrrLvl = lvl;
			DrawBrgLvl();
			goto AFTER_SHOW;
		case WND_DSP:
		case WND_WRL:
		default:
			break;
			/* if not match, continue hide and show */
		}
	} 
	if (m_pCrrWnd)
		gtk_widget_hide(m_pCrrWnd);

	/* get current screen resolution to recalc icon's position */
	scr = gdk_screen_get_default();
	HGH_SCR = gdk_screen_get_height(scr);
	WDT_SCR = gdk_screen_get_width(scr);

	switch (id) {
	case WND_BRG:
		VLM_X = (WDT_SCR - WDT_VLM) / 2;
		VLM_Y = HGH_SCR - HGH_BTT - HGH_VLM;
		gtk_window_move((GtkWindow *)m_pWnd[id], VLM_X, VLM_Y);
		break;
	case WND_DSP:
		DSP_X = (WDT_SCR - WDT_DSP) / 2;
		DSP_Y = HGH_SCR - HGH_BTT - HGH_DSP;
		gtk_window_move((GtkWindow *)m_pWnd[id], DSP_X, DSP_Y);
		break;
	case WND_WRL:
		WRL_X = WDT_SCR - WDT_LCK - WDT_RGH;
		WRL_Y = HGH_SCR - HGH_BTT - HGH_LCK;
		gtk_window_move((GtkWindow *)m_pWnd[id], WRL_X, WRL_Y);
		break;
	}

        m_pCrrWnd = m_pWnd[id];
        m_CrrLvl = lvl;
	gtk_widget_show(m_pCrrWnd);
	gtk_widget_hide(m_pCrrWnd);
	gtk_widget_show(m_pCrrWnd);

AFTER_SHOW:
	m_CrrTmr = g_timeout_add(TIMEOUT, &TmrHlp, NULL);
	gdk_threads_leave();
	g_usleep(10);
}

/**
 * Hide current window
 */
void OnScrDsp::HideWnd()
{
	g_usleep(10);
	gdk_threads_enter();
	gtk_widget_hide(m_pCrrWnd);
	m_CrrLvl = 0;
	m_pCrrWnd = NULL;
	gdk_threads_leave();
	g_usleep(10);
}

/**
 * draw brightness window
 */
void OnScrDsp::DrawBrg()
{
	gdk_draw_pixbuf(m_pWnd[WND_BRG]->window, NULL, m_pBrgBckGrn,
		0, 0, 0, 0, WDT_VLM, HGH_VLM, GDK_RGB_DITHER_NORMAL, 0, 0);
	DrawBrgLvl();
}

/**
 * draw brighness level
 */
void OnScrDsp::DrawBrgLvl()
{
	int i;

	/* check if the brightness value is correct */
	if (m_CrrLvl > BRG_MAX_LVL) {
		m_CrrLvl = BRG_MAX_LVL;
	} else if (m_CrrLvl < 0) {
		m_CrrLvl = 0;
	}

	/* draw level */
	for (i = 0; i < m_CrrLvl; i++) {
		gdk_draw_pixbuf(m_pWnd[WND_BRG]->window, NULL, m_pBrgLvl,
			0, 0, BRG_LVL_X + WDT_BRG_LVL * i, BRG_LVL_Y, WDT_BRG_LVL,
			HGH_BRG_LVL, GDK_RGB_DITHER_NORMAL, 0, 0);
	}

	/* draw blank level */
	for (i = m_CrrLvl; i < BRG_MAX_LVL; i++) {
		gdk_draw_pixbuf(m_pWnd[WND_BRG]->window, NULL, m_pBrgLvlBln,
			0, 0, BRG_LVL_X + WDT_BRG_LVL * i, BRG_LVL_Y, WDT_BRG_LVL,
			HGH_BRG_LVL, GDK_RGB_DITHER_NORMAL, 0, 0);
	}
}

/**
 * draw display window
 */
void OnScrDsp::DrawDsp()
{
	if (DSP_LCD == m_CrrLvl) {
		gdk_draw_pixbuf(m_pWnd[WND_DSP]->window, NULL, m_pDspLcd,
			0, 0, 0, 0, WDT_DSP, HGH_DSP, GDK_RGB_DITHER_NORMAL, 0, 0);
	} else if (DSP_VGA == m_CrrLvl) {
		gdk_draw_pixbuf(m_pWnd[WND_DSP]->window, NULL, m_pDspVga,
			0, 0, 0, 0, WDT_DSP, HGH_DSP, GDK_RGB_DITHER_NORMAL, 0, 0);
	} else if (DSP_DUAL == m_CrrLvl) {
		gdk_draw_pixbuf(m_pWnd[WND_DSP]->window, NULL, m_pDspDual,
			0, 0, 0, 0, WDT_DSP, HGH_DSP, GDK_RGB_DITHER_NORMAL, 0, 0);
	} else if (DSP_NONE == m_CrrLvl) {
		return;
	}
}

/**
 * draw wireless window
 */
void OnScrDsp::DrawWrl()
{
	if (m_CrrLvl) {
		gdk_draw_pixbuf(m_pWnd[WND_WRL]->window, NULL, m_pWrlOn,
			0, 0, 0, 0, WDT_LCK, HGH_LCK, GDK_RGB_DITHER_NORMAL, 0, 0);
	} else {
		gdk_draw_pixbuf(m_pWnd[WND_WRL]->window, NULL, m_pWrlOff,
			0, 0, 0, 0, WDT_LCK, HGH_LCK, GDK_RGB_DITHER_NORMAL, 0, 0);
	}
}
