//inspired by
//https://github.com/gingerBill/gb
//https://github.com/tsoding/nob.h
//https://github.com/nothings/stb

#ifndef _CYC_CDT_H
#define _CYC_CDT_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

//copy paster from `https://stackoverflow.com/a/26408195`
// get number of arguments with __NARG__
#define __NARG__(...)  __NARG_I_(__VA_ARGS__,__RSEQ_N())
#define __NARG_I_(...) __ARG_N(__VA_ARGS__)
#define __ARG_N( \
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
     _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
     _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
     _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
     _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
     _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
     _61,_62,_63,N,...) N
#define __RSEQ_N() \
     63,62,61,60,                   \
     59,58,57,56,55,54,53,52,51,50, \
     49,48,47,46,45,44,43,42,41,40, \
     39,38,37,36,35,34,33,32,31,30, \
     29,28,27,26,25,24,23,22,21,20, \
     19,18,17,16,15,14,13,12,11,10, \
     9,8,7,6,5,4,3,2,1,0

// general definition for any function name
#define _VFUNC_(name, n) name##n
#define _VFUNC(name, n) _VFUNC_(name, n)
#define VFUNC(func, ...) _VFUNC(func, __NARG__(__VA_ARGS__)) (__VA_ARGS__)

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;
typedef size_t usize;
typedef ptrdiff_t isize;
typedef float  f32;
typedef double f64;

#define U8_MIN 0u
#define U8_MAX 0xffu
#define I8_MIN (-0x7f - 1)
#define I8_MAX 0x7f

#define U16_MIN 0u
#define U16_MAX 0xffffu
#define I16_MIN (-0x7fff - 1)
#define I16_MAX 0x7fff

#define U32_MIN 0u
#define U32_MAX 0xffffffffu
#define I32_MIN (-0x7fffffff - 1)
#define I32_MAX 0x7fffffff

#define U64_MIN 0ull
#define U64_MAX 0xffffffffffffffffull
#define I64_MIN (-0x7fffffffffffffffll - 1)
#define I64_MAX 0x7fffffffffffffffll

#define F32_MIN 1.17549435e-38f
#define F32_MAX 3.40282347e+38f

#define F64_MIN 2.2250738585072014e-308
#define F64_MAX 1.7976931348623157e+308

#define ARRAY_SIZE(_x) (sizeof(_x) / sizeof((_x)[0]))

typedef union iv2 {
    struct { i32 x; i32 y; };
    struct { i32 r; i32 g; };
    struct { i32 w; i32 h; };
} iv2;
static_assert(sizeof(iv2) == sizeof(i32) * 2);

#define _V2I2(_x, _y) ((iv2){ .x = _x, .y = _y })
#define _V2I1(_x) ((iv2){ .x = _x, .y = _x })
#define iv2(...) VFUNC(_V2I, __VA_ARGS__)

typedef union fv2 {
    struct { f32 x; f32 y; };
    struct { f32 r; f32 g; };
    struct { f32 w; f32 h; };
} fv2;
static_assert(sizeof(fv2) == sizeof(f32) * 2);

#define _V2F2(_x, _y) ((fv2){ .x = _x, .y = _y })
#define _V2F1(_x) ((fv2){ .x = _x, .y = _x })
#define fv2(...) VFUNC(_V2F, __VA_ARGS__)
#define v2(...) VFUNC(_V2F, __VA_ARGS__)

typedef fv2 v2;

typedef union iv3 {
    struct { i32 x; i32 y; i32 z; };
    struct { i32 r; i32 g; i32 b; };
} iv3;
static_assert(sizeof(iv3) == sizeof(i32) * 3);

#define _V3I3(_x, _y, _z) ((iv3){ .x = _x, .y = _y, .z = _z })
#define _V3I2(_v, _z) ((iv3){ .x = (_v).x, .y = (_v).y, .z = _z })
#define _V3I1(_x) ((iv3){ .x = _x, .y = _x, .z = _x })
#define iv3(...) VFUNC(_V3I, __VA_ARGS__)

typedef union fv3 {
    struct { f32 x; f32 y; f32 z; };
    struct { f32 r; f32 g; f32 b; };
} fv3;
static_assert(sizeof(fv3) == sizeof(f32) * 3);

#define _V3F3(_x, _y, _z) ((fv3){ .x = _x, .y = _y, .z = _z })
#define _V3F2(_v, _z) ((fv3){ .x = (_v).x, .y = (_v).y, .z = _z })
#define _V3F1(_x) ((fv3){ .x = _x, .y = _x, .z = _x })
#define fv3(...) VFUNC(_V3F, __VA_ARGS__)
#define v3(...) VFUNC(_V3F, __VA_ARGS__)

typedef fv3 v3;

typedef union iv4 {
    struct { i32 x; i32 y; i32 z; i32 w; };
    struct { i32 r; i32 g; i32 b; i32 a; };
} iv4;
static_assert(sizeof(iv4) == sizeof(i32) * 4);

#define _V4I4(_x, _y, _z, _w) ((iv4){ .x = _x, .y = _y, .z = _z, .w = _w })
#define _V4I2(_v, _w) ((iv4){ .x = (_v).x, .y = (_v).y, .z = (_v).z, .w = _w })
#define _V4I1(_x) ((iv4){ .x = _x, .y = _x, .z = _x, .w = _x })
#define iv4(...) VFUNC(_V4I, __VA_ARGS__)

typedef union fv4 {
    struct { f32 x; f32 y; f32 z; f32 w; };
    struct { f32 r; f32 g; f32 b; f32 a; };
} fv4;
static_assert(sizeof(fv4) == sizeof(f32) * 4);

#define _V4F4(_x, _y, _z, _w) ((fv4){ .x = _x, .y = _y, .z = _z, .w = _w })
#define _V4F2(_v, _w) ((fv4){ .x = (_v).x, .y = (_v).y, .z = (_v).z, .w = _w })
#define _V4F1(_x) ((fv4){ .x = _x, .y = _x, .z = _x, .w = _x })
#define fv4(...) VFUNC(_V4F, __VA_ARGS__)
#define v4(...) VFUNC(_V4F, __VA_ARGS__)

typedef fv4 v4;

typedef struct icolor3 {
    u8 r, g, b;
} icolor3;

#define _I3C3(_r, _g, _b) ((icolor3){ .r = _r, .g = _g, .b = _b })
#define icolor3(...) VFUNC(_I3C, __VA_ARGS__)

typedef fv3 fcolor3;
typedef fcolor3 color3;

#define fcolor3(...) VFUNC(_V3F, __VA_ARGS__)
#define color3(...) VFUNC(_V3F, __VA_ARGS__)

typedef struct icolor4 {
    u8 r, g, b, a;
} icolor4;

#define _I4C4(_r, _g, _b, _a) ((icolor3){ .r = _r, .g = _g, .b = _b, .a = _a })
#define _I4C2(_c, _a) ((icolor3){ .r = (_c).r, .g = (_c).g, .b = (_c).b, .a = _a })
#define icolor4(...) VFUNC(_I4C, __VA_ARGS__)

typedef fv4 fcolor4;
typedef fcolor4 color4;

#define fcolor4(...) VFUNC(_V4F, __VA_ARGS__)
#define color4(...) VFUNC(_V4F, __VA_ARGS__)

void cdt_error_handler(const char *file, i32 line, const char *pre_msg, const char *msg, ...);

#define CDT_ASSERT assert
#define CDT_TRAP() abort()

#define cast(T) (T)
#define unused (void)

#define panic(msg, ...) do { \
    cdt_error_handler(__FILE__, __LINE__, NULL, msg, ##__VA_ARGS__); \
    CDT_TRAP(); \
} while(0)

#define todo() do { \
    panic("not yet implemented"); \
} while(0)

//an interesting error system implementaiton
typedef struct Code_Location {
    usize line;
    const char *file;
} Code_Location;

#define _current_location ((Code_Location){ .line = cast(usize) __LINE__, .file = __FILE__ })
#define location_fmt "%s:%ld: "
#define location_arg(_a) _a.file, _a.line

typedef struct Error {
    Code_Location loc;
    usize code;
} Error;

#define zero_error() ((Error){ .loc = _current_location, .code = 0 })
#define new_error(_err) ((Error){ .loc = _current_location, .code = _err })
#define is_error(_e) ((_e).code != 0)

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define clamp(x, lower, upper) min(max((x), (lower)), (upper))
#define in_between(x, l, u) ((x) >= (l) && (x) <= (u))

//TODO: implement proper allocator
void *xcalloc (usize num_elems, usize elem_size);
void *xrealloc(void *ptr, usize num_bytes);
void *xmalloc (usize num_bytes);
void  xfree   (void *ptr);

//TODO: implement string view fully
typedef struct String_View {
    const char *bytes;
    usize count;
} String_View;

String_View sv_from_parts(const char *bytes, usize count);
bool sv_eq(String_View s1, String_View s2);
bool sv_starts_with(String_View sv, String_View pre);

//TODO: implement string builder fully
typedef char * String_Builder;
int sb_appendf(String_Builder *sb, const char *fmt, ...);
bool sb_read_file(String_Builder *sb, const char *file_path);

//TODO: add formatter
#define SV(x) sv_from_parts(x, strlen(x))
#define sv_arg(x) cast(i32)(x.count), (x.bytes)
#define sb_arg(x) cast(i32)vec_len(x), (x)

#define sv_fmt "%.*s"
#define sb_fmt sv_fmt

#define sv_from_sb(x) ((String_View){ .bytes = x, .count = vec_len(x) })

typedef struct Vector_Header {
    usize count;
    usize capacity;
    char data[];
} Vector_Header;

#define VECTOR_INIT_CAP 256

#define vec_hdr(_v) ((Vector_Header *)((char *)(_v) - offsetof(Vector_Header, data)))

//TODO: make every push releated function to take pointer
//      because vec_fit modifies the array pointer

#define Vector(_T) _T *
#define vec_len(_v) ((_v) ? vec_hdr(_v)->count : 0)
#define vec_cap(_v) ((_v) ? vec_hdr(_v)->capacity : 0)
#define vec_end(_v) ((_v) + vec_len(_v))
#define vec_free(_v) ((_v) ? (xfree(vec_hdr(_v)), (_v) = NULL) : 0)
#define vec_fit(_v, _n) ((_n) <= vec_cap(_v) ? 0 : ((_v) = vec__grow((_v), (_n), sizeof(*(_v)))))
#define vec_push(_v, ...) (vec_fit((_v), 1 + vec_len(_v)), (_v)[vec_hdr(_v)->count++] = (__VA_ARGS__))
#define vec_clear(_v) ((_v) ? vec_hdr(_v)->count = 0 : 0)

void *vec__grow(const void *vec, usize new_len, usize elem_size);

#endif


#ifdef CDT_IMPL

void cdt_error_handler(const char *file, i32 line, const char *pre_msg, const char *msg, ...) {
    fprintf(stderr, "thread panicked at %s:%d:\n", file, line);
    if (pre_msg) {
        fprintf(stderr, "%s: ", pre_msg);
    }
    if (msg) {
        va_list va;
        va_start(va, msg);
        vfprintf(stderr, msg, va);
        va_end(va);
    } else {
        fprintf(stderr, "unspecified");
    }
    fprintf(stderr, "\n");
}

void *xcalloc(usize num_elems, usize elem_size) {
    void *ptr = calloc(num_elems, elem_size);
    if (!ptr) {
        panic("xcalloc");
    }
    return ptr;
}

void *xrealloc(void *ptr, usize num_bytes) {
    ptr = realloc(ptr, num_bytes);
    if (!ptr) {
        panic("xrealloc");
    }
    return ptr;
}

void *xmalloc(usize num_bytes) {
    void *ptr = malloc(num_bytes);
    if (!ptr) {
        panic("xmalloc");
    }
    return ptr;
}

void xfree(void *ptr) {
    if (!ptr) {
        panic("xfree");
    }
    free(ptr);
}

void *vec__grow(const void *vec, usize new_len, usize elem_size) {
    usize cap = vec_cap(vec);
    usize new_cap = cap;
    if (cap <= 0) {
        new_cap = VECTOR_INIT_CAP;
    }
    while (new_len > new_cap) {
        new_cap *= 2;
    }
    assert(new_len <= new_cap);
    usize new_size = offsetof(Vector_Header, data) + new_cap*elem_size;
    Vector_Header *new_hdr;
    if (vec) {
        new_hdr = xrealloc(vec_hdr(vec), new_size);
    } else {
        new_hdr = xmalloc(new_size);
        new_hdr->count = 0;
    }
    new_hdr->capacity = new_cap;
    return new_hdr->data;
}

String_View sv_from_parts(const char *bytes, usize count) {
    String_View sv = {
        bytes,
        count,
    };
    return sv;
}

bool sv_eq(String_View s1, String_View s2) {
    if (s1.count != s2.count) return false;

    return memcmp(s1.bytes, s2.bytes, s1.count) == 0;
}

bool sv_starts_with(String_View sv, String_View pre) {
    if (pre.count <= sv.count) {
        String_View act = sv_from_parts(sv.bytes, pre.count);
        return sv_eq(act, pre);
    }

    return false;
}

int sb_appendf(String_Builder *sb, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    vec_fit(*sb, vec_len(*sb) + n + 1);
    char *dest = vec_end(*sb);

    va_start(args, fmt);
    vsnprintf(dest, n+1, fmt, args);
    va_end(args);

    vec_hdr(*sb)->count += n;

    return n;
}

bool sb_read_file(String_Builder *sb, const char *file_path) {
    FILE* f;
    usize len;

    f = fopen(file_path, "rb");
    if (f == NULL) {
        return false;
    }
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if (len <= 0) {
        return false;
    }
    fseek(f, 0, SEEK_SET);
    vec_fit(*sb, vec_len(*sb) + len + 1);
    char *dest = vec_end(*sb);
    fread(dest, len, 1, f);
    vec_hdr(*sb)->count += len;
    fclose(f);

    return true;
}


//NOTE: do i really need that for include order?
#undef CDT_IMPL
#endif
