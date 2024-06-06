/* include/divers.h - Proto de divers.c
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

int send_msg(aClient *cl, int parc, char **parv);
int bybye(aClient *cl, int parc, char **parv);
int server_info(aClient *cl, int parc, char **parv);
int user_auth(aClient *cl, int parc, char **parv);
int register_account(aClient *cl, int parc, char **parv);
int show_motd(aClient *cl);
int show_stats(aClient *cl, int parc, char **parv);
