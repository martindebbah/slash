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
    if (!str)
        goto error;
    
    cmd -> name = malloc(strlen(str) + 1);
    memcpy(cmd -> name, str, strlen(str));
    if (!cmd -> name)
        goto error;
    cmd -> name[strlen(str)] = '\0';

    cmd -> param = create_param();
    cmd -> nbParam = getNbParam(cmd -> param);

    return cmd;

    error:
    if (cmd != NULL)   
        delete_cmd(cmd);
    return NULL;
}

parametres *create_param() {
    char *str = strtok(NULL, " ");
    if (!str)
        return NULL;
        
    parametres *p = malloc(sizeof(parametres));
    if (!p)
        goto error;

    p -> str = malloc(strlen(str));
    memcpy(p -> str, str, strlen(str));
    if (!p -> str)
        goto error;
    p->str[strlen(str)] = '\0';

    p -> suivant = create_param();

    return p;

    error:
        if (p)
            delete_param(p);
        return NULL;
}

void delete_cmd(commande *cmd) {
    if (cmd -> name)
        free(cmd -> name);
    if (cmd -> param)
        delete_param(cmd -> param);
    free(cmd);
}

int getNbParam(parametres *p) {
    if (!p)
        return 0;
    return getNbParam(p -> suivant) + 1;
}

char *getParamAt(commande *cmd, int i) {
    if (!cmd)
        return NULL;
    return getParam(cmd -> param, i);
}

char *getParam(parametres *p, int i) {
    if (!p || i < 0)
        return NULL;
    return i == 0 ? p -> str : getParam(p -> suivant, i - 1);
}

void delete_param(parametres *p) {
    if (p -> suivant)
        delete_param(p -> suivant);
    free(p -> str);
    free(p);
}
