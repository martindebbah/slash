#ifndef CMD
#define CMD

#include "commande.h"

typedef struct string_list string_list;

/*
Affiche la (plus precisement, une) reference absolue du repertoire de travail courant avec les options -P et -L.
La valeur de retour est 0 en cas de succes, 1 en cas d'echec.
*/
int cmd_pwd(commande *cmd);

// Renvoie le pwd (ne pas oublier de free)
char * pwd(int followSymLink);

/*
Change de repertoire de travail courant en le repertoire ref (s'il s'agit d'une reference valide), le precedent repertoire de travail si le parametre est -, ou $HOME en l'absence de parametre avec les options -P et -L.
La valeur de retour est 0 en cas de succes, 1 en cas d'echec.
*/
int cmd_cd();
char *update_path(char *path, char *up);

// Termine le processus slash avec la valeur de sortie 'val'
void cmd_exit(int val);

// parcours le repertoire dir_to_open et stock chaque nom dans les paramètres de cmd
string_list* parcours_repertoire(char* dir_to_open, char* suf, char* word);

#endif
