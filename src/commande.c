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

    cmd->premier = NULL;
    cmd->dernier = NULL;
    
    str = strtok(NULL, sep);
    while (str != NULL){
        parametres * p = create_param(str);
        add_param(cmd, p);
        str = strtok(NULL, sep);
    }

    return cmd;

    error:
    if (cmd != NULL)   
        delete_cmd(cmd);
    return NULL;
}

parametres *create_param(char * name) {
    if(name == NULL) return NULL;
    parametres *p = malloc(sizeof(parametres));
    if (!p)
        return NULL;

    p -> str = malloc(sizeof(char));
    if (!p -> str)
        goto error;
    memcpy(p -> str, name, strlen(name));
    p->str[strlen(name)] = '\0';

    p -> suivant = NULL;

    return p;

    error:
        delete_param(p);
        return NULL;
}

void delete_cmd(commande *cmd) {
    if (cmd -> name != NULL)
        free(cmd -> name);
    if (cmd -> premier != NULL)
        delete_param(cmd -> premier);
    free(cmd);
}

void delete_param(parametres *p) {
    if (p -> suivant != NULL)
        delete_param(p -> suivant);
    free(p -> str);
    free(p);
}

void add_param(commande * cmd, parametres *p){
    if(p == NULL) return;
    if(cmd->dernier != NULL){
        cmd->dernier->suivant = p;
        cmd->dernier = cmd->dernier->suivant;
    }
    else {
        cmd->premier = p;
        cmd->dernier = cmd->premier;
    }
}
