#ifndef STRINGLIST
#define STRINGLIST

typedef struct string_list string_list;

struct string_list {
    char *s;
    string_list *suivant;
};

void list_append(string_list* l, char* str);
string_list* list_create();
void list_delete(string_list* l);
string_list* list_cat(string_list* l1, string_list* l2);

#endif