#include <stddef.h>
#include <stdbool.h>
#ifndef MAX_PARTIAL_NARG
#define MAX_PARTIAL_NARG 16
#endif // MAX_PARTIAL_NARG

#define CONCAT2(A,B) A##B
#define CONCAT(A,B) CONCAT2(A,B)
#define UNIQUE_VAR_NAME(BASENAME) CONCAT(BASENAME,__LINE__)

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

// workhorse
#define DEREF_POINTER(type, pointer)\
(type == PARTIAL_BOOL ? *(bool*)(pointer) : \
(type == PARTIAL_CHAR ? *(char*)(pointer) : \
(type == PARTIAL_UCHAR ? *(unsigned char*)(pointer) : \
(type == PARTIAL_SHORT ? *(short*)(pointer) : \
(type == PARTIAL_USHORT ? *(unsigned short*)(pointer) : \
(type == PARTIAL_INT ? *(int*)(pointer) : \
(type == PARTIAL_UINT ? *(unsigned int*)(pointer) : \
(type == PARTIAL_LONG ? *(long int*)(pointer) : \
(type == PARTIAL_ULONG ? *(unsigned long int*)(pointer) : \
(type == PARTIAL_FLT ? *(float*)(pointer) : \
(type == PARTIAL_DBL ? *(double*)(pointer) : \
(type == PARTIAL_LLONG ? *(long long int*)(pointer) : \
(type == PARTIAL_ULLONG ? *(unsigned long long int*)(pointer) : \
(type == PARTIAL_SIZE_T ? *(size_t*)(pointer) : \
(type == PARTIAL_PVOID ? *(void**)(pointer) : \
(type == PARTIAL_VOIDPVOID ? *(void (**)())(pointer) : \
(type == PARTIAL_PVOIDPVOID ? *(void*(**)())(pointer) : 0)))))))))))))))))


/*
#define DEREF_POINTER(type, pointer)\
(type == PARTIAL_BOOL ? *(bool*)(pointer) : 0)
*/

#define ARG_LIST_1(types, buffer, offsets) DEREF_POINTER(types[0], buffer + offsets[0])
#define ARG_LIST_2(types, buffer, offsets) ARG_LIST_1(types, buffer, offsets), DEREF_POINTER(types[1], buffer + offsets[1])
#define ARG_LIST_3(types, buffer, offsets) ARG_LIST_2(types, buffer, offsets), DEREF_POINTER(types[2], buffer + offsets[2])
#define ARG_LIST_4(types, buffer, offsets) ARG_LIST_3(types, buffer, offsets), DEREF_POINTER(types[3], buffer + offsets[3])
#define ARG_LIST_5(types, buffer, offsets) ARG_LIST_4(types, buffer, offsets), DEREF_POINTER(types[4], buffer + offsets[4])
#define ARG_LIST_6(types, buffer, offsets) ARG_LIST_5(types, buffer, offsets), DEREF_POINTER(types[5], buffer + offsets[5])
#define ARG_LIST_7(types, buffer, offsets) ARG_LIST_6(types, buffer, offsets), DEREF_POINTER(types[6], buffer + offsets[6])
#define ARG_LIST_8(types, buffer, offsets) ARG_LIST_7(types, buffer, offsets), DEREF_POINTER(types[7], buffer + offsets[7])
#define ARG_LIST_9(types, buffer, offsets) ARG_LIST_8(types, buffer, offsets), DEREF_POINTER(types[8], buffer + offsets[8])
#define ARG_LIST_10(types, buffer, offsets) ARG_LIST_9(types, buffer, offsets), DEREF_POINTER(types[9], buffer + offsets[9])
#define ARG_LIST_11(types, buffer, offsets) ARG_LIST_10(types, buffer, offsets), DEREF_POINTER(types[10], buffer + offsets[10])
#define ARG_LIST_12(types, buffer, offsets) ARG_LIST_11(types, buffer, offsets), DEREF_POINTER(types[11], buffer + offsets[11])
#define ARG_LIST_13(types, buffer, offsets) ARG_LIST_12(types, buffer, offsets), DEREF_POINTER(types[12], buffer + offsets[12])
#define ARG_LIST_14(types, buffer, offsets) ARG_LIST_13(types, buffer, offsets), DEREF_POINTER(types[13], buffer + offsets[13])
#define ARG_LIST_15(types, buffer, offsets) ARG_LIST_14(types, buffer, offsets), DEREF_POINTER(types[14], buffer + offsets[14])
#define ARG_LIST_16(types, buffer, offsets) ARG_LIST_15(types, buffer, offsets), DEREF_POINTER(types[15], buffer + offsets[15])
#define GET_ARG_LIST_S(types, buffer, offsets, narg) CONCAT(ARG_LIST_, narg)(types, buffer, offsets)
#define BUILD_CALL_S(type_out, pobj_p, narg) ((type_out (*)())(pobj_p)->func)(GET_ARG_LIST_S((pobj_p)->type, (pobj_p)->buffer, (pobj_p)->byte_loc, narg))

typedef struct Partial {
    unsigned int byte_loc[MAX_PARTIAL_NARG+1];
    unsigned int type[MAX_PARTIAL_NARG];
    void* (*func)();
    size_t argset; // flags for frozen data
    size_t buffer_size;
    unsigned int flags; // for tracking memory allocations
    unsigned short narg;
    unsigned char * buffer;
} Partial;

enum partial_status { 
    PARTIAL_VALUE_ERROR = -4,
    PARTIAL_REALLOC_FAILURE = -3,
    PARTIAL_MALLOC_FAILURE = -2,
    PARTIAL_FAILURE = -1,
    PARTIAL_SUCCESS,
};

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

enum partial_status Partial_copy_buffer(Partial * pobj, size_t index, size_t size, va_list *args_p);

void Partial_init(Partial * pobj, void* (*func)(), char * format, unsigned char * call_stack_buffer, size_t buffer_size, unsigned int * sizes, unsigned short nparam, unsigned int flags);

// bind arguments to Partial object with pairs (unsigned int, value) when unsigned int == PARTIAL_SENTINEL, va_list terminates 
enum partial_status Partial_bind(Partial * pobj, ...);

// fill stack buffer with remaining arguments in function call. va_list terminates when all arguments are occupied
enum partial_status Partial_fill(Partial * pobj, ...);

// variadic are the remaining arguments in 
#define Partial_call(type_out, pobj_p, ...) (Partial_fill(pobj_p, __VA_ARGS__), BUILD_CALL_S(type_out, pobj_p, (pobj_p)->narg))