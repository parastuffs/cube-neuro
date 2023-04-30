Attention, veillez à ne connecter le micro-contrôleur à votre ordinateur **que si le cube est éteint !** Le micro-contrôleur n'apprécie pas de recevoir 5 V de la batterie *et* du câble USB en m

## Comment mettre le code à jour sur le micro-contrôleur ?
Le micro-contrôleur utilisé est un **Arduino Nano** (ou équivalent). La méthode la plus facile pour le programmer est d'utiliser l'environnement de développement intégré (IDE) officiel.
Ce dernier est téléchargeable [sur le site du fabricant](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE).


![Arduino IDE](https://github.com/parastuffs/cube-neuro/raw/main/images/arduino_ide.png "Arduino IDE")

Avant tout, assurez-vous que les bibliothèques nécessaires sont correctement installées.

1. Allez dans le menu `Tools > Boards > Boards Manager...` et vérifiez que « Arduino AVR Boards » est installé.
![Boards manager](https://github.com/parastuffs/cube-neuro/raw/main/images/arduino_boards_manager.png "Boards manager")
2. Allez dans le menu `Tools > Manage Libraries...` et installez la bibliothèques `SD` (gestion de la carte SD) ainsi que `TMRpcm` (gestion de l'audio).
![Libraries Manager](https://github.com/parastuffs/cube-neuro/raw/main/images/arduino_libraries.png "Libraries Manager")

Sélectionnez ensuite la bonne carte dans la liste des cartes supportées dans le menu `Tools > Boards > Arduino AVR Boards > Arduino Nano`.

Allez ensuite dans le menu `Tools > Port` et sélectionnez le bon port auquel est connecté la carte. La plupart du temps, il sera sélectionné par défaut.

Vous pouvez **compiler** votre code en cliquant sur le « ✓ » (checkmark) en haut à gauche de l'interface.
Vous pouvez **uploader** (téléverser) votre code sur l'Arduino en cliquant sur la « → » (flèche pointant à droite) en haut à gauche de l'interface.


## Troubleshooting
Si le code refuse de s'envoyer vers le micro-contrôleur, vérifiez les éléments suivants :
- Il est bien connecté à votre ordinateur.
- Le bon type de carte est sélectionné : Arduino Nano.
- Sélectionnez le bon port.
- Essayez de changer le programmeur : `Tools > Processor > ATmega328P (Old Bootloader)`.

Si le programme ne fonctionne pas, essayez de l'exécuté tout en étant connecté à l'IDE et appuyez sur la petite loupe en haut à droite de l'interface.
Appuyez sur le bouton au centre de l'Arduino pour le redémarrer et vérifiez qu'il ne vous dit pas `SD Failed`, ce qui vous indiquerait que la carte SD n'est pas correctement insérée ou que le fichier audio `sound.wav` est absent de la carte.
