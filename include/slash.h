#ifndef SLASH
#define SLASH

// Renvoie le prompt à afficher
char *prompt(int val);

// Change la couleur du prompt
int changeColor(char *s, char c);

// Ajoute s au prompt
int addToPrompt(char *p, char *s);

// Découpe le path pour ne pas dépasser la limite de caractères
char *cutPath(char *path, int max);

#endif
