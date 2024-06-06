CC=gcc
PRG=../troudukd
INC_DIR=../include
CFLAGS=-g -O2 -Wall -I${INC_DIR}
LIBS=-lcrypt
CFG=../

OBJS= 	main.o serveur.o infos.o divers.o gamecmds.o ia.o outils.o \
	config.o admin_cmds.o database.o

INC=${INC_DIR}/main.h ${INC_DIR}/structs.h

${PRG}: ${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS}

${OBJS}: ${INC_DIR}/main.h
${OBJS}: ${INC_DIR}/structs.h

clean: rm -f *.o *~ *.~* *.save *.bak \#*
serveur.o: ${INC_DIR}/serveur.h
infos.o: ${INC_DIR}/infos.h
divers.o: ${INC_DIR}/divers.h
gamecmds.o: ${INC_DIR}/gamecmds.h
ia.o: ${INC_DIR}/ia.h
outils.o: ${INC_DIR}/outils.h
config.o: ${INC_DIR}/config.h
admin_cmds.o: ${INC_DIR}/admin_cmds.h
database.o: ${INC_DIR}/database.h
