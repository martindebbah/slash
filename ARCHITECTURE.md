slash (small laudable shell) :

# Fonctionnalitées 

## Commandes internes (**jalon-1**)

- exit [val]

Termine le processus en retournant `val` (0 si pas d'argument)

- pwd [-L | -P]

Affiche le répertoire courant

- cd [-L | -P] [ref | -]

Change le répertoire courant

## Redirection des flots (**rendu-final**)

- `>`, `<`, `>|` et `>>` pour le flot standard
- `2>`, `>|` et `2>>` pour le flot erreur

## Combinaison par tube (**rendu-final**)

- `cmd1 | cmd2` -> Sortie standard de `cmd1` dans entrée standard de `cmd2` (enchaînable plusieurs fois)

## Formattage du prompt avant chaque commande (**jalon-1** *sans valeur de retour*) :

1. Changer la couleur : "\033[32m" -> vert pour succès | "\033[91m" -> rouge pour échec
2. Afficher `[val]` -> Avec val la valeur de retour de la dernière commande (0 pour le premier prompt) **(jalon-2)**
3. Changer la couleur : "\033[34m" | "\033[36m"
4. Afficheer `rep` : "..." + répertoire courant (tronqué pour ne pas dépasser 30 caractères du prompt)
4. Changer la couleur : "\033[00m"
5. Afficher `$ `

## Expansion de chemins contenant Jokers

- `*` -> préfixe quelconque **(jalon-2)**
- `**/` -> `*/*/.../` **(rendu-final)**

# Modalités de rendu

## Jalon-1

- Compléter `AUTHORS.md`
- Compléter `Makefile`
- Fonctionnalités marquées de **jalon-1**

## Jalon-2

- Jalon-1
- Commandes externes sans redirection
- Fonctionnalités marquées de **jalon-2**

## Rendu-final

- Toutes les fonctionnalités