#ifndef _BASE_H_
#define _BASE_H_

#include <stdint.h>
#include "arena.h"
#include "nob.h"

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

Arena arena = {0};

#endif // _BASE_H_
