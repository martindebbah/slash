#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mystring.h"
#include "commande.h"
#include "cmd.h"

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

    // line += strlen(str)+1;
    // char *l = calloc(strlen(line) + 1, 1);
    // memcpy(l, line, strlen(line));
    cmd -> param = create_param();
    cmd -> nbParam = getNbParam(cmd -> param);
    // free(l);

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

    // verifier le joker
    if(strchr(str,'*') != NULL){
        if(is_joker_prefix(str)){
            struct string *dir_to_open = string_new(strlen(str)+1);
            string_append(dir_to_open, str);
            string_truncate(dir_to_open, strlen(strchr(str,'*')));
            if(dir_to_open->length == 0) string_append(dir_to_open, ".");

            char *suf = calloc(strlen(strchr(str,'*'))+1, 1);
            memcpy(suf, strchr(str,'*'), strlen(strchr(str,'*')));
            if(!suf)
                return NULL;
            suf++;

            string_list* path;
            if(suf[0] == '/'){
                suf++;
                path = parcours_repertoire(dir_to_open->data, suf, NULL);
            }
            else if(suf[0] == '\0'){
                path = parcours_repertoire(dir_to_open->data, NULL, NULL);
            }
            else {
                struct string *word_to_compare = string_new(strlen(suf)+1);
                string_append(word_to_compare, suf);
                string_truncate_where(word_to_compare, '/');
                if(strlen(suf) > word_to_compare->length) suf += word_to_compare->length+1;
                else suf += word_to_compare->length;
                
                if(strlen(suf) == 0) suf = NULL;

                path = parcours_repertoire(dir_to_open->data, suf, word_to_compare->data);

                string_delete(word_to_compare);
            }

            if(path->s != NULL){
                parametres *p = create_param_list(path);

                list_delete(path);
                string_delete(dir_to_open);
                return p;
            }
            list_delete(path);
            string_delete(dir_to_open);
        }
    }

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

parametres *create_param_list(string_list *l) {
    if (!l || !l -> s)
        return create_param();

    parametres *p = malloc(sizeof(parametres));
    if (!p)
        goto error;

    p -> str = calloc(strlen(l -> s) + 1, 1);
    memcpy(p -> str, l -> s, strlen(l -> s));
    if (!p -> str)
        goto error;

    p -> suivant = create_param_list(l -> suivant);

    return p;

    error:
        if (p)
            delete_param(p);
        return NULL;
}

int is_joker_prefix(char *str){
    int res = 1;
    char *joker = strchr(str,'*');
    if(strlen(str) == 1) return res;
    while(joker != NULL){
        if(strcmp(joker,str) == 0){
            joker++;
            joker = strchr(joker,'*');
            continue;
        }
        joker--;
        if(joker[0] != '/'){
            res = 0;
            break;
        }
        else{
            joker = joker + 2;
        }
        joker = strchr(joker,'*');
    }
    //free(joker);
    return res;
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
