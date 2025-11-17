/*
cook.h - v0.05 - Dylaris 2025
===================================================

BRIEF:
  Pieces of C implemented using C99.

NOTICE:
  Not compatible with C++.

USAGE:
  In exactly one source file, define the implementation macro
  before including this header:
  ```
    #define COOK_IMPLEMENTATION
    #include "cook.h"
  ```
  In other files, just include the header without the macro.

HISTORY:
    v0.06 Support 'temp allocator', 'string view', 'string builder'
    v0.05 Remove 'list', 'deque', 'string', 'file'
    v0.04 Support data-structure 'mini hash table'
    v0.03 Support data-structure 'doubly instrusive linked list'
    v0.02 Support data-structure 'deque'

LICENSE:
  See the end of this file for further details.
*/

#ifndef COOK_H
#define COOK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

#define COOK_ASSERT(x) assert(x)

/*
 * dynamic array
 */
typedef struct Cook_Vector_Header {
    size_t size;
    size_t capacity;
} Cook_Vector_Header;

#define cook_vec_header(vec) ((Cook_Vector_Header*)((char*)(vec) - sizeof(Cook_Vector_Header)))
#define cook_vec_size(vec) ((vec) ? cook_vec_header(vec)->size : 0)
#define cook_vec_capacity(vec) ((vec) ? cook_vec_header(vec)->capacity : 0)
#define cook_vec_push(vec, item)                                        \
    do {                                                                \
        if (cook_vec_size(vec) + 1 > cook_vec_capacity(vec)) {          \
            size_t new_capacity = cook_vec_capacity(vec) < 16           \
                            ? 16 : 2 * cook_vec_capacity(vec);          \
            (vec) = cook_vec_resize(vec, new_capacity, sizeof(*(vec))); \
        }                                                               \
        (vec)[cook_vec_header(vec)->size++] = (item);                   \
    } while (0)
#define cook_vec_pop(vec) ((vec)[--cook_vec_header(vec)->size])
#define cook_vec_empty(vec) (cook_vec_size(vec) == 0)
#define cook_vec_full(vec) (cook_vec_size(vec) == cook_vec_capacity(vec))
#define cook_vec_end(vec) ((vec) + cook_vec_size(vec))
#define cook_vec_grow(vec, cap)                                \
    do {                                                       \
        size_t new_cap = cook_vec_capacity(vec) + (cap);       \
        (vec) = cook_vec_resize((vec), new_cap, sizeof(*vec)); \
    } while (0)
#define cook_vec_free(vec)                   \
    do {                                     \
        if (vec) free(cook_vec_header(vec)); \
        (vec) = NULL;                        \
    } while (0)
#define cook_vec_reset(vec)                      \
    do {                                         \
        if (vec) cook_vec_header(vec)->size = 0; \
    } while (0)
#define cook_vec_foreach(type, vec, iter) for (type *iter = (vec); iter < cook_vec_end(vec); iter++)
void *cook_vec_resize(void *vec, size_t new_capacity, size_t item_size);


/*
 * static array
 */
#define cook_arr_size(arr) (sizeof(arr)/sizeof(arr[0]))
#define cook_arr_end(arr) ((arr)+cook_arr_size(arr))
#define cook_arr_foreach(type, arr, iter) for (type *iter = (arr); iter < cook_arr_end(arr); iter++)


/*
 * mini hash table
 *
 * A simple hash table that uses hash values as keys and external array indices as values.
 *
 * Features:
 * - Can use integers directly as keys/values, no external array needed
 * - Cannot handle hash collisions (different keys producing same hash)
 * - Low collision probability with good hash function or small datasets
 *
 * Designed for simplicity and small size, not safety.
 */
typedef struct Cook_Mini_Hash_Bucket {
    size_t key;
    size_t val;
} Cook_Mini_Hash_Bucket;

typedef struct Cook_Mini_Hash {
    size_t count;
    size_t capacity;
    Cook_Mini_Hash_Bucket *buckets;
} Cook_Mini_Hash;

#define COOK_MINI_HASH_MAX_LOAD 0.75
#define COOK_MINI_HASH_EMPTY    ((size_t)-1)

bool cook_hash_set(Cook_Mini_Hash *lookup, size_t key, size_t val);
bool cook_hash_get(Cook_Mini_Hash *lookup, size_t key, size_t *outval);
void cook_hash_free(Cook_Mini_Hash *lookup);


/*
 * dynamic library
 */
#ifdef _WIN32
    #include <windows.h>
    #define Cook_DLL_Handle HMODULE
    #define cook_dll_load(path) LoadLibrary(path)
    #define cook_dll_getsym(handle, name) GetProcAddress(handle, name)
    #define cook_dll_close(handle) FreeLibrary(handle)
    const char *cook_dll_geterr(void);
#else
    #include <dlfcn.h>
    #define Cook_DLL_Handle void*
    #define cook_dll_load(path) dlopen(path, RTLD_LAZY)
    #define cook_dll_getsym(handle, name) dlsym(handle, name)
    #define cook_dll_close(handle) dlclose(handle)
    #define cook_dll_geterr() dlerror()
#endif

/*
 * memory layout
 */
#define COOK_IS_POWER_2(k) ((k) != 0 && ((k) & ((k) - 1)) == 0)
#define COOK_ALIGN_UP(n, k) (COOK_IS_POWER_2(k) ? (((n) + (k) - 1) & ~((k) - 1)) : (n))
#define COOK_ALIGN_DOWN(n, k) (COOK_IS_POWER_2(k) ? ((n) & ~((k) - 1)) : (n))
#define COOK_OFFSET_OF(type, member) ((size_t)&(((type*)0)->member))
#define COOK_CONTAINER_OF(ptr, type, member) \
    ((type*)((char*)(ptr) - COOK_OFFSET_OF(type, member)))

/*
 * string view
 */
typedef struct {
    const char *data;
    size_t length;
} Cook_String_View;

/* steal from https://github.com/tsoding/nob.h.git */
#define SV_FMT "%.*s"
#define SV_ARG(sv) (int)(sv).length, (sv).data

Cook_String_View cook_sv_from_cstr(const char *cstr);
Cook_String_View cook_sv_from_parts(const char *data, size_t length);
bool cook_sv_equal(Cook_String_View a, Cook_String_View b);
int cook_sv_compare(Cook_String_View a, Cook_String_View b);
bool cook_sv_starts_with(Cook_String_View sv, Cook_String_View prefix);
bool cook_sv_ends_with(Cook_String_View sv, Cook_String_View postfix);
bool cook_sv_empty(Cook_String_View sv);
Cook_String_View cook_sv_slice(Cook_String_View sv, size_t begin, size_t end);
Cook_String_View cook_sv_trim(Cook_String_View sv);
Cook_String_View cook_sv_ltrim(Cook_String_View sv);
Cook_String_View cook_sv_rtrim(Cook_String_View sv);

/*
 * string builder
 */
typedef char *Cook_String_Builder; /* vector of char */

void cook_sb_append_sv(Cook_String_Builder *sb, Cook_String_View sv);
void cook_sb_append_parts(Cook_String_Builder *sb, const char *data, size_t length);
void cook_sb_reset(Cook_String_Builder *sb);
void cook_sb_free(Cook_String_Builder *sb);
Cook_String_View cook_sb_view(const Cook_String_Builder *sb);
#define cook_sb_append(sb_ptr, fmt, ...)                             \
    do {                                                             \
        cook_temp_scope(1) {                                         \
            const char *cstr = cook_temp_strfmt(fmt, ##__VA_ARGS__); \
            size_t length = strlen(cstr);                            \
            size_t size = COOK_ALIGN_UP(length, sizeof(uintptr_t));  \
            cook_vec_grow(*(sb_ptr), size);                          \
            memcpy(cook_vec_end(*(sb_ptr)), cstr, length);           \
            cook_vec_header(*(sb_ptr))->size += length;              \
        }                                                            \
    } while (0)

/*
 * temporary allocator (steal from https://github.com/tsoding/nob.h.git)
 */
#define COOK_TEMP_BUFFER_CAPACITY (1024*8)
void *cook_temp_alloc(size_t size);
const char *cook_temp_strdup(const char *cstr);
const char *cook_temp_strndup(const char *cstr, size_t n);
const char *cook_temp_strsub(const char *cstr, size_t begin, size_t end);
const char *cook_temp_strfmt(const char *fmt, ...);
const char *cook_temp_sv_to_cstr(Cook_String_View sv);
size_t cook_temp_save(void);
void cook_temp_rewind(size_t checkpoint);
void cook_temp_reset(void);
#define cook_temp_scope(id)                                       \
    for (size_t _marked ## id = cook_temp_save(), _done ## id= 0; \
         !_done ## id;                                            \
         cook_temp_rewind(_marked ## id), _done ## id= 1)

#endif /* COOK_H */

#ifdef COOK_IMPLEMENTATION

Cook_String_View cook_sv_from_cstr(const char *cstr)
{
    return (Cook_String_View) {
        .data = cstr,
        .length = cstr ? strlen(cstr) : 0
    };
}

Cook_String_View cook_sv_from_parts(const char *data, size_t length)
{
    return (Cook_String_View) {
        .data = data,
        .length = length
    };
}

int cook_sv_compare(Cook_String_View a, Cook_String_View b)
{
    int limit = a.length > b.length ? b.length : a.length;
    for (int i = 0; i < limit; i++) {
        if (a.data[i] > b.data[i]) {
            return 1;
        } else if (a.data[i] < b.data[i]) {
            return -1;
        } else {
            continue;
        }
    }
    return 0;
}

bool cook_sv_equal(Cook_String_View a, Cook_String_View b)
{
    if (a.length == b.length && memcmp(a.data, b.data, a.length) == 0) {
        return true;
    } else {
        return false;
    }
}

bool cook_sv_starts_with(Cook_String_View sv, Cook_String_View prefix)
{
    if (prefix.length > sv.length) return false;
    return memcmp(sv.data, prefix.data, prefix.length) == 0;
}

bool cook_sv_ends_with(Cook_String_View sv, Cook_String_View postfix)
{
    if (postfix.length > sv.length) return false;
    size_t offset = sv.length - postfix.length;
    return memcmp(sv.data + offset, postfix.data, postfix.length) == 0;
}

bool cook_sv_empty(Cook_String_View sv)
{
    return sv.data == NULL || sv.length == 0;
}

Cook_String_View cook_sv_slice(Cook_String_View sv, size_t begin, size_t end)
{
    Cook_String_View res = {0};

    if (begin >= end || end - begin > sv.length) return res; /* [begin, end) */
    res.data = sv.data + begin;
    res.length = end - begin;
    return res;
}

Cook_String_View cook_sv_ltrim(Cook_String_View sv)
{
    Cook_String_View res = {0};
    for (size_t i = 0; i < sv.length; i++) {
        if (sv.data[i] == ' ' || sv.data[i] == '\t') continue;
        res.data = sv.data + i;
        res.length = sv.length - i;
        break;
    }
    return res;
}

Cook_String_View cook_sv_rtrim(Cook_String_View sv)
{
    Cook_String_View res = {0};
    for (size_t i = sv.length; i > 0; i--) {
        if (sv.data[i-1] == ' ' || sv.data[i-1] == '\t') continue;
        res.data = sv.data;
        res.length = i;
        break;
    }
    return res;
}

Cook_String_View cook_sv_trim(Cook_String_View sv)
{
    Cook_String_View res;
    res = cook_sv_ltrim(sv);
    res = cook_sv_rtrim(res);
    return res;
}

void cook_sb_append_sv(Cook_String_Builder *sb, Cook_String_View sv)
{
    size_t size = COOK_ALIGN_UP(sv.length, sizeof(uintptr_t));
    cook_vec_grow(*sb, size);
    memcpy(cook_vec_end(*sb), sv.data, sv.length);
    cook_vec_header(*sb)->size += sv.length;
}

void cook_sb_append_parts(Cook_String_Builder *sb, const char *data, size_t length)
{
    size_t size = COOK_ALIGN_UP(length, sizeof(uintptr_t));
    cook_vec_grow(*sb, size);
    memcpy(cook_vec_end(*sb), data, length);
    cook_vec_header(*sb)->size += length;
}

void cook_sb_reset(Cook_String_Builder *sb)
{
    cook_vec_reset(*sb);
}

void cook_sb_free(Cook_String_Builder *sb)
{
    cook_vec_free(*sb);
}

Cook_String_View cook_sb_view(const Cook_String_Builder *sb)
{
    return (Cook_String_View) {
        .data = *sb,
        .length = cook_vec_size(*sb)
    };
}

static unsigned char _temp_buffer[COOK_TEMP_BUFFER_CAPACITY] = {0};
static size_t _temp_buffer_used = 0;

void *cook_temp_alloc(size_t size)
{
    if (size == 0) return NULL;
    size_t aligned_used = COOK_ALIGN_UP(_temp_buffer_used, sizeof(uintptr_t));
    if (aligned_used + size > COOK_TEMP_BUFFER_CAPACITY) return NULL;
    void *ptr = _temp_buffer + aligned_used;
    _temp_buffer_used = aligned_used + size;
    return ptr;
}

size_t cook_temp_save(void)
{
    return _temp_buffer_used;
}

void cook_temp_rewind(size_t checkpoint)
{
    _temp_buffer_used = checkpoint;
}

void cook_temp_reset(void)
{
    _temp_buffer_used = 0;
}

const char *cook_temp_strdup(const char *cstr)
{
    return cook_temp_strsub(cstr, 0, strlen(cstr) + 1);
}

const char *cook_temp_strndup(const char *cstr, size_t n)
{
    return cook_temp_strsub(cstr, 0, n);
}

const char *cook_temp_strsub(const char *cstr, size_t begin, size_t end)
{
    if (!cstr || begin >= end) return NULL; /* [begin, end) */

    size_t sub_length = end - begin;
    char *ptr = cook_temp_alloc(sub_length + 1);
    COOK_ASSERT(ptr != NULL && "out of temporary buffer");

    for (size_t i = 0; i < sub_length ; i++) {
        ptr[i] = cstr[begin + i];
    }
    ptr[sub_length] = '\0';

    return ptr;
}

const char *cook_temp_strfmt(const char *fmt, ...)
{
    if (!fmt) return NULL;

    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (len < 0) return NULL;

    void *ptr = cook_temp_alloc((size_t)len + 1);
    COOK_ASSERT(ptr != NULL && "out of temporary buffer");

    va_start(args, fmt);
    vsnprintf(ptr, (size_t)len + 1, fmt, args);
    va_end(args);

    return ptr;
}

const char *cook_temp_sv_to_cstr(Cook_String_View sv)
{
    char *ptr = cook_temp_alloc(sv.length + 1);
    COOK_ASSERT(ptr != NULL && "out of temporary buffer");
    memcpy(ptr, sv.data, sv.length);
    ptr[sv.length] = '\0';
    return ptr;
}

void *cook_vec_resize(void *vec, size_t new_capacity, size_t item_size)
{
    Cook_Vector_Header *new_header;
    size_t alloc_size = sizeof(Cook_Vector_Header) + new_capacity*item_size;

    if (vec) {
        new_header = realloc(cook_vec_header(vec), alloc_size);
        COOK_ASSERT(new_header != NULL && "out of memory");
    } else {
        new_header = malloc(alloc_size);
        COOK_ASSERT(new_header != NULL && "out of memory");
        new_header->size = 0;
    }
    new_header->capacity = new_capacity;

    return (void*)((char*)new_header + sizeof(Cook_Vector_Header));
}

static Cook_Mini_Hash_Bucket *cook_hash__find_bucket(Cook_Mini_Hash_Bucket *buckets, size_t capacity, size_t key)
{
    size_t index = key % capacity;
    size_t start_index = index;

    for (;;) {
        if (buckets[index].key == key || buckets[index].key == COOK_MINI_HASH_EMPTY) {
            return &buckets[index];
        }
        index = (index + 1) % capacity;
        if (index == start_index) return NULL;
    }
}

static bool cook_hash__expand(Cook_Mini_Hash *lookup, size_t new_capacity)
{
    if (new_capacity <= lookup->capacity) return false;

    Cook_Mini_Hash_Bucket *new_buckets = malloc(sizeof(Cook_Mini_Hash_Bucket)*new_capacity);
    if (!new_buckets) return false;
    for (size_t i = 0; i < new_capacity; i++) new_buckets[i].key = COOK_MINI_HASH_EMPTY;

    for (size_t i = 0; i < lookup->capacity; i++) {
        Cook_Mini_Hash_Bucket *bucket = &lookup->buckets[i];
        if (bucket->key == 0) continue;
        Cook_Mini_Hash_Bucket *dest = cook_hash__find_bucket(new_buckets, new_capacity, bucket->key);
        dest->key = bucket->key;
        dest->val = bucket->val;
    }

    lookup->capacity = new_capacity;
    if (lookup->buckets) free(lookup->buckets);
    lookup->buckets = new_buckets;

    return true;
}

bool cook_hash_set(Cook_Mini_Hash *lookup, size_t key, size_t val)
{
    COOK_ASSERT(lookup != NULL);

    if (lookup->count + 1 > lookup->capacity * COOK_MINI_HASH_MAX_LOAD) {
        size_t new_capacity = lookup->capacity < 16 ? 16 : 2*lookup->capacity;
        if (!cook_hash__expand(lookup, new_capacity)) return false;
    }

    Cook_Mini_Hash_Bucket *bucket = cook_hash__find_bucket(lookup->buckets, lookup->capacity, key);
    if (!bucket) return false;

    bucket->key = key;
    bucket->val = val;
    lookup->count++;

    return true;
}

bool cook_hash_get(Cook_Mini_Hash *lookup, size_t key, size_t *outval)
{
    COOK_ASSERT(lookup != NULL);

    if (lookup->count == 0) return false;

    Cook_Mini_Hash_Bucket *bucket = cook_hash__find_bucket(lookup->buckets, lookup->capacity, key);
    if (!bucket || bucket->key != key) return false;

    if (outval) *outval = bucket->val;
    return true;
}

void cook_hash_free(Cook_Mini_Hash *lookup)
{
    COOK_ASSERT(lookup != NULL);

    if (lookup->buckets) free(lookup->buckets);
    lookup->buckets = NULL;
    lookup->capacity = 0;
    lookup->count = 0;
}

#ifdef _WIN32
const char *cook_dll_geterr(void)
{
    static char errmsg[256] = {0};
    DWORD errcode = GetLastError();

    if (errcode == 0) return "No error";

    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  errmsg, sizeof(errmsg), NULL);

    return errmsg;
}
#endif

#endif /* COOK_IMPLEMENTATION */

#ifdef COOK_STRIP_PREFIX

#define vec_header         cook_vec_header
#define vec_size           cook_vec_size
#define vec_capacity       cook_vec_capacity
#define vec_push           cook_vec_push
#define vec_pop            cook_vec_pop
#define vec_foreach        cook_vec_foreach
#define vec_resize         cook_vec_resize
#define vec_grow           cook_vec_grow
#define vec_empty          cook_vec_empty
#define vec_full           cook_vec_full
#define vec_end            cook_vec_end
#define vec_free           cook_vec_free
#define vec_reset          cook_vec_reset

#define arr_size           cook_arr_size
#define arr_foreach        cook_arr_foreach

#define hash_set           cook_hash_set
#define hash_get           cook_hash_get
#define hash_free          cook_hash_free

#define DLL_Handle         Cook_DLL_Handle
#define dll_load           cook_dll_load
#define dll_close          cook_dll_close
#define dll_getsym         cook_dll_getsym
#define dll_geterr         cook_dll_geterr

#define IS_POWER_2         COOK_IS_POWER_2
#define ALIGN_UP           COOK_ALIGN_UP
#define ALIGN_DOWN         COOK_ALIGN_DOWN
#define OFFSET_OF          COOK_OFFSET_OF
#define CONTAINER_OF       COOK_CONTAINER_OF

#define temp_alloc         cook_temp_alloc
#define temp_strdup        cook_temp_strdup
#define temp_strndup       cook_temp_strndup
#define temp_strsub        cook_temp_strsub
#define temp_strfmt        cook_temp_strfmt
#define temp_save          cook_temp_save
#define temp_rewind        cook_temp_rewind
#define temp_reset         cook_temp_reset
#define temp_scope         cook_temp_scope

#define sv_from_cstr       cook_sv_from_cstr
#define sv_from_parts      cook_sv_from_parts
#define sv_equal           cook_sv_equal
#define sv_compare         cook_sv_compare
#define sv_starts_with     cook_sv_starts_with
#define sv_ends_with       cook_sv_ends_with
#define sv_empty           cook_sv_empty
#define sv_slice           cook_sv_slice
#define sv_trim            cook_sv_trim
#define sv_ltrim           cook_sv_ltrim
#define sv_rtrim           cook_sv_rtrim

#define sb_append_sv       cook_sb_append_sv
#define sb_append_parts    cook_sb_append_parts
#define sb_reset           cook_sb_reset
#define sb_free            cook_sb_free
#define sb_view            cook_sb_view
#define sb_append          cook_sb_append

#endif /* COOK_STRIP_PREFIX */

/*
------------------------------------------------------------------------------
This software is available under MIT license.
------------------------------------------------------------------------------
Copyright (c) 2025 Dylaris
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, COOKING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
