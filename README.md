# GAME-PUZZLE-JigSaw

Jigsaw puzzle game for OS/2 PM -- 32-bit port, Version 1.1.  
Illustrates the use of GPI retained segments, async drawing threads, and bitmap manipulation.

![JigSaw](/doc/JigSaw.png)

The original 16-bit source was written by Microsoft and included in the OS/2 1.x and 2.0 Toolkits.

## Features

- Load any `.BMP` bitmap and cut it into 36 puzzle pieces (6x6)
- Drag pieces with the mouse to reassemble the image
- Zoom in/out (8 levels)
- Toggle scrollbars and menu bar (frame controls)
- 6 languages: English, Español, Nederlands, Deutsch, Français, Italiano
- Settings saved to `jigsaw.cfg` on exit
- Keyboard shortcuts for all common actions

## Requirements

- ArcaOS, eComStation, or OS/2 Warp 4.52
- OS/2 Toolkit 4.5 (headers expected at `C:\os2tk45\h`)
- OpenWatcom 2.0 (at `C:\watcom2` or `C:\watcom`)

## Building

```
compile-wat.cmd
```

Output: `bin\jigsaw.exe`

## Controls

| Key / Action        | Function               |
|---------------------|------------------------|
| Ctrl+N              | Load bitmap            |
| Ctrl+J              | Jumble pieces          |
| Ctrl++              | Zoom in                |
| Ctrl+-              | Zoom out               |
| Ctrl+F              | Toggle frame controls  |
| Ctrl+X              | Exit                   |
| Drag                | Move puzzle piece      |

## Directory layout

```
src\          Source (jigsaw.c, jigsaw.h, lang.h, jigsaw.rc, jigsaw.def)
bin\          Build output
doc\          Readme.txt, Changelog.txt, LICENSE.txt
legacy\       Original 16-bit source (reference only)
```

## License

32-bit port and enhancements: BSD 3-Clause License.  
Original code: Copyright (c) 1988 Microsoft Corporation, license not specified.  
See `doc\LICENSE.txt` for full text.

## Authors

- Microsoft Corporation (original 1988 16-bit sample)
- OS2World (32-bit port, 2026)

## Links

- https://hobbes.nmsu.edu/h-viewer.php?dir=/pub/os2/games&file=jigsaw.zip
- [DEV-SAMPLES-C-PM-Jigsaw](https://github.com/os2World/DEV-SAMPLES-C-PM-Jigsaw)
- https://github.com/OS2World/DEV-SAMPLES-IBM_OS2_2-0_Toolkit
