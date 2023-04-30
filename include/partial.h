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

#define PARTIAL_DEFAULT_ABI FFI_DEFAULT_ABI

#define DYNAMIC_BUFFER_FLAG 1

#define PARTIAL_SENTINEL -1
#define EMPTY 
#define FUNC_PROTOTYPE(name) void (*name) (void)
#define FUNC_CAST(func) (FUNC_PROTOTYPE(EMPTY)) func

typedef ffi_abi partial_abi;
typedef struct PartialType PartialType;

typedef enum partial_status {
    PARTIAL_KEY_ERROR = -15,
    PARTIAL_NOT_YET_IMPLEMENTED = -14,
    PARTIAL_TYPE_ERROR = -13,
    PARTIAL_BAD_FORMAT = -12,
    PARTIAL_INIT_FAILED = -11,
    PARTIAL_COPY_FAILURE = -10,
    PARTIAL_INSUFFICIENT_ARGS = -9,
    PARTIAL_INSUFFICIENT_BUFFER_SIZE = -8,
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
    partial_abi ABI;
    partial_status status;
} Partial;

Partial * Partial_new(partial_abi ABI, FUNC_PROTOTYPE(func), char * format, unsigned int nargin, unsigned int nkwargin, ...);
partial_status Partial_init(Partial * pobj, partial_abi ABI, FUNC_PROTOTYPE(func), char * format, unsigned char * buffer, size_t buffer_size, unsigned int flags);
partial_status Partial_bind_n(Partial * pobj, unsigned int nargin, ...);
partial_status Partial_bind(Partial * pobj, ...);
partial_status Partial_bind_nargs(Partial * pobj, unsigned int nargin, ...); // bind null-terminated list of arguments in order
partial_status Partial_bind_nkwargs(Partial * pobj, unsigned int nkwargin, ...); // bind null-terminated list of keyword arguments
partial_status Partial_call(Partial * pobj, void * ret, ...);
partial_status Partial_calln(Partial * pobj, void * ret, unsigned int nargin, ...);
void Partial_del(Partial * pobj);