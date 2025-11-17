#include <stdio.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <ffi.h>

#define COOK_IMPLEMENTATION
#define COOK_STRIP_PREFIX
#include "cook.h"

#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

#define RAYWHITE   (Color){ 245, 245, 245, 255 }   // My own White (raylib logo)

// Color, 4 components, R8G8B8A8 (32bit)
typedef struct Color {
    unsigned char r;        // Color red value
    unsigned char g;        // Color green value
    unsigned char b;        // Color blue value
    unsigned char a;        // Color alpha value
} Color;

typedef void (*fn_t)(void);

int main(void)
{
    void *raylib;             /* dll handle */
    ffi_status status;
    ffi_cif cif = {0};
    ffi_type **atypes = NULL; /* array of arg type pointer */
    void **avalues = NULL;    /* array of arg value pointer */
    fn_t fn;                  /* function to call */
    char buffer[128] = {0};   /* store input */
    stb_lexer lexler = {0};


    stb_c_lexer_init(&lexer, const char *input_stream, const char *input_stream_end, char *string_store, int store_length);

    raylib = dlopen("raylib/lib/libraylib.so", RTLD_NOW);
    if (!raylib) {
        fprintf(stderr, "ERROR: %s\n", dlerror());
        return 1;
    }

    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        printf("echo: %s", buffer);

        #if 0

        vec_push(atypes, &ffi_type_sint32);
        vec_push(atypes, &ffi_type_sint32);
        vec_push(atypes, &ffi_type_pointer);

        status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, vec_size(atypes), &ffi_type_void, atypes);
        if (status != FFI_OK) {
            fprintf(stderr, "ERROR: failed to call ffi_prep_cif()\n");
            return 1;
        }

        fn = (fn_t)dlsym(raylib, "InitWindow");

        int w = 100;
        int h = 100;
        const char *title = "hello from libffi";

        vec_push(avalues, &w);
        vec_push(avalues, &h);
        vec_push(avalues, &title);

        ffi_call(&cif, fn, NULL, avalues);

        #endif
    }

    if (atypes) vec_free(atypes);
    if (avalues) vec_free(avalues);
    dlclose(raylib);

    return 0;
}
