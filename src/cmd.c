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
    int parent_fd = openat(dirfd(dir),"..", O_RDONLY | O_DIRECTORY);
    DIR* parent = fdopendir(parent_fd);

    if(!dir || !parent) return NULL;

    while(!is_root(dir)){
        char* dirname = get_dirname(dir,parent);
        printf("%s ",dirname);
        if(dirname == NULL) return NULL;
        string_append(path,dirname);
        string_append(path,"/");
        closedir(dir);
        dir = parent;
        parent_fd = openat(dirfd(dir),"..", O_RDONLY | O_DIRECTORY);
        parent = fdopendir(parent_fd);
    }
    
    closedir(parent);

    printf("%s ",path->data);

    return path->data;
}
