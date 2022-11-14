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
    
    while (1) {
        // Affichage du prompt + récupération de la ligne de commande
        char *line = readline(prompt(retVal));

        // Découpage de la ligne de commande
        commande *cmd = create_cmd(line);
        if (!cmd) {
            free(line);
            retVal = 127;
            continue;
        }

        // Ajout de la ligne à l'historique
        add_history(line);
        free(line);

        // Interprétation de la ligne de commande

        // Exécution des commandes (fork) et attente de la fin du processus fils
        // Si exit -> break
        if (strcmp(cmd -> name, "exit") == 0) {
            delete_cmd(cmd);
            // Stocker la valeur de sortie
            break;
        }
        delete_cmd(cmd);
    }

    clear_history();
    return 0;
}

char *prompt(int val) {
    char *prompt = malloc(46); // 30 (longueur maximale du prompt) + 15 (Pour changer la couleur)
                            // + 1 pour le caractère null final
    int i = 0;

    // Valeur de retour
    if (val == 0) { // Succès -> vert
        i += changeColor(prompt, 'v'); // 5
    }else { // Echec -> rouge
        i += changeColor(prompt + i, 'r'); // 5
    }
    prompt[i++] = '[';
    prompt[i++] = val + '0';
    prompt[i++] = ']';

    // Chemin du répertoire
    i += changeColor(prompt+ i, 'c'); // 5
    i += addToPrompt(prompt + i, "bash");

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
