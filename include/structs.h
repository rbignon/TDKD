/* include/structs.h - Structures
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

typedef struct serverinfo {
	int sock;
	int CONtotal;
	int CONact;
	int GAMEtotal;
	time_t uptime;
} aServer;

typedef struct statinfo {
	int score;
	int grades[5]; /* 1 -> 4, president -> trouduk */
	int nbparties;
} aStats;

typedef struct userinfo {
	char nick[NICKLEN];
	char pass[16];
	char mail[101];
	struct clientinfo *cl;
	aStats *stats;
	unsigned int flag;
#define U_ADMIN 0x01
	struct userinfo *next;
	struct userinfo *last;
} anUser;

typedef struct configinfo {
	int port;
	char server[100 + 1];
	char motd_file[200];
	int flag;
#define C_FIRST 0x01
} aConf;

typedef struct clientinfo {
	char nick[NICKLEN + 1];
	int fd;
	unsigned int flag;	/* divers infos */
#define TDKD_AUTH 	0x01
#define TDKD_FREE 	0x02
#define TDKD_FLUSH 	0x04
#define TDKD_PING 0x08
#define TDKD_INLOGIN 0x10
	char ip[16];
	size_t buflen;
	size_t recvlen;
	time_t lastread;
	char QBuf[TDKD_SENDSIZE+1];
	char RecvBuf[TDKD_RECVSIZE+1];
	struct playerinfo *player;
	struct userinfo *user;
	struct clientinfo *next;
} aClient;

typedef struct carteinfo {
	char name[3];
	int value;
#define V_TROIS 1
#define V_QUATRE 2
#define V_CINQ 3
#define V_SIX 4
#define V_SEPT 5
#define V_HUIT 6
#define V_NEUF 7
#define V_DIX 8
#define V_VALET 9
#define V_DAME 10
#define V_ROI 11
#define V_AS 12
#define V_DEUX 13
	int nb;
} aCarte;

typedef struct playerinfo {
#define G_PRESIDENT 1
#define G_VPRESIDENT 2
#define G_VTROUDUK 3
#define G_TROUDUK 4
	int grade; /* 1-president, 2-vice president, 3-vice trou du cul, 4-trou du cul */
	int place; /* game->players[i].place = i */
	int score;
	unsigned int flag;
#define P_OWNER		0x001
#define P_MAITRE	0x002
#define P_CANT		0x004
#define P_TOHIM		0x008
#define P_END		0x020
#define P_WAIT		0x040
#define P_CHANGE	0x080
#define P_IA		0x200
	aCarte played[4]; /* carte jouée */
	aCarte carte[15 + 1]; /* cartes en sa possession */
	char IAname[NICKLEN + 1];
	aStats *stats;
	int lastcarte;
	struct gameinfo *game;
	struct clientinfo *user;
} aPlayer;

typedef struct statsinfo {
	int nb[14];
	int best; /* 1 à 13 */
} aStat;

typedef struct gameinfo {
	char name[GAMELEN + 1];
	char pass[PASSLEN + 1];
	int reflex;
	int nbplayers;
	int iwin; /* indicateur de position de gagnants */
	aStat *stats;
	unsigned int flag;
#define G_INGAME	0x001
#define G_NOGRADE	0x002
#define G_INCHANGE	0x004
#define G_PASSWD	0x008
#define G_STOP		0x020
#define G_IASPEAK	0x040
#define G_52C		0x080
	struct playerinfo player[MAXPLAYERS];
	struct gameinfo *next;
	struct gameinfo *last;
} aGame;
