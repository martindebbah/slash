#ifndef REDIRECTION
#define REDIRECTION

#include "commande.h"

typedef struct redirection redirection;

struct redirection {
    commande *cmd;
    char *pipe;
    char *in;
    char *out;
    char *err;
    char *fic_in;
    char *fic_out;
    char *fic_err;
    redirection *suivante;
};

// Crée une redirection
redirection *create_redir(char *l);

// Supprime la redirection
void delete_redir(redirection *r);

// 1 si la commande contient une redirection
// 0 si c'est seulement une commande
int isRedir(redirection *r);

// 1 si la redirection est de type pipeline
// 0 si c'est une redirection vers/depuis un fichier
int isPipeline(redirection *r);

// void print(redirection *r);

#endif
