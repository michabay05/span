#ifndef _BASE_H_
#define _BASE_H_

#include <stdint.h>
#include "raylib.h"
#include "raymath.h"
#include "arena.h"
#include "nob.h"

#define SP_LEN(arr) ((int)(sizeof(arr) / sizeof(arr[0])))
#define SP_ASSERT assert
#define SP_UNUSED(x) ((void)x)
#define SP_UNIMPLEMENTED(message)                                                                 \
    do {                                                                                          \
        fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message);                 \
        abort();                                                                                  \
    } while (0)
#define SP_UNREACHABLEF(message, ...)                                                             \
    do {                                                                                          \
        fprintf(stderr, "%s:%d: UNREACHABLE: " message "\n", __FILE__, __LINE__, __VA_ARGS__);    \
        abort();                                                                                  \
    } while (0)

#define SP_PRINT_V2(v) (printf("%s = (%f, %f)\n", #v, v.x, v.y))
#define SP_PRINT_CLR(c) (printf("%s = (%d, %d, %d, %d)\n", #c, c.r, c.g, c.b, c.a))
#define SP_STRUCT_ARR(st_name, type) \
    typedef struct {   \
        type *items;   \
        int count;     \
        int capacity;  \
    } st_name

typedef float f32;
typedef double f64;
typedef uint16_t Id;

typedef struct {
    int x, y;
} IVector2;

typedef struct {
    f64 x, y;
} DVector2;

Arena arena = {0};

#endif // _BASE_H_
