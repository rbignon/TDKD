/* src/outils.c - Outils
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

#include <ctype.h>
#include <stdarg.h>
#include "main.h"
#include "outils.h"

int puiss (int x, int y)
{
  int resultat;
  for (resultat = 1; y > 0; y--) resultat*=x;
  return resultat;
}

char *Strncpy(char *to, const char *from, size_t n)/* copie from dans to. ne copie que n char */
{							/* MAIS AJOUTE LE CHAR \0 à la fin, from DOIT donc faire n+1 chars. */
	const char *end = to + n;
	char *save = to;
	while(to < end && (*to++ = *from++));
	*to = 0;
	return save;
}

char *Strtok(char **save, char *str, char sep)/* fonction tirée d'ircu, simplifiée */
{
	register char *pos = *save; 	/* keep last position across calls */
	char *tmp;

	if(str) pos = str; 	/* new string scan */

	while(pos && *pos && sep == *pos) pos++; /* skip leading separators */

	if (!pos || !*pos) return (pos = *save = NULL); /* string contains only sep's */

	tmp = pos; /* now, keep position of the token */

	if((pos = strchr(pos, sep))) *pos++ = 0;/* get sep -Cesar*/

	*save = pos;
	return tmp;
}

void strip_newline(char *string)
{
   register char *p = string;
   while(*p && *p != '\n' && *p != '\r') p++;
   *p = '\0';
}

char *duration(int s)
{
	static char dur[200];
	int i = 0;

	if(s >= 86400)
		i += sprintf(dur + i, "%d", s/86400), s %= 86400, strcpy(dur + i, " jours "), i += 7;
	if(s >= 3600)
		i += sprintf(dur + i, "%d", s/3600), s %= 3600, strcpy(dur + i, " heures "), i += 8;
	if(s >= 60)
		i += sprintf(dur + i, "%d", s/60), s %= 60, strcpy(dur + i, " minutes "), i += 9;
	if(s) i += sprintf(dur + i, "%d",s), strcpy(dur + i, " secondes");
	else dur[i-2]= 0;

	return dur;
}

int GetValue(char *s)
{
	char c, i = 0;

	c = *s;

	switch(c)
	{
		case '1': i = 1; break;
		case '2': i = 2; break;
		case '3': i = 3; break;
		case '4': i = 4; break;
		case '5': i = 5; break;
		case '6': i = 6; break;
		case '7': i = 7; break;
		case '8': i = 8; break;
		case '9': i = 9; break;
		case 'A': i = 10; break;
		case 'B': i = 12; break;
		case 'C': i = 13; break;
		case 'D': i = 14; break;
	}

	return i;
}

int is_num(const char *num)
{
   while(*num) if(!isdigit(*num++)) return 0;
   return 1;
}

char *get_time(time_t mytime)
{
	static char buftime[40];
	register struct tm *lt = localtime(&mytime);

	snprintf(buftime, sizeof buftime,"%d-%02d-%02d %02d:%02d:%02d",
		1900 + lt->tm_year,	lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);

	return buftime;
}

void putlog(const char *fichier, const char *fmt, ...)
{
	va_list vl;
	FILE *fp = fopen(fichier, "a");

	if(!fp) return;

	fputc('[', fp);
	fputs(get_time(CurrentTS), fp);
	fputs("] ", fp);

	va_start(vl, fmt);
	vfprintf(fp, fmt, vl);
	va_end(vl);

	fclose(fp);
}
