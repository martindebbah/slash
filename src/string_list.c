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
    if(l == NULL) return;
    free(l -> s);
    list_delete(l -> suivant);
    free(l);
}