#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "redirection.h"
#include "mystring.h"
#include "commande.h"

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
    redir -> pipe = NULL;
    redir -> in = NULL;
    redir -> out = NULL;
    redir -> err = NULL;
    redir -> fic_in = NULL;
    redir -> fic_out = NULL;
    redir -> fic_err = NULL;
    redir -> suivante = NULL;

    if (!line) {
        delete_redir(redir);
        return NULL;
    }

    // Création des string
    struct string *s = string_new(strlen(line) + 1); // La commande avant la possible redirection
    struct string *nextCmd = string_new(strlen(line) + 1); // La(les) commande(s) après la possible pipeline
    string_append(nextCmd, line);
    
    char *str = strtok(line, " ");

    if (!redir || !s || !nextCmd || !line || strlen(line) < 1)
        goto error;

    // Boucle sur tokens délimités par des espaces
    do {
        if (isTokRed(str)) { // Si token de redirection
            while (str && isTokPipe(str) == 0) {
                // Boucle sur les redirections standard
                if (strcmp(str, "<") == 0) { // Redirection d'entrée standard
                    redir -> in = calloc(strlen(str) + 1, 1);
                    memcpy(redir -> in, str, strlen(str));
                    if (!redir -> in)
                        goto error;

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    str = strtok(NULL, " ");
                    if (str == NULL || isTokRed(str))
                        goto error;

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    // Si le fichier contient une étoile
                    if (strchr(str, '*') != NULL && is_joker_prefix(str)) {
                        string_list *fic = process_joker(str);
                        if (fic && fic -> s && !fic -> suivant) {
                            redir -> fic_in = calloc(strlen(fic -> s) + 1, 1);
                            memcpy(redir -> fic_in, fic -> s, strlen(fic -> s));
                        }
                        list_delete(fic);
                    }else {
                        redir -> fic_in = calloc(strlen(str) + 1, 1);
                        memcpy(redir -> fic_in, str, strlen(str));
                    }

                    if (!redir -> fic_in)
                        goto error;
                }else if (strcmp(str, ">") == 0 || strcmp(str, ">>") == 0 || strcmp(str, ">|") == 0) {
                    // Redirection de sortie standard
                    redir -> out = calloc(strlen(str) + 1, 1);
                    memcpy(redir -> out, str, strlen(str));
                    if (!redir -> out)
                        goto error;

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    str = strtok(NULL, " ");
                    if (str == NULL || isTokRed(str))
                        goto error;

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    // Si le fichier contient une étoile
                    if (strchr(str, '*') != NULL && is_joker_prefix(str)) {
                        string_list *fic = process_joker(str);
                        if (fic && fic -> s && !fic -> suivant) {
                            redir -> fic_out = calloc(strlen(fic -> s) + 1, 1);
                            memcpy(redir -> fic_out, fic -> s, strlen(fic -> s));
                        }
                        list_delete(fic);
                    }else {
                        redir -> fic_out = calloc(strlen(str) + 1, 1);
                        memcpy(redir -> fic_out, str, strlen(str));
                    }

                    if (!redir -> fic_out)
                        goto error;
                }else if (strcmp(str, "2>") == 0 || strcmp(str, "2>>") == 0 || strcmp(str, "2>|") == 0) {
                    // Redirection de sortie erreur
                    redir -> err = calloc(strlen(str) + 1, 1);
                    memcpy(redir -> err, str, strlen(str));
                    if (!redir -> err)
                        goto error;

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    str = strtok(NULL, " ");
                    if (str == NULL || isTokRed(str))
                        goto error;

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));
                            
                    // Si le fichier contient une étoile
                    if (strchr(str, '*') != NULL && is_joker_prefix(str)) {
                        string_list *fic = process_joker(str);
                        if (fic && fic -> s && !fic -> suivant) {
                            redir -> fic_err = calloc(strlen(fic -> s) + 1, 1);
                            memcpy(redir -> fic_err, fic -> s, strlen(fic -> s));
                        }
                        list_delete(fic);
                    }else {
                        redir -> fic_err = calloc(strlen(str) + 1, 1);
                        memcpy(redir -> fic_err, str, strlen(str));
                    }

                    if (!redir -> fic_err)
                        goto error;
                }

                str = strtok(NULL, " ");
            }

            if (str && isTokPipe(str)) { // Si pipeline
                redir -> pipe = calloc(strlen(str) + 1, 1);
                memcpy(redir -> pipe, str, strlen(str));
                if (!redir -> pipe)
                    goto error;

                nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                char *c = copy(nextCmd);
                nextCmd = NULL; // Pour éviter le double free lors d'un `goto error`
                redir -> suivante = create_redir(c); // Appel récursif pour les commandes suivantes
                free(c);
                if (!redir -> suivante)
                    goto error;
            }else { // Si pas de pipeline on est à la fin des redirections
                string_delete(nextCmd);
                nextCmd = NULL;
            }

            // Ecriture de la commande à rediriger
            char *c = copy(s);
            s = NULL; // Pour éviter le double free lors d'un `goto error`
            if (!c)
                goto error;
            redir -> cmd = create_cmd(c);
            free(c);

            if (!redir -> cmd)
                goto error;

            return redir;

        } else { // Si pas token de redirection
            // Mettre dans mystring les tokens
            string_append(s, str);
            string_append(s, " ");
            nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));
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
    if (redir -> pipe)
        free(redir -> pipe);
    if (redir -> in)
        free(redir -> in);
    if (redir -> out)
        free(redir -> out);
    if (redir -> err)
        free(redir -> err);
    if (redir -> fic_in)
        free(redir -> fic_in);
    if (redir -> fic_out)
        free(redir -> fic_out);
    if (redir -> fic_err)
        free(redir -> fic_err);
    delete_redir(redir -> suivante);
    free(redir);
}

int isRedir(redirection *redir) {
    return redir -> in != NULL
        || redir -> out != NULL
        || redir -> err != NULL
        || redir -> pipe != NULL;
}

int isPipeline(redirection *redir) {
    return redir -> pipe != NULL;
}
