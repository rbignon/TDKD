/* src/gamecmds.c - Commandes en cours de partie
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
#include "gamecmds.h"
#include "serveur.h"
#include "infos.h"
#include "outils.h"
#include "ia.h"

char cards[52][3] = {
		"1P", "2P", "3P", "4P", "5P", "6P", "7P", "8P", "9P", "AP", "BP", "CP", "DP",
		"1T", "2T", "3T", "4T", "5T", "6T", "7T", "8T", "9T", "AT", "BT", "CT", "DT",
		"1C", "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "AC", "BC", "CC", "DC",
		"1A", "2A", "3A", "4A", "5A", "6A", "7A", "8A", "9A", "AA", "BA", "CA", "DA"
	};

char *GetIdent(struct playerinfo *player)
{
	static char Ident[2];
	sprintf(Ident, "%d%d", player->place, player->grade);
	return Ident;
}

int end_of_game(aGame *g)
{
	int i, j = 5, k = -1;
	char players[300];
	aPlayer *pl = NULL;

	for(i=0; i < MAXPLAYERS; i++)
	{
		int l;
		if(!IsEnd(g->player[i]))
		{
			sendto_game(NULL, g, MSG_WIN, GetIdent(&g->player[i]), (g->player[i].grade = g->iwin++));
			ia_speak(&g->player[i], ia_speak_lose);
		}
		else DelEnd(g->player[i]);
		if(IsMaitre(g->player[i]))
			DelMaitre(g->player[i]);
		if(IsToHim(g->player[i]))
			DelToHim(g->player[i]);
		if(IsCant(g->player[i]))
			DelCant(g->player[i]);
		if(!IsIA(g->player[i]) && g->player[i].grade < j)
		{
			SetOwner(g->player[i]);
			if(k != -1) DelOwner(g->player[k]);
			k = i;
			j = g->player[i].grade;
			pl = &g->player[i];
		}
		if(IsWait(g->player[i]) && g->player[i].user)
		{
			g->player[i].stats->score = 0;
			g->player[i].stats->nbparties = 0;
			for(l=1;l<=4;l++)
				g->player[i].stats->grades[l] = 0;
			DelIA(g->player[i]);
			DelWait(g->player[i]);
			sendrpl(g->player[i].user, MSG_JOINED, g->name, GetIdent(&g->player[i]));
			sendrpl(g->player[i].user, MSG_RULES, g->reflex, Is52C(g) ? 1 : 0);
			for(l=0;l<MAXPLAYERS;l++)
			{
				if(g->player[l].user != g->player[i].user)
				{
					if(g->player[l].user)
						sendrpl(g->player[l].user, MSG_JOIN, g->player[i].user->nick, GetIdent(&g->player[i]), 0);
					sendrpl(g->player[i].user, MSG_JOIN, IsIA(g->player[l]) ? g->player[l].IAname : g->player[l].user->nick, GetIdent(&g->player[l]), IsIA(g->player[l]) ? 1 : 0);
				}
			}
		}
		if(g->player[i].grade == 1) g->player[i].score = (g->player[i].score * 2);
		if(g->player[i].grade == 4) g->player[i].score = (g->player[i].score / 2);
		if(IsUser(g->player[i]))
		{
			g->player[i].user->user->stats->grades[g->player[i].grade]++;
			g->player[i].user->user->stats->score += g->player[i].score;
		}
		else
		{
			g->player[i].stats->grades[g->player[i].grade]++;
			g->player[i].stats->score += g->player[i].score;
		}
		if(i)
		{
			strcat(players, " ");
			strcat(players, GetIdent(&g->player[i]));
		}
		else
			strcpy(players, GetIdent(&g->player[i]));
		strcat(players, ":");
		sprintf(players + strlen(players), "%d", !IsUser(g->player[i]) ? g->player[i].stats->nbparties : g->player[i].user->user->stats->nbparties);
		for(l=1;l<=4;l++)
			strcat(players, ":"), sprintf(players + strlen(players), "%d", !IsUser(g->player[i]) ? g->player[i].stats->grades[l] : g->player[i].user->user->stats->grades[l]);

		strcat(players, ":");
		sprintf(players + strlen(players), "%d", !IsUser(g->player[i]) ? g->player[i].stats->score : g->player[i].user->user->stats->score);

	}
	if(!pl) return sendto_game(NULL, g, MSG_ERREUR, "end_of_game()", "pl est NULL, il n'y a pas d'owner.");
	sendto_game(NULL, g, MSG_ENDOFGAME, GetIdent(pl), players);
	DelInGame(g);
	if(IsNoGrade(g)) DelNoGrade(g);

	return 0;
}

int next_player(aPlayer *pl)
{
	aGame *g = pl->game;
	aPlayer *p = NULL;
	int i = pl->place, j, end = 0, k;
	char tosend[(NbCartes(g)*2+10)];

	while(1)
	{
		if(end >= 3) /* PERSONNE NE PEUT SURCOUCHER */
		{
			for(k = 0;k < 4;k++)
			{
				if(IsCant(g->player[k])) DelCant(g->player[k]);
				if(IsMaitre(g->player[k]))
				{
					DelMaitre(g->player[k]);
					p = &g->player[k];
				}
			}
			if(IsEnd(*p))
				for(k = p->place;;)
				{
					if(k >= 3) k = 0;
					else k++;
					if(k == p->place)
					{
						sendto_players(NULL, g, MSG_ERREUR, "next_player();", "Pas de joueurs suivants !?");
						return -1;
					}
					if(!IsEnd(g->player[k]))
					{
						p = &g->player[k];
						break;
					}
				}
			sendto_players(NULL, g, MSG_PERSONNEPEU, GetIdent(p));
			sendto_players(p->user, g, MSG_BEGIN, GetIdent(p));
			for(j = 0, k=0;j < NbCartes(g);j++)
			{
				if(!p->carte[j].value) break;
				if(k)
				{
					strcat(tosend, ",");
					strcat(tosend, p->carte[j].name);
				}
				else
				{
					strcpy(tosend, p->carte[j].name);
					k++;
				}
			}
			printf("- %s cartes: %s\n", p->user->nick, tosend);
			p->flag |= P_TOHIM;
			end = 0;
			if(!(p->flag & P_IA)) sendrpl(p->user, MSG_TOYOU);
			else ia_joue(p);
			return 0;
		}
		else
		{
			if(i == 3) i = 0;
			else i++;

			if(IsCant(g->player[i]) || IsMaitre(g->player[i]) || IsEnd(g->player[i]))
			{
				end++;
				continue;
			}
			SetToHim(g->player[i]);
			if(!IsIA(g->player[i]))
			{
				sendrpl(g->player[i].user, MSG_TOYOU);
				for(j = 0, k=0;j < NbCartes(g);j++)
				{
					if(!g->player[i].carte[j].value) break;
					if(k)
					{
						strcat(tosend, ",");
						strcat(tosend, g->player[i].carte[j].name);
					}
					else
					{
						strcpy(tosend, g->player[i].carte[j].name);
						k++;
					}
				}
				printf("- %s cartes: %s\n", g->player[i].user->nick, tosend);
			}
			else ia_joue(&g->player[i]);
			return 0;
		}
	}
	return 0;
}

int send_cartes(struct playerinfo *pl)
{
	int i, j = 0;
	char adonner[(NbCartes(pl->game)*2+10)];

	for(i = 0;i < NbCartes(pl->game);i++)
	{
		if(!(pl->carte[i].value)) break;
		if(j)
		{
			strcat(adonner, ",");
			strcat(adonner, pl->carte[i].name);
		}
		else
		{
			strcpy(adonner, pl->carte[i].name);
			j++;
		}
	}
	if(j)
		sendrpl(pl->user, MSG_DONE, adonner);
	return i;
}

int distribuer_cartes(aGame *game)
{
#if 0 /* N'a plus d'utilitée */
	char crt52[52][3] = {
		"1P", "2P", "3P", "4P", "5P", "6P", "7P", "8P", "9P", "AP", "BP", "CP", "DP",
		"1T", "2T", "3T", "4T", "5T", "6T", "7T", "8T", "9T", "AT", "BT", "CT", "DT",
		"1C", "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "AC", "BC", "CC", "DC",
		"1A", "2A", "3A", "4A", "5A", "6A", "7A", "8A", "9A", "AA", "BA", "CA", "DA"
	};
	char crt32[32][3] = {
		"5P", "6P", "7P", "8P", "9P", "AP", "BP", "CP",
		"5T", "6T", "7T", "8T", "9T", "AT", "BT", "CT",
		"5C", "6C", "7C", "8C", "9C", "AC", "BC", "CC",
		"5A", "6A", "7A", "8A", "9A", "AA", "BA", "CA"
	};
#endif
	int i = 0, nb = Is52C(game) ? 51 : 31, begin = 0, init;
	time_t nb_secs;
	aPlayer *beginer = NULL;
	char adonner[(NbCartes(game)*2+10)];
	char (*crt)[3] = malloc(53*sizeof(char*));

	if(Is52C(game))
	{
		strcpy(crt[i++], "DP"); strcpy(crt[i++], "DT");	strcpy(crt[i++], "DC"); strcpy(crt[i++], "DA");
		strcpy(crt[i++], "1P"); strcpy(crt[i++], "1T");	strcpy(crt[i++], "1C"); strcpy(crt[i++], "1A");
		strcpy(crt[i++], "2P"); strcpy(crt[i++], "2T");	strcpy(crt[i++], "2C"); strcpy(crt[i++], "2A");
		strcpy(crt[i++], "3P"); strcpy(crt[i++], "3T");	strcpy(crt[i++], "3C"); strcpy(crt[i++], "3A");
		strcpy(crt[i++], "4P"); strcpy(crt[i++], "4T");	strcpy(crt[i++], "4C"); strcpy(crt[i++], "4A");
	}
	strcpy(crt[i++], "5P"); strcpy(crt[i++], "5T");	strcpy(crt[i++], "5C"); strcpy(crt[i++], "5A");
	strcpy(crt[i++], "6P"); strcpy(crt[i++], "6T");	strcpy(crt[i++], "6C"); strcpy(crt[i++], "6A");
	strcpy(crt[i++], "7P"); strcpy(crt[i++], "7T");	strcpy(crt[i++], "7C"); strcpy(crt[i++], "7A");
	strcpy(crt[i++], "8P"); strcpy(crt[i++], "8T");	strcpy(crt[i++], "8C"); strcpy(crt[i++], "8A");
	strcpy(crt[i++], "9P"); strcpy(crt[i++], "9T");	strcpy(crt[i++], "9C"); strcpy(crt[i++], "9A");
	strcpy(crt[i++], "AP"); strcpy(crt[i++], "AT");	strcpy(crt[i++], "AC"); strcpy(crt[i++], "AA");
	strcpy(crt[i++], "BP"); strcpy(crt[i++], "BT");	strcpy(crt[i++], "BC"); strcpy(crt[i++], "BA");
	strcpy(crt[i++], "CP"); strcpy(crt[i++], "CT");	strcpy(crt[i++], "CC"); strcpy(crt[i++], "CA");

	nb_secs = time(NULL);
	init = nb_secs;
	srand(init);

	for(i = 0;i < MAXPLAYERS;i++)
	{
		int j, k, dbl = 0, r;

		for(j = 0;j < NbCartes(game);j++)
		{
			int si;
			if(nb)
				do
				{
					r = rand() % nb;
					si = ((r/RAND_MAX)*nb);
				} while(crt[r][0] == '\0');
			else r = 0;

			strcpy(game->player[i].carte[j].name, crt[r]);
			if(!strcmp(crt[r], "AC")) begin = 1;
			strcpy(crt[r], crt[nb]);
			game->player[i].carte[j].nb = 1;
			game->player[i].carte[j].value = GetValue(game->player[i].carte[j].name);
			crt[nb][0] = '\0';

			nb--;
		}
		for(j = 0; j < NbCartes(game); j++)
		{
			dbl = 0;
			for(k=0; k < NbCartes(game); k++)
				if(game->player[i].carte[j].value == game->player[i].carte[k].value)
					dbl++;
			game->player[i].carte[j].nb = dbl;
		}
		game->player[i].lastcarte = (NbCartes(game) - 1);
		game->player[i].carte[NbCartes(game)].nb = game->player[i].carte[NbCartes(game)].value = 0;
		if(IsUser(game->player[i])) game->player[i].user->user->stats->nbparties++;
		else game->player[i].stats->nbparties++;
		game->player[i].score = 0;

		for(r = 0, j=0;r < NbCartes(game);r++)
		{
			if(j)
			{
				strcat(adonner, ",");
				strcat(adonner, game->player[i].carte[r].name);
			}
			else if(game->player[i].carte[r].nb)
			{
				strcpy(adonner, game->player[i].carte[r].name);
				j++;
			}
		}
		if(j)
			printf("%s a les cartes: %s\n", GetIdent(&game->player[i]), adonner);
		if(!IsIA(game->player[i]))
			send_cartes(&game->player[i]);
		if(IsNoGrade(game))
		{
			if(begin == 1)
			{
				begin = 2;
				beginer = &game->player[i];
			}
		}
		else SetChange(game->player[i]);
	}
	for(i=0;i <= V_DEUX; i++)
		game->stats->nb[i] = (Is52C(game) || (i >= V_SEPT && i <= V_AS)) ? 4 : 0;
	game->stats->best = BestCarte(game);
	server.GAMEtotal++;
	free(crt);
	if(beginer)
	{
		SetToHim(*beginer);
		sendto_players(beginer->user, game, MSG_BEGIN, GetIdent(beginer));
		if(!(beginer->flag & P_IA)) sendrpl(beginer->user, MSG_TOYOU);
		else ia_joue(beginer);
		SetInGame(game);
	}
	else
	{
		SetInChange(game);
		for(i=0; i < MAXPLAYERS; i++)
		{
			if(game->player[i].grade <= 2) /* [vice] president */
			{
				SetChange(game->player[i]);
				if(!IsIA(game->player[i])) sendrpl(game->player[i].user, MSG_ADONNER,
						game->player[i].grade == G_PRESIDENT ? 2 : 1, "-");
				else ia_change(&game->player[i], game->player[i].grade == G_PRESIDENT ? 2 : 1, 0);
			}
			else /* [vice] trou du cul */
			{
				SetChange(game->player[i]);
				if(!IsIA(game->player[i])) sendrpl(game->player[i].user, MSG_ADONNER,
						game->player[i].grade == G_TROUDUK ? 2 : 1, "+");
				else ia_change(&game->player[i], game->player[i].grade == G_TROUDUK ? 2 : 1, 1);
			}
		}
	}
	return 1;
}

/* CREATE <jeu> <reflexion> <Ia Speak ?> <52c?> [:<pass>] */
int create_game(aClient *cl, int parc, char **parv)
{
	char nameia[][NICKLEN] = {
	"Pompidou", "Degaulle", "Chirac", "Bush", "Clinton",
        "Nixon", "Mitterrand"
        };
	aGame *g;
	int i, init, r, nb = 6; /* incrémenter lors d'ajout de nameia */
        time_t nb_secs;
        char *pass = (parc >= 6) ? parv[5] : NULL;

	if(cl->player) return sendrpl(cl, ERR_008, cl->player->game->name);

	for(g=gamehead;g;g=g->next)
		if(!strcasecmp(g->name, parv[1]))
			return sendrpl(cl, ERR_002, g->name);

	if(!is_num(parv[2])) return sendrpl(cl, MSG_ERREUR, parv[0], "parv[2] n'est pas un nombre");

	g = add_game(parv[1], atoi(parv[2]));

	nb_secs = time(NULL);
	init = nb_secs;
	srand(init);

	SetNoGrade(g);
	if(atoi(parv[3])) SetIASpeak(g);
	if(atoi(parv[4])) Set52C(g);
	if(pass)
	{
		strncpy(g->pass, pass, PASSLEN);
		SetPass(g);
	}
	for(i = 0; i<MAXPLAYERS; i++)
	{
		int si;
		aStats *stats = calloc(1, sizeof *stats);

		stats->nbparties = 0;
		for(r=1;r<=4;r++)
			stats->grades[r] = 0;
		stats->score = 0;
		g->player[i].stats = stats;

		if(nb)
			do
			{
				r = rand() % nb;
				si = ((r/RAND_MAX)*nb);
			} while(nameia[r][0] == '\0');
		else r = 0;

		strcpy(g->player[i].IAname, nameia[r]);
		strcpy(nameia[r], nameia[nb]);
		nameia[nb][0] = '\0';

		nb--;
		g->player[i].grade = 0;
		g->player[i].place = i;
		g->player[i].game = g;
		g->player[i].flag = 0;
		if(i > 0)
		{
			SetIA(g->player[i]);
			sendrpl(cl, MSG_JOIN, g->player[i].IAname, GetIdent(&g->player[i]), 1);
		}
		else
		{
			g->nbplayers++;
			SetOwner(g->player[i]);
			cl->player = &g->player[i];
			g->player[i].user = cl;
			sendrpl(cl, MSG_JOINED, g->name, GetIdent(&g->player[0]));
			sendrpl(cl, MSG_RULES, g->reflex, Is52C(g) ? 1 : 0);
		}
	}

	return 0;
}

int game_list(aClient *cl, int parc, char **parv)
{
	aGame *game;
	char nicklist[(NICKLEN*4+4)];
	int i, j;

	for(game = gamehead; game; game = game->next)
	{
		nicklist[0] = '\0';
		for(i=0, j=0;i < MAXPLAYERS;i++)
		{
			if(!game->player[i].user) continue;
			if(!j)
				strcpy(nicklist, game->player[i].user->nick);
			else
			{
				strcat(nicklist, " ");
				strcat(nicklist, game->player[i].user->nick);
			}
			j++;
		}
		sendrpl(cl, MSG_LIST, game->name, Is52C(game) ? 1 : 0, game->nbplayers, Playing(game) ? 1 : 0, nicklist);
	}
	sendrpl(cl, MSG_LISTEND);
	return 0;
}

/* JOIN <name> [:<pass>] */
int join_game(aClient *cl, int parc, char **parv)
{
	aGame *g;
	int j = 0, i = 0;
        char *pass = (parc >= 3) ? parv[2] : NULL;

	if(cl->player) return sendrpl(cl, ERR_008, cl->player->game->name);

	for(g=gamehead;g && strcasecmp(g->name, parv[1]);g=g->next);

	if(!g) return sendrpl(cl, ERR_003, parv[1]);

	for(j = 0;j < MAXPLAYERS;j++)
		if(IsIA(g->player[j]) && !g->player[j].user) i=j;

	if(!i) return sendrpl(cl, ERR_004, g->name);

	if(IsPass(g) && (!pass || strcasecmp(pass, g->pass)))
		return sendrpl(cl, ERR_011, g->name);

	g->player[i].user = cl;
	g->player[i].game = g;
	g->nbplayers++;
	cl->player = &g->player[i];

	if(Playing(g))
	{
		char nicklist[(NICKLEN*4+4)];
		int k = 0;

		for(j=0;j < MAXPLAYERS;j++)
		{
			if(!g->player[j].user) continue;
			if(!k)
			{
				k++;
				strcpy(nicklist, g->player[j].user->nick);
			}
			else
			{
				strcat(nicklist, " ");
				strcat(nicklist, g->player[j].user->nick);
			}
		}
		sendrpl(cl, MSG_WAIT, g->name, nicklist);
		sendto_game(cl, g, MSG_WAITJOIN, cl->nick, g->player[i].IAname);
		SetWait(g->player[i]);
	}
	else
	{
		DelIA(g->player[i]);
		sendrpl(cl, MSG_JOINED, g->name, GetIdent(&g->player[i]));
		sendrpl(cl, MSG_RULES, g->reflex, Is52C(g) ? 1 : 0);
		for(j=0;j<MAXPLAYERS;j++)
		{
			if(g->player[j].user != cl)
			{
				if(g->player[j].user)
					sendrpl(g->player[j].user, MSG_JOIN, cl->nick, GetIdent(&g->player[i]), 0);
				sendrpl(cl, MSG_JOIN, IsIA(g->player[j]) ? g->player[j].IAname : g->player[j].user->nick, GetIdent(&g->player[j]), IsIA(g->player[j]) ? 1 : 0);
			}
		}
	}

	return 0;
}

int leave_game(aClient *cl, aGame *g, int silence)
{
	aPlayer *p = cl->player;

	if(!p) return silence ? 0 : sendrpl(cl, MSG_ERREUR, "leave_game()", "p est nul");

	if(p->flag & P_WAIT)
	{
		if(!silence) sendrpl(cl, MSG_PART, cl->nick);
		p->flag &= ~P_WAIT;
		return 0;
	}

	if(g->nbplayers <= 1)
	{
		if(!silence) sendrpl(cl, MSG_PART, GetIdent(cl->player));
		del_game(g);
	}
	else
	{
		g->nbplayers--;
		cl->player = NULL;
		if(IsWait(*p))
		{
			DelWait(*p);
			if(!silence) sendrpl(cl, MSG_PART, GetIdent(p));
			p->user = NULL;
		}
		else
		{
			int i;
			p->flag |= P_IA;
			for(i=0;i<MAXPLAYERS;i++)
				if((!silence || g->player[i].user != cl) && g->player[i].user)
					sendrpl(g->player[i].user, MSG_PART, IsWait(g->player[i]) ? p->user->nick : GetIdent(p));
			sendto_game(cl, g, MSG_JOIN, p->IAname, GetIdent(p), 1);
			if(IsOwner(*p) && Playing(g) && IsGameStop(g))
			{
				DelGameStop(g);
				sendto_game(NULL, g, MSG_STOP, GetIdent(cl->player), 0);
			}
			p->user = NULL;
			if(IsChange(*p))
				ia_change(p, (p->grade == G_PRESIDENT || p->grade == G_TROUDUK) ? 2 : 1, p->grade <= 2 ? 0 : 1);
			else if(IsToHim(*p)) ia_joue(p);
			else if(IsOwner(*p) && !Playing(g))
			{
				distribuer_cartes(g);
				g->iwin = 1;
			}
		}
	}

	return 0;
}

int part_game(aClient *cl, int parc, char **parv)
{
	aGame *g;

	if(!cl->player) return sendrpl(cl, MSG_ERREUR, parv[0], "Vous n'êtes dans aucune partie");

	g = cl->player->game;

	if(!IsWait(*cl->player) && Playing(g))
		return sendrpl(cl, ERR_009, g->name);

	leave_game(cl, g, 0);

	return 0;
}

// KICK <ident>
int kick_user(aClient *cl, int parc, char **parv)
{
	aGame *g;
	aPlayer *p = NULL;
	int i;
	char s[3];

	if(!cl->player) return sendrpl(cl, MSG_ERREUR, parv[0], "Vous n'êtes dans aucune partie");

	g = cl->player->game;

	if(!IsOwner(*cl->player) && !IsAdmin(cl->user)) return sendrpl(cl, MSG_ERREUR, parv[0], "Vous n'êtes pas owner du jeu");

	for(i=0;i < MAXPLAYERS; i++)
		if(!strcasecmp(GetIdent(&g->player[i]), parv[1]))
		{
			p = &g->player[i];
			break;
		}

	if(!p) return sendrpl(cl, MSG_ERREUR, parv[0], "Joueur introuvable");
	if(IsIA(*p) || !p->user) return sendrpl(cl, MSG_ERREUR, parv[0], "On ne peut pas kicker une IA");

	strcpy(s, GetIdent(cl->player));
	sendto_game(NULL, g, MSG_KICK, s, GetIdent(p));
	leave_game(p->user, g, 0);

	return 0;
}

/* STOP */
int stop_game(aClient *cl, int parc, char **parv)
{
	aGame *g;

	if(!cl->player) return sendrpl(cl, MSG_ERREUR, parv[0], "Vous n'êtes dans aucune partie");
	if(!IsOwner(*cl->player)) return sendrpl(cl, ERR_006, cl->player->game->name);
	if(!Playing(cl->player->game)) return
#if 0
	sendrpl(cl, MSG_ERREUR, parv[0], "Aucune partie en cours");
#else
	0;
#endif

	g = cl->player->game;

	if(IsGameStop(g))
		DelGameStop(g);
	else
		SetGameStop(g);

	sendto_game(NULL, g, MSG_STOP, GetIdent(cl->player), IsGameStop(g) ? 1 : 0);

	if(!IsGameStop(g))
	{
		int i;
		for(i=0;i<MAXPLAYERS;i++)
		{
			if(IsIA(g->player[i]))
			{
				aPlayer *p = &g->player[i];

				if(IsChange(g->player[i]))
					ia_change(p, (p->grade == G_PRESIDENT || p->grade == G_TROUDUK) ? 2 : 1, p->grade <= 2 ? 0 : 1);
				else if(IsToHim(g->player[i])) ia_joue(p);
				else if(IsOwner(g->player[i]) && !Playing(g))
					putlog(ERREURS_LOG, "pas possible (%s:%d)\n", __FILE__, __LINE__);
			}
		}
	}

	return 0;
}

int distribuer(aClient *cl, int parc, char **parv)
{
	if(!cl->player) return sendrpl(cl, MSG_ERREUR, parv[0], "Vous etes dans aucune partie");
	if(!(cl->player->flag & P_OWNER)) return sendrpl(cl, ERR_006, cl->player->game->name);
	if(Playing(cl->player->game)) return sendrpl(cl, ERR_009, cl->player->game->name);

	distribuer_cartes(cl->player->game);
	cl->player->game->iwin = 1;

	return 0;
}

/* DONNER <cartes> */
int donner_cartes(aClient *cl, int parc, char **parv)
{
	char don[3][3];
	struct playerinfo *p = NULL;
	aGame *g;
	aPlayer *beginer = NULL;
	char *carte = NULL, *tmp = NULL, cartes[6];
	int nbcartes = 0, i, j = 0, k, endchange = 1, nbocartes;

	if(!cl->player) return sendrpl(cl, MSG_ERREUR, parv[0], "Vous etes dans aucune partie");
	if(!(cl->player->flag & P_CHANGE) || cl->player->flag & P_IA || !IsInChange(cl->player->game))
		return sendrpl(cl, "ERREUR %s :Vous n'êtes pas censé donner de cartes [%s][%s][%s]", parv[0],
				IsChange(*cl->player) ? "change" : "non", IsIA(*cl->player) ? "IA" : "humain",
				IsInChange(cl->player->game) ? "inchange" : "??");

	if(!strchr(parv[1], ','))
	{
		if (cl->player->grade == G_PRESIDENT || cl->player->grade == G_TROUDUK)
			return sendrpl(cl, MSG_ERREUR, parv[0], "Vous devez donner deux cartes");
		nbcartes = 1;
	} else nbcartes = 2; /* très bourin mais bon */

	g = cl->player->game;

	for(k = 0; k < MAXPLAYERS; k++)
	{
		if((cl->player->grade == G_TROUDUK && g->player[k].grade == G_PRESIDENT) ||
		   (cl->player->grade == G_VTROUDUK && g->player[k].grade == G_VPRESIDENT) ||
		   (cl->player->grade == G_VPRESIDENT && g->player[k].grade == G_VTROUDUK) ||
		   (cl->player->grade == G_PRESIDENT && g->player[k].grade == G_TROUDUK))
				p = &g->player[k];
		if(cl != g->player[k].user && IsChange(g->player[k])) endchange = 0;
		if(g->player[k].grade == G_TROUDUK) beginer = &g->player[k];
	}
	if(!p) return sendrpl(cl, MSG_ERREUR, parv[0], "Erreur grave: p est nul !");

	strcpy(cartes, parv[1]);

	for(carte = Strtok(&tmp, parv[1], ',');carte;carte = Strtok(&tmp, NULL, ','), j++)
	{
		int is_ok;
		is_ok = 0;
		if(j >= nbcartes) return sendrpl(cl, MSG_ERREUR, parv[0], "Trop de cartes");
		for(i = 0;i <= cl->player->lastcarte;i++)
		{
			printf("carte %s <-> %s\n", carte, cl->player->carte[i].name);
			if(!strcasecmp(carte, cl->player->carte[i].name))
			{
				strcpy(don[j], cl->player->carte[i].name);
				is_ok = 1;
				printf("ok!\n");
				break;
			}
		}
		if(!is_ok) return sendrpl(cl, "ERREUR %s :Carte introuvable (%s)", parv[0], carte);
	}
	nbocartes = cl->player->lastcarte;
	for(i=nbocartes+1, j=0; j < nbcartes; i++, j++)
	{
		int a;
		for(a=0;a <= cl->player->lastcarte;a++)
		{
			if(!strcasecmp(cl->player->carte[a].name, don[j]))
			{
				DelICarte(cl->player, a);
				AddCarte(p, don[j]);
				break;
			}
		}
	}
	cl->player->flag &= ~P_CHANGE;
	if(!IsIA(*p)) sendrpl(p->user, MSG_DONNE, GetIdent(cl->player), cartes);

	if(!endchange || !beginer) return 0;

	DelInChange(g);
	SetInGame(g);

	beginer->flag |= P_TOHIM;
	sendto_players(beginer->user, g, MSG_BEGIN, GetIdent(beginer));
	if(!(beginer->flag & P_IA)) sendrpl(beginer->user, MSG_TOYOU);
	else ia_joue(beginer);

	return 0;
}

int user_cant(aClient *cl, int parc, char **parv)
{
	if(!cl->player) return sendrpl(cl, MSG_ERREUR, parv[0], "Vous etes dans aucune partie");
	if(cl->player->flag & P_IA || !IsInGame(cl->player->game) || IsGameStop(cl->player->game) || !IsToHim(*cl->player))
#if 1 /* TODO: corriger ce putain de bug !! */
		return sendrpl(cl, MSG_ERREUR, parv[0], "Vous n'êtes pas censé jouer");
#else
		return 0;
#endif
	cl->player->flag |= P_CANT;
	sendto_players(cl, cl->player->game, MSG_CANT, GetIdent(cl->player));
	cl->player->flag &= ~P_TOHIM;
	next_player(cl->player);

	return 0;
}

/* PLAY <cartes> */
int user_play(aClient *cl, int parc, char **parv)
{
	aPlayer *maitre = NULL;
	aGame *g = NULL;
	aCarte *c = NULL;
	int nbcartes = 0, i, j, is_ok, valmin = 0, valdef = 0;
	char *carte = NULL, *tmp = NULL, cartes[50];

	strcpy(cartes, parv[1]);
	if(!cl->player) return sendrpl(cl, MSG_ERREUR, parv[0], "Vous etes dans aucune partie");
	g = cl->player->game;
	if(cl->player->flag & P_IA || !IsInGame(g) || IsGameStop(g) || !IsToHim(*cl->player))
		sendrpl(cl, MSG_ERREUR, parv[0], "Vous n'êtes pas censé jouer");

	for(i=0;i<MAXPLAYERS;i++)
		if(IsMaitre(g->player[i])) {maitre = &g->player[i]; break;}

	if(maitre) c = &maitre->played[0];

	for(i=0;i<MAXPLAYERS;i++)
	{
		if(g->player[i].flag & P_MAITRE)
		{
			valmin = g->player[i].played[0].value;
			break;
		}
	}

	for(carte = Strtok(&tmp, parv[1], ','), i=0;carte;carte = Strtok(&tmp, NULL, ','), i++)
	{
		is_ok = 0;
		nbcartes++;
		if((c && nbcartes > c->nb) || nbcartes > 4)
			return sendrpl(cl, MSG_ERREUR, parv[0], "Vous devez jouer le bon nombre de cartes");

		for(j = 0;j <= cl->player->lastcarte;j++)
		{
			if(!strcasecmp(carte, cl->player->carte[j].name))
			{
				if((valdef && cl->player->carte[j].value != valdef) ||
				   (valmin && cl->player->carte[j].value <= valmin))
					return sendrpl(cl, MSG_ERREUR, parv[0], "Jouez correctement !");
				strcpy(cl->player->played[i].name, cl->player->carte[j].name);
				cl->player->played[i].value = cl->player->carte[j].value;
				is_ok = 1;
				if(!valdef) valdef = cl->player->played[i].value;
				break;
			}
		}
		if(!is_ok) return sendrpl(cl, MSG_ERREUR, parv[0], "Carte introuvable");
	}
	if(c && nbcartes < c->nb)
		return sendrpl(cl, MSG_ERREUR, parv[0], "Vous devez jouer le bon nombre de cartes");

	DelToHim(*cl->player);
	for(i=0;i < nbcartes; i++)
	{
		cl->player->played[i].nb = nbcartes;
		for(j=0;j <= cl->player->lastcarte;j++)
		{
			if(!strcasecmp(cl->player->carte[j].name, cl->player->played[i].name))
			{
				int k;
				g->stats->nb[cl->player->carte[j].value]--;

				if(valmin && !i && valmin <= V_NEUF && cl->player->played[i].value >= V_DAME)
				{
					int r;
					k=0;
					do
					{
						r = rand()%3;
						k++;
					} while (!IsIA(g->player[r]) && k < MAXPLAYERS);

					if(r) ia_speak_to(&g->player[r], cl->nick, ia_speak_trophaut);
				}

				if(!g->stats->nb[cl->player->carte[j].value])
					for(k=BestCarte(g);k>0;k--)
						if(g->stats->nb[k])
							{ g->stats->best = k; break; }
				if(!i)
					cl->player->score += (puiss(2,nbcartes) * cl->player->carte[j].value);
				DelICarte(cl->player, j);
				break;
			}
		}
	}
	sendto_players(NULL, g, MSG_PLAY, GetIdent(cl->player), cartes);

	if(cl->player->lastcarte == -1)
	{
		SetEnd(*cl->player);
		sendto_game(NULL, g, MSG_WIN, GetIdent(cl->player), g->iwin);
		cl->player->grade = g->iwin++;
		if(cl->player->grade == 1)
			for(i=0;i<4;i++)
				if(g->player[i].grade == 1 && i != cl->player->place)
					ia_speak_to(&g->player[i], cl->nick, ia_speak_detroned);
		if(g->iwin == 4) return end_of_game(g);
	}
	for(i=0;i<MAXPLAYERS; i++)
		if(IsMaitre(g->player[i])) DelMaitre(g->player[i]);

	cl->player->flag |= P_MAITRE;

	next_player(cl->player);

	return 0;
}
