/* src/ia.c - Fonctions d'Intelligence Artificielle
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
#include "ia.h"
#include "infos.h"
#include "outils.h"
#include "serveur.h"
#include "gamecmds.h"
#include <math.h>

struct ia_speak_tab ia_speak_win[] = {
	{0, "Vous vous êtes fait niqués !"},
	{0, "Yes j'ai gagné"},
	{0, "Ouh j'ai eu chaud !"},
	{0, "Putain ça fait du bien !"},
	{0, "Je crois bien que j'ai gagné"},
	{0, "Vous l'avez pas vu venir je crois !"},
	{0, ""}
};
struct ia_speak_tab ia_speak_detroned[] = {
	{0, "SALAUD !!! C'ETAIT MOI LE PRESIDENT !!"},
	{0, "MAIS QUEL ENCULE"},
	{0, "Putain, j'ai un jeu de merde aussi..."},
	{0, "Mais merde de merde"},
	{0, "Vas te faire foutre !"},
	{0, "Suicide toi"},
	{0, ""}
};
struct ia_speak_tab ia_speak_wose[] = {
	{0, "Bon je suis pas trou de bal c'est déjà ça"},
	{0, "Ouai ! J'y ai échapé ! J'avais un jeu de merde pourtant"},
	{0, "Au moin je ne suis pas trou du cul"},
	{0, "Putain je voulais être président"},
	{0, "Batard !"},
	{0, "Je serais président à la prochaine partie !"},
	{0, ""}
};
struct ia_speak_tab ia_speak_lose[] = {
	{0, "Putain je me suis fait niqué..."},
	{0, "Bande de connards !"},
	{0, "J'avais un jeu de merde aussi..."},
	{0, "Bon ben je suis trou du cul..."},
	{0, "Là j'ai vraiment joué le con..."},
	{0, "Pourquoiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
	{0, ""}
};
struct ia_speak_tab ia_speak_parfait[] = {
	{0, "Parfait !"},
	{0, "Exactement ce qu'il me fallait !"},
	{0, "Perfecto !"},
	{0, "Je me débarasse !"},
	{0, ""}
};
struct ia_speak_tab ia_speak_trophaut[] = {
	{0, "Ho le ouf"},
	{0, "Putain mais pourquoi tu joues aussi haut ?"},
	{0, "Genre !"},
	{0, "Putain !"},
	{0, ""}
};

int ia_speak_to(aPlayer *pl, char *nick, struct ia_speak_tab *speak)
{
	int i, r;
	char s[(250+NICKLEN+2)];

	if(!IASpeak(pl->game) || (pl && !IsIA(*pl))) return 0;

	for(i=0;speak[i].msg[0] != '\0';i++) if(speak[i].rest < 0) speak[i].rest++;

	do {
		r = rand() % i;
	}
	while(speak[r].rest < 0);

	speak[r].rest = -3;

	sprintf(s, "%s: %s", nick, speak[r].msg);

	sendto_game(NULL, pl->game, MSG_MSG, pl->IAname, s);

	return 0;
}

int ia_speak(aPlayer *pl, struct ia_speak_tab *speak)
{
	int i, r;

	if(!IASpeak(pl->game) || !IsIA(*pl)) return 0;

	for(i=0;speak[i].msg[0] != '\0';i++) if(speak[i].rest < 0) speak[i].rest++;

	do {
		r = rand() % i;
	}
	while(speak[r].rest < 0);

	speak[r].rest = -3;

	sendto_game(NULL, pl->game, MSG_MSG, pl->IAname, speak[r].msg);

	return 0;
}

int ia_change(aPlayer *pl, int nb, int meilleurs)
{
	int i, j, k, endchange = 1;
	aPlayer *p = NULL;
	aPlayer *beginer = NULL;
	aGame *g = pl->game;
	char cartes[6], don[3][3];

	for(k = 0; k < MAXPLAYERS; k++)
	{
			if((pl->grade == G_TROUDUK && g->player[k].grade == G_PRESIDENT) ||
			   (pl->grade == G_VTROUDUK && g->player[k].grade == G_VPRESIDENT) ||
			   (pl->grade == G_VPRESIDENT && g->player[k].grade == G_VTROUDUK) ||
			   (pl->grade == G_PRESIDENT && g->player[k].grade == G_TROUDUK))
					p = &g->player[k];
			if(pl != &g->player[k] && IsChange(g->player[k])) endchange = 0;
			if(g->player[k].grade == G_TROUDUK) beginer = &g->player[k];
	}
	if(!p) return 0;

	for(i=0; i < nb; i++)
	{
		int value = meilleurs ? 0 : BestCarte(g);

		for(j=0; j <= pl->lastcarte; j++) /* essaye sans casser de paires */
			if((meilleurs && pl->carte[j].value > value) ||
			   (!meilleurs && pl->carte[j].value < value &&
				pl->carte[j].value <= V_VALET && pl->carte[j].nb < 2))
			{
				strcpy(don[i], pl->carte[j].name);
				value = pl->carte[j].value;
			}
		if(value == BestCarte(g)) /* si il n'a que des paires, les casser (sauf triples ou quadruples bien sur) */
			for(j=0; j <= pl->lastcarte; j++)
				if((meilleurs && pl->carte[j].value > value) ||
				   (!meilleurs && pl->carte[j].value < value && pl->carte[j].nb < 3))
				{
					strcpy(don[i], pl->carte[j].name);
					value = pl->carte[j].value;
				}

		DelCarte(pl, don[i]);
		AddCarte(p, don[i]);
		if(i)
		{
			strcat(cartes, ",");
			strcat(cartes, don[i]);
		}
		else
			strcpy(cartes, don[i]);
	}

	printf("%s donne à %s: %s\n", GetIdent(pl), GetIdent(p), cartes);
	if(!IsIA(*p)) sendrpl(p->user, MSG_DONNE, GetIdent(pl), cartes);
	pl->flag &= ~P_CHANGE;

	if(!endchange || !beginer) return 0;

	DelInChange(g);
	SetInGame(g);

	beginer->flag |= P_TOHIM;
	sendto_players(beginer->user, g, MSG_BEGIN, GetIdent(beginer));
	if(!(beginer->flag & P_IA)) sendrpl(beginer->user, MSG_TOYOU);
	else ia_joue(beginer);

	return 0;
}

int ia_joue(aPlayer *pl)
{
	int i, end = 0, k = 0, valmin = 0, nbcartes = 0, maxdbl = 0;
	aCarte *c = calloc(1, sizeof *c);
	aGame *g = pl->game;
	aPlayer *back = NULL;
	char tosend[(NbCartes(g)*2+10)];
	int nb_as = 0, best = 0, j, val_as = 0;

	c->value = 0;
	for(i = 0;i < NbCartes(g);i++)
	{
		if(!pl->carte[i].value) break;
		if(k)
		{
			strcat(tosend, ",");
			strcat(tosend, pl->carte[i].name);
		}
		else
		{
			strcpy(tosend, pl->carte[i].name);
			k++;
		}
	}
	printf("cartes: %s\n", tosend);
	pl->flag &= ~P_TOHIM;

	if(pl->flag & P_CANT || IsEnd(*pl))
	{
		free(c);
		printf("- ne peut pas jouer\n");
		next_player(pl);
		return 0;
	}

	for(i=0;i<4;i++)
	{
		if(g->player[i].flag & P_MAITRE)
		{
			back = &g->player[i];
			break;
		}
	}

	if(back)
	{
		valmin = back->played[0].value;
		nbcartes = back->played[0].nb;
	}

	for(i=NbCartes(g); i>0; i--)
		if(g->stats->nb[i])
		{
			k = g->stats->nb[i];
			for(j=0; j <= pl->lastcarte; j++)
				if(pl->carte[i].value == k)
					k--;
			if(k)
			{
				best = i;
				break;
			}
		}

	if(NbPlayers(g) == 2 && !back)
	{
		aPlayer *adv = NULL;

		for(i=0;i < MAXPLAYERS;i++)
			if(!IsEnd(g->player[i]) && i != pl->place)
			{
				adv = &g->player[i];
				break;
			}
		if(!adv)
			putlog(ERREURS_LOG, "ia_joue(): deux joueurs mais impossible de trouver le second !\n");
		else if((adv->lastcarte+1) == 1)
		{ /* Une carte restante */
				for(i=0;i <= pl->lastcarte;i++)
		 		{
		 			if(((!c->value) || (c->nb < pl->carte[i].nb) ||
						(c->nb == pl->carte[i].nb && c->value > pl->carte[i].value))
						&& pl->carte[i].nb >= 2)
		 			{
		 				strcpy(c->name, pl->carte[i].name);
		 				c->value = pl->carte[i].value;
		 				c->nb = pl->carte[i].nb;
		 				end = 1;
		 			}
				}
				if(!c->value) /* Pas de paires */
					for(i=0;i <= pl->lastcarte;i++)
			 		{
			 			if(!c->value || (c->nb < pl->carte[i].nb) ||
							(c->value < pl->carte[i].value))
			 			{
			 				strcpy(c->name, pl->carte[i].name);
			 				c->value = pl->carte[i].value;
			 				c->nb = pl->carte[i].nb;
			 				end = 1;
			 			}
					}
		}
		else if((adv->lastcarte+1) == 2 && (pl->lastcarte+1) > 2)
		{ /* Deux cartes restantes */
			if(!(rand()%2))
			{ /* Suppose que c'est une paire */
				printf("- SUPPOSE PAIRE\n");
				for(i=0;i <= pl->lastcarte;i++)
		 		{
		 			if(pl->carte[i].nb == 1 && (!c->value || c->value > pl->carte[i].value))
		 			{
		 				strcpy(c->name, pl->carte[i].name);
		 				c->value = pl->carte[i].value;
		 				c->nb = 1;
		 				end = 1;
		 			}
				}
				if(!end) /* n'a aucune simple, va donc casser des doubles */
					for(i=0;i <= pl->lastcarte;i++)
			 		{
			 			if(!c->value || (c->nb < pl->carte[i].nb) ||
							(c->value > pl->carte[i].value))
			 			{
			 				strcpy(c->name, pl->carte[i].name);
			 				c->value = pl->carte[i].value;
			 				c->nb = pl->carte[i].nb > 2 ? pl->carte[i].nb : 1;
			 				end = 1;
			 			}
					}
			}
			else
			{ /* Suppose que ce sont deux simples */
				printf("- SUPPOSE SIMPLES\n");
				for(i=0;i <= pl->lastcarte;i++)
		 		{
		 			if((!c->value && pl->carte[i].nb >= 2) || (c->nb < pl->carte[i].nb) ||
						(c->nb == pl->carte[i].nb && c->value > pl->carte[i].value &&
						  (((pl->lastcarte+1) <= 3) || pl->carte[i].value < V_ROI)))
		 			{
		 				strcpy(c->name, pl->carte[i].name);
		 				c->value = pl->carte[i].value;
		 				c->nb = pl->carte[i].nb;
		 				end = 1;
		 			}
				}
				if(!c->value) /* Pas de paires */
					for(i=0;i <= pl->lastcarte;i++)
			 		{
			 			if(!c->value || (c->nb < pl->carte[i].nb) ||
							(c->nb == pl->carte[i].nb && c->value > pl->carte[i].value))
			 			{
			 				strcpy(c->name, pl->carte[i].name);
			 				c->value = pl->carte[i].value;
			 				c->nb = pl->carte[i].nb;
			 				end = 1;
			 			}
					}
			}
		}
	}
	if(!end) for(end=0,k=0;!end;k++)
	{
		if(k == 1) /* première boucle: collecte d'infos. */
			for(i=0;i <= pl->lastcarte;i++) /*pour back: recherche pour surcoucher (bon nombre)*/
			{
				if(maxdbl < pl->carte[i].nb && pl->carte[i].value < best)
					maxdbl = pl->carte[i].nb;
				if(pl->carte[i].value >= best && pl->carte[i].nb > nb_as && pl->carte[i].value > valmin
				  && (!back || pl->carte[i].nb >= nbcartes))
				{
					nb_as = pl->carte[i].nb;
					val_as = pl->carte[i].value;
				}
				if(back && (pl->carte[i].nb == nbcartes && pl->carte[i].value > valmin) &&
                                   (pl->carte[i].value < V_ROI || pl->carte[i].nb >= pl->lastcarte))
				{
					if(!c->value || pl->carte[i].value < c->value)
					{
						printf("- je fais pipi ici [%s] [%d][%d] & [%d][%d]\n", pl->carte[i].name, pl->carte[i].nb, nbcartes, pl->carte[i].value, valmin);
						strcpy(c->name, pl->carte[i].name);
						c->value = pl->carte[i].value;
						c->nb = pl->carte[i].nb;
						end = 1;
					}
				}
			}
		else if(k == 2 && nb_as && (pl->lastcarte+1) <= (nb_as + maxdbl))
		{
			for(i=0;i<=pl->lastcarte;i++)
				if(pl->carte[i].value == val_as)
				{
					printf("- et oui on passe par là [%s] [%d][%d] & [%d][%d]\n", pl->carte[i].name, pl->carte[i].nb, nbcartes, pl->carte[i].value, valmin);
					strcpy(c->name, pl->carte[i].name);
					c->value = pl->carte[i].value;
					c->nb = back ? nbcartes : pl->carte[i].nb;
					end = 1;
				}
		}
		else if(k == 3 && back && /* cassage possible si il a un triple ou un quadruple */
				maxdbl > 2 && (pl->lastcarte+1) <= (nbcartes+maxdbl+1))
                	for(i=0;i <= pl->lastcarte;i++)
			{
				if(pl->carte[i].nb >= nbcartes && pl->carte[i].value > valmin)
				{
					if(!c->value || pl->carte[i].value < c->value)
					{
						printf("- je fais pipi ici [%s] [%d][%d] & [%d][%d]\n", pl->carte[i].name, pl->carte[i].nb, nbcartes, pl->carte[i].value, valmin);
						strcpy(c->name, pl->carte[i].name);
						c->value = pl->carte[i].value;
						c->nb = nbcartes;
						end = 1;
					}
				}
			}
		else if(k == 4 && back) /* recherche pour surcoucher (cassage de doublons) */
			for(i=0;i <= pl->lastcarte;i++)
			{
				if((pl->carte[i].nb >= nbcartes && pl->carte[i].value > valmin) &&
                                   (pl->carte[i].value < V_ROI || pl->carte[i].nb >= pl->lastcarte))
				{
					printf("- je prout là [%s] [%d][%d] & [%d][%d]\n", pl->carte[i].name, pl->carte[i].nb, nbcartes, pl->carte[i].value, valmin);
					if(!c->value || pl->carte[i].value < c->value)
					{
						printf("- je suis là5\n");
						strcpy(c->name, pl->carte[i].name);
						c->value = pl->carte[i].value;
						c->nb = nbcartes;
						end = 1;
					}
				}
			}
		else if(k == 5 && !back)            /* recherche pour commencer avec un nombre   */
			for(i=0;i <= pl->lastcarte;i++) /* le plus bas et le plus de cartes possible */
			{
				printf("- je pète gna [%s] [%d][%d] & [%d][%d]\n", pl->carte[i].name, pl->carte[i].nb, nbcartes, pl->carte[i].value, valmin);
				if((!c->value || (c->nb < pl->carte[i].nb) ||
				   (c->nb == pl->carte[i].nb && c->value > pl->carte[i].value)) &&
                                   (pl->carte[i].value < V_ROI || pl->carte[i].nb >= pl->lastcarte))
				{
					strcpy(c->name, pl->carte[i].name);
					c->value = pl->carte[i].value;
					c->nb = pl->carte[i].nb;
					end = 1;
				}
			}
		else if(k == 6 && !back)
			for(i=0;i <= pl->lastcarte;i++) /* le plus bas et le plus de cartes possible */
		 	{
		 				printf("- je  prout gna [%s] [%d][%d] & [%d][%d]\n", pl->carte[i].name, pl->carte[i].nb, nbcartes, pl->carte[i].value, valmin);
		 				if(!c->value || (c->nb < pl->carte[i].nb) ||
							(c->nb == pl->carte[i].nb && c->value > pl->carte[i].value))
		 				{
		 					strcpy(c->name, pl->carte[i].name);
		 					c->value = pl->carte[i].value;
		 					c->nb = pl->carte[i].nb;
		 					end = 1;
		 				}
			}
		else if(k == 7) break;
	}
	if(!end) /* NE PEUT PAS JOUER */
	{
		pl->flag |= P_CANT;
		sendto_players(NULL, g, MSG_CANT, GetIdent(pl));
	}
	else /* JOUE SES CARTES */
	{
		char adonner[(NbCartes(g)*2+10)];

		for(i=0, k=0; i <= pl->lastcarte; i++)
		{
			if(pl->carte[i].value == c->value)
			{
				if(k < c->nb)
				{
					int l;
					if(k)
					{
						strcat(adonner, ",");
						strcat(adonner, pl->carte[i].name);
					}
					else
						strcpy(adonner, pl->carte[i].name);
					strcpy(pl->played[k].name, pl->carte[i].name);
					pl->played[k].value = pl->carte[i].value;
					pl->played[k].nb = c->nb;

					g->stats->nb[pl->carte[i].value]--;
					if(!g->stats->nb[pl->carte[i].value])
						for(l=BestCarte(g);l>0;l--)
							if(g->stats->nb[l])
								{ g->stats->best = l; break; }

					DelICarte(pl, i);
					i--;
					k++;
				}
			}
		}
		sendto_players(NULL, pl->game, MSG_PLAY, GetIdent(pl), adonner);
		if(valmin == (c->value-1) && valmin)
			ia_speak(pl, ia_speak_parfait);

		if(valmin <= V_NEUF && c->value >= V_DAME && valmin)
		{
			int r;

			i=0;
			do
			{
				r = rand()%3;
				i++;
			} while (!IsIA(g->player[r]) && &g->player[r] != pl && i < MAXPLAYERS);
			if(r) ia_speak_to(&g->player[r], pl->IAname, ia_speak_trophaut);
		}
		pl->score += (puiss(2,c->nb) * c->value);
		if(pl->lastcarte == -1)
		{
			SetEnd(*pl);
			sendto_game(NULL, g, MSG_WIN, GetIdent(pl), g->iwin);
			pl->grade = g->iwin++;
			if(pl->grade == 1)
			{
				ia_speak(pl, ia_speak_win);
				for(i=0;i<MAXPLAYERS;i++)
					if(g->player[i].grade == 1 && i != pl->place)
						ia_speak_to(&g->player[i], pl->IAname, ia_speak_detroned);
			}
			else ia_speak(pl, ia_speak_wose);
			if(g->iwin == 4)
			{
				free(c);
				return end_of_game(g);
			}
		}
		for(i=0;i<4;i++)
			if(IsMaitre(g->player[i]))
				DelMaitre(g->player[i]);
		pl->flag |= P_MAITRE;
	}
	free(c);
	next_player(pl);
	return 0;
}
