/* src/serveur.c - Commandes serveur
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

#ifdef USEBSD
#include <netinet/in.h>
#endif
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include "main.h"
#include "infos.h"
#include "divers.h"
#include "serveur.h"
#include "gamecmds.h"
#include "database.h"
#include "admin_cmds.h"
#include "outils.h"

#define MAXPARAM 15

int highsock = 0;
fd_set global_fd_set;
struct clientinfo myUser[MAXCONNEX+1];

static struct commands {
   char cmd[CMDLEN + 1];
   int (*func) (aClient *, int, char **);
   int status; /* -1 = in login, 0 = nothing, 1 = user, 2 = admin */
   int args;
} cmds[] = {
	{"nick",        change_nick,	0, 1},
	{"list",        game_list,      0, 0},
	{"create",      create_game,    0, 4},
	{"join",        join_game,      0, 1},
	{"distribue",   distribuer,     0, 0},
	{"msg",         send_msg,       0, 1},
	{"quit",        bybye,         -1, 0},
	{"part",        part_game,      0, 0},
	{"pong",        m_pong,        -1, 0},
	{"ping",        m_ping,        -1, 0},
	{"peutpas",     user_cant,      0, 0},
	{"donner",      donner_cartes,  0, 1},
	{"play",        user_play,      0, 1},
	{"serverinfo",  server_info,    0, 0},
	{"kick",        kick_user,      0, 1},
	{"die",         die_server,     2, 1},
	{"wall",        wall_msg,       2, 1},
	{"auth",        user_auth,     -1, 1},
	{"register",   register_account,0, 2},
	{"kill",        kill_user,      2, 2},
	{"userlist",    user_list,      2, 0},
	{"stop",        stop_game,      0, 0},
	{"stats",       show_stats,     1, 1},
	{"web",         web_connec,    -1, 0},
	{"rehash",      rehash_server,  2, 0}
};

int m_pong(aClient *cl, int parc, char **parv)
{
	if(IsPing(cl)) DelPing(cl);
	return 0;
}

int m_ping(aClient *cl, int parc, char **parv)
{
	sendrpl(cl, MSG_PONG);
	return 0;
}

static inline int dequeue(aClient *cl)
{
	send(cl->fd, cl->QBuf, cl->buflen, 0);
	DelFlush(cl);
	cl->buflen = 0;
	return 0;
}

char *correct_nick(char *nick)
{
	static char newnick[NICKLEN + 1];
	int i = 0;

	while(*nick && i < NICKLEN)
	{
		if(*nick == '@') newnick[i++] = 'a';
		else if(*nick == ' ') newnick[i++] = '_';
		else if(isalnum(*nick) || *nick == '-' || *nick == '_') newnick[i++] = *nick;
		*nick++;
	}
	newnick[i] = '\0';
	return newnick;
}

/* Connexion web */
int web_connec(aClient *cl, int parc, char **parv)
{
	server_info(cl, parc, parv);
	exit_client(cl, MSG_QUIT);
	return 0;
}

/* Connexion user */
int change_nick(aClient *cl, int parc, char **parv)
{
	char *newnick = NULL;
	int i;
	anUser *u = userhead;

	if(IsAuth(cl)) return sendrpl(cl, MSG_ERREUR, parv[0], "Possède déjà un pseudo");

	if(!is_num(parv[1]))
	{
		char tmp[100];
		sprintf(tmp, MSG_ERREUR, "Connexion impossible", "Veuillez mettre votre version à jour");
		return exit_client(cl, tmp);
	}
	else if(atoi(parv[1]) > PROTOCOLE_VERSION)
		return exit_client(cl, ERR_010_P);
	else if(atoi(parv[1]) < PROTOCOLE_VERSION)
		return exit_client(cl, ERR_010_M);

	newnick = correct_nick(parv[2]);
	if(!strcmp(newnick, "")) return sendrpl(cl, ERR_005, parv[1]);

	for(i = 0; i <= highsock; i++)
		if(!strcasecmp(myUser[i].nick, newnick))
			return sendrpl(cl, ERR_001, myUser[i].nick);

	strcpy(cl->nick, newnick);
	SetAuth(cl);
	server.CONtotal++;
	server.CONact++;

	for(;u;u=u->next)
		if(!strcasecmp(cl->nick, u->nick))
			break;

	if(u) sendrpl(cl, MSG_MUSTLOGIN), SetInLogin(cl);
	else
	{
		show_motd(cl);
		sendrpl(cl, MSG_CONNECTED, cl->nick);
	}


	return 1;
}

void sig_alarm(int c)
{
	int i;

	CurrentTS = time(NULL);
	for(i = 0; i<= highsock;i++)
	{
		if(myUser[i].flag & TDKD_FREE) continue;
		if(IsPing(&myUser[i]))
		{
			DelPing(&myUser[i]);
			exit_client(&myUser[i], MSG_QUIT " :Ping Timeout");
		}
		else if((myUser[i].lastread + PINGINTERVAL) <= CurrentTS)
		{
			sendrpl(&myUser[i], MSG_PING);
			SetPing(&myUser[i]);
		}
	}

	alarm(PINGINTERVAL);
}

int findcmd(const char *cmd)
{
	unsigned int i = 0;
	for(;i < ASIZE(cmds);++i)
		if(!strcasecmp(cmds[i].cmd, cmd)) return i;
	return -1;
}

int exit_client(aClient *cl, const char *msg)
{
	if(msg && *msg) sendrpl(cl, msg);

	delclient(cl);
	return 0;
}

/* parc est le nombre de cases de parv[], soit parv[parc-1] = dernier argument*/
int parsemsg(aClient *cl)
{
	char *parv[MAXPARAM + 1];
	register char *ptr;
	int cmd, parc = 1;

	parv[0] = cl->RecvBuf; /* cmd */
	if((ptr = strchr(cl->RecvBuf, ' ')) || (ptr = strchr(cl->RecvBuf, '\0')))
		*ptr++ = 0;
	else sendrpl(cl, MSG_ERREUR, "parsemsg()", "Impossible de trouver la commande");

	if((cmd = findcmd(parv[0])) < 0)
		return sendrpl(cl, MSG_ERREUR, parv[0], "Commande inconnue");

	while(*ptr) /* split buf in a char * array */
	{
		while(*ptr == ' ') *ptr++ = 0; /* drop spaces */
		if(*ptr == ':') /* last param*/
		{
			parv[parc++] = ptr + 1;
			break;
		}
		if(!*ptr || parc >= MAXPARAM) break;
		else parv[parc++] = ptr;
		while(*ptr && *ptr != ' ') ++ptr; /* go through token till end or space */
	}
	parv[parc] = NULL; /* ends array with NULL */

	if((IsInLogin(cl) && cmds[cmd].status != -1) || (cmds[cmd].status >= 1 && !cl->user) ||
	   (cmds[cmd].status >= 2 && !IsAdmin(cl->user)))
		return sendrpl(cl, MSG_ERREUR, parv[0], "Permissions incorrectes");
	if(parc <= cmds[cmd].args) return sendrpl(cl, MSG_ERREUR " [%s]", parv[0], "Syntaxe incorrecte", cl->RecvBuf);
	cmds[cmd].func(cl, parc, parv);

	return 0;
}

/* cl est une exception, mettre à NULL pour aucune exception */
int sendto_players(aClient *cl, aGame *g, const char *pattern, ...)
{
	static char buf[512];
	va_list vl;
	int len, i;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf - 2, pattern, vl); /* format */
	if(len < 0) len = sizeof buf -2;

	buf[len++] = '\n';
	buf[len] = 0;
	va_end(vl);

	for(i = 0; i < MAXPLAYERS;i++)
	{
		if(IsWait(g->player[i]) || IsIA(g->player[i]) ||
		   g->player[i].user == NULL || (cl && g->player[i].user == cl)) continue;

#ifdef DEBUG
		printf("S(%s@%s) - %s", g->player[i].user->nick, g->player[i].user->ip, buf);
		putlog(PARSES_LOG, "S(%s@%s) - %s", g->player[i].user->nick, g->player[i].user->ip, buf);
#endif

		if(g->player[i].user->buflen + len >= TDKD_SENDSIZE) dequeue(g->player[i].user); /* would overflow, flush it */
		strcpy(g->player[i].user->QBuf + g->player[i].user->buflen, buf); /* append reply to buffer */
		g->player[i].user->buflen += len;
		/*if(IsFlush(cl)) */dequeue(g->player[i].user); /* Is flush forced ? */
	}
	return 0;
}

/* cl est une exception, mettre à NULL pour aucune exception */
int sendto_game(aClient *cl, aGame *g, const char *pattern, ...)
{
	static char buf[512];
	va_list vl;
	int len, i;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf - 2, pattern, vl); /* format */
	if(len < 0) len = sizeof buf -2;

	buf[len++] = '\n';
	buf[len] = 0;
	va_end(vl);

	for(i = 0; i < MAXPLAYERS;i++)
	{
		if(g->player[i].user == NULL || (cl && g->player[i].user == cl)) continue;

#ifdef DEBUG
		printf("S(%s@%s) - %s", g->player[i].user->nick, g->player[i].user->ip, buf);
		putlog(PARSES_LOG, "S(%s@%s) - %s", g->player[i].user->nick, g->player[i].user->ip, buf);
#endif

		if(g->player[i].user->buflen + len >= TDKD_SENDSIZE) dequeue(g->player[i].user); /* would overflow, flush it */
		strcpy(g->player[i].user->QBuf + g->player[i].user->buflen, buf); /* append reply to buffer */
		g->player[i].user->buflen += len;
		/*if(IsFlush(cl)) */dequeue(g->player[i].user); /* Is flush forced ? */
	}
	return 0;
}

int sendrpl(aClient *cl, const char *pattern, ...)
{
	static char buf[512];
	va_list vl;
	int len;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf - 2, pattern, vl); /* format */
	if(len < 0) len = sizeof buf -2;

	buf[len++] = '\n';
	buf[len] = 0;
	va_end(vl);

#ifdef DEBUG
	if(strncasecmp(buf, "PING", 4) && strncasecmp(buf, "PONG", 4))
		printf("S(%s@%s) - %s", cl->nick, cl->ip, buf),
		putlog(PARSES_LOG, "S(%s@%s) - %s", cl->nick, cl->ip, buf);
#endif

	if(cl->buflen + len >= TDKD_SENDSIZE) dequeue(cl); /* would overflow, flush it */
	strcpy(cl->QBuf + cl->buflen, buf); /* append reply to buffer */
	cl->buflen += len;
	/*if(IsFlush(cl)) */dequeue(cl); /* Is flush forced ? */

	return 0;
}

aClient *addclient(int fd, const char *ip)
{
	aClient *new = NULL;

	if((unsigned) fd >= ASIZE(myUser))
		printf("Trop de clients connectés! (Dernier %d sur %s, Max: %d)",
			fd, ip, ASIZE(myUser));
	else if(!(myUser[fd].flag & TDKD_FREE))
		printf("Connexion sur un slot déjà occupé!? (%s -> %d[%s])",
			ip, fd, myUser[fd].ip);
	else
	{	/* register the socket in client list */
		new = &myUser[fd];
		new->fd = fd;
		if(fd > highsock) highsock = fd;
		FD_SET(fd, &global_fd_set);
		strcpy(new->ip, ip);
		new->flag = 0;
		sendrpl(new, MSG_SERVER, config.server, SERVER_VERSION);
	}

	return new;
}

void delclient(aClient *del)
{
	if(del->fd >= highsock) /* Was the highest..*/
		for(;highsock >= server.sock;--highsock) /* loop only till bot.sock */
			if(!(myUser[highsock].flag & TDKD_FREE)) break;

	if(del->buflen) dequeue(del); /* flush data if any */
	if(del->user) write_users();
	if(del->player) leave_game(del, del->player->game, 1); /* part des autres chans (silencieux) */
	if(IsAuth(del)) server.CONact--;
	FD_CLR(del->fd, &global_fd_set);

	close(del->fd);
	memset(del, 0, sizeof *del); /* clean up */
	del->flag = TDKD_FREE; /* now available for a new con */
	return;
}

int parse_this(int fd)
{
	aClient *cl;
	char buf[400];
	int read;

	if((unsigned) fd >= ASIZE(myUser))
		return printf("reading data from sock #%d, not registered ?", fd);

	cl = &myUser[fd];

	if((read = recv(fd, buf, sizeof buf -1, 0)) <= 0)
	{
		printf("Erreur lors de recv(%s): [%s] (%d)\n",
			cl->ip, strerror(errno), errno);
		delclient(cl);
	}
	else
	{
		char *ptr = buf;

		buf[read] = 0;

		/* split read buffer in lines,
		   if its length is more than acceptable, truncate it.
		   if a newline is found is the trailing buffer, go on parsing,
		   otherwise abort..
		   if line is unfinished, store it in fd own recv's buffer (cl->RecvBuf)
		  */
		while(*ptr)
		{
			if(*ptr == '\n' || cl->recvlen >= TDKD_RECVSIZE)
			{
				cl->RecvBuf[cl->recvlen-1] = 0;
				cl->lastread = CurrentTS;
#ifdef DEBUG
				if(strncasecmp(cl->RecvBuf, "PING", 4) && strncasecmp(cl->RecvBuf, "PONG", 4))
					printf("R(%s@%s) - %s\n", cl->nick, cl->ip, cl->RecvBuf),
					putlog(PARSES_LOG, "R(%s@%s) - %s\n", cl->nick, cl->ip, cl->RecvBuf);
#endif
				parsemsg(cl);
				if(cl->flag & TDKD_FREE) break; /* in case of failed pass */

				if(cl->recvlen >= TDKD_RECVSIZE && !(ptr = strchr(ptr + 1, '\n')))
				{ 	/* line exceeds size and no newline found */
					cl->recvlen = 0;
					break; /* abort parsing */
				}
				cl->recvlen = 0; /* go on on newline */
			}
			else cl->RecvBuf[cl->recvlen++] = *ptr; /* copy */
			/*next char, Note that if line was to long but newline was found, it drops the \n */
			++ptr;
		}
	}
	return 0;
}

int init_socket(void)
{
	unsigned int reuse_addr = 1;
	struct sockaddr_in localhost; /* bind info structure */

	memset(&localhost, 0, sizeof localhost);
	FD_ZERO(&global_fd_set);

	server.sock = socket(AF_INET, SOCK_STREAM, 0); /* on demande un socket */
	if(server.sock < 0)
	{
		printf("Impossible de lancer le serveur. (socket non créé)");
		exit(EXIT_FAILURE);
	}
	fcntl(server.sock, F_SETFL, O_NONBLOCK);

	setsockopt(server.sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof reuse_addr);

	localhost.sin_family = AF_INET;
	localhost.sin_addr.s_addr = INADDR_ANY;
	localhost.sin_port = htons(config.port);

	if(bind(server.sock, (struct sockaddr *) &localhost, sizeof localhost) < 0)
	{
		printf("Impossible d'écouter au port %d pour le serveur.", config.port);
		close(server.sock);
		exit(EXIT_FAILURE);
	}

	listen(server.sock, 5);
	if(server.sock > highsock) highsock = server.sock;
	FD_SET(server.sock, &global_fd_set);

	printf("Serveur Trou du cul (%s) lancé (Port %d)\n", SERVER_VERSION, config.port);
	server.uptime = CurrentTS;
	server.CONtotal = 0;
	server.CONact = 0;
	server.GAMEtotal = 0;

	for(reuse_addr = 0;reuse_addr < ASIZE(myUser);++reuse_addr)
		myUser[reuse_addr].flag = TDKD_FREE;

	alarm(PINGINTERVAL);

	return 1;
}

int run_server(void)
{
	fd_set tmp_fdset;
	int i;

	while(running)
	{
		CurrentTS = time(NULL);
		tmp_fdset = global_fd_set; /* save */
		if(select(highsock + 1, &tmp_fdset, NULL, NULL, NULL) < 0)
		{
			if(errno != EINTR)
			{
				printf("Erreur lors de select() (%d: %s)", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			for(i = 0;i <= highsock;++i)
			{
				if(!FD_ISSET(i, &tmp_fdset)) continue;
				if(i == server.sock)
				{
					struct sockaddr_in newcon;
					unsigned int addrlen = sizeof newcon;
					int newfd = accept(server.sock, (struct sockaddr *) &newcon, &addrlen);
					if(newfd > 0)
					{
						if(!addclient(newfd, inet_ntoa(newcon.sin_addr))) close(newfd);
					}
				}
				else parse_this(i);
			}
		}
	}
	return 1;
}
