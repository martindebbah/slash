#ifndef COMMANDE
#define COMMANDE

typedef struct commande commande;
typedef struct parametres parametres;

// Le type de données pour les commandes.
struct commande {
    char *name;
    parametres *premier;
    parametres *dernier;
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

// Désaloue la mémoire prise par les paramètres.
void delete_param(parametres *p);

// Ajoute un parametre à commande
void add_param(commande * cmd, parametres *p);

#endif
