/* src/main.c - Fichier principal
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

#include <sys/resource.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include "main.h"
#include "outils.h"
#include "serveur.h"
#include "config.h"
#include "database.h"

struct serverinfo server;
struct gameinfo *gamehead;
struct userinfo *userhead;
struct configinfo config;
time_t CurrentTS = 0;
char conf_file[200];
int running = 1;
int background = 1;

int main(int argc, char **argv)
{
	struct rlimit rlim; /* used for core size */
	int tmp;

	strcpy(conf_file, CONFIG_FILE);
	while((tmp = getopt(argc, argv, "nc:")) != EOF)
		switch(tmp)
		{
			case 'n':
				background = 0;
				break;
			case 'c':
				strcpy(conf_file, optarg);
				break;
			default:
				printf("Syntaxe: %s [-n] [-c <fichier de configuration]\n", argv[0]);
				exit(EXIT_FAILURE);
		}


	if(load_config(conf_file) == -1)
	{
			printf("Erreur lors de la lecture de la configuration\n");
			exit(EXIT_FAILURE);
	}

	CurrentTS = time(NULL);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, &sig_alarm);

	tmp = load_users();
	if(tmp) printf("Chargement des users OK (%d)\n", tmp);

	if(background)
	{
		if((tmp = fork()) == -1)
		{
			printf("Impossible de lancer en background\n");
			exit(0);
		}
		if(tmp > 1) exit(0);
	}

	if(!getrlimit(RLIMIT_CORE, &rlim) && rlim.rlim_cur != RLIM_INFINITY)
	{
		printf("Core size limitée à %ldk, changement en illimité.\n", rlim.rlim_cur);
		rlim.rlim_cur = RLIM_INFINITY;
		rlim.rlim_max = RLIM_INFINITY;
		setrlimit(RLIMIT_CORE, &rlim);
	}

	if(init_socket())
		run_server();

	exit(0);
}
