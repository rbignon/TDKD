/* src/config.c - Configuration du serveur
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
#include "config.h"
#include "infos.h"
#include "outils.h"

static struct tab {
	const char *item;
	int id;
	int must;
	const char *description;
} 	conftab[] = {
	{"server_info",	2, 1, "configuration des informations serveur"},
	{"misc_conf", 	3, 1, "configuration des diverses options"},
	{"", 0, 0, ""}
},
	server_info[] = {
	{"name", 0, 1, "Nom du serveur"},
	{"port", 0, 1, "Port du serveur"},
	{"", 0, 0, ""}
},
	misc_conf[] = {
	{"motd_file", 0, 1, "Fichier de motd"},
	{"", 0, 0, ""}
};



static struct tab *tabptr[] = { NULL, NULL, server_info, misc_conf, NULL };

static int get_item(struct tab *tab, const char *item)
{
	int i = 0;
	for(;tab[i].item[0] != '\0'; i++) if(!strcasecmp(tab[i].item, item)) return i;
	return -1;
}

static int check_missing_item(struct tab *tab)
{
	int i = 0, miss = 0;
	for(;tab[i].item[0] != '\0';i++)
		if(tab[i].id != 1 && !(tab[i].id & CONF_READ) && tab[i].must)
		{
			printf("conf: item '%s' manquant (%s)\n", tab[i].item, tab[i].description);
			miss++;
		}
		else if((tab[i].id & CONF_READ) && !tab[i].must)
			tab[i].id &= ~CONF_READ;
	return miss;
}

static char *conf_parse(char *ptr, struct tab *tab, int current_tab, int item)
{
	ptr = strtok(NULL, "=\r\n");
	while(ptr && *ptr && (*ptr == ' ' || *ptr == '\t')) ptr++;
	if(!ptr || !*ptr)
	{
		printf("conf: tab '%s', item '%s' sans valeur.\n", conftab[current_tab].item, tab[item].item);
		return NULL;
	}
	tab[item].id = 1;
	return ptr;
}

static int readconf(FILE *fp)
{
	int line = 0, current_tab = 0, tmp = 0, real_tab = 0;
	char buf[512] = {0};
	struct tab *real_tabptr = NULL;

	while(fgets(buf, sizeof buf, fp))
	{
		char *ptr = buf;
		while(*ptr == '\t' || *ptr == ' ') ptr++; /* on saute les espaces/tabs*/
		if(*ptr == '#' || *ptr == '\r' || *ptr == '\n' || !*ptr) continue; /* commentaire OU ligne vide*/
		ptr = strtok(ptr, " ");

		line++;
		if(!real_tab) /* no tab selected!*/
		{
			if((current_tab = get_item(conftab, ptr)) == -1) printf("conf line: %d, tab inconnue: %s\n", line, ptr);
			else
			{
				real_tab = (conftab[current_tab].id & ~CONF_READ); /* index de la tab*/
				conftab[current_tab].id |= CONF_READ; /* tab trouvée -- marquons le */
				real_tabptr = tabptr[real_tab];
			}
			continue;
		}
		else
		{
			if(EndOfTab(ptr))
			{
				if(check_missing_item(real_tabptr)) return -1;/* end of /tab*/
				real_tab = 0;
				continue;
			}
			if((tmp = get_item(real_tabptr, ptr)) == -1) {
				printf("conf: line %d, tab %s, item inconnu: %s\n", line, conftab[current_tab].item, ptr);
				continue;
			}
			else if(!(ptr = conf_parse(ptr, real_tabptr, current_tab, tmp))) return -1;
		}

		switch(real_tab)
		{
			case 2: /* server_info */
			switch(tmp)
			{
				case 0:
					if(strchr(ptr, ' '))
					{
						printf("Il ne doit pas y avoir d'espace dans le nom du serveur (ligne %d)\n", line);
						return -1;
					}
					strcpy(config.server, ptr);
					break;
				case 1:
					if(!is_num(ptr) || (tmp = atoi(ptr)) <= 0 || tmp > 65535)
					{
						printf("Port invalide: (ligne %d)\n", line);
						return -1;
					}
					config.port = tmp;
					break;
			}
			break;

			case 3: /* misc_conf*/
			switch(tmp)
			{
				case 0:
					strcpy(config.motd_file, ptr);
					break;
			}
			break;
		}
	}

	if(real_tab)
	{
		printf("Conf: erreur, parsage incomplet, missing '}', last tab : %s\n", conftab[current_tab-1].item);
		return -1;
	}
	if(check_missing_item(conftab)) return -1;
	real_tab = 0;
	return 1;
}

int load_config(const char *file)
 {
 	FILE *fp = fopen(file, "r");
 	int error = 0;
 	if(!fp)
 	{
 		printf("conf: Impossible d'ouvrir le fichier de conf %s (%s)\n", file, strerror(errno));
 		return -1;
 	}
 	error = readconf(fp);
 	fclose(fp);
 	return error;
 }
