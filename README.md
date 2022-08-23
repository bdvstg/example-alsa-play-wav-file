# C Example code of ALSA play wav file (native build)

## build instructions

1. install libasound-dev first, `apt install libasound-dev`
2. find the path to asoundlib.h, `find / -name asoundlib.h`
3. build it `gcc wavplay.c -I <path-to-asoundlib.h> -lasound`
4. run it, `./a.out <path-to-wav-file>`

## references

ALSA official minimal example code
https://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_min_8c-example.html

Jon-Schneider/wav_header.h
https://gist.github.com/Jon-Schneider/8b7c53d27a7a13346a643dac9c19d34f


