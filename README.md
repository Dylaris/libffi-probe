# libffi-probe

A small example (a C repl to call raylib function) to use libffi.

## Quick Start

```console
$ cc -o nob nob.c
$ ./nob
```

## Usage

**Rule:**

1. **Function call**
   ```
   func params ...
   ```

2. **Supported types**: int, string, Color

3. **Color format**: `@ r g b a`
   ```
   @ 255 0 0 255   # Red color
   ```

**Example:**
```
> InitWindow 640 480 "hello, world"
> SetWindowSize 800 600
> BeginDrawing
> ClearBackground @ 255 0 0 255
> DrawText "Hello" 100 100 20 @ 0 0 255
> EndDrawing
> CloseWindow
```

## Reference

- [Tsoding Daily: This Library is a Hidden Gem](https://www.youtube.com/watch?v=0o8Ex8mXigU)
