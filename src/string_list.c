#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <limits.h>
#include <errno.h>

#include "string_list.h"

string_list *list_create() {
    string_list *l = malloc(sizeof(string_list));
    l -> s = NULL;
    l -> suivant = NULL;
    return l;
}

void list_append(string_list *l, char *s) {
    if (l -> s == NULL) {
        l -> s = calloc(strlen(s) + 1, 1);
        memcpy(l -> s, s, strlen(s));
        return;
    }
    if (!l -> suivant)
        l->suivant = list_create();
    list_append(l -> suivant, s);
}

void list_delete(string_list *l) {
    if (!l)
        return;
    if (l -> s)
        free(l -> s);
    list_delete(l -> suivant);
    free(l);
}

int find(string_list *l, char *s) {
    while (l != NULL) {
        if (strcmp(l->s, s) == 0) {
            return 1;
        }
        l = l->suivant;
    }
    return 0;
}

string_list* list_cat(string_list* l1, string_list* l2){
    if(!l1 || !l1->s) {
        list_delete(l1);
        return l2;
    }
    if(!l2 || !l2->s) {
        list_delete(l2);
        return l1;
    }

    string_list* l = l2;
    while (l != NULL) {
        if (find(l1, l->s) == 0) {
            list_append(l1, l->s);
        }
        l = l->suivant;
    }

    list_delete(l2);
    return l1;
}
