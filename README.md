# Otage, a vieux programme d'Othello

La première version de mon programme d'Othello était plutôt faible. Elle utilisait un algorithme alpha-béta simple, que j'avais découvert dans la revue "l'Ordinateur Individuel", à la suite d'une série d'articles écrits par David Levy, un des pionniers de la programmation des jeux. La fonction d'évaluation était mauvaise, exclusivement basé sur des facteurs positionnels statiques.

J'ai repris mon programme en 1984, à l'X, où je l'ai réécrit en Fortran sur VAX-8600. La fonction d'évaluation était déjà meilleure (elle commençait à prendre en compte un certain nombre d'aspects dynamiques des positions). Le programme fut testé par un grand nombre d'élèves et remporta beaucoup plus de victoires qu'il ne connut de défaites. Cependant, il n'était pas de taille face aux meilleurs programmes et aux meilleurs joueurs d'Othello.

En 1987, le programme fut ré-écrit une troisième fois, cette fois-ci pour partie en C (l'algorithme de recherche) et pour partie en assembleur 68000 (la fonction d'évaluation), pour fonctionner sur Amiga et sur stations SUN-3. Cette nouvelle version avait un algorithme alpha-beta amélioré, une fonction d'évaluation plus rapide. Il fut testé par un des meilleurs joueurs français de l'époque et était déjà devenu un assez fort adversaire.

Après quelques années de pause, le programme connut de nouvelles évolutions en 1995. A cette époque, nous travaillions sur l'utilisation des algorithmes génétiques, et sur le développement de fonctions de fitness probabiliste. OTAGE semblait un candidat parfait pour ce type de tests, et un certain nombre d'évolutions furent faites, et le programme fut totalement ré-écrit en C. Les résultats ne furent pas forcément trés probants concernant Othello (voir cet article), mais permit de traiter certains problèmes comme l'optimisation de matrices de turbo-codes (voir cet autre article).

En 1996, le programme connut ses dernières évolutions. L'algorithme de recherche fut modifié (on passa d'un alpha-béta à un algorithme de type mtd(f)). La fonction d'évaluation fut totalement redéveloppée pour devenir une fonction auto-réalisée par apprentissage et co-évolution (voir ce [rapport de DEA](http://www.alliot.fr/papers/fiorio.pdf) pour plus de détails).

Cette version d'OTAGE fut testé sur le serveur IOS, qui regroupait la quasi-totalité des meilleurs programmes d'Othello, et il arriva, au temps de sa splendeur, au cinquième rang (ce qui en faisait un programme quasiment imbattable pour n'importe quel joueur humain). Il resta cependant en deça des meilleurs programmes, comme Logistello ou Hannibal. Il lui manquait en particulier une bibliothèque d'ouverture de bonne qualité.

Le code source d'OTAGE est en C, et il n'est pas particulièrement de bonne qualité... Il est rempli de verrues de l'époque (comme des fonctions pour se connecter à IOS). Il y a des restes de mon ancien programme de l'X qui récupérait les noms des joueurs et faisait des classements: c'est une véritable horreur. Il reste même certainement des bugs...


Il est peu probable que le programme connaisse de nouvelles évolutions. Il faudrait en fait le réécrire complètement.

Aujourd'hui la recherche sur Othello est au ralenti, voire à l'arrêt. La victoire de Logistello en 1997 contre Takeshi Murakami (alors champion du monde) sur un score sans appel de 6-0 a grandement éteint les travaux sur la programmation de ce jeu. Le code source de Logistello est disponible sur la page Web de son auteur, Michael Buro. 

Il y a une interface en tcl/tk nommé board. Pour jouer, il suffit de lancer board.
