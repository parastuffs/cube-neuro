# Early Creativity - Cube

![Cubes](https://github.com/parastuffs/cube-neuro/raw/main/images/20231104_170055.jpg "Cubes")

Ceci est la documentation pour la seconde version du cube. La première version (V1) est disponible [ici](https://github.com/parastuffs/cube-neuro/blob/main/README_V1.md)
D'autres photos du développement du cube sont disponibles [en ligne](https://photos.app.goo.gl/r4oXzCXrc1rNYwXG9).


Le cube présente un bouton sur cinq de ses faces et une trappe sous sa face inférieure.
Sous la trappe se trouvent :
- Un port micro-USB pour reprogrammer le micro-contrôleur et recharger la batterie. **Attention, pour recharger la batterie, il faut connecter le câble USB et allumer le cube.**
- Un interrupteur permettant d'allumer (ON) ou éteindre (OFF) le cube.
- Un lecteur de carte micro-SD.

En mode démonstration, appuyer sur un bouton peut générer un son à l'aide du haut-parleur exposé sur la face inférieure.
**Pour passer au mode expérience, il faut maintenir un bouton enfoncé au moins deux secondes.**
Pour revenir au mode démonstration, il faut redémarrer le cube.

En mode expérience, appuyer sur un bouton ne génère aucun son, mais les pressions sont enregistrée sur la carte micro-SD dans un fichier `.csv`. Ce fichier suit la structure suivante :

| observation | face | pression_start | pression_stop | sound | isExperiment |
| --- | --- | --- | --- | --- | --- |
| Numéro de l'observation, incrémental | Valeur entre 1 et 5 du bouton appuyé | Instant de début de pression | Instant de fin de pression | Son joué (1) ou non (0) | Pression durant la démo (0) ou l'expérience (1) |

La mesure du temps se fait en milliseconde, l'instant initial étant l'allumage du cube.

Les boutons sont numérotés comme suit :
1. Face supérieure
2. Face latérale côté haut-parleur
3. Face latérale côté circuit imprimé (opposée au haut-parleur)
4. Face latérale gauche (face inférieure vers soi)
5. Face latérale droite (face inférieure vers soi)


### Changer le son joué en mode démonstration
Le son joué doit se trouver à la racine de la carte micro-SD et **doit** se nommer `sound.mp3`.

### Le cube et la notion de temps
Le micro-contrôleur n'a pas de notion de date. À chaque démarrage, il recommence à compte le temps qui passe à partir de zéro. Les fichiers créés ont donc tous la même date de création erronée qui ne doit pas être prise en compte.

### Structure des fichiers
Lors de son premier démarrage, le micro-contrôleur vérifie si le fichier `exp_id.txt` existe et le crée s'il n'est pas présent sur la carte micro-SD.
Ce dernier contient une liste des identifiants incrémentaux d'expériences enregistrées.
Au passage du cube en mode expérience (mode B), le micro-contrôleur regarde le dernier identifiant dans ce fichier et incrémente sa valeur afin de créer un nouvel identifiant d'expérience.
Cet identifiant `<id>` est ensuite utilisé pour créer un fichier `exp_<id>.csv`.

Tout comme les fichiers audio, les noms des fichiers d'expérience ne sont pas sensible à la casse (`EXP_20.CSV` est identique à `exp_20.csv`) et ne peuvent pas dépasser huit caractères, extension non-comprise (`exp_20.csv` est valable, mais `experience_20.csv` ou `exp_12345.csv` ne sont pas valable).

### Mettre à jour la configuration des cubes
Dans leur configuration par défaut, les cubes se comportent comme suit en mode démonstration :
- Alpha : son à chaque pression
- Beta : aucun son
- Gamma : son une pression sur deux
- 
- Les boutons rouges suivent le schéma suivant lors de pressions successives : pas de son / son / pas de son *ou* son / pas de son / son en fonction de la configuration souhaitée.
- Les boutons verts ne génère aucun sons.

Un guide plus détaillé est disponible [ici](https://github.com/parastuffs/cube-neuro/tree/main/code) sur la mise à jour du code sur le micro-contrôleur.

### Matériel utilisé
Le fichier [suivant](https://github.com/parastuffs/cube-neuro/blob/main/BOM.md) contient un tableau avec l'ensemble du matériel utilisé pour réaliser un cube complet.

### Améliorations possibles
- Ajouter une RTC pour que le cube connaisse la date des expériences.
- Prévoir un meilleur circuit d'amplification pour le haut-parleur afin d'obtenir un meilleur volume sonore.
- Choisir le mode de fonctionnement du cube en mode démonstration (A) via un fichier de configuration sur la carte SD.
