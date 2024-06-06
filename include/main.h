/* include/main.h - Declarations principales
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#undef DEBUG

/* Constantes d'infos globales */
#define SERVER_VERSION "TDKD1.0"
#define PROTOCOLE_VERSION 7 /* version du protocole, incrémenter en cas de modification importante
                             * du protocole qui rendrait le client incompatible */

/* fichiers */
#define USERS_FILE "database/users.db"
#define USERVERSION 2

/* logs */
#define PARSES_LOG "logs/parses.log"
#define ERREURS_LOG "logs/erreurs.log"

/* Constantes de longueurs */
#define NICKLEN 15
#define GAMELEN 20
#define CMDLEN 15
#define PASSLEN 20
#define MAXPLAYERS 4 /* Ne SURTOUT *PAS* changer ! Le client ne supporte que 4 users ! */
#define PINGINTERVAL 30

/* Constantes de sockets, ne pas toucher */
#define TDKD_SENDSIZE 1024
#define TDKD_RECVSIZE 256
#define MAXCONNEX 100 /* de préférence un multiple de 4 */
#define MAXUSERS 100 /* augmenter si nécessaire */
#define TCPWINDOWSIZE 513 /* à augmenter si le réseau augmente de taille */

/* Messages */
#define MSG_SERVER "SERVER %s :%s" /* message de bienvenue (nom + version) */
#define MSG_ERREUR "ERREUR %s :%s" /* message d'erreur */
#define MSG_CONNECTED "CONNECTED :%s" /* notification de connexion */
#define MSG_LIST "LIST %s %d %d %d :%s" /* <nom> <52c?> <places libres> <en jeu?> :<joueurs> */
#define MSG_LISTEND "LISTEND" /* fin de la liste */
#define MSG_JOINED "JOINED %s %s" /* notification de join (jeu, ident) */
#define MSG_WAIT "WAIT %s %s" /* notification de join sur attente de fin de jeu (jeu, nicks) */
#define MSG_JOIN ":%s JOIN %s %d" /* join d'un user (login, ident, ia ? 1 : 0) */
#define MSG_PART ":%s PART" /* départ d'un connard (ident) */
#define MSG_KICK ":%s KICK %s" /* quelqu'un est kické (ident, victime) */
#define MSG_RULES "RULES %d %d" /* regles (reflexiontime, 52c?1:0) */
#define MSG_DONE "DONE %s" /* distribue les cartes */
#define MSG_TOYOU "TOYOU" /* indique que c'est à ce connard de jouer */
#define MSG_PLAY ":%s PLAY %s" /* quelqu'un joue (ident, cartes) */
#define MSG_PERSONNEPEU ":%s PERSONNEPEU" /* remporte le pli (ident) */
#define MSG_WIN ":%s WIN %d" /* un mec gagne (ident, grade) */
#define MSG_MSG ":%s MSG :%s" /* recoi un message */
#define MSG_PING "PING" /* ping ?*/
#define MSG_PONG "PONG" /* Pong !*/
#define MSG_QUIT "QUIT" /* confirmation du départ */
#define MSG_ADONNER "ADONNER %d%s" /* les cartes que l'user a à donner */
#define MSG_DONNE ":%s DONNE %s" /* cartes qu'un joueur donne à un autre */
#define MSG_CANT ":%s CANT" /* anonce qu'un joueur ne peut pas (ident) */
#define MSG_BEGIN ":%s BEGIN" /* anonce qu'un joueur commence (ident) */
#define MSG_ENDOFGAME "ENDOFGAME %s :%s" /* fin d'une partie (idents des joueurs) */
#define MSG_SERVERINFO "SERVERINFO %d %d %d %d :%s" /* infos serveur (nombre de connectés,
											   * connexions totales, nombre parties, tusers, uptime) */
#define MSG_DIE "DIE :%s" /* le serveur se fait kill (reason) */
#define MSG_WALL ":%s WALL :%s" /*message global */
#define MSG_MOTD "MOTD :%s" /* message du jour */
#define MSG_ENDOFMOTD "ENDOFMOTD" /* fin du motd */
#define MSG_WAITJOIN ":%s WAITJOIN %s" /* un waiter join (login + nom de l'ia qu'il va remplacer)*/
#define MSG_LOGIN "LOGIN %d" /* logué ! (admin ? 1 : 0) */
#define MSG_MUSTLOGIN "MUSTLOGIN" /* doit de loguer */
#define MSG_KILL ":%s KILL :%s" /* vous vous etes fait kill */
#define MSG_USERLIST "USERLIST %s %d %d %s" /* liste un user (nick, admin ? 1 : 0, grade, jeu) */
#define MSG_ENDOFUL "ENDOFUL" /* fin de l'userlist */
#define MSG_STOP ":%s STOP %d" /* stop game (ident, 0/1) */
#define MSG_STATS_RESETED "STATS RESETED" /* prouve que les stats ont bien été réinitialisées */
#define MSG_STATS_SHOW "STATS SHOW %d:%d:%d:%d:%d:%d" /* affiche les stats d'un joueur */
#define MSG_STATS_TOP5 "STATS TOP5 :%s" /* affiche le top5 */
#define MSG_REHASHED "REHASHED" /* a bien rehashé le serveur */

/* ERREURS HUMAINES */
#define ERR_001 "001 %s :Pseudonyme déjà pris"
#define ERR_002 "002 %s :Un jeu porte déjà ce nom là"
#define ERR_003 "003 %s :Le jeu n'existe pas"
#define ERR_004 "004 %s :Plus de places"
#define ERR_005 "005 %s :Pseudonyme incorrect"
#define ERR_006 "006 %s :Vous devez etre owner pour distribuer"
#define ERR_007 "007 %s :Vous n'êtes pas en train de jouer"
#define ERR_008 "008 %s :Vous être déjà en train de jouer"
#define ERR_009 "009 %s :Une partie est déjà en cours"
#define ERR_010_P "010 + :Veuillez mettre à jour votre client"
#define ERR_010_M "010 - :Veuillez mettre à jour votre client"
#define ERR_011 "011 %s :Mot de pass incorrect"
#define ERR_012 "012 %s :User introuvable"

/* Macros générales */
#define ASIZE(x) 				(sizeof (x) / sizeof *(x))

#include "structs.h"

/* Variables (très) globales */
extern time_t CurrentTS;
extern int running;
extern char conf_file[];
extern struct serverinfo server;
extern struct configinfo config;
extern struct gameinfo *gamehead;
extern struct userinfo *userhead;

/* Macros flags */

/* - Client */
#define SetFlush(x) 	(x)->flag |= TDKD_FLUSH
#define SetPing(x)		(x)->flag |= TDKD_PING
#define SetInLogin(x)	(x)->flag |= TDKD_INLOGIN
#define DelFlush(x) 	(x)->flag &= ~TDKD_FLUSH
#define DelInLogin(x)	(x)->flag &= ~TDKD_INLOGIN
#define SetAuth(x)	(x)->flag |= TDKD_AUTH
#define DelPing(x)		(x)->flag &= ~TDKD_PING
#define IsFlush(x) 	((x)->flag & TDKD_FLUSH)
#define IsPing(x)	((x)->flag & TDKD_PING)
#define IsAuth(x)	((x)->flag & TDKD_AUTH)
#define IsInLogin(x) 	((x)->flag & TDKD_INLOGIN)

/* - User */
#define IsAdmin(x)	((x)->flag & U_ADMIN)
#define SetAdmin(x)	(x)->flag |= U_ADMIN

/* - Player */
#define SetOwner(x)		(x).flag |= P_OWNER
#define SetMaitre(x)	(x).flag |= P_MAITRE
#define SetCant(x)		(x).flag |= P_CANT
#define SetIA(x)		(x).flag |= P_IA
#define SetToHim(x)		(x).flag |= P_TOHIM
#define SetEnd(x)		(x).flag |= P_END
#define SetWait(x)		(x).flag |= P_WAIT
#define SetChange(x)	(x).flag |= P_CHANGE
#define DelOwner(x)		(x).flag &= ~P_OWNER
#define DelMaitre(x)	(x).flag &= ~P_MAITRE
#define DelCant(x)		(x).flag &= ~P_CANT
#define DelIA(x)		(x).flag &= ~P_IA
#define DelToHim(x)		(x).flag &= ~P_TOHIM
#define DelEnd(x)		(x).flag &= ~P_END
#define DelWait(x)		(x).flag &= ~P_WAIT
#define DelChange(x)	(x).flag &= ~P_CHANGE
#define IsOwner(x)		((x).flag & P_OWNER)
#define IsMaitre(x)		((x).flag & P_MAITRE)
#define IsCant(x)		((x).flag & P_CANT)
#define IsIA(x)			((x).flag & P_IA)
#define IsToHim(x)		((x).flag & P_TOHIM)
#define IsEnd(x)		((x).flag & P_END)
#define IsWait(x)		((x).flag & P_WAIT)
#define IsChange(x)		((x).flag & P_CHANGE)
#define IsUser(x)		((x).user && (x).user->user && !IsIA(x))

/* - Game */
#define SetInGame(x)	(x)->flag |= G_INGAME
#define SetNoGrade(x)	(x)->flag |= G_NOGRADE
#define SetInChange(x)	(x)->flag |= G_INCHANGE
#define SetPass(x)		(x)->flag |= G_PASSWD
#define SetGameStop(x)	(x)->flag |= G_STOP
#define SetIASpeak(x)	(x)->flag |= G_IASPEAK
#define Set52C(x)		(x)->flag |= G_52C
#define DelInGame(x)	(x)->flag &= ~G_INGAME
#define DelNoGrade(x)	(x)->flag &= ~G_NOGRADE
#define DelInChange(x)	(x)->flag &= ~G_INCHANGE
#define DelPass(x)		(x)->flag &= ~G_PASSWD
#define DelGameStop(x)	(x)->flag &= ~G_STOP
#define DelIASpeak(x)	(x)->flag &= ~G_IASPEAK
#define Del52C(x)		(x)->flag &= ~G_52C
#define IsInGame(x)		((x)->flag & G_INGAME)
#define IsNoGrade(x)	((x)->flag & G_NOGRADE)
#define IsInChange(x)	((x)->flag & G_INCHANGE)
#define Playing(x)		(IsInGame(x) || IsInChange(x))
#define IsPass(x)		((x)->flag & G_PASSWD)
#define IsGameStop(x)	((x)->flag & G_STOP)
#define IASpeak(x)		((x)->flag & G_IASPEAK)
#define NbPlayers(x)	(MAXPLAYERS - (x)->iwin + 1)
#define Is52C(x)		((x)->flag & G_52C)
#define BestCarte(x)	(Is52C(x) ? V_DEUX : V_AS)
#define NbCartes(x)		(Is52C(x) ? 13 : 8)

/* - Config */
#define SetFirst(x)		(x).flag |= C_FIRST
#define DelFirst(x)		(x).flag &= ~C_FIRST
#define IsFirst(x)		((x).flag & C_FIRST)
