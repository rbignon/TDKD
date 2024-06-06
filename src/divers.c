/* src/divers.c - Commandes diverses
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

#include "main.h"
#include "serveur.h"
#include "infos.h"
#include "gamecmds.h"
#include "divers.h"
#include "outils.h"
#include "database.h"

int show_motd(aClient *cl)
{
	FILE *fp;
	/* MOTD */
	fp = fopen(config.motd_file, "r");
	if(fp)
	{
		char buf[512];

		sendrpl(cl, MSG_MOTD, "Message du jour :");
		sendrpl(cl, MSG_MOTD, "");
		while(fgets(buf, sizeof buf, fp))
		{
			strip_newline(buf);
			sendrpl(cl, MSG_MOTD, buf);
		}
		sendrpl(cl, MSG_ENDOFMOTD);
		fclose(fp);
	}

	return 0;
}

/* REGISTER <email> :<pass> */
int register_account(aClient *cl, int parc, char **parv)
{
	char *pass = parv[2];
	anUser *u;

	if(cl->user)
		return sendrpl(cl, MSG_ERREUR, parv[0], "Vous êtes déjà logué");

	if(tusers >= MAXUSERS)
		return sendrpl(cl, MSG_ERREUR, parv[0], "Trop d'users, prévenez le propriétaire du serveur");

	for(u=userhead;u;u=u->next)
		if(!strcasecmp(u->nick, cl->nick)) break;

	if(u)	return sendrpl(cl, MSG_ERREUR, parv[0], "Cet username est déjà enregistré");

	if(!strchr(parv[1], '@') || !strchr(parv[1], '.'))
		return sendrpl(cl, MSG_ERREUR, parv[0], "Email incorrecte");

	if(strchr(pass, ' ') || strlen(pass) < 8 || strlen(pass) > 15)
		return sendrpl(cl, MSG_ERREUR, parv[0], "Mot de passe sans espace, longueur entre 8 et 15");

	cl->user = AddUser(cl->nick, pass, parv[1], IsFirst(config) ? 1 : 0, 0, 0, 0);

	if(!cl->user) return sendrpl(cl, MSG_ERREUR, parv[0], "IMPOSSIBLE DE CREER UN USERNAME !!");

	sendrpl(cl, MSG_LOGIN, IsAdmin(cl->user) ? 1 : 0);

	DelFirst(config);
	write_users();

	return 0;
}

/* AUTH <pass> */
int user_auth(aClient *cl, int parc, char **parv)
{
	anUser *u = userhead;

	if(!IsInLogin(cl))
		return sendrpl(cl, MSG_ERREUR, parv[0], "Votre pseudo n'est pas enregistré");

	if(cl->user)
		return sendrpl(cl, MSG_ERREUR, parv[0], "Vous êtes déjà logué");

	for(;u;u=u->next)
		if(!strcasecmp(u->nick, cl->nick))
			break;

	if(!u) return sendrpl(cl, MSG_ERREUR, parv[0], "Votre username n'est pas enregistré !?");

	if(strcasecmp(parv[1], u->pass))
		return sendrpl(cl, ERR_011, parv[1]);

	cl->user = u;
	cl->user->cl = cl;

	DelInLogin(cl);

	sendrpl(cl, MSG_CONNECTED, cl->nick);

	sendrpl(cl, MSG_LOGIN, IsAdmin(u) ? 1 : 0);
	show_motd(cl);

	return 0;
}

/* STATS <SHOW|RESET> */
int show_stats(aClient *cl, int parc, char **parv)
{
	if(!cl->user) return sendrpl(cl, MSG_ERREUR, parv[0], "N'a pas d'username");
	if(!strcasecmp(parv[1], "SHOW"))
	{
		aStats *s = cl->user->stats;

		return sendrpl(cl, MSG_STATS_SHOW, s->nbparties, s->grades[1], s->grades[2], s->grades[3],
			s->grades[4], s->score);
	}
	if(!strcasecmp(parv[1], "RESET"))
	{
		aStats *s = cl->user->stats;
		int i;

		s->nbparties = 0;
		for(i=1;i<=4;i++)
			s->grades[i] = 0;
		s->score = 0;

		return sendrpl(cl, MSG_STATS_RESETED);
	}
	if(!strcasecmp(parv[1], "TOP5"))
	{
		struct list_stats {
			anUser *u;
		} lstats[MAXUSERS+1];
		int i, k;
		char s[(NICKLEN + 6 + 20)*5] = { 0 };
		anUser *u = userhead;

		for(i=0;i<=MAXUSERS && u;i++, u=u->next)
			lstats[i].u = u;

		for(i=1; i < tusers;i++)
		{
			k=i;
			while(k && lstats[k].u->stats->score > lstats[k-1].u->stats->score)
			{
				u = lstats[k-1].u;
				lstats[k-1].u = lstats[k].u;
				lstats[k].u = u;
				k--;
			}
		}

		for(i=0;i<5 && i < tusers;i++)
		{
			if(!i)
				strcpy(s, lstats[i].u->nick);
			else
			{
				strcat(s, " ");
				strcat(s, lstats[i].u->nick);
			}

			strcat(s, ":");
			sprintf(s + strlen(s), "%d", lstats[i].u->stats->nbparties);

			for(k=1;k<=4;k++)
				strcat(s, ":"), sprintf(s + strlen(s), "%d", lstats[i].u->stats->grades[k]);

			strcat(s, ":");
			sprintf(s + strlen(s), "%d", lstats[i].u->stats->score);
		}

		return sendrpl(cl, MSG_STATS_TOP5, s);
	}
	sendrpl(cl, MSG_ERREUR, parv[0], "Sous-commande inconnue");

	return 0;
}

int server_info(aClient *cl, int parc, char **parv)
{
	sendrpl(cl, MSG_SERVERINFO, server.CONact, server.CONtotal, server.GAMEtotal, tusers,
			duration(CurrentTS - server.uptime));
	return 0;
}

/* MSG :<message> */
int send_msg(aClient *cl, int parc, char **parv)
{
	if(!cl->player) return sendrpl(cl, ERR_007, parv[0]);

	if(cl->player->game->nbplayers <= 1) return 0;
	sendto_game(cl, cl->player->game, MSG_MSG, cl->nick, parv[1]);

	return 0;
}

int bybye(aClient *cl, int parc, char **parv)
{
	exit_client(cl, MSG_QUIT);

	return 0;
}
