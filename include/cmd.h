#ifndef CMD
#define CMD

#include "commande.h"

/*
Affiche la (plus precisement, une) reference absolue du repertoire de travail courant avec les options -P et -L.
La valeur de retour est 0 en cas de succes, 1 en cas d'echec.
*/
char * cmd_pwd(commande *cmd);
char * pwd(int followSymLink);

/*
Change de repertoire de travail courant en le repertoire ref (s'il s'agit d'une reference valide), le precedent repertoire de travail si le parametre est -, ou $HOME en l'absence de parametre avec les options -P et -L.
La valeur de retour est 0 en cas de succes, 1 en cas d'echec.
*/
int cmd_cd();

#endif
