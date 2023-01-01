#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
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

void reset(int fd_in, int fd_out, int fd_err) {
    // Réinitialisation de l'entrée standard
    dup2(fd_in, 0);
    close(fd_in);
    
    // Réinitialisation de la sortie standard
    dup2(fd_out, 1);
    close(fd_out);

    // Réinitialisation de la sortie erreur
    dup2(fd_err, 2);
    close(fd_err);
}

// Redirection d'entrée
int redirect_in(char *fic) {
    int fd = open(fic, O_RDONLY);
    if (fd == -1) {
        perror("Error while opening file for input redirection");
        return -1;
    }

    dup2(fd, 0);
    close(fd);
    return 0;
}

int redirect_out(char *r, char *fic) {
    int fd = -1;
    if (strcmp(r, ">") == 0) { // Redirection de sortie
        fd = open(fic, O_WRONLY | O_CREAT | O_EXCL, 0644);

    }else if (strcmp(r, ">|") == 0) { // Redirection de sortie avec création
        fd = open(fic, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    }else if (strcmp(r, ">>") == 0) { // Redirection d'ajout
        fd = open(fic, O_WRONLY | O_APPEND | O_CREAT, 0644);
    }

    if (fd == -1) {
        perror("Error while creating file for standard output redirection");
        return -1;
    }

    dup2(fd, 1);
    close(fd);
    return 0;
}

int redirect_err(char *r, char *fic) {
    int fd = -1;
    if (strcmp(r, "2>") == 0) { // Redirection d'erreur
        fd = open(fic, O_WRONLY | O_CREAT | O_EXCL, 0644);

    }else if (strcmp(r, "2>|") == 0) { // Redirection d'erreur avec création
        fd = open(fic, O_WRONLY | O_TRUNC | O_CREAT, 0644);

    }else if (strcmp(r, "2>>") == 0) { // Redirection d'ajout d'erreur
        fd = open(fic, O_WRONLY | O_APPEND | O_CREAT, 0644);
    }

    if (fd == -1) {
        perror("Error while creating file for error output redirection");
        return -1;
    }

    dup2(fd, 2);
    close(fd);
    return 0;
}

int executeRedirection(redirection *redir) {
    int stdin_fd = dup(0);
    int stdout_fd = dup(1);
    int stderr_fd = dup(2);

    // Execute la redirection utilisée dans la ligne de commande
    if (isRedir(redir) == 0 || strcmp(redir -> cmd -> name, "exit") == 0) { // Pas de redirection
        val = executeCmd(redir -> cmd);

    }else { // Redirection
        redirection *current = redir;
        int isFirst = 1;
        while (current != NULL) {
            int fd[2];
            if (pipe(fd) == -1) {
                // Erreur lors de la création du tube
                return 1;
            }

            pid_t pid = fork();

            if (pid == 0) { // Processus enfant
                if (isPipeline(current)) { // Si encore une commande après
                    dup2(fd[1], 1); // Redirection de la sortie standard vers l'entrée du tube
                    close(fd[0]);
                    close(fd[1]);

                }else { // Si c'est la dernière commande
                    int r;
                    if (current -> out) // Elle a une redirection de sortie standard
                        r = redirect_out(current -> out, current -> fic_out);
                    else // Sinon, réinitialisation de la sortie standard
                        r = dup2(stdout_fd, 1);
                    if (r == -1)
                        exit(1);
                }

                if (current -> err) // Si redirection de la sortie erreur
                    if (redirect_err(current -> err, current -> fic_err) == -1)
                        exit(1);

                if (isFirst && current -> in) // Si redirection de l'entrée standard
                    if (redirect_in(current -> fic_in) == -1)
                        exit(1);

                exit(executeCmd(current->cmd));
                
            } else { // Processus parent
                dup2(fd[0], 0); // Redirection de l'entrée depuis le tube
                close(fd[0]);
                close(fd[1]);

                int status;
                waitpid(pid, &status, 0); // Attente de la terminaison du processus enfant

                val = WEXITSTATUS(status);
            }
            
            current = current->suivante;
            isFirst = 0;

            if (val != 0)
                current = NULL;
        }
    }
    reset(stdin_fd, stdout_fd, stderr_fd);
    
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

/*
    do {
        nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

        if (isTokRed(str)) { // Si token de redirection
            while (str && isTokRed(str) && isTokPipe(str) == 0) {
                if (strcmp(str, "<") == 0) {
                    redir -> in = calloc(strlen(str) + 1, 1);
                    memcpy(redir -> in, str, strlen(str));
                    if (!redir -> in)
                        goto error;

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    str = strtok(NULL, " ");
                    if (str == NULL)
                        goto error;

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

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    if (!redir -> fic_in)
                        goto error;
                }else if (strcmp(str, ">") == 0 || strcmp(str, ">>") == 0 || strcmp(str, ">|") == 0) {
                    redir -> out = calloc(strlen(str) + 1, 1);
                    memcpy(redir -> out, str, strlen(str));
                    if (!redir -> out)
                        goto error;

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    str = strtok(NULL, " ");
                    if (str == NULL)
                        goto error;

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

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    if (!redir -> fic_out)
                        goto error;
                }else if (strcmp(str, "2>") == 0 || strcmp(str, "2>>") == 0 || strcmp(str, "2>|") == 0) {
                    redir -> err = calloc(strlen(str) + 1, 1);
                    memcpy(redir -> err, str, strlen(str));
                    if (!redir -> err)
                        goto error;

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    str = strtok(NULL, " ");
                    if (str == NULL)
                        goto error;
                        
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

                    nextCmd = string_truncate_token_and_spaces(nextCmd, strlen(str));

                    if (!redir -> fic_err)
                        goto error;
                }

                    str = strtok(NULL, " ");
            }

            if (str && isTokPipe(str)) { // Si la redirection est une pipeline
            redir -> pipe = calloc(strlen(str) + 1, 1);
            memcpy(redir -> pipe, str, strlen(str));
            if (!redir -> pipe)
                goto error;

            char *c = copy(nextCmd);
            nextCmd = NULL; // Pour éviter le double free lors d'un `goto error`
            redir -> suivante = create_redir(c);
            free(c);
            if (!redir -> suivante)
                goto error;
            }else
                string_delete(nextCmd);

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

*/
