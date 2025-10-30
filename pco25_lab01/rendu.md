## Timestamps

Le programme du dossier `timestamps` lance plusieurs threads qui enregistrent à chaque itération le temps courant (en nanosecondes) et l’identifiant du processeur utilisé. Les données sont ensuite sauvegardées et visualisées via un script Python.

En lançant le programme plusieurs fois, on observe que les threads peuvent s’exécuter sur différents cœurs et que l’ordonnancement varie d’une exécution à l’autre. Si l’on force l’utilisation d’un seul cœur avec `taskset`, tous les threads s’exécutent sur le même CPU et les changements de contexte sont plus fréquents.

Cela permet d’illustrer l’influence de l’ordonnanceur et du nombre de cœurs sur l’exécution concurrente.
**IMPORTANT ! Ce modèle est donné à titre d'exemple.** Vous êtes entièrement libres d’ajouter des sections ou des informations qui vous semblent pertinentes dans le rapport. N'oubliez pas de rendre seulement un PDF nommé *rapport.pdf*.

Tous les laboratoires ne demandent pas la même quantité de contenu écrit. Certains présentent un problème simple avec de nombreux tests envisageables, et l'inverse est aussi possible. Ne vous inquiétez pas pour la quantité.

# Labo 01

Auteurs : Nicolas Reymond , Nadia Cattin

## Interlacement sur std::cout

On pent voir que lorsqu'on utilise `cout` dans la fonction `run()` pour afficher le numéro du thread, on observe que les messages des différents threads peuvent s’entremêler sur la sortie standard. Cela se traduit par un affichage désordonné.
Ce phénomène est dû à l’accès concurrent à `cout` qui n’est pas protégé contre l’interlacement des sorties de plusieurs threads.

Lorsque l’on remplace `std::cout` par `logger()` (après avoir inclus `<pcosynchro/pcologger.h>` et ajouté `logger().setVerbosity(1);` dans le `main()`), l’affichage devient beaucoup plus lisible : chaque message de thread apparaît de façon distincte et ordonnée, sans chevauchement.

Cela s’explique par le fait que la classe `PcoLogger` utilisée par `logger()` met en place une synchronisation interne (par exemple via un mutex) pour garantir que les messages issus de différents threads ne s’interfèrent pas lors de l’affichage. Ainsi, chaque message est affiché de manière atomique, ce qui évite l’interlacement observé avec `std::cout`.


## Ordre d’exécution

> Observez-vous quelque chose de surprenant ?

l'affichage varie entre `Final value -> 0` et `Final value -> 1`

> Qu’en est-il de la reproductibilité des tests ?

Cela rend les tests variables donc pas fiable.

> Avec ce que vous connaissez, pourriez-vous modifier le code pour faire que les trois modifications
> de variables se fassent, mais que le résultat final soit toujours 1 ?

grâce à un ordonanceur.

## Introduction au problème

Ici, on attend une analyse du problème auquel vous faites face. De quoi parle-t-on ? Que doit faire le programme ? Etc. Ne copiez pas l'énoncé du laboratoire, décrivez simplement le sujet traité, etc.

## Choix d'implémentation

Dans cette section, vous **décrivez** vos choix et votre logique pour résoudre le problème posé. Ce que nous attendons de vous, c'est une explication claire et concise. Nous ne voulons pas que vous copiez votre code dans le rapport en disant : "Voilà ce que j'ai fait." Aucun code ne doit apparaître dans ce rapport. Expliquez simplement vos choix, les raisonnements que vous avez suivis, etc.

## Tests effectués

Les tests sont importants, ne les négligez pas. Le but d'un test n'est pas simplement de valider votre code dans un cas simple et de s'arrêter là. Il ne s'agit pas non plus de faire tourner votre programme de plus en plus longtemps. Réfléchissez aux moments potentiellement critiques de l'exécution du programme et cherchez à le faire planter ! C'est également ce que nous ferons lors de la correction. :)