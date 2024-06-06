/* include/outils.h - Proto de outils.c
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

char *Strncpy(char *to, const char *from, size_t n);
char *Strtok(char **save, char *str, char sep);
void strip_newline(char *string);
char *duration(int s);
int is_num(const char *num);
int puiss (int x, int y);
void putlog(const char *fichier, const char *fmt, ...);
char *get_time(time_t mytime);
int GetValue(char *s);
