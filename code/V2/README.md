## Comment mettre le code à jour sur le micro-contrôleur ?
Le micro-contrôleur utilisé est un **Wemos Lolin32 Lite**. La méthode la plus facile pour le programmer est d'utiliser l'environnement de développement intégré (IDE) Arduino.
Ce dernier est téléchargeable [sur le site du fabricant](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE).


![Arduino IDE](https://github.com/parastuffs/cube-neuro/raw/main/images/arduino_ide.png "Arduino IDE")

Avant tout, assurez-vous que les bibliothèques nécessaires sont correctement installées.

1. Allez dans le menu `Tools > Boards > Boards Manager...` et vérifiez que `ESP32` est installé.
![Boards manager](https://github.com/parastuffs/cube-neuro/raw/main/images/arduino_boards_manager_esp32.png "Boards manager")
2. Allez dans le menu `Tools > Manage Libraries...` et installez la bibliothèques `SD` (gestion de la carte SD) ainsi que `ESP32LitePack`.
![Libraries Manager](https://github.com/parastuffs/cube-neuro/raw/main/images/arduino_libraries.png "Libraries Manager")
3. Télécharger la bibliothèque audio [ici](https://github.com/schreibfaul1/ESP32-audioI2S/archive/refs/tags/3.0.8.zip) et suivez [ce guide](https://github.com/schreibfaul1/ESP32-audioI2S/wiki#can-the-arduino-ide-be-used) pour l'installer.

Sélectionnez ensuite la bonne carte dans la liste des cartes supportées dans le menu `Tools > Boards > ESP32 Arduino > WEMOS LOLIN32 Lite`.

Allez ensuite dans le menu `Tools > Port` et sélectionnez le bon port auquel est connecté la carte. La plupart du temps, il sera sélectionné par défaut.

Veillez à bien sélectionner l'option suivante : `Tools > Partition Scheme > No OTA (Large APP)`

Vous pouvez **compiler** votre code en cliquant sur le « ✓ » (checkmark) en haut à gauche de l'interface.
Vous pouvez **uploader** (téléverser) votre code sur l'Arduino en cliquant sur la « → » (flèche pointant à droite) en haut à gauche de l'interface.


## Troubleshooting
Si le code refuse de s'envoyer vers le micro-contrôleur, vérifiez les éléments suivants :
- Il est bien connecté à votre ordinateur.
- Le bon type de carte est sélectionné : `WEMOS LOLIN32 Lite`.
- Sélectionnez le bon port.

Si le programme ne fonctionne pas, essayez de l'exécuter tout en étant connecté à l'IDE et appuyez sur la petite loupe en haut à droite de l'interface.
Sur le micro-contrôleur, appuyez sur le bouton à côté du port USB pour le redémarrer et vérifiez qu'il ne vous dit pas `SD Failed`, ce qui vous indiquerait que la carte SD n'est pas correctement insérée ou que le fichier audio `sound.mp3` est absent de la carte.
