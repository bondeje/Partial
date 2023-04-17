#include <stddef.h> // size_t
#include <stdlib.h> // maybe not necessary
#include <stdarg.h> // va_list, etc.
//#include <stdio.h>
#include <string.h> // memcpy

#ifndef MAX_PARTIAL_NARG
#define MAX_PARTIAL_NARG 16
#endif // MAX_PARTIAL_NARG

#define HEAP_BUFFER_FLAG 1
#define MIN_VA_ARG_SIZE sizeof(int)

#define CONCAT2(A,B) A##B
#define CONCAT(A,B) CONCAT2(A,B)
#define UNIQUE_VAR_NAME(BASENAME) CONCAT(BASENAME,__LINE__)

#define BUFFER(n) CONCAT(struct PartialBuffer_,n)
#define DEF_BUFFER(n) BUFFER(n) {unsigned char x[n];}
#define POLL_VA_ARG(var, va_args, n) DEF_BUFFER(n) var = va_arg(va_args, BUFFER(n))

//#define GET_SIZE(type) (sizeof(type) == sizeof(CONFIG_FLOAT) ? (sizeof(double)) : (sizeof(type) < sizeof(int) ? sizeof(int) : sizeof(type)))
#define GET_SIZE(type) sizeof(type)

#define  TYPE_LIST1(type) GET_SIZE(type)
#define  TYPE_LIST2(type, ...) GET_SIZE(type), TYPE_LIST1(__VA_ARGS__)
#define  TYPE_LIST3(type, ...) GET_SIZE(type), TYPE_LIST2(__VA_ARGS__)
#define  TYPE_LIST4(type, ...) GET_SIZE(type), TYPE_LIST3(__VA_ARGS__)
#define  TYPE_LIST5(type, ...) GET_SIZE(type), TYPE_LIST4(__VA_ARGS__)
#define  TYPE_LIST6(type, ...) GET_SIZE(type), TYPE_LIST5(__VA_ARGS__)
#define  TYPE_LIST7(type, ...) GET_SIZE(type), TYPE_LIST6(__VA_ARGS__)
#define  TYPE_LIST8(type, ...) GET_SIZE(type), TYPE_LIST7(__VA_ARGS__)
#define  TYPE_LIST9(type, ...) GET_SIZE(type), TYPE_LIST8(__VA_ARGS__)
#define TYPE_LIST10(type, ...) GET_SIZE(type), TYPE_LIST9(__VA_ARGS__)
#define TYPE_LIST11(type, ...) GET_SIZE(type), TYPE_LIST10(__VA_ARGS__)
#define TYPE_LIST12(type, ...) GET_SIZE(type), TYPE_LIST11(__VA_ARGS__)
#define TYPE_LIST13(type, ...) GET_SIZE(type), TYPE_LIST12(__VA_ARGS__)
#define TYPE_LIST14(type, ...) GET_SIZE(type), TYPE_LIST13(__VA_ARGS__)
#define TYPE_LIST15(type, ...) GET_SIZE(type), TYPE_LIST14(__VA_ARGS__)
#define TYPE_LIST16(type, ...) GET_SIZE(type), TYPE_LIST15(__VA_ARGS__)

#define GET_SIZE_LIST(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME
#define SIZE_LIST(...) GET_SIZE_LIST(__VA_ARGS__, TYPE_LIST16, TYPE_LIST15, TYPE_LIST14, TYPE_LIST13, \
                                                  TYPE_LIST12, TYPE_LIST11, TYPE_LIST10, TYPE_LIST9,  \
                                                  TYPE_LIST8, TYPE_LIST7, TYPE_LIST6, TYPE_LIST5,     \
                                                  TYPE_LIST4, TYPE_LIST3, TYPE_LIST2, TYPE_LIST1, UNUSED)(__VA_ARGS__)

#define CAST_POINTER(pobjp, index, type) *(type*)((pobjp)->buffer + (pobjp)->byte_loc[index])

#define MAKE_PARTIAL_ARG_LIST1(pobjp, index, type) CAST_POINTER(pobjp, index, type)
#define MAKE_PARTIAL_ARG_LIST2(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST1(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST3(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST2(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST4(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST3(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST5(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST4(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST6(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST5(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST7(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST6(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST8(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST7(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST9(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST8(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST10(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST9(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST11(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST10(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST12(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST11(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST13(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST12(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST14(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST13(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST15(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST14(pobjp, index+1, __VA_ARGS__)
#define MAKE_PARTIAL_ARG_LIST16(pobjp, index, type, ...) CAST_POINTER(pobjp, index, type), MAKE_PARTIAL_ARG_LIST15(pobjp, index+1, __VA_ARGS__)

#define GET_NARG( _1, _2, _3, _4, _5, _6, _7, _8,\
                  _9,_10,_11,_12,_13,_14,_15,_16,\
                 _17,_18,_19,_20,_21,_22,_23,_24,\
                 _25,_26,_27,_28,_29,_30,_31,_32,\
                 _33,_34,_35,_36,_37,_38,_39,_40,\
                 _41,_42,_43,_44,_45,_46,_47,_48,\
                 _49,_50,_51,_52,_53,_54,_55,_56,\
                 _57,_58,_59,_60,_61,_62,_63,  N,...) N
#define NARG(...) GET_NARG(__VA_ARGS__,  \
                 63,62,61,60,59,58,57,56,\
                 55,54,53,52,51,50,49,48,\
                 47,46,45,44,43,42,41,40,\
                 39,38,37,36,35,34,33,32,\
                 31,30,29,28,27,26,25,24,\
                 23,22,21,20,19,18,17,16,\
                 15,14,13,12,11,10, 9, 8,\
                  7, 6, 5, 4, 3, 2, 1, 0, UNUSED)

#define Partial_new(func, ...) \
(unsigned int UNIQUE_VAR_NAME(sizes)[NARG(__VA_ARGS__)] = {SIZE_LIST(__VA_ARGS__)}, \
Partial_init(pobjp, (void* (*)())func, NULL, 0, UNIQUE_VAR_NAME(sizes), NARG(__VA_ARGS__), HEAP_BUFFER_FLAG))

#define Partial(pobjp, func, buffer, buffer_size, ...) \
unsigned int UNIQUE_VAR_NAME(sizes)[NARG(__VA_ARGS__)] = {SIZE_LIST(__VA_ARGS__)};\
Partial_init(pobjp, (void* (*)())func, buffer, buffer_size, UNIQUE_VAR_NAME(sizes), NARG(__VA_ARGS__), 0);

#define GET_ARG_LIST_MACRO(n) CONCAT(MAKE_PARTIAL_ARG_LIST, n)
#define Partial_call(type, pobjp, ...) ((type (*)())(pobjp)->func)(GET_ARG_LIST_MACRO(NARG(__VA_ARGS__))(pobjp, 0, __VA_ARGS__))

typedef struct {
    unsigned char x[3];
} CONFIG_FLOAT;

typedef struct Partial {
    unsigned int byte_loc[MAX_PARTIAL_NARG+1];
    //unsigned int type[MAX_PARTIAL_NARG];
    void* (*func)();
    size_t argset; // flags for frozen data
    size_t buffer_size;
    unsigned int flags; // for tracking memory allocations
    unsigned short narg;
    unsigned char * buffer;
} Partial;

enum partial_status { 
    PARTIAL_UNSUPPORTED_TYPE = -6,
    PARTIAL_INCOMPLETE = -5,
    PARTIAL_VALUE_ERROR = -4,
    PARTIAL_REALLOC_FAILURE = -3,
    PARTIAL_MALLOC_FAILURE = -2,
    PARTIAL_FAILURE = -1,
    PARTIAL_SUCCESS,
};

#define PARTIAL_SENTINEL -1

/*
enum partial_types {
    PARTIAL_SENTINEL = -1,  // NULL type/index indicator
    PARTIAL_VOID,           // "v"
    PARTIAL_BOOL,           // "b"
    PARTIAL_CHAR,           // "c"
    PARTIAL_UCHAR,          // "cu"
    PARTIAL_SHORT,          // "hd"
    PARTIAL_USHORT,         // "hu"
    PARTIAL_INT,            // "d"
    PARTIAL_UINT,           // "u"
    PARTIAL_LONG,           // "ld"
    PARTIAL_ULONG,          // "lu"
    PARTIAL_FLT,            // "f"
    PARTIAL_DBL,            // "lf"
    PARTIAL_LLONG,          // "lld"
    PARTIAL_ULLONG,         // "llu"
    PARTIAL_SIZE_T,         // "zu"
    PARTIAL_PVOID,          // "p"
    PARTIAL_VOIDPVOID,      // "pv" void (*)()
    PARTIAL_PVOIDPVOID,     // "pp" void * (*)()
};
*/

Partial * Partial_new_(void* (*func)(), unsigned int * sizes, unsigned short nparam, unsigned int flags);

void Partial_init(Partial * pobj, void* (*func)(), unsigned char * buffer, size_t buffer_size, unsigned int * sizes, unsigned short nparam, unsigned int flags);

enum partial_status Partial_bind(Partial * pobj, ...);

enum partial_status Partial_fill(Partial * pobj, ...);