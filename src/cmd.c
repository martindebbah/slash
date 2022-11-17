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
static char* get_dirname(DIR* dir, DIR* parent){
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
        if(fstatat(dirfd(parent), entry->d_name, &st, AT_SYMLINK_NOFOLLOW) < 0) return NULL;

        if(dir_ino == st.st_ino && dir_dev == st.st_dev) return entry->d_name;
    }
    
}

/*
    Renvoie le chemin absolu du repertoire courant
*/
char * cmd_pwd(){
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
        char* dirname = get_dirname(dir,parent);
        if(dirname == NULL) return NULL;
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
    //TODO 2 : sinon
        // a) si param->str == '-' alors se déplacer dans le répertoire précédent et return 0
            //garder en mémoire le rep précédent donc avant dernier pwd
        // b) si param->str == ref alors on se déplace dans ref en ignorant les liens symboliques avec return 0 et return 1 avec message d'erreur sinon
        // c) si param->str == "-P ref" alors on se déplace dans ref en ignorant les liens symboliques avec return 0 ou si juste "-P" alors TODO 1 et return 1 sinon
        // d) si param->str == "-L ref" alors on se déplace dans ref en suivant les liens symboliques avec return 0 ou si juste "-L" alors TODO 1 et  et return 1 sinon

    //TODO 1 
    if (cmd->nbParam == 0) {
        printf("cd \n");
        printf("voici le pwd avant le cd : %s\n", cmd_pwd());
        char *dir = getenv("HOME");
        printf("dir qui stocke getenv(HOME): %s\n", dir); //renvoie bien le chemin du home
        if (chdir(dir) != 0) { //il ne va pas dans ce cas
            perror("chdir pas ok");
            return 1;
        }
        else {
            /* Attention ! bon là c'est que des tests ça va mais fais gaffe à bien free les zones de mémoire qu'on 
            dynamiquement. Par exemple ici, il faut free ce que renvoie cmd_pwd() */
            printf("chdir ok\n");
            printf("voici le pwd apres le cd : %s\n", cmd_pwd()); //renvoie bien le home du depot
            return 0;
        }
    }




    /*
    //TODO 1 bis
    if (cmd->nbParam < 2) {
        if ((strcmp(cmd->param->str, "~") == 0 || strcmp(cmd->param->str, "-P") == 0)) {
            chdir(getenv("HOME"));
            return 0;
        }
        //TODO 2 a)
        if (strcmp(cmd->param->str, "-") == 0) {
            //chdir(getenv("OLDPWD")); ca marche pas encore
            return 0;
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
    //TODO 2 c)
    //TDOD 2 d)
    */
    return 1;
}
