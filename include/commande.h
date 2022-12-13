#ifndef COMMANDE
#define COMMANDE

#include "string_list.h"

typedef struct commande commande;
typedef struct parametres parametres;

// Le type de données pour les commandes.
struct commande {
    char *name;
    int nbParam;
    parametres *param;
};

struct parametres {
    char *str;
    parametres *suivant;
};

// Crée une commande à partir d'une ligne.
commande *create_cmd(char *line);

// Désaloue la mémoire prise par une commande
void delete_cmd(commande *cmd);

// Crée une liste de paramètres
parametres *create_param();

parametres *create_param_list(string_list *s);

// Evalue la chaîne de caractère en résolvant les étoiles
string_list *process_joker(char *str);

// Retourne le nombre de paramètres
int getNbParam(parametres *p);

// Retourne les paramètres sous forme de tableau
char **paramToTab(commande *cmd);

// Renvoie le paramètre à la position i
// La position 0 étant la première position de la liste
char *getParamAt(commande *cmd, int i);

// Renvoie le paramètre si c'est le i-ème de la liste
char *getParam(parametres *p, int i);

// Désaloue la mémoire prise par les paramètres.
void delete_param(parametres *p);

// Désalloue la mémoire de tab
void delete_tab(char **tab);

// Renvoie 1 si tous les jokers sont prefixes sinon 0
int is_joker_prefix(char *str);

#endif
