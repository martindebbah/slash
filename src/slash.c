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
    
    while (1) {
        // Affichage du prompt + récupération de la ligne de commande
        char *line = readline(prompt(0)); // A remplacer par le prompt
        add_history(line);

        // Découpage de la ligne de commande
        commande *cmd = create_cmd(line);
        free(line);
        if (!cmd)
            continue;

        // Interprétation de la ligne de commande

        // Exécution des commandes (fork) et attente de la fin du processus fils
        // Si exit -> break
        if (strcmp(cmd -> name, "exit") == 0) {
            delete_cmd(cmd);
            exit(0);
        }
        //delete_cmd(cmd);
    }

    return 0;
}

char *prompt(int val) {
    return "prompt$ ";
}
