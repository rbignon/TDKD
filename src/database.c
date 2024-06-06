/* src/database.c - Base de donnée
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

#include <errno.h>
#include "main.h"
#include "outils.h"
#include "infos.h"

void write_users(void)
{
	FILE *fp;
	anUser *u = userhead;

	if(!(fp = fopen(USERS_FILE, "w"))) return;

	fprintf(fp, "VERSION %d\n", USERVERSION);
	for(; u; u = u->next)
		fprintf(fp, "USER %s %s %s %d %d %d %d %d %d %d\n", u->nick, u->pass, u->mail, u->flag,
			u->stats->nbparties, u->stats->grades[1], u->stats->grades[2], u->stats->grades[3],
			u->stats->grades[4], u->stats->score);

	fclose(fp);
	return;
}

int load_users(void)
{
	char buff[300];
	int stats = 0, version = 1;
	FILE *fp = fopen(USERS_FILE, "r");

	if(!fp)
	{
		SetFirst(config);
		return 0;
	}
	while(fgets(buff, sizeof buff, fp))
	{
		char *field = NULL;
		strip_newline(buff);
		if((field = strchr(buff, ' '))) *field++ = 0;
		else continue;

		if(!strcmp(buff, "VERSION")) version = atoi(field);
		if(!strcmp(buff, "USER"))
		{
			char *nick = NULL, *mail = NULL, *pass = NULL;
			int flag = 0, nbparties = 0, grades[5] = { 0 }, score = 0;

			nick = strtok(field, " ");
			pass = strtok(NULL, " ");
			mail = strtok(NULL, " ");
			flag = atoi(strtok(NULL, " "));
			if(version >= 2)
			{
				nbparties = atoi(strtok(NULL, " "));
				grades[1] = atoi(strtok(NULL, " "));
				grades[2] = atoi(strtok(NULL, " "));
				grades[3] = atoi(strtok(NULL, " "));
				grades[4] = atoi(strtok(NULL, " "));
				score = atoi(strtok(NULL, " "));
			}
			AddUser(nick, pass, mail, flag, nbparties, grades, score);
			stats++;
		}
	}
	fclose(fp);
	return stats;
}

void append_file(const char *file, const char *ligne)
{
	FILE *fp = fopen(file, "a");
	if(fp)
	{
		fputs(ligne, fp), fputc('\n', fp);
		fclose(fp);
	}
	else printf("Impossible d'écrire [%s] dans '%s' (%s)\n", ligne, file, strerror(errno));
	return;
}

