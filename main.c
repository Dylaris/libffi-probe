#include <stdio.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <ffi.h>

#define COOK_IMPLEMENTATION
#define COOK_STRIP_PREFIX
#include "cook.h"

#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

typedef void (*fn_t)(void);

// Color, 4 components, R8G8B8A8 (32bit)
typedef struct Color {
    unsigned char r;        // Color red value
    unsigned char g;        // Color green value
    unsigned char b;        // Color blue value
    unsigned char a;        // Color alpha value
} Color;

int main(void)
{
    void *raylib;             /* dll handle */
    ffi_status status;
    ffi_cif cif = {0};
    ffi_type **atypes = NULL; /* array of arg type pointer */
    void **avalues = NULL;    /* array of arg value pointer */
    const char *funcname;
    fn_t fn;                  /* function to call */
    char inbuf[256];          /* store input */
    char strbuf[64];          /* store string */
    stb_lexer lex;
    ffi_type ffi_type_color;  /* raylib struct Color */

    ffi_type_color.size = sizeof(Color);
    ffi_type_color.alignment = 1;
    ffi_type_color.type = FFI_TYPE_STRUCT;
    ffi_type_color.elements = (ffi_type **) temp_alloc(sizeof(ffi_type*)*5);
    ffi_type_color.elements[0] = &ffi_type_uint8;
    ffi_type_color.elements[1] = &ffi_type_uint8;
    ffi_type_color.elements[2] = &ffi_type_uint8;
    ffi_type_color.elements[3] = &ffi_type_uint8;
    ffi_type_color.elements[4] = NULL;

    raylib = dlopen("raylib/lib/libraylib.so", RTLD_NOW);
    if (!raylib) {
        fprintf(stderr, "ERROR: %s\n", dlerror());
        return 1;
    }

    while (1) {
        vec_reset(atypes);
        vec_reset(avalues);
        fn = NULL;
        funcname = NULL;

        printf("> ");
        fflush(stdout);
        if (!fgets(inbuf, sizeof(inbuf), stdin)) break;

        stb_c_lexer_init(&lex, inbuf, inbuf + strlen(inbuf), strbuf, sizeof(strbuf));

        temp_scope(1) {
            while (stb_c_lexer_get_token(&lex)) {
                switch (lex.token) {
                case CLEX_id:
                    funcname = temp_strdup(lex.string);
                    break;
                case CLEX_dqstring: {
                    const char **x = temp_alloc(sizeof(char *));
                    *x = temp_strdup(lex.string);
                    vec_push(avalues, (void *) x);
                    vec_push(atypes, &ffi_type_pointer);
                } break;
                case CLEX_intlit: {
                    int *x = temp_alloc(sizeof(int));
                    *x = (int) lex.int_number;
                    vec_push(avalues, (void *) x);
                    vec_push(atypes, &ffi_type_sint32);
                } break;
                default:
                    if (lex.token == '@') {
                        /* parse to Color */
                        Color *color = temp_alloc(sizeof(Color));
                        stb_c_lexer_get_token(&lex); color->r = (unsigned char) lex.int_number;
                        stb_c_lexer_get_token(&lex); color->g = (unsigned char) lex.int_number;
                        stb_c_lexer_get_token(&lex); color->b = (unsigned char) lex.int_number;
                        stb_c_lexer_get_token(&lex); color->a = (unsigned char) lex.int_number;
                        vec_push(avalues, (void *) color);
                        vec_push(atypes, &ffi_type_color);
                    }
                }
            }

            if (vec_size(atypes) == 0) vec_push(atypes, &ffi_type_void);

            if (!funcname) goto end;

            status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, vec_size(atypes), &ffi_type_void, atypes);
            if (status != FFI_OK) {
                fprintf(stderr, "ERROR: failed to call ffi_prep_cif()\n");
                return 1;
            }

            fn = (fn_t) dlsym(raylib, funcname);
            if (fn) ffi_call(&cif, fn, NULL, avalues);
end:
        }
    }

    if (atypes) vec_free(atypes);
    if (avalues) vec_free(avalues);
    dlclose(raylib);

    return 0;
}
