/* include/gamecmds.h - Proto gamecmds.c
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

int create_game(aClient *cl, int parc, char **parv);
int game_list(aClient *cl, int parc, char **parv);
int distribuer_cartes(aGame *);
char *GetIdent(struct playerinfo *);
int join_game(aClient *cl, int parc, char **parv);
int send_cartes(struct playerinfo *pl);
int distribuer(aClient *cl, int parc, char **parv);
int part_game(aClient *cl, int parc, char **parv);
int leave_game(aClient *cl, aGame *g, int silence);
int next_player(aPlayer *pl);
int user_cant(aClient *cl, int parc, char **parv);
int donner_cartes(aClient *cl, int parc, char **parv);
int user_play(aClient *cl, int parc, char **parv);
int end_of_game(aGame *g);
int kick_user(aClient *cl, int parc, char **parv);
int stop_game(aClient *cl, int parc, char **parv);
