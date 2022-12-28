#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include "cmd.h"
#include "commande.h"
#include "redirection.h"
#include "slash.h"

#define MAX_ARGS_NUMBER 4096
#define MAX_ARGS_STRLEN 4096

static int val = 0; // La valeur de retour des commandes

int main(int argc, char **argv) {
    rl_outstream = stderr;

    // Ignorer SIGINT + SIGTERM
    struct sigaction action = {0};
    action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);

    char *oldpwd = pwd(1);
    setenv("OLDPWD", oldpwd, strlen(oldpwd)); // On set la valeur de OLDPWD pour premier appel à cd
    free(oldpwd);
    
    while (1) {
        // Affichage du prompt + récupération de la ligne de commande
        char *p = prompt(val);
        char *line = readline(p); // Les couleurs du prompt ne s'affichent pas sur MacOS
        free(p);

        // Si la ligne est NULL, est trop longue ou est une chaîne de caractères vide
        if (!line || strlen(line) == 0 || strlen(line) > MAX_ARGS_STRLEN) {        
            if (line == NULL) { // Si la ligne est NULL (EOF ou Ctrl-D)
                free(line);
                printf("\n");
                cmd_exit(val); // On quitte le programme
            }else // Si la ligne est vide ou trop longue, on attend la prochaine commande
                continue;
        }

        // Historique
        char *hist = calloc(strlen(line) + 1, 1);
        if (hist)
            memcpy(hist, line, strlen(line));

        // Découpage de la ligne de commande + gestion des jokers et redirections
        redirection *redir = create_redir(line);
        if (!redir) {
            free(line);
            free(hist);
            val = 127;
            continue;
        }

        // Ajout de la ligne à l'historique
        add_history(hist);
        free(line);
        free(hist);

        // Exécution des commandes
        val = executeRedirection(redir);
        delete_redir(redir);
    }

    clear_history();
    return 1;
}

// Execute la redirection utilisée dans la ligne de commande
int executeRedirection(redirection *redir) {
    int val = 0;

    if (redir -> type == NULL) { // Pas de redirection
        val = executeCmd(redir -> cmd);
    }else { //Redirection
        int fd[2];
        pipe(fd);

        pid_t pid = fork();

        char **p = paramToTab(redir -> cmd);
        if (pid == 0) { // Child process
            // Prise en compte des signaux par la commande externe
            struct sigaction action = {0};
            action.sa_handler = SIG_DFL;
            sigaction(SIGINT, &action, NULL);
            sigaction(SIGTERM, &action, NULL);

            // Redirection
            if (strcmp(redir -> type, ">")) { // Redirection de sortie
            }else if (strcmp(redir -> type, "<")) { // Redirection d'entrée
                freopen(redir -> fic, "r", stdin);
            }else if (strcmp(redir -> type, ">|")) { // Redirection de sortie avec création 
                freopen(redir -> fic, "w", stdout);
            }else if (strcmp(redir -> type, ">>")) { // Redirection d'ajout
                freopen(redir -> fic, "a", stdout);
            }else if (strcmp(redir -> type, "2>")) { // Redirection d'erreur
                freopen(redir -> fic, "a", stderr);
            }else if (strcmp(redir -> type, "2>|")) { // Redirection d'erreur avec création
                freopen(redir -> fic, "w", stderr);
            }else if (strcmp(redir -> type, "2>>")) { // Redirection d'ajout d'erreur
                freopen(redir -> fic, "a", stderr);
            }//else if (strcmp(redir -> type, "|")) { // Pipe}
            execvp(redir -> cmd -> name, p); // Exécution de la commande externe
            // Si la commande fonctionne -> recouvrement du processus fils
            // -> changement de main donc pas accès au code qui suit

            // Sinon, la commande n'existe pas
            exit(127);

        }else { // Parent process
            int status;
            waitpid(pid, &status, 0);

            if (WIFSIGNALED(status)) { // Si processus arrêté par un signal
                val = 255;
                printf("\n");
            }else // Sinon la valeur de retour du processus fils
                val = WEXITSTATUS(status);

        }

        delete_tab(p);
        close(fd[0]);
        close(fd[1]);
    }

    return val;
}


int executeCmd(commande *cmd) {
    if (strcmp(cmd -> name, "exit") == 0) { // Si exit
        // Stocker la valeur de sortie si spécifiée
        if (cmd -> nbParam > 0) {
            char *param = getParamAt(cmd, 0);
            char *end;
            long value = strtol(param, &end, 10);

            if (end == param) { // Valeur en argument invalide
                val = 2;
            }else { // Valeur valide
                val = (int) value;
            } 
        }

        delete_cmd(cmd);
        clear_history();
        cmd_exit(val);

    }else if (strcmp(cmd -> name, "pwd") == 0) { // Si pwd
        val = cmd_pwd(cmd);

    }else if (strcmp(cmd -> name, "cd") == 0) { // Si cd
        val = cmd_cd(cmd);

    }else { // Pas une commande interne
        pid_t pid = fork();
        char **p = paramToTab(cmd);

        if (pid == 0) { // Child process
            // Prise en compte des signaux par la commande externe
            struct sigaction action = {0};
            action.sa_handler = SIG_DFL;
            sigaction(SIGINT, &action, NULL);
            sigaction(SIGTERM, &action, NULL);

            execvp(cmd -> name, p); // Exécution de la commande externe
            // Si la commande fonctionne -> recouvrement du processus fils
            // -> changement de main donc pas accès au code qui suit

            // Sinon, la commande n'existe pas
            exit(127);
        }else { // Parent process
            int status;
            waitpid(pid, &status, 0);

            if (WIFSIGNALED(status)) { // Si processus arrêté par un signal
                val = 255;
                printf("\n");
            }else // Sinon la valeur de retour du processus fils
                val = WEXITSTATUS(status);
        }
        delete_tab(p);
    }

    return val;
}

char *prompt(int val) {
    int size = 52;  // 30 (longueur maximale du prompt) + 15 (Pour changer la couleur) + 6 (balises) + 1 ('\0')
    char *prompt = calloc(size, 1);
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
    char *wDir = pwd(1);
    char *path = cutPath(wDir, max);
    i += addToPrompt(prompt + i, path);
    free(wDir);
    free(path);

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
    if (val == 255) { // Valeur SIGNAL
        v[0] = 'S';
        v[1] = 'I';
        v[2] = 'G';
        size = 3;
    }else if (val > -1) { // Valeurs positives
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
    char *p = calloc(max + 1, 1);
    if (strlen(path) <= max) {
        memcpy(p, path, strlen(path));
    }else {
        memcpy(p, "...", 3);
        memcpy(p + 3, path + strlen(path) - max + 3, max - 3);
    }
    return p;
}
