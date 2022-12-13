#ifndef STRINGLIST
#define STRINGLIST

typedef struct string_list string_list;

struct string_list {
    char *s;
    string_list *suivant;
};

// Crée une liste de chaîne de caractères
string_list* list_create();

// Ajoute une chaîne de caractères à la liste
void list_append(string_list* l, char* str);

// Concatène deux listes
string_list* list_cat(string_list* l1, string_list* l2);

// Désalloue la mémoire
void list_delete(string_list* l);

#endif