#ifndef SLASH
#define SLASH

// Exécute la redirection
int executeRedirection(redirection *redir);

// Exécute la commande
int executeCmd(commande *cmd);

// Renvoie le prompt à afficher
char *prompt(int val);

// Change la couleur du prompt
int changeColor(char *s, char c);

// Ajoute s au prompt
int addToPrompt(char *p, char *s);

// Ajoute la valeur de retour au prompt
int addVal(char *path, int val);

// Découpe le path pour ne pas dépasser la limite de caractères
char *cutPath(char *path, int max);

// Traite le joker si il y en a un dans les parametres de cmd
int joker_processing(commande *cmd);

#endif
