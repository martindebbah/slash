# Stratégie adoptée pour répondre au sujet 

## Architecture logicielle 

À la racine de notre projet nous avons 2 sous-arborescences :
- L'une correspondant au dossier include comportant nos bibliothèques code.h .
- L'autre correspondant au dossier src comportant nos fichiers source code.c .
Chaque code.c a sa bibliotèque code.h dans notre architecture.
Cela nous permet de rendre notre code modulaire.
Également à la racine de notre dépot nous avons un Makefile qui nous sert à tout compiler et à créer notre exécutable slash.

## Structure de données

Dans le dossier include nous retrouvons les fichiers correspondants :

1. cmd.h : correspond à la documentation des commandes internes que nous devons implémenter

2. commande.h : correspond à la création des structures commande et parametres et à la documentation des fonctions qui s'occuperont de la création/suppression des commandes et la création/manipulation/suppression des paramêtres.

3. mystring.h : correspond à la création de la structure string et à la documentation des fonctions qui s'occuperont de création/manipulation/suppression de string

4. slash.h : correspond à la documentation des fonctions qui manipuleront le prompt et qui exécutera la commande

Dans le dossier src nous retrouvons les mêmes fichiers :
cmd.c, commandes.c, mystring.c et slash.c qui sont l'implémentation des fichiers du dossier include.

## Algorithmes implémentés
- Pour cmd.c :
    - static int is_root(DIR* dir) : Renvoie si le repertoire dir est la racine.
    - static char* get_dirname(DIR* dir, DIR* parent) : Renvoie le nom du repertoire dir.
    - cmd_pwd(commande *cmd) : Affiche la (plus precisement, une) reference absolue du repertoire de travail courant avec les options -P et -L.
    - cmd_cd(commande *cmd) : Change de repertoire de travail courant.
    - Pour la commande interne exit elle est gêrée directement dans le fichier principal slash.c dans executeCmd(commande *cmd).

- Pour commande.c :
    - commande *create_cmd(char *line) : Crée une commande à partir d'une ligne.
    - parametres *create_param() : Crée une liste de paramètres.
    - void delete_cmd(commande *cmd) : Désaloue la mémoire prise par une commande.
    - int getNbParam(parametres *p) : Retourne le nombre de paramètres.
    - char **paramToTab(commande *cmd) : Retourne les paramètres sous forme de tableau.
    - char *getParamAt(commande *cmd, int i) : Renvoie le paramètre à la position i (la position 0 étant la première position de la liste).
    - char *getParam(parametres *p, int i) : Renvoie le paramètre si c'est le i-ème de la liste.
    - void delete_param(parametres *p) : Désaloue la mémoire prise par les paramètres.
    - char *update_path(char *path, char *update) : Créer un path en résolvant les liens symboliques

- Pour mystring.c :
    - struct string * string_new(size_t capacity) : Crée une nouvelle chaîne de longueur 0 et de capacité donnée, allouée sur le tas (c'est-à-dire avec malloc).
    - void string_delete(struct string * str) : Détruit une chaîne, en libérant la mémoire occupée.
    - int string_append (struct string * dest, char * src) : Ajoute SRC à la fin de DEST.
    - int string_prepend(struct string * dest, char * src) : Ajoute SRC au début de DEST.
    - void string_truncate (struct string * str, size_t nchars) : Tronque la chaîne en enlevant les nchars derniers caractères.
    - char *copy(struct string *path) : Copie et supprime le string.

- Pour slash.c :
    - int executeCmd(commande *cmd) : Exécute la commande.
    - char *prompt(int val) : Renvoie le prompt à afficher.
    - int changeColor(char *s, char color) : Change la couleur du prompt.
    - int addToPrompt(char *p, char *s) : Ajoute s au prompt.
    - int addVal(char *p, int val) : Ajoute la valeur de retour au prompt.
    - char *cutPath(char *path, int max) : Découpe le path pour ne pas dépasser la limite de caractères.
