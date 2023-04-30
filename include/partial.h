// turn-on ignoring GCC diagnostics for -Wpedantic in C99 linking to ffi.h
#if defined(__GNUC__) && (__GNUC__ > 4 || \
                         (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || \
                                           (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 4))))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include "ffi.h"
// turn-off ignoring GCC diagnostics for -Wpedantic in C99 linking to ffi.h
#if defined(__GNUC__) && (__GNUC__ > 4 || \
                         (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || \
                                           (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 4))))
#pragma GCC diagnostic pop
#endif


#ifndef PARTIAL_MAX_NARG
#define PARTIAL_MAX_NARG 16
#endif

#define DYNAMIC_BUFFER_FLAG 1

#define PARTIAL_SENTINEL -1
#define EMPTY 
#define FUNC_PROTOTYPE(name) void (*name) (void)
#define FUNC_CAST(func) (FUNC_PROTOTYPE(EMPTY)) func

typedef struct PartialType PartialType;

typedef enum partial_status {
    PARTIAL_NOT_YET_IMPLEMENTED = -14,
    PARTIAL_TYPE_ERROR = -13,
    PARTIAL_BAD_FORMAT = -12,
    PARTIAL_INIT_FAILED = -11,
    PARTIAL_COPY_FAILURE = -10,
    PARTIAL_INSUFFICIENT_ARGS = -9,
    PARTIAL_INSUFFICIENT_BUFFER_SIZE = -8,
    PARTIAL_TOO_MANY_ARGS = -7,
    PARTIAL_UNSUPPORTED_TYPE = -6,
    PARTIAL_INCOMPLETE_SPEC = -5,
    PARTIAL_VALUE_ERROR = -4,
    PARTIAL_REALLOC_FAILURE = -3,
    PARTIAL_MALLOC_FAILURE = -2,
    PARTIAL_FAILURE = -1,
    PARTIAL_SUCCESS = FFI_OK,
    PARTIAL_BAD_TYPEDEF = FFI_BAD_TYPEDEF,
    PARTIAL_BAD_ABI = FFI_BAD_ABI,
    PARTIAL_BAD_ARGTYPE = FFI_BAD_ARGTYPE
} partial_status;

typedef struct PartialArg {
    PartialType * type;
    size_t buf_loc;
} PartialArg;

typedef struct Partial {
    FUNC_PROTOTYPE(func);
    //void* (*func)(); // cannot use this. libffi uses void (*)(void)...for some weird reason. I thought this explicitly excluded arguments
    PartialArg args[PARTIAL_MAX_NARG+1]; // first is for return value
    size_t argset; // flags for frozen data
    // when keywords are implemented, add a c-str to int map element
    size_t buffer_size;
    unsigned char * buffer;
    unsigned int flags; // for tracking memory allocations
    unsigned int narg;
    ffi_abi ABI;
} Partial;

partial_status Partial_init(Partial * pobj, FUNC_PROTOTYPE(func), char * format, unsigned char * buffer, size_t buffer_size, unsigned int flags);
partial_status Partial_bind(Partial * pobj, ...);
partial_status Partial_bind_args(Partial * pobj, ...); // bind null-terminated list of arguments in order
partial_status Partial_bind_kwargs(Partial * pobj, ...); // bind null-terminated list of keyword arguments
partial_status Partial_call(void * ret, Partial * pobj, ...);
void Partial_del(Partial * pobj);