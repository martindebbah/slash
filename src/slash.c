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
        char *line = readline(p); // Les couleurs du prompt ne s'affichent pas sur MacOS
        free(p);

        if (!line || !*line || strlen(line) > MAX_ARGS_STRLEN) {
            free(line);
            continue;
        }

        // Historique
        char *hist = malloc(strlen(line));
        if (hist)
            memcpy(hist, line, strlen(line));

        // Découpage de la ligne de commande
        commande *cmd = create_cmd(line);
        if (!cmd) {
            free(line);
            free(hist);
            retVal = 127;
            continue;
        }

        // Ajout de la ligne à l'historique
        add_history(hist);
        free(line);
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
                exitVal = atoi(getParamAt(cmd, 0)); // Convertir la valeur si possible !

            delete_cmd(cmd);
            break;
        }
        if (strcmp(cmd -> name, "cd") == 0) {
            cmd_cd(cmd);
        }
        delete_cmd(cmd);
    }

    clear_history();
    return exitVal;
}

char *prompt(int val) {
    int size = 52;  // 30 (longueur maximale du prompt) + 15 (Pour changer la couleur) + 6 (balises)
    char *prompt = calloc(size, 1);    // +1 ('\0')
    int i = 0;

    // Valeur de retour
    prompt[i++] = '\001';
    if (val == 0) { // Succès -> vert
        i += changeColor(prompt + i, 'v');
    }else { // Echec -> rouge
        i += changeColor(prompt + i, 'r');
    }
    prompt[i++] = '\002';
    i += addVal(prompt + i, val);

    // Chemin du répertoire
    prompt[i++] = '\001';
    i += changeColor(prompt + i, 'c');
    prompt[i++] = '\002';
    int max = size - i - 10;
    char *path = cutPath(cmd_pwd(), max);
    i += addToPrompt(prompt + i, path);

    // Prompt
    prompt[i++] = '\001';
    i += changeColor(prompt + i, 'b');
    prompt[i++] = '\002';
    i += addToPrompt(prompt + i, "$ ");

    return prompt;
}

int changeColor(char *s, char color) {
    char *c;
    int size = 5;
    switch (color) {
        case 'v':   c = "\033[32m"; // Vert
            break;
        case 'r':   c = "\033[91m"; // Rouge
            break;
        case 'c':   c = "\033[36m"; // Cyan
            break;
        default:    c = "\033[00m"; // Blanc
    }
    memcpy(s, c, size);
    return size;
}

int addToPrompt(char *p, char *s) {
    memcpy(p, s, strlen(s));
    return strlen(s);
}

int addVal(char *p, int val) {
    char v[3] = {0};
    int size;
    if (val > -1) { // Valeurs positives
        if (val < 10) { // Une case
            size = 1;
        } else if (val < 100) { // Deux cases
            size = 2;
        } else { // Trois cases
            size = 3;
        }
        for (int i = 0; i < size; i++) {
            char n = (val % 10) + '0'; // On récupère le chiffre des unités (dizaines/centaines pour i == 1/2)
            v[size - i - 1] = n;
            val /= 10; // On divise par 10 pour avoir le chiffre des dizaines (centaines pour i == 1)
        }
    } else { // Valeurs négatives (seulement -1 pour le moment)
        size = 2;
        v[0] = '-';
        v[1] = '1';
    }

    int i = 0;
    p[i++] = '[';
    memcpy(p + i, v, size);
    i += size;
    p[i++] = ']';

    return i;
}

char *cutPath(char *path, int max) {
    max -= 3;
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
