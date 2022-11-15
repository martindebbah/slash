#ifndef COMMANDE
#define COMMANDE

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

// Retourne le nombre de paramètres
int getNbParam(parametres *p);

// Renvoie le paramètre à la position i
char *getParamAt(int i);

// Désaloue la mémoire prise par les paramètres.
void delete_param(parametres *p);

#endif
