# Laboratoire 02 - Détection de nombres premiers multi-threadé

Auteurs : Nicolas Reymond, Nadia Cattin

## Introduction au problème

Le but de ce laboratoire est d'implémenter et de comparer deux paradigmes de détection de nombres premiers : une version séquentielle (mono-threadée) et une version concurrente (multi-threadée). L'algorithme de base repose sur la vérification des diviseurs potentiels $d$ pour un nombre $n$, où $2 \leq d \leq \sqrt{n}$.

## Choix d'implémentation

### Version mono-threadée

L'implémentation séquentielle se déroule selon les étapes suivantes :

1.  **Gestion des cas triviaux :** Les nombres inférieurs à 2 sont exclus, ainsi que les nombres pairs.
2.  **Détermination de l'intervalle :** La limite de recherche est définie par la racine carrée de $n$ ($\sqrt{n}$).
3.  **Optimisation des tests :** Le parcours des diviseurs potentiels commence à 3 et se limite aux nombres impairs.
4.  **Terminaison immédiate :** La fonction s'arrête dès qu'un diviseur est trouvé, puisque $n$ ne peut alors plus être premier.

L'ajout du test de parité a permis de minimiser le temps de test pour les nombres impairs.

### Version multi-threadée

Afin d'améliorer les performances, la tâche de détection est décomposée et exécutée de manière concurrente par plusieurs threads. Le modèle d'implémentation repose sur la partition de l'intervalle de recherche. Les étapes clés de cette version sont :

1.  **Gestion des cas triviaux :** Les nombres inférieurs à 2 sont exclus, ainsi que les nombres pairs.
2.  **Décomposition de la tâche :** L'intervalle de recherche, de 3 à $\sqrt{n}$, est divisé en sous-intervalles de taille égale, chaque sous-intervalle étant attribué à un thread distinct créé à chaque appel de la fonction `isPrime()`.
3.  **Synchronisation et ressources partagées :** Une variable partagée (`divisorFound`) est mise en place pour signaler la découverte d'un diviseur par n'importe quel thread. L'accès concurrent à cette variable est protégé par un mutex.
3.  **Politique de terminaison :** Si un thread trouve un diviseur, il met à jour la variable partagée et se termine. Les autres threads vérifient l'état de cette variable périodiquement, toutes les `checkInterval` itérations, et s'arrêtent s'ils détectent l'arrêt demandé.
4.  **Jointure (`join`) :** La fonction principale attend la terminaison de tous les threads avant de reprendre son exécution pour retourner le résultat de la fonction.

## Tests effectués

En plus des tests fournis et du benchmark, nous avons testé tous les nombres proposés dans le main avec différents nombres de threads, qui ont toujours donné des résultats cohérents avec le benchmark. Les tests ont été effectués sur une machine disposant de **12 threads** (sans VM). Le benchmark a donnée les résultats suivants :

| Benchmark                                      | Time      | CPU       | Iterations   |
|-----------------------------------------------|-----------|-----------|--------------|
| BM_SingleThread/433494437/real_time           | 5.28 us   | 5.25 us   | 132754       |
| BM_SingleThread/433494436/real_time           | 0.006 us  | 0.006 us  | 122186085    |
| BM_SingleThread/99194853094755497/real_time   | 104 ms    | 104 ms    | 7            |
| BM_SingleThread/99194853094755499/real_time   | 0.000 ms  | 0.000 ms  | 40809664     |
| BM_MultiThread/1/433494437/real_time          | 32.5 us   | 9.42 us   | 25922        |
| BM_MultiThread/2/433494437/real_time          | 29.0 us   | 15.1 us   | 26470        |
| BM_MultiThread/4/433494437/real_time          | 40.2 us   | 34.7 us   | 19597        |
| BM_MultiThread/8/433494437/real_time          | 68.7 us   | 67.1 us   | 9018         |
| BM_MultiThread/1/433494436/real_time          | 0.005 us  | 0.005 us  | 130670659    |
| BM_MultiThread/2/433494436/real_time          | 0.005 us  | 0.005 us  | 130353716    |
| BM_MultiThread/4/433494436/real_time          | 0.005 us  | 0.005 us  | 135031190    |
| BM_MultiThread/8/433494436/real_time          | 0.005 us  | 0.005 us  | 136131926    |
| BM_MultiThread/1/99194853094755497/real_time  | 156 ms    | 0.026 ms  | 4            |
| BM_MultiThread/2/99194853094755497/real_time  | 82.0 ms   | 0.042 ms  | 9            |
| BM_MultiThread/4/99194853094755497/real_time  | 44.0 ms   | 0.057 ms  | 16           |
| BM_MultiThread/8/99194853094755497/real_time  | 25.7 ms   | 0.094 ms  | 27           |
| BM_MultiThread/1/99194853094755499/real_time  | 0.014 ms  | 0.008 ms  | 43825        |
| BM_MultiThread/2/99194853094755499/real_time  | 0.021 ms  | 0.014 ms  | 35957        |
| BM_MultiThread/4/99194853094755499/real_time  | 0.033 ms  | 0.029 ms  | 21238        |
| BM_MultiThread/8/99194853094755499/real_time  | 0.063 ms  | 0.062 ms  | 12115        |

Le gain de vitesse **n'est pas linéaire** par rapport au nombre de threads alloués. Nous avons même observé des cas où l'implémentation multi-threadée était plus lente que la version séquentielle (gain négatif), notamment pour le nombre 433494437. Ce phénomène peut probablement être expliqué par la création et destruction des threads à chaque tests et par le coût de la synchronisation entre eux.

La valeur du `checkInterval` (fréquence de vérification de l'arrêt) influence directement l'efficacité. Nous avons testés quelques valeurs (entre 1 et 10'000) pour tenter de l'optimiser, le meilleur compromis a été trouvé avec un intervalle de 1'000 itérations.

Les vérifications pour les nombres premiers prennent systématiquement plus de temps que pour les nombres composés. Pour les nombres premiers, l'ensemble de l'intervalle de diviseurs potentiels jusqu'à $\sqrt{n}$ doit être testé avant de confirmer qu'il est premier. Inversement, pour les nombres composés, la recherche s'arrête dès la découverte d'un facteur, ce qui peut limiter considérablement le temps de calcul.