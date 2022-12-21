#include <stdlib.h>
#include <string.h>
#include "redirection.h"
#include "mystring.h"

// Si str est un token de redirection
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

// Si str est un token de redirection de type pipeline
int isTokPipe(char *str) {
    return strcmp(str, "|") == 0;
}

redirection *create_redir(char *line) {
    redirection *redir = malloc(sizeof(redirection));

    // Initialisation de toutes les valeurs à NULL
    redir -> cmd = NULL;
    redir -> type = NULL;
    redir -> fic = NULL;
    redir -> suivante = NULL;

    // Création des string
    struct string *s = string_new(strlen(line) + 1); // La commande avant la possible redirection
    struct string *nextCmd = string_new(strlen(line) + 1); // La(les) commande(s) après la possible pipeline
    string_append(nextCmd, line);
    
    char *str = strtok(line, " ");

    if (!redir || !s || !nextCmd || strlen(line) < 1)
        goto error;

    // Boucle sur tokens délimités par des espaces
    do {
        nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

        if (isTokRed(str)) { // Si token de redirection
            redir -> type = calloc(strlen(str) + 1, 1);
            memcpy(redir -> type, str, strlen(str));
            if (!redir -> type)
                goto error;

            if (isTokPipe(str)) { // Si la redirection est une pipeline
                char *c = copy(nextCmd);
                nextCmd = NULL; // Pour éviter le double free lors d'un `goto error`
                redir -> suivante = create_redir(c);
                free(c);
                if (!redir -> suivante)
                    goto error;

            }else { // Si la redirection est autre qu'une pipeline
                str = strtok(NULL, " ");
                if (str == NULL)
                    goto error;
                
                redir -> fic = calloc(strlen(str) + 1, 1);
                memcpy(redir -> fic, str, strlen(str));
                // On prend en compte seulement le premier fichier
                // Si `ls > fic1 fic2`, fic2 sera ignoré

                if (!redir -> fic)
                    goto error;

                string_delete(nextCmd);
            }

            char *c = copy(s);
            s = NULL; // Pour éviter le double free lors d'un `goto error`
            redir -> cmd = create_cmd(c);
            free(c);

            if (!redir -> cmd)
                goto error;

            return redir;

        } else {
            // Mettre dans mystring les tokens
            string_append(s, str);
            string_append(s, " ");
        }
    }while ((str = strtok(NULL, " ")) != NULL);

    // Si sortie de la boucle -> pas de redirection
    char *c = copy(s);
    s = NULL; // Pour éviter le double free lors d'un `goto error`
    redir -> cmd = create_cmd(c);
    free(c);

    if (!redir -> cmd)
        goto error;
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
