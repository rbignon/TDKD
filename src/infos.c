/* src/infos.c - Gestion des structures
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
#include "outils.h"

int tusers = 0;

int AddCarte(aPlayer *pl, const char *name)
{
	int i, j = 0;

	pl->lastcarte++;
	strcpy(pl->carte[pl->lastcarte].name, name);
	pl->carte[pl->lastcarte].value = GetValue(pl->carte[pl->lastcarte].name);

	for(i=0; i <= pl->lastcarte; i++)
		if(pl->carte[i].value == pl->carte[pl->lastcarte].value)
		{
			pl->carte[i].nb++;
			j++;
		}
	pl->carte[pl->lastcarte].nb = j;

	return 1;
}

int DelICarte(aPlayer *pl, int i)
{
	int value;

	value = pl->carte[i].value;
	strcpy(pl->carte[i].name, pl->carte[pl->lastcarte].name);
	pl->carte[i].value = pl->carte[pl->lastcarte].value;
	pl->carte[i].nb = pl->carte[pl->lastcarte].nb;
	pl->carte[pl->lastcarte].value = 0;
	pl->carte[pl->lastcarte].nb = 0;
	pl->carte[pl->lastcarte].name[0] = '\0';
	pl->lastcarte--;

	for(i=0;i <= pl->lastcarte;i++)
		if(pl->carte[i].value == value)
			pl->carte[i].nb--;

	return 1;
}

int DelCarte(aPlayer *pl, const char *name)
{
	int i, value = 0;

	for(i=0; i <= pl->lastcarte; i++)
		if(!strcasecmp(pl->carte[i].name, name))
		{
			strcpy(pl->carte[i].name, pl->carte[pl->lastcarte].name);
			value = pl->carte[i].value;
			pl->carte[i].value = pl->carte[pl->lastcarte].value;
			pl->carte[i].nb = pl->carte[pl->lastcarte].nb;
			pl->carte[pl->lastcarte].value = 0;
			pl->carte[pl->lastcarte].nb = 0;
			pl->carte[pl->lastcarte].name[0] = '\0';
			pl->lastcarte--;
			break;
		}
		else if(i == pl->lastcarte) return 0;

	for(i=0;i <= pl->lastcarte;i++)
		if(pl->carte[i].value == value)
			pl->carte[i].nb--;

	return 1;
}

struct userinfo *AddUser(const char *name, const char *pass, const char *mail, unsigned int flag, int nbparties, int grades[], int score)
{
	anUser *new = calloc(1, sizeof *new), *tmp = userhead;
	aStats *stats = calloc(1, sizeof *stats);
	int i;

	if(!new)
	{
		printf("AddAdmin, malloc a échoué pour addUser %s", name);
		return NULL;
	}

	new->next = NULL;
	strncpy(new->nick, name, NICKLEN);
	strncpy(new->pass, pass, 15);
	strncpy(new->mail, mail, 100);
	new->flag = flag;
	stats->nbparties = nbparties;

	if(grades != NULL)
	for(i=1;i<=4;i++)
		stats->grades[i] = grades[i];
	stats->score = score;
	new->stats = stats;
	tusers++;

	if(!tmp)
	{
		new->next = NULL;
		userhead = new;
	}
	else
	{
		new->next = tmp;
		new->next->last = new;
		userhead = new;
	}
	return new;
}

void DelUser(anUser *u)
{
	if(u->cl) u->cl->user = NULL;

	if(userhead == u)
	{
		userhead = u->next;
		if(u->next) userhead->last = NULL;
	}
	else
	{
		if(u->last) u->last->next = u->next;
		if(u->next) u->next->last = u->last;
	}
	tusers--;
	free(u->stats);
	free(u);
	return;
}


int ClearUsers()
{
	anUser *a = userhead, *b = NULL;

	while(a)
	{
		b = a;
		free(a);
		a = b;
	}
	return 1;
}

struct gameinfo *add_game(const char *name, int reflex, const char *pass)
{
	aGame *new = calloc(1, sizeof *new), *tmp = gamehead;
	aStat *stat = calloc(1, sizeof *stat);

	if(!new)
	{
		printf("add_game, malloc a échoué pour aGame %s", name);
		return NULL;
	}

	new->next = NULL;
	strncpy(new->name, name, GAMELEN);
	new->reflex = reflex;
	new->nbplayers = 0;
	new->last = NULL;
	new->stats = stat;

	if(!tmp)
	{
		new->next = NULL;
		gamehead = new;
	}
	else
	{
		new->next = tmp;
		new->next->last = new;
		gamehead = new;
	}
	return new;
}

void del_game(aGame *game)
{
	int i;

	for(i=0;i < MAXPLAYERS;i++)
	{
		if(game->player[i].user) game->player[i].user->player = NULL;
		free(game->player[i].stats);
	}

	if(gamehead == game)
	{
		gamehead = game->next;
		if(game->next) gamehead->last = NULL;
	}
	else
	{
		if(game->last) game->last->next = game->next;
		if(game->next) game->next->last = game->last;
	}
	//free(game->player);
	//free(game->name);
	free(game->stats);
	free(game);
	return;
}
