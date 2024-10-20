# Early Curiosity - Cube

![Cubes](https://github.com/parastuffs/cube-neuro/raw/main/images/20231104_170055.jpg "Cubes")

This is the documentation for the **third version** of the cube. v1 is available [here](https://github.com/parastuffs/cube-neuro/blob/main/README_V1.md) and v2 [here](https://github.com/parastuffs/cube-neuro/blob/main/README_V2.md).
Pictures of the development are available [online](https://photos.app.goo.gl/r4oXzCXrc1rNYwXG9).

The cube has a button on five of its faces, and a hatch on the bottom face.
Under the hatch, there is:
- A USB-C port for programming and battery charging. 
> [!IMPORTANT]  
> To charge the battery, the cube needs to be **on**.
- A switch to turn the cube on and off.
- A micro-SD reader.

In the demonstration mode, pressing a button might trigger a sound depending on the cube configuration.
> [!IMPORTANT]
> To switch to experiment mode, you need to keep any of the buttons pressed for two seconds.
In order to come back to demonstration mode, you need to reboot the cube by turning it off and on again.

In experiment mode, pressing a button never triggers a sound, but all events are saved to the SD card in a `.csv` file. It follows this structure:

| observation | face | pression_start | pression_stop | sound | isExperiment |
| --- | --- | --- | --- | --- | --- |
| Observation count, incremental | 1 to 5 value of the pressed button | Time of start of press | Time of end of press | Sound played (1) or not (0) | Press durring demo (0) or experiment (1) |

Time measurement is in milliseconds, 0 being the powering up of the cube.


### Change the sound played during demonstration mode
The sound must be located at the root of the micro-SD card and **must** be named `sound.mp3`.

### The cube and dates
The micro-controller does not have any notion of the actual date. With each reboot, it starts couting time from zero again. All files are thus created with an erroneous creation date which should be ignored.

### File structure
Upon starting for the first time, the micro-controller checks if the file `exp_id.txt` exists and does create it if it's not found.
This file contains a list of all incremental IDs of previously saved experiments.
When the cube switches to experiment mode, the micro-controller checks the last ID in that file, increment it, and create a new experiment ID.
This number `<id>` is then used to create a file `exp_<id>.csv`.

Just like audio file, experiment file names are not case sensitive (`EXP_20.csv` is the same as `exp_20.csv`) and cannot exceed eight characters without the file extension (`exp_20.csv` is valid, `experience_20.csv` or `exp_12345.csv` are not).

### Update the cube configuration
In their default configuration, the cube behave as such in demonstration mode:
- Alpha: Always play sound
- Beta: No sound
- Gamma: Sound / No sound
- Delta: No sound x7 / Sound

A more detailed guide is available [here](https://github.com/parastuffs/cube-neuro/tree/main/code) on updating the code.

### Hardware
This [file](https://github.com/parastuffs/cube-neuro/blob/main/BOM.md) has a full bill of materials. Since v3, the cube is laser cutted instead of 3D printed.

### Improvement considered
- Add an RTC to keep track of time.
- Having a configuration file for the cube type.
