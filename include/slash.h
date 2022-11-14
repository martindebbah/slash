#ifndef SLASH
#define SLASH

// Renvoie le prompt à afficher
char *prompt(int val);

// Change la couleur du prompt
int changeColor(char *s, char c);

// Ajoute s au prompt
int addToPrompt(char *p, char *s);

#endif
