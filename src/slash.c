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
    int val = 0;
    
    while (1) {
        // Affichage du prompt + récupération de la ligne de commande
        char *p = prompt(val);
        char *line = readline(p); // Les couleurs du prompt ne s'affichent pas sur MacOS
        free(p);

        if (!line || !*line || strlen(line) > MAX_ARGS_STRLEN) {
            free(line);
            continue;
        }

        // Historique
        char *hist = calloc(strlen(line) + 1, 1);
        if (hist)
            memcpy(hist, line, strlen(line));

        // Découpage de la ligne de commande
        commande *cmd = create_cmd(line);
        if (!cmd) {
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
        val = executeCmd(cmd);
        delete_cmd(cmd);
    }

    clear_history();
    return 1;
}

int executeCmd(commande *cmd) {
    int val = 0; // val de retour des fonctions

    if (strcmp(cmd -> name, "exit") == 0) { // Si exit -> break
        // Stocker la valeur de sortie (0 si pas de paramètres)
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
        printf("Le processus slash s'est terminé avec le code de retour %d\n", val);
        exit(val);

    }else if (strcmp(cmd -> name, "pwd") == 0) { // PWD
        char *wDir = cmd_pwd();
        if (wDir) {
            printf("%s \n", wDir);
            free(wDir);
        }else {
            perror("Erreur pwd");
            return -1;
        }

    }else if (strcmp(cmd -> name, "cd") == 0) { // CD
        val = cmd_cd(cmd);

    }else { // Pas une commande interne
        char **p = paramToTab(cmd);
        pid_t pid = fork();
        if (pid == 0) { // Child
            execvp(cmd -> name, p);
        }else { // Parent
            int status;
            waitpid(pid, &status, 0);
        }
        // Comment retourne la valeur si échec ?

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
    char *wDir = cmd_pwd();
    char *path = cutPath(wDir, max);
    i += addToPrompt(prompt + i, path);
    free(wDir);

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
    max -= 3; // Pour "..." au début
    if (strlen(path) <= max)
        return path;
        
    int d = strlen(path); // On commence avec une balise sur le dernier caractère
    int i = d;
    while (strlen(path) - i < max) { // On lit les caractères de la fin vers le début
        if (path[i] == '/') {
            d = i; // On balise dès qu'on trouve un '/'
        }
        i--;
    }

    // On ajoute "..."
    path[--d] = '.';
    path[--d] = '.';
    path[--d] = '.';

    // On retourne ".../**", en ne coupant pas de nom de répertoire
    return path + d;
}
