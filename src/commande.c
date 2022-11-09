#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "commande.h"

commande *create_cmd(char *line) {
    commande *cmd = malloc(sizeof(commande));
    const char *sep = " ";

    if (!cmd || strlen(line) < 1)
        goto error;

    char *str = strtok(line, sep);

    cmd -> name = malloc(strlen(str));
    memcpy(cmd -> name, str, strlen(str));
    if (!cmd -> name)
        goto error;

    cmd -> param = create_param();

    return cmd;

    error:
    if (cmd != NULL)   
        delete_cmd(cmd);
    return NULL;
}

parametres *create_param() {
    char *str = strtok(NULL, " ");
    if (str == NULL) 
        return NULL;

    parametres *p = malloc(sizeof(parametres));
    if (!p)
        goto error;

    p -> str = malloc(strlen(str));
    memcpy(p -> str, str, strlen(str));
    if (!p -> str)
        goto error;

    p -> suivant = create_param();

    return p;

    error:
        if (!p)
            return NULL;
        delete_param(p);
        return NULL;
}

void delete_cmd(commande *cmd) {
    if (cmd -> name != NULL)
        free(cmd -> name);
    if (cmd -> param != NULL)
        delete_param(cmd -> param);
}

void delete_param(parametres *p) {
    if (p -> suivant != NULL)
        delete_param(p -> suivant);
    free(p -> str);
    free(p);
}
