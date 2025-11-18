# libffi-probe

A small example (a C repl to call raylib function) to use libffi.

## Quick Start

```console
$ cc -o nob nob.c
$ ./nob
```

## Usage

**Rule:**

1. **Setting functions** (e.g., SetWindowPosition, SetWindowSize): Call directly
   ```
   func params ...
   ```

2. **Render functions** (e.g., ClearBackground, DrawText): Prefix with `!`
   ```
   ! func params ...
   ```

3. **Supported types**: int, string, Color

4. **Color format**: `@ r g b`
   ```
   @ 255 0 0    # Red color
   ```

**Example:**
```
> InitWindow 640 480 "hello, world"
> SetWindowSize 800 600
> ! ClearBackground @ 255 255 255
> ! DrawText "Hello" 100 100 20 @ 0 0 255
> CloseWindow
```
## Reference

- [Tsoding Daily: This Library is a Hidden Gem](https://www.youtube.com/watch?v=0o8Ex8mXigU)
