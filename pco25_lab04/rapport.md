# PCO – Laboratoire 4 : Les locomotives

## Arrêt d'urgence

La méthode emergency stop arrête (loco.arreter) les locos et met leur vitesse à 0. Puisque la vitesse n'est mise à 0 que
dans ce cas, locomotivebehavior sort de sa boucle lorsque la vitesse est nulle, et appelle stopAll pour s'assurer que toutes 
les lcoos qui partagent la section partagée soient arrêtées. Sauf qu'on sait pas quoi mettre dedans hahaha.


