#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "cmd.h"
#include "commande.h"
#include "slash.h"

#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096

int main(int argc, char **argv) {

    rl_outstream = stderr;
    int retVal = 0;
    int exitVal = 0;
    
    while (1) {
        // Affichage du prompt + récupération de la ligne de commande
        char *p = prompt(retVal);
        char *line = readline(p);

        if (strlen(line) == 0) {
            free(line);
            free(p);
            continue;
        }

        // Historique
        char *hist = malloc(strlen(line) + 1);
        if (hist != NULL)
            memcpy(hist, line, strlen(line));
        hist[strlen(line)] = '\0';

        // Découpage de la ligne de commande
        commande *cmd = create_cmd(line);
        if (!cmd) {
            free(line);
            free(hist);
            free(p);
            retVal = 127;
            continue;
        }

        // Ajout de la ligne à l'historique
        add_history(hist);
        free(hist);

        // Interprétation de la ligne de commande

        // Exécution des commandes (fork) et attente de la fin du processus fils
        if (strcmp(cmd -> name, "pwd") == 0) {
            //test
            printf("%s \n",cmd_pwd());
        }

        // Si exit -> break
        if (strcmp(cmd -> name, "exit") == 0) {
            // Stocker la valeur de sortie
            if (cmd -> nbParam > 0)
                exitVal = atoi(getParamAt(cmd, 0));
                
            delete_cmd(cmd);
            break;
        }
        if (strcmp(cmd -> name, "cd") == 0) {
            cmd_cd(cmd);
        }
        delete_cmd(cmd);
        free(p);
    }

    clear_history();
    return exitVal;
}

char *prompt(int val) {
    int size = 46;  // 30 (longueur maximale du prompt) + 15 (Pour changer la couleur)
    char *prompt = malloc(size);    // + 1 pour le caractère null final
    int i = 0;

    // Valeur de retour
    if (val == 0) { // Succès -> vert
        i += changeColor(prompt, 'v'); // 5
    }else { // Echec -> rouge
        i += changeColor(prompt + i, 'r'); // 5
    }
    i += addVal(prompt + i, val);

    // Chemin du répertoire
    i += changeColor(prompt+ i, 'c'); // 5
    int max = size - i - 8; // (5 (couleur) + 3 ("$ \0"))
    char *path = cutPath(cmd_pwd(), max);
    i += addToPrompt(prompt + i, path);

    // Prompt
    i += changeColor(prompt + i, 'b'); // 5
    i += addToPrompt(prompt + i, "$ ");
    prompt[i++] = '\0';
    return prompt;
}

int changeColor(char *s, char color) {
    char *c;
    switch (color) {
        case 'v': // Vert
            c = "\033[32m";
            memcpy(s, c, 5);
            break;
        case 'r': // Rouge
            c = "\033[91m";
            memcpy(s, c, 5);
            break;
        case 'c': // Cyan
            c = "\033[36m";
            memcpy(s, c, 5);
            break;
        case 'b': // Blanc
            c = "\033[00m";
            memcpy(s, c, 5);
            break;
    }
    return 5;
}

int addToPrompt(char *p, char *s) {
    memcpy(p, s, strlen(s));
    return strlen(s);
}

int addVal(char *p, int val) {
    int i = 0;
    p[i++] = '[';
    if (val < 0) {
        p[i++] = '-';
        p[i++] = '1';
    }else {
        p[i++] = val + '0';
    }
    p[i++] = ']';
    return i;
}

char *cutPath(char *path, int max) {
    if (strlen(path) <= max)
        return path;

    int d = strlen(path);
    int i = d;
    while (strlen(path) - i < max) {
        if (path[i] == '/') {
            d = i;
        }
        i--;
    }
    path[--d] = '.';
    path[--d] = '.';
    path[--d] = '.';
    return path + d;
}
