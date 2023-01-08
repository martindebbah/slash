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

4. redirection.h : 

5. slash.h : correspond à la documentation des fonctions qui manipuleront le prompt et qui exécutera la commande

6. string_list.h :

Dans le dossier src nous retrouvons les mêmes fichiers :
cmd.c, commandes.c, mystring.c, redirection.c, slash.c et string_list.c qui sont l'implémentation des fichiers du dossier include.

## Algorithmes implémentés
- Pour cmd.c :
    - static int is_root(DIR* dir) : Renvoie si le repertoire dir est la racine.
    - static char* get_dirname(DIR* dir, DIR* parent) : Renvoie le nom du repertoire dir.
    - int is_prefix(char *pre, char *suf) : Renvoie 1 si pre est un prefixe de suf sinon renvoie 0.
    - string_list* parcours_repertoire(char* dir_to_open, char* suf, char *word, int double_star) : parcours le repertoire dir_to_open et stock chaque nom dans les paramètres de cmd.
    - int compare_word(char *tmp, char *word) : Verifie si il y a une occurence de name dans word.
    - cmd_pwd(commande *cmd) : Affiche la (plus precisement, une) reference absolue du repertoire de travail courant avec les options -P et -L.
    - char * pwd(int l) : Renvoie le chemin absolu du repertoire courant.
    - cmd_cd(commande *cmd) : Change de repertoire de travail courant.
    - char *update_path(char *path, char *update) : met à jour le path du prompt.
    - cmd_exit(int val) : Termine le processus slash avec comme valeur de retour val (ou par défaut la valeur de retour de la dernière commande exécutée).


- Pour commande.c :
    - commande *create_cmd(char *line) : Crée une commande à partir d'une ligne.
    - void delete_cmd(commande *cmd) : Désaloue la mémoire prise par une commande.
    - parametres *create_param() : Crée un paramètre.
    - parametres *create_param_list(string_list *l) : Crée une liste de paramètres.
    - int is_joker_prefix(char *str) : Renvoie 1 si tous les jokers sont prefixes sinon 0
    - string_list *process_joker(char *str) : Evalue la chaîne de caractère en résolvant les étoiles
    - int getNbParam(parametres *p) : Retourne le nombre de paramètres.
    - char **paramToTab(commande *cmd) : Retourne les paramètres sous forme de tableau.
    - void delete_tab(char **tab) : Désaloue les paramètres qui étaient sous forme de tableau.
    - char *getParamAt(commande *cmd, int i) : Renvoie le paramètre à la position i (la position 0 étant la première position de la liste).
    - char *getParam(parametres *p, int i) : Renvoie le paramètre si c'est le i-ème de la liste.
    - void delete_param(parametres *p) : Désaloue la mémoire prise par les paramètres.

- Pour mystring.c :
    - struct string * string_new(size_t capacity) : Crée une nouvelle chaîne de longueur 0 et de capacité donnée, allouée sur le tas (c'est-à-dire avec malloc).
    - void string_delete(struct string * str) : Détruit une chaîne, en libérant la mémoire occupée.
    - int string_append (struct string * dest, char * src) : Ajoute SRC à la fin de DEST.
    - int string_prepend(struct string * dest, char * src) : Ajoute SRC au début de DEST.
    - struct string *string_truncate_token_and_spaces(struct string *str, size_t n) : Enlève les n premiers caractères + les espaces qui suivent.
    - void string_truncate (struct string * str, size_t nchars) : Tronque la chaîne en enlevant les nchars derniers caractères (ou tous les caractères s'il y en a moins de nchars).
    - void string_truncate_where (struct string * str, char delimit) : Tronque la chaîne après delimit
    - char *copy(struct string *path) : Copie et supprime le string.
    - struct string *clean(struct string *path) : Nettoye le path en enlevant les '/' en trop.

- Pour redirection.c :
    - int isTokRed(char *str) : renvoie si str est un token de redirection
    - int isTokPipe(char *str) : renvoie si str est un token de redirection de type pipeline
    - redirection *create_redir(char *line) : Crée une redirection.
    - void delete_redir(redirection *redir) : Supprime la redirection.
    - int isRedir(redirection *redir) : Renvoie 1 si la commande contient une redirection sinon renvoie 0 si c'est seulement une commande.
    - int isPipeline(redirection *redir) : Renvoie 1 si la redirection est de type pipeline sinon renvoie 0 si c'est une redirection vers/depuis un fichier.

- Pour slash.c :
    - int executeRedirection(redirection *redir) : Exécute la redirection
    - int executeCmd(commande *cmd, int val) : Exécute la commande.
    - char *prompt(int val) : Renvoie le prompt à afficher.
    - int changeColor(char *s, char color) : Change la couleur du prompt.
    - int addToPrompt(char *p, char *s) : Ajoute s au prompt.
    - int addVal(char *p, int val) : Ajoute la valeur de retour au prompt.
    - char *cutPath(char *path, int max) : Découpe le path pour ne pas dépasser la limite de caractères.
    - int joker_processing(commande *cmd) : Traite le joker si il y en a un dans les parametres de cmd

- Pour string_list.c :
    - string_list *list_create() : Crée une liste de chaîne de caractères.
    - void list_append(string_list *l, char *s) : Ajoute une chaîne de caractères à la liste.
    - void list_delete(string_list *l) : Désalloue la mémoire.
    - string_list* list_cat(string_list* l1, string_list* l2) : Concatène deux listes.
