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
    if(is_root(dir)){
        string_prepend(path, "/");
        closedir(dir);
        return path->data;
    }
    int parent_fd = openat(dirfd(dir),"..", O_RDONLY | O_DIRECTORY);
    DIR* parent = fdopendir(parent_fd);

    if(!dir || !parent) return NULL;

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
    
    closedir(parent);

    return path->data;
}

int cmd_cd(parametres *p) {
    //Squelette de la fonction cmd_cd
    //TODO 1 : si parametre->str est null ou "~" alors se déplacer à la racine du depot et return 0
    //TODO 2 : sinon
        // a) si parametre->str == '-' alors se déplacer dans le répertoire précédent et return 0
        // b) si parametre->str == ref alors on se déplace dans ref en ignorant les liens symboliques avec return 0 et return 1 avec message d'erreur sinon
        // c) si parametre->str == "-P ref" alors on se déplace dans ref en ignorant les liens symboliques avec return 0 ou si juste "-P" alors TODO 1 et return 1 sinon
        // d) si parametre->str == "-L ref" alors on se déplace dans ref en suivant les liens symboliques avec return 0 ou si juste "-L" alors TODO 1 et  et return 1 sinon

    //TODO 1 
    
    if (p != NULL ) {
        if (p->str == NULL || strcmp(p->str, "~")==0) {
            printf("je suis la 0");
            return 0;
        }
        printf("je suis la 1");
        return 1;
    }
    chdir(getenv("HOME"));
    printf("je suis la 2");
    return 2;
    //TODO 2 a)
    /*
    if (strcmp(p->str, "-")==0) {
        chdir(getenv("HOME"));
        return 0;
    }
    //TODO 2 b)
    else if (strcmp(p->str, NULL)!=0 && strcmp(p->str, "-")!=0 && strcmp(p->str, "-P")!=0 && strcmp(p->str, "-L")!=0) {
        chdir(p->str);
        return 0;
    }
    //TODO 2 c)
    else if (strcmp(p->str, "-P")==0) {
        if (strcmp(p->suivant->str, NULL)==0) {
            chdir("/");
            return 0;
        }
        else {
            chdir(p->suivant->str);
            return 0;
        }
    }
    //TODO 2 d)
    else if (strcmp(p->str, "-L")==0) {
        if (strcmp(p->suivant->str, NULL)==0) {
            chdir("/");
            return 0;
        }
        else {
            //gerer les liens symboliques
            return 0;
        }
    }    
    */
   /*
    else {
        perror("Le repertoire n'existe pas");
        return 1;
    }
    */
    
    
}
