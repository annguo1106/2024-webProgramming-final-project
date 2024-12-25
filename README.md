# webProgramming
1. steps to build up SFML
    ```shell
    sudo apt update && sudo apt upgrade
    
    # install prerequisites
    sudo apt install build-essential libglew-dev libjpeg-dev libfreetype6-dev libx11-dev libxrandr-dev libflac-dev libogg-dev libvorbis-dev libopenal-dev libudev-dev libpthread-stubs0-dev
    
    # install fonts
    sudo apt install ttf-mscorefonts-installer
    sudo cp /usr/share/fonts/truetype/msttcorefonts/arial.ttf .../projectDirectory

    # install SFML
    sudo apt install libsfml-dev
    
    # compile (sfml is made of 5 modules: system, window, graphics, network, audio)
    g++ -c ui.cpp
    g++ ui.o -o ui -lsfml-graphics -lsfml-window -lsfml-system

    # execute
    ./ui
    ```


```shell
make serv
./serv
```
