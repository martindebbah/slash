#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <limits.h>
#include <errno.h>

#include "cmd.h"
#include "mystring.h"
#include "commande.h"

/*
    Renvoie si le repertoire dir est la racine
*/
static int is_root(DIR* dir){
    struct stat st;
    
    if(stat("/",&st) < 0) return -1;

    ino_t root_ino = st.st_ino;
    dev_t root_dev = st.st_dev;

    fstat(dirfd(dir), &st);

    return root_ino == st.st_ino && root_dev == st.st_dev;
}

/*
    Renvoie le nom du repertoire dir
*/
static char* get_dirname(DIR* dir, DIR* parent) {
    struct stat st;
    struct dirent *entry;

    if(dir == NULL || parent == NULL)
        return NULL;  
    
    rewinddir(parent); // rembobine le repertoire parent car il a déjà été parcouru

    if(fstat(dirfd(dir),&st) < 0)
        return NULL;
    ino_t dir_ino = st.st_ino;
    dev_t dir_dev = st.st_dev;

    while(1){
        entry = readdir(parent);
        if(entry == NULL)
            return NULL;

        if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0)
            continue;
            
        if(fstatat(dirfd(parent), entry->d_name, &st, AT_SYMLINK_NOFOLLOW) < 0)
            return NULL;

        if(dir_ino == st.st_ino && dir_dev == st.st_dev)
            return entry->d_name;
    }
    
}

char *cmd_pwd(commande *cmd) {
    if (cmd -> nbParam == 0)
        return pwd(1);
    if (cmd -> nbParam == 1) {
        if(strcmp(getParamAt(cmd, 0), "-L") == 0)
            return pwd(1);

        if(strcmp(getParamAt(cmd, 0), "-P") == 0)
            return pwd(0);
    }

    char *ret = calloc(31, 1);
    memcpy(ret, "slash: pwd: too many arguments", 30);
    return ret;
}

/*
    Renvoie le chemin absolu du repertoire courant
*/
char * pwd(int l){
    if (l) {
        char *path = calloc(strlen(getenv("PWD")) + 1, 1);
        memcpy(path, getenv("PWD"), strlen(getenv("PWD")));
        return path;
    }else {
        struct string* path = string_new(100);
        DIR* dir = opendir(".");
        if (!dir)
            goto error;

        if(is_root(dir)){
            string_prepend(path, "/");
            closedir(dir);

            return copy(path);
        }
        int parent_fd = openat(dirfd(dir),"..", O_RDONLY | O_DIRECTORY);
        DIR* parent = fdopendir(parent_fd);

        if(!parent)
            goto error;

        while(!is_root(dir)){
            char* dirname = get_dirname(dir, parent);
            if(dirname == NULL)
                goto error;

            string_prepend(path,dirname);
            string_prepend(path,"/");
            closedir(dir);
            dir = parent;
            parent_fd = openat(dirfd(dir),"..", O_RDONLY | O_DIRECTORY);
            parent = fdopendir(parent_fd);
        }

        closedir(dir);
        closedir(parent);
        close(parent_fd);

        return copy(path);

        error:
        if (path)
            string_delete(path);
        if (dir) {
            closedir(dir);
            if (parent) {
                closedir(parent);
                if (parent_fd > -1)
                    close(parent_fd);
            }
        }
        return NULL;
    }
}

int cmd_cd(commande *cmd) {
    // Alloc une variable path qui sera le chemin vers lequel on change le répertoire courant
    int size = 256;
    char *path = calloc(size, 1);
    int l = 1;
    char *currentpwd = pwd(1);
    char *oldpwd = calloc(strlen(getenv("OLDPWD")) + 1, 1);
    memcpy(oldpwd, getenv("OLDPWD"), strlen(getenv("OLDPWD")));

    // Si pas de paramètres
    if (cmd -> nbParam == 0) {
        memcpy(path, getenv("HOME"), strlen(getenv("HOME")));
    }

    // Si 1 paramètre
    if (cmd -> nbParam == 1) {
        if (strcmp(getParamAt(cmd, 0), "-P") == 0) { // Si "-P" l = 0
            l = 0;
            memcpy(path, getenv("HOME"), strlen(getenv("HOME")));

        }else if (strcmp(getParamAt(cmd, 0), "-L") == 0) { // Si "-L"
            memcpy(path, getenv("HOME"), strlen(getenv("HOME")));

        }else if (strcmp(getParamAt(cmd, 0), "-") == 0) { // Si "-"
            memcpy(path, oldpwd, strlen(oldpwd));

        }else if (getParamAt(cmd, 0)[0] == '-' && strlen(getParamAt(cmd, 0)) > 1) { // Si mauvaise option
            printf("slash: cd: Option invalide: cd [-p | -L] <ref>\n");
            goto error;

        }else {// Si ref -> "-L ref"
            memcpy(path, getParamAt(cmd, 0), strlen(getParamAt(cmd, 0)));
        }
    }

    // Si 2 paramètres
    if (cmd -> nbParam == 2) {
        if (strcmp(getParamAt(cmd, 0), "-P") == 0) { // Si "-P ref"
            l = 0;
            memcpy(path, getParamAt(cmd, 1), strlen(getParamAt(cmd, 1)));

        }else if (strcmp(getParamAt(cmd, 0), "-L") == 0) { // Si "-L ref"
            memcpy(path, getParamAt(cmd, 1), strlen(getParamAt(cmd, 1)));

        }else { // Option invalide
            perror("slash: cd: Option invalide: cd [-p | -L] <ref>\n");
            goto error;
        }
    }

    if (cmd -> nbParam >= 3) {
        perror("slash: cd: Trop d'arguments: cd [-p | -L] <ref>\n");
        goto error;
    }

    if (!path)
        goto error;

    if (!l) { // Option "-P"
        if (chdir(path) != 0) { // Changement de dossier
            l = 1; // Si "-P" ne fonctionne pas on essaie "-L"
        }else {
            char *dir = pwd(0);
            setenv("PWD", dir, 1); // Changement de variable d'environnement pour "pwd -L"
            free(dir);
        }
    }

    if (l) { // Si pas option "-P" (ou échec)
        char *newpwd = calloc(size, 1);
        if (path[0] == '/') { // C'est la racine
            memcpy(newpwd, path, strlen(path));

        }else { // On modifie le path en conséquence
            char *updated = update_path(currentpwd, path); // Voir fonction
            memcpy(newpwd, updated, strlen(updated));
            free(updated);
        }

        if (chdir(newpwd) != 0) { // Changement de dossier
            free(newpwd);
            if (chdir(path) != 0) { // On essaie "-P" si échec
                perror("slash: cd"); // La référence n'existe pas
                goto error;
            }
            newpwd = pwd(0);
        }

        // Changement de variable d'environnement pour "pwd -L"
        setenv("PWD", newpwd, 1);
        free(newpwd);
    }

    // Changement de variable d'environnement pour "cd -"
    setenv("OLDPWD", currentpwd, 1);

    free(path);
    free(currentpwd);
    free(oldpwd);
    return 0;

    error:
        if (path)
            free(path);
        if (currentpwd)
            free(currentpwd);
        if (oldpwd)
            free(oldpwd);
        return 1;
}

char *update_path(char *path, char *update) {
    char *up = calloc(strlen(update) + 1, 1);
    memcpy(up, update, strlen(update));

    struct string *updated = string_new(strlen(path) + strlen(up) + 2); // Le path corrigé
    string_append(updated, path); // On ajoute le path actuel

    char *str = strtok(up, "/");
    while (str != NULL) { // Pour chaque élément de up séparé par '/'
        if (strcmp(str, "..") == 0) { // Si ".."
            int n = 0;
            for (int i = updated -> length - 1; i > 0; i--) { // Compte le nombre de caractère à enlever
                n++;
                if (updated -> data[i] == '/') // On s'arrête à '/'
                    break;
            }
            string_truncate(updated, n); // Enlève le dernier dossier du path et le '/'
        } else if (strcmp(str, ".") == 0) { // Si '.'
            // Ne fait rien
        }else {
            if (updated -> length > 1) // Si pas seulement '/'
                string_append(updated, "/"); // Ajoute '/'
            string_append(updated, str); // Ajoute l'élément
        }
        str = strtok(NULL, "/"); // Passe à l'élément suivant
    }

    // Retourne le path et désalloue la struct string
    return copy(updated);
}

// int cmd_cd(commande *cmd) {
//     //Squelette de la fonction cmd_cd
//     //TODO 1 : si param est null alors se déplacer à la racine du depot et return 0
//     //TODO 1 bis : si param est "~" ou "-P" sans suivant alors on se déplace à la racine du depot et return 0
//     //TODO 1 ter : si param est "-L" sans suivant alors on se déplace à la racine du depot en acceptant les liens symbolique et return 0
//     //TODO 2 : sinon
//         // a) si param->str == '-' alors se déplacer dans le répertoire précédent et return 0
//             //garder en mémoire le rep précédent donc avant dernier pwd
//         // b) si param->str == ref alors on se déplace dans ref en ignorant les liens symboliques avec return 0 et return 1 avec message d'erreur sinon
//         // c) si param->str == "-P ref" alors on se déplace dans ref en ignorant les liens symboliques avec return 0 ou si juste "-P" alors TODO 1 et return 1 sinon
//         // d) si param->str == "-L ref" alors on se déplace dans ref en suivant les liens symboliques avec return 0 ou si juste "-L" alors TODO 1 et  et return 1 sinon

//     char *oldpwd = getenv("OLDPWD"); // recupere l'ancien pwd
//     char *path = pwd(1); // recupere le pwd actuel
//     setenv("OLDPWD", path, 1); // met a jour l'ancien pwd
//     free(path);
    
//     //TODO 1 
//     if (cmd->nbParam == 0) {
//         if (chdir(getenv("HOME")) != 0) {
//             perror("chdir pour cd sans arguments echec \n");
//             return 1;
//         }
//         else {
//             return 0;
//         }
//     }    
//     if (cmd->nbParam == 1) {
//         //TODO 1 bis
//         if ((strcmp(cmd->param->str, "~") == 0 || strcmp(cmd->param->str, "-P") == 0)) {
//             if (chdir(getenv("HOME")) != 0) {
//                 perror("chdir pour cd ~ ou cd -P echec \n");
//                 return 1;
//             }
//             else {
//                 return 0;
//             }
//         }
//         //TODO 1 ter
//         if (strcmp(cmd->param->str, "-L") == 0) {
//             if (chdir(getenv("HOME")) != 0) {
//                 perror("chdir pour cd -L echec \n");
//                 return 1;
//             }
//             else {
//                 return 0;
//             }
//         }
//         //TODO 2 a)
//         if (strcmp(cmd->param->str, "-") == 0) {
//             if (chdir(oldpwd) != 0) {
//                 perror("chdir pour cd - echec \n");
//                 return 1;
//             }
//             else {
//                 return 0;
//             }
//         }
//         //TODO 2 b)
//         if (strcmp(cmd->param->str, "-L") != 0 && 
//             strcmp(cmd->param->str, "-P") != 0 &&
//             strcmp(cmd->param->str, "-") != 0 &&
//             strcmp(cmd->param->str, "~") != 0) {
//                 if (chdir(cmd->param->str) == 0) {
//                     return 0;
//                 }
//                 else {
//                     printf("bash: cd: %s: Aucun fichier ou dossier de ce type\n", cmd->param->str);
//                     return 1;
//                 }            
//         }
//     }
//     if (cmd->nbParam == 2) {
//         //TODO 2 c)
//         if (strcmp(getParamAt(cmd, 0), "-P") == 0) {
//             if (chdir(getParamAt(cmd, 1)) != 0) {
//                 perror("chdir pour cd -P repertoire echec \n");
//                 return 1;
//             }
//             else {
//                 return 0;
//             }
//         }
//         //TODO 2 d)
//         else if (strcmp(getParamAt(cmd, 0), "-L") == 0) {
//             if (chdir(getParamAt(cmd, 1)) != 0) {
//                 perror("chdir pour cd -L repertoire echec \n");
//                 return 1;
//             }
//             else {
//                 return 0;
//             }
//         }
//         else {
//             if (getParamAt(cmd, 0)[0] == '-') {
//                 printf("bash: cd: option non valable\n");
//                 printf("cd : utilisation : cd [-L|-P] [repertoire]\n");
//                 return 1;
//             }
//             else {
//                 printf("bash: cd: trop d'arguments\n");
//                 return 1;
//             }
//         }
//     }
//     if (cmd->nbParam > 2) {
//         printf("bash: cd: trop d'arguments\n");
//         return 1;
//     }
//     perror("aucun cas de cd fonctionne : echec \n");
//     return 1;
// }
