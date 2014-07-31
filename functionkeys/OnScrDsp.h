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

#ifndef ON_SCR_DSP_H
#define ON_SCR_DSP_H

#include <assert.h>
#include <stdio.h>
#include <gtk/gtk.h>


/**
 * @class OnScrDsp
 *
 * Display status of WLAN, volume, display, brightness, Caps Lock, Scroll
 * Lock and Num Lock.
 */
class OnScrDsp
{
public:
	
	typedef enum {
		DSP_NONE,
		DSP_LCD,
		DSP_VGA,
		DSP_DUAL,
	} DSP_STA;

	typedef enum {
		WND_BRG,
		WND_DSP,
		WND_WRL,
	} WND_ID;

	/**
	 * Get the single instance of OnScrDsp.
	 *
	 * @return instance of OnScrDsp
	 */
	static OnScrDsp &GetIns()
	{
		static OnScrDsp osd;
		return osd;
	};

	void ShowWnd(WND_ID id, int lvl);

private:
	OnScrDsp();
	~OnScrDsp();

	/* declare copy constructor and equal operator to prevent
	 * copied instance
	 */
	OnScrDsp(const OnScrDsp &);
	OnScrDsp &operator =(const OnScrDsp &);

	void SetWnd(GtkWidget **pWnd, GCallback gcb, gint wdt, gint hgh, 
		gint x, gint y);
	void HideWnd();
	void DrawBrg();
	void DrawBrgLvl();
	void DrawCps();
	void DrawDsp();
	void DrawMut();
	void DrawNum();
	void DrawPrv();
	void DrawScr();
	void DrawVlm();
	void DrawVlmLvl();
	void DrawWrl();

	GdkPixbuf *m_pBrgBckGrn; /**< image of brightness background */
	GdkPixbuf *m_pBrgLvl; /**< image of brightness level */
	GdkPixbuf *m_pBrgLvlBln; /**< image of blank brightness level */
	GdkPixbuf *m_pDspDual; /**< image of dual display */
	GdkPixbuf *m_pDspVga; /**< image of external display */
	GdkPixbuf *m_pDspLcd; /**< image of internal LCD display */
	GdkPixbuf *m_pWrlOff; /**< image of wireless off */
	GdkPixbuf *m_pWrlOn; /**< image of wireless on */

	GtkWidget *m_pWnd[WND_WRL]; /**< window pointers from WND_BRG to WND_WRL */
	
	GdkScreen *m_pScr; /**< pointer to screen */

	GtkWidget *m_pCrrWnd; /**< current showing window */
	gint m_CrrTmr; /**< current timer id */
	gint m_CrrLvl; /**< current level of brightness or volume */

	static gint WDT_SCR; /**< width of screen */
	static gint HGH_SCR; /**< height of screen */

	static const gint HGH_BTT; /**< heihgt of bottom */
	static const gint WDT_RGH; /**< width of right margin */

	static const gint WDT_VLM; /**< width of brightness and volume images */
	static const gint HGH_VLM; /**< height of brightness and volume images */
	static const gint WDT_DSP; /**< width of display images */
	static const gint HGH_DSP; /**< height of display images */
	static const gint WDT_LCK; /**< width of Caps, Num, Scroll Lock images */ 
	static const gint HGH_LCK; /**< height of Caps, Num, Scroll Lock images */
	static const gint WDT_BRG_LVL; /**< width of brightness level image */
	static const gint HGH_BRG_LVL; /**< height of brightness level image */
	static const gint WDT_VLM_LVL; /**< width of brightness level image */
	static const gint HGH_VLM_LVL; /**< height of brightness level image */

	static const gint BRG_LVL_X; /**< x offset of brightness level */
	static const gint BRG_LVL_Y; /**< y offset of brightnes level */

	static const gint VLM_LVL_X; /**< x offset of volume level */
	static const gint VLM_LVL_Y; /**< y offset of volume level */

	static const gint TIMEOUT; /**< timeout to hide window */

	static const gint BRG_MAX_LVL; /**< max level of brightness */
	static const gint VLM_MAX_LVL; /**< max level of volume */

	static gint VLM_X; /**< x position of brightness and volume images */
	static gint VLM_Y; /**< y position of brightness and volume images */
	static gint CPS_X; /**< x position of Caps Lock images */
	static gint CPS_Y; /**< y position of Caps Lock images */
	static gint DSP_X; /**< x position of display images */
	static gint DSP_Y; /**< y position of display images */
	static gint NUM_X; /**< x position of Num Lock images */
	static gint NUM_Y; /**< y position of Num Lock images */
	static gint PRV_X; /**< x position of No Privilege images */
	static gint PRV_Y; /**< y position of No Privilege images */
	static gint SCR_X; /**< x position of Scroll Lock images */
	static gint SCR_Y; /**< y position of Scroll Lock images */
	static gint WRL_X; /**< x position of wireless images */
	static gint WRL_Y; /**< y position of wireless images */

	friend gboolean TmrHlp(gpointer);

	friend gboolean BrgExp(GtkWidget *, GdkEventExpose *, gpointer);
	friend gboolean DspExp(GtkWidget *, GdkEventExpose *, gpointer);
	friend gboolean WrlExp(GtkWidget *, GdkEventExpose *, gpointer);
};
#endif /* ON_SCR_DSP_H */
