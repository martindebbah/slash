#include <stdlib.h>
#include <string.h>
#include "redirection.h"
#include "mystring.h"

int isTokRed(char *str) {
    return strcmp(str, "<") == 0 
        || strcmp(str, ">") == 0
        || strcmp(str, ">|") == 0
        || strcmp(str, ">>") == 0
        || strcmp(str, "2>") == 0
        || strcmp(str, "2>|") == 0
        || strcmp(str, "2>>") == 0
        || strcmp(str, "|") == 0;
}

int isTokPipe(char *str) {
    return strcmp(str, "|") == 0;
}

redirection *create_redir(char *line) {
    redirection *redir = malloc(sizeof(redirection));

    // création des mystring
    struct string *s = string_new(strlen(line) + 1);
    struct string *nextCmd = string_new(strlen(line) + 1);
    string_append(nextCmd, line);
    char *str = strtok(line, " ");

    if (!redir || !s || !nextCmd || strlen(line) < 1)
        goto error;

    do {
        nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));
        if (isTokRed(str)) { // si token de redirection
            redir -> type = calloc(strlen(str) + 1, 1);
            memcpy(redir -> type, str, strlen(str));
            if (!redir -> type)
                goto error;

            if (isTokPipe(str)) { // si redir == |
                redir -> fic = NULL;

                char *c = copy(nextCmd);
                redir -> suivante = create_redir(c);
                free(c);
                if (!redir -> suivante)
                    goto error;

            }else { // si redir != |
                str = strtok(NULL, " ");
                if (str == NULL) // Condition vraie !!
                    goto error;
                
                redir -> fic = calloc(strlen(str) + 1, 1);
                memcpy(redir -> fic, str, strlen(str));

                if (!redir -> fic)
                    goto error;

                redir -> suivante = NULL;
                string_delete(nextCmd);
            }

            char *c = copy(s);
            redir -> cmd = create_cmd(c);
            free(c);

            if (!redir -> cmd)
                goto error;

            return redir;

        } else {
            // mettre dans mystring les tokens
            char *c = calloc(strlen(str) + 1, 1);
            memcpy(c, str, strlen(str));
            string_append(s, c);
            free(c);
            string_append(s, " ");
        }
    }while ((str = strtok(NULL, " ")) != NULL);

    // si sortie de la boucle -> pas de redirection
    char *c = copy(s);
    redir -> cmd = create_cmd(c);
    free(c);

    if (!redir -> cmd)
        goto error;

    redir -> type = NULL;
    redir -> fic = NULL;
    redir -> suivante = NULL;
    string_delete(nextCmd);

    return redir;

    error:
        if (redir)
            delete_redir(redir);
        if (s)
            string_delete(s);
        if (nextCmd)
            string_delete(nextCmd);
        return NULL;
}

void delete_redir(redirection *redir) {
    if (!redir)
        return;
    if (redir -> cmd)
        delete_cmd(redir -> cmd);
    if (redir -> type)
        free(redir -> type);
    if (redir -> fic)
        free(redir -> fic);
    delete_redir(redir -> suivante);
    free(redir);
}

int isRedir(redirection *redir) {
    return redir -> type != NULL;
}

int isPipeline(redirection *redir) {
    return isRedir(redir) && isTokPipe(redir -> type);
}
