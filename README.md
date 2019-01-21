# autotag-loudness

## Requirements

- C++17
- [cpplint](https://github.com/cpplint/cpplint)
- [TagLib](https://taglib.org/)
- [FFmpeg](https://www.ffmpeg.org/)

## Usage

```
sh build.sh && ./a.out <your_audio_library_dir>
```

## Tested on

- Ubuntu 18.04.1 LTS
- g++ (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0
- libtag1v5 1.11.1+dfsg.1-0.2build2
- libtag1-dev 1.11.1+dfsg.1-0.2build2
- ffmpeg version 3.4.4-0ubuntu0.18.04.1

## Known issues

- Cannot process `.mp4` file correctly.
    - Output information is broken: `Loudness: AAAAA LUFS (Range: DDDDD LU) (Min: CCCCC LUFS) (Max: BBBBB LUFS)`.
