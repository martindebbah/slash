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
static char* get_dirname(DIR* dir, DIR* parent, int followSymLink) {
    struct stat st;
    struct dirent *entry;

    if(dir == NULL || parent == NULL) return NULL;  
    
    rewinddir(parent); // rembobine le repertoire parent car il a déjà été parcouru

    if(fstat(dirfd(dir),&st) < 0) return NULL;
    ino_t dir_ino = st.st_ino;
    dev_t dir_dev = st.st_dev;

    while(1){
        entry = readdir(parent);
        if(entry == NULL) return NULL;
        if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0) continue;
        /*
        Comment utiliser AT_SYMLINK_FOLLOW ??
        */
        // if (followSymLink) {
            // if(fstatat(dirfd(parent), entry->d_name, &st, AT_SYMLINK_FOLLOW) < 0) return NULL;
        // }else {
            if(fstatat(dirfd(parent), entry->d_name, &st, AT_SYMLINK_NOFOLLOW) < 0) return NULL;
        // }

        if(dir_ino == st.st_ino && dir_dev == st.st_dev) return entry->d_name;
    }
    
}

char *cmd_pwd(commande *cmd) {
    if (cmd -> nbParam == 0)
        return pwd(1);
    if (cmd -> nbParam == 1 && (strcmp(getParamAt(cmd, 0), "-L") == 0 || strcmp(getParamAt(cmd, 0), "-P") == 0)) {
        if (strcmp(getParamAt(cmd, 0), "-L") == 0)
            return pwd(1);
        return pwd(0);
    }
    char *ret = calloc(24, 1);
    memcpy(ret, "pwd: too many arguments", 23);
    return ret;
}

/*
    Renvoie le chemin absolu du repertoire courant
*/
char * pwd(int followSymLink){
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
        char* dirname = get_dirname(dir, parent, followSymLink);
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

int cmd_cd(commande *cmd) {
    //Squelette de la fonction cmd_cd
    //TODO 1 : si param est null alors se déplacer à la racine du depot et return 0
    //TODO 1 bis : si param est "~" ou "-P" sans suivant alors on se déplace à la racine du depot et return 0
    //TODO 1 ter : si param est "-L" sans suivant alors on se déplace à la racine du depot en acceptant les liens symbolique et return 0
    //TODO 2 : sinon
        // a) si param->str == '-' alors se déplacer dans le répertoire précédent et return 0
            //garder en mémoire le rep précédent donc avant dernier pwd
        // b) si param->str == ref alors on se déplace dans ref en ignorant les liens symboliques avec return 0 et return 1 avec message d'erreur sinon
        // c) si param->str == "-P ref" alors on se déplace dans ref en ignorant les liens symboliques avec return 0 ou si juste "-P" alors TODO 1 et return 1 sinon
        // d) si param->str == "-L ref" alors on se déplace dans ref en suivant les liens symboliques avec return 0 ou si juste "-L" alors TODO 1 et  et return 1 sinon

    char *oldpwd = getenv("OLDPWD"); // recupere l'ancien pwd
    char *path = pwd(1); // recupere le pwd actuel
    setenv("OLDPWD", path, 1); // met a jour l'ancien pwd
    free(path);
    
    //TODO 1 
    if (cmd->nbParam == 0) {
        if (chdir(getenv("HOME")) != 0) {
            perror("chdir pour cd sans arguments echec \n");
            return 1;
        }
        else {
            return 0;
        }
    }    
    if (cmd->nbParam == 1) {
        //TODO 1 bis
        if ((strcmp(cmd->param->str, "~") == 0 || strcmp(cmd->param->str, "-P") == 0)) {
            if (chdir(getenv("HOME")) != 0) {
                perror("chdir pour cd ~ ou cd -P echec \n");
                return 1;
            }
            else {
                return 0;
            }
        }
        //TODO 1 ter
        if (strcmp(cmd->param->str, "-L") == 0) {
            if (chdir(getenv("HOME")) != 0) {
                perror("chdir pour cd -L echec \n");
                return 1;
            }
            else {
                return 0;
            }
        }
        //TODO 2 a)
        if (strcmp(cmd->param->str, "-") == 0) {
            if (chdir(oldpwd) != 0) {
                perror("chdir pour cd - echec \n");
                return 1;
            }
            else {
                return 0;
            }
        }
        //TODO 2 b)
        if (strcmp(cmd->param->str, "-L") != 0 && 
            strcmp(cmd->param->str, "-P") != 0 &&
            strcmp(cmd->param->str, "-") != 0 &&
            strcmp(cmd->param->str, "~") != 0) {
                if (chdir(cmd->param->str) == 0) {
                    return 0;
                }
                else {
                    printf("bash: cd: %s: Aucun fichier ou dossier de ce type\n", cmd->param->str);
                    return 1;
                }            
        }
    }
    if (cmd->nbParam == 2) {
        //TODO 2 c)
        if (strcmp(getParamAt(cmd, 0), "-P") == 0) {
            if (chdir(getParamAt(cmd, 1)) != 0) {
                perror("chdir pour cd -P repertoire echec \n");
                return 1;
            }
            else {
                return 0;
            }
        }
        //TODO 2 d)
        else if (strcmp(getParamAt(cmd, 0), "-L") == 0) {
            if (chdir(getParamAt(cmd, 1)) != 0) {
                perror("chdir pour cd -L repertoire echec \n");
                return 1;
            }
            else {
                return 0;
            }
        }
        else {
            if (getParamAt(cmd, 0)[0] == '-') {
                printf("bash: cd: option non valable\n");
                printf("cd : utilisation : cd [-L|-P] [repertoire]\n");
                return 1;
            }
            else {
                printf("bash: cd: trop d'arguments\n");
                return 1;
            }
        }
    }
    if (cmd->nbParam > 2) {
        printf("bash: cd: trop d'arguments\n");
        return 1;
    }
    perror("aucun cas de cd fonctionne : echec \n");
    return 1;
}
