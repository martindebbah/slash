#ifndef MYSTRING_H
#define MYSTRING_H

// La structure string stocke une chaîne de caractères terminée par un \0
// dans un buffer de taille connue, pour éviter les débordements.

struct string {
  size_t capacity; // Taille du buffer .data
  size_t length;   // Taille de la chaîne, sans compter le \0 terminal
  char * data;     // Contenu de la chaîne, avec un \0 terminal
};

struct string * string_new(size_t capacity);
// Crée une nouvelle chaîne de longueur 0 et de capacité donnée, allouée
// sur le tas (c'est-à-dire avec malloc).

void string_delete(struct string * str);
// Détruit une chaîne, en libérant la mémoire occupée.

void string_truncate (struct string * str, size_t nchars);
// Tronque la chaîne en enlevant les nchars derniers caractères
// (ou tous les caractères s'il y en a moins de nchars).

void string_truncate_where (struct string * str, char delimit);
// Tronque la chaîne en enlevant les nchars depuis le delimit
// (ou tous les caractères s'il y en a moins de nchars).

// Ajoute SRC à la fin de DEST
int string_append (struct string * dest, char * src);

// Ajoute SRC au début de DEST
int string_prepend(struct string * dest, char * src);

// Copie et supprime le string
// il faut free après utilisation
char *copy(struct string *path);

struct string *clean(struct string *path);
  
#endif // MYSTRING_H
