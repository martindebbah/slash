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

int cmd_pwd(commande *cmd) {
    char *dir;
    if (cmd -> nbParam == 0)
        dir = pwd(1);

    if (cmd -> nbParam == 1) {
        if(strcmp(getParamAt(cmd, 0), "-L") == 0)
            dir = pwd(1);

        if(strcmp(getParamAt(cmd, 0), "-P") == 0)
            dir = pwd(0);
    }

    if (!dir)
        return 1;

    printf("%s\n", dir);
    free(dir);
    return 0;
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
    char *oldpwd = calloc(size, 1);
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
            printf("slash: cd: Option invalide: cd [-P | -L] [ref | -]\n");
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
            printf("slash: cd: Option invalide: cd [-P | -L] [ref | -]\n");
            goto error;
        }
    }

    if (cmd -> nbParam >= 3) {
        printf("slash: cd: Trop d'arguments: cd [-P | -L] [ref | -]\n");
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
        char *updated;
        if (path[0] == '/') // C'est la racine
            updated = update_path("/", path); // On modifie avec comme dir "/"
        else
            updated = update_path(currentpwd, path); // On modifie avec currentPwd
        
        memcpy(newpwd, updated, strlen(updated));
        free(updated);

        if (chdir(newpwd) != 0) { // Changement de dossier
            free(newpwd);
            if (chdir(path) != 0) { // On essaie "-P" si échec
                printf("slash: cd: Référence invalide: cd [-P | -L] [ref | -]\n"); // La référence n'existe pas
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
    free(up);

    // Retourne le path et désalloue la struct string
    return copy(updated);
}

void cmd_exit(int val) {
    printf("Le processus slash s'est terminé avec le code de retour %d\n", val);
    exit(val);
}
