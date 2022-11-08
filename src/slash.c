#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "cmd.h"

#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096

int main(int argc, char **argv) {

    rl_outstream = stderr;
    
    while (1) {
        // Affichage du prompt + récupération de la ligne de commande
        char *cmd = readline("test $"); // A remplacer par le prompt
        add_history(cmd);

        // Découpage de la ligne de commande
        // char **l = process_cmd(cmd);

        // Interprétation de la ligne de commande

        // Exécution des commandes (fork) et attente de la fin du processus fils
        // Si exit -> break
    }

    return 0;
}
