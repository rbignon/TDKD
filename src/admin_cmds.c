/* src/admin_cmds.c - Commandes admins
 * Copyright (C) 2002-2005 Inter System
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

#include "main.h"
#include "serveur.h"
#include "config.h"

int user_list(aClient *cl, int parc, char **parv)
{
	int i;

	for(i=0;i <= highsock; i++)
		if(!(myUser[i].flag & TDKD_FREE))
			sendrpl(cl, MSG_USERLIST, myUser[i].nick,
				myUser[i].user ? (IsAdmin(myUser[i].user) ? 2 : 1) : 0,
				myUser[i].player ? myUser[i].player->grade : -1,
				myUser[i].player ? myUser[i].player->game->name : "");

	sendrpl(cl, MSG_ENDOFUL);

	return 0;
}

/* KILL <user> :<message> */
int kill_user(aClient *cl, int parc, char **parv)
{
	char s[200 + 1];
	int i;
	aClient *k = NULL;

	for(i=0;i <= highsock; i++)
		if(!(myUser[i].flag & TDKD_FREE) && !strcasecmp(myUser[i].nick, parv[1]))
		{
			k = &myUser[i];
			break;
		}

	if(!k)
		return sendrpl(cl, ERR_012, parv[1]);

	snprintf(s, 200, MSG_KILL, cl->nick, parv[2]);
	exit_client(k, s);

	return 0;
}

/* DIE :<message> */
int die_server(aClient *cl, int parc, char **parv)
{
	int i;
	char s[200 + 1];

	if(!IsAdmin(cl->user)) return sendrpl(cl, MSG_ERREUR, parv[0], "Droits insuffisants");

	snprintf(s, 200, MSG_DIE, parv[1]);
	for(i=0; i <= highsock;i++)
		if(!(myUser[i].flag & TDKD_FREE))
			exit_client(&myUser[i], s);

	running = 0;

	return 0;
}

int rehash_server(aClient *cl, int parc, char **parv)
{
	if(!IsAdmin(cl->user)) return sendrpl(cl, MSG_ERREUR, parv[0], "Droits insuffisants");

	if(load_config(conf_file) == -1)
		return sendrpl(cl, MSG_ERREUR, parv[0], "Impossible de lire la conf !");

	sendrpl(cl, MSG_REHASHED);

	return 0;
}

int wall_msg(aClient *cl, int parc, char **parv)
{
	int i;

	for(i=0;i <= highsock; i++)
		if(!(myUser[i].flag & TDKD_FREE))
			sendrpl(&myUser[i], MSG_WALL, cl->nick, parv[1]);

	return 0;
}
