/* include/ia.h - Proto de ia.c
 * Copyright (C) 2002-2004 Inter System
 *
 * contact: Progs@Inter-System.Net
 *
 * Serveur du jeu de carte Trou du cul.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * $Id: $
 */

struct ia_speak_tab {
	int rest;
	char msg[250];
};
extern struct ia_speak_tab ia_speak_win[];
extern struct ia_speak_tab ia_speak_wose[];
extern struct ia_speak_tab ia_speak_lose[];
extern struct ia_speak_tab ia_speak_detroned[];
extern struct ia_speak_tab ia_speak_parfait[];
extern struct ia_speak_tab ia_speak_trophaut[];
int ia_joue(aPlayer *pl);
int ia_change(aPlayer *pl, int nb, int meilleurs);
int ia_speak_to(aPlayer *pl, char *nick, struct ia_speak_tab *speak);
int ia_speak(aPlayer *pl, struct ia_speak_tab *speak);
