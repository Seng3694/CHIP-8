# CHIP-8 Emulator #

CHIP-8 Emulator written in C.

![6]

## References ##
- [Article from Laurence Muller][4]
- [Wikipedia][3]

## Dependencies ##

Project uses [SDL2][5] for graphics.

## Build ##

Project uses [CMake][2].

1. Clone repository

    ```
    git clone https://github.com/Seng3694/CHIP-8
    ```

2. Generate build files

    ```
    mkdir bin
    cd bin
    cmake -G "Your Generator" -DSDL2_PATH="PATH/TO/SDL2" ../CHIP-8
    ```

3. Build the files

    ```
    cmake --build . --config Release
    ```

## License ##

This Code is licensed under the MIT License. See [LICENSE][1] for more information.

[1]:LICENSE
[2]:https://cmake.org/
[3]:https://en.wikipedia.org/wiki/CHIP-8
[4]:http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
[5]:https://www.libsdl.org/index.php
[6]:resources/pong.gif