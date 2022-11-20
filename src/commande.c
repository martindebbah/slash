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
    
    cmd -> name = calloc(strlen(str) + 1, 1);
    memcpy(cmd -> name, str, strlen(str));
    if (!cmd -> name)
        goto error;

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

    p -> str = calloc(strlen(str) + 1, 1);
    memcpy(p -> str, str, strlen(str));
    if (!p -> str)
        goto error;

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

char **paramToTab(commande *cmd) {
    int n = cmd -> nbParam;
    char **p = malloc(sizeof(*p) * (n + 2));
    p[0] = calloc(strlen(cmd -> name) + 1, 1);
    memcpy(p[0], cmd -> name, strlen(cmd -> name));
    for (int i = 0; i < n; i++) {
        char *s = getParamAt(cmd, i);
        p[i + 1] = calloc(strlen(s) + 1, 1);
        memcpy(p[i + 1], s, strlen(s));
    }
    p[n + 1] = NULL;
    return p;
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
    if (p -> str)
        free(p -> str);
    free(p);
}
