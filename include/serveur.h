/* include/serveur.h - Proto serveur.c
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

int parse_this(int);
int init_socket(void);
int run_server(void);
extern fd_set global_fd_set;
extern struct clientinfo myUser[];
extern int highsock;
struct clientinfo *addclient(int, const char *);
void delclient(struct clientinfo *);
int exit_client(aClient *, const char *);
int parsemsg(aClient *);
int sendrpl(aClient *, const char *, ...);
int findcmd(const char *);
int sendto_game(aClient *cl, aGame *g, const char *pattern, ...);
void sig_alarm(int c);
int change_nick(aClient *, int, char **);
char *correct_nick(char *);
int m_pong(aClient *cl, int parc, char **parv);
int m_ping(aClient *cl, int parc, char **parv);
int sendto_players(aClient *cl, aGame *g, const char *pattern, ...);
int web_connec(aClient *cl, int parc, char **parv);
