# ray-games

Some games made with raylib by me.

Just for fun.

:D

## Tetris

### HOW TO BUILD

`cd` to repo root first, then run

```bash
cmake -S . -B build && cmake --build build
```

You can also specify build type and C compiler with cmake flag `-DCMAKE_BUILD_TYPE` and `-DCMAKE_C_COMPILER`

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang
```

### HOW TO START

`./tetris` --- Default table size: 12 * 16

`./tetris [width:int[4,60]] [height:int[4,60]]` --- Custom table Size

### HOW TO PLAY

| HOTKEY | DETAILS |
| --- | --- |
| `H` `A` `LEFT` | Move left |
| `L` `D` `RIGHT` | Move right |
| `J` `S` `DOWN` | Rotate left (anticlockwise) |
| `K` `W` `UP` | Rotate right (clockwise) |
| `;` `LSHIFT` `RSHIFT` | Soft drop |
| `P` `ESC` | Pause game |
| `R` | Restart game |
| `CTRL + Q` | Quit game |

