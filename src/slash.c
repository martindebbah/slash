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
        if (strcmp(cmd -> name, "pwd") == 0) {
            //test
            printf("%s \n",cmd_pwd());
        }

        // Si exit -> break
        if (strcmp(cmd -> name, "exit") == 0) {
            delete_cmd(cmd);
            
            // Stocker la valeur de sortie
            break;
        }
        if (strcmp(cmd -> name, "cd") == 0) {
            cmd_cd(cmd);
        }
        delete_cmd(cmd);
    }

    printf("%d\n", history_length);
    clear_history();
    printf("%d\n", history_length);
    return 0;
}

char *prompt(int val) {
    return "bash$ ";
}
