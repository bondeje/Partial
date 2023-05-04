#include <stddef.h>

// libffi using an unnamed union, which is not allowed by ISO C99, but it should not be a problem since I'm linked against libffi compiled to a newer standard
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

#ifndef PARTIAL_ALIAS_MAP_SIZE
#define PARTIAL_ALIAS_MAP_SIZE (2*PARTIAL_MAX_NARG+1)
#endif
#include "keyword_map.h"

#ifndef PARTIAL_MAX_DEFAULT_SIZE
#define PARTIAL_MAX_DEFAULT_SIZE 256
#endif

#define PARTIAL_DEFAULT_ABI FFI_DEFAULT_ABI

#define ALLOCED_BUFFER_FLAG     1
#define ALLOCED_PARTIAL_FLAG    2
#define PYTHON_STYLE            4

// use only as flag at compile time
#ifdef PARTIAL_PYTHON_STYLE
#define PARTIAL_DEFAULT_FLAGS ALLOCED_BUFFER_FLAG | ALLOCED_PARTIAL_FLAG | PYTHON_STYLE
#else
#define PARTIAL_DEFAULT_FLAGS ALLOCED_BUFFER_FLAG | ALLOCED_PARTIAL_FLAG
#endif

#define PARTIAL_SENTINEL -1
#define EMPTY 
#define FUNC_PROTOTYPE(name) void (*name) (void)
#define FUNC_CAST(func) (FUNC_PROTOTYPE(EMPTY)) func

typedef ffi_abi partial_abi;
typedef struct PartialType PartialType;

typedef enum partial_status {
    PARTIAL_CANNOT_FILL_BOUND_ARG = -17,
    PARTIAL_DEFAULT_STRING_TOO_LARGE = -16,
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
    AliasMap map; // this is going to be a very large component
    FUNC_PROTOTYPE(func);
    //void* (*func)(); // cannot use this. libffi uses void (*)(void)...for some weird reason. I thought this explicitly excluded arguments
    PartialArg args[PARTIAL_MAX_NARG+1]; // first is for return value
    size_t arg_bound; // flags for frozen data
    size_t py_keys; // for python_style keys
    // when keywords are implemented, add a c-str to int map element
    size_t buffer_size;
    size_t stack_size;
    unsigned char * buffer;
    unsigned int flags; // for tracking memory allocations
    unsigned int narg;
    partial_abi ABI;
    partial_status status;
} Partial;


partial_status Partial_init(Partial * pobj, partial_abi abi, FUNC_PROTOTYPE(func), char * format, unsigned char * buffer, size_t buffer_size, unsigned int flags);
partial_status Partial_bind_nargs(Partial * pobj, unsigned int nargin, ...);
#ifndef PARTIAL_PYTHON_STYLE
partial_status Partial_bind_npairs(Partial * pobj, unsigned int nargin, ...);
partial_status Partial_fill_npairs(Partial * pobj, unsigned int nargin, ...);
partial_status Partial_fill_nargs(Partial * pobj, unsigned int nargin, ...);
partial_status Partial_bind_nkwargs(Partial * pobj, unsigned int nkwargin, ...);
#endif
partial_status Partial_fill_nkwargs(Partial * pobj, unsigned int nkwargin, ...);

// since ffi_abi always starts at 0, <0 will indicate to use default. user only needs to go below FFI_FIRST_ABI
Partial * Partial_new(partial_abi abi, FUNC_PROTOTYPE(func), char * format, unsigned int nargin, unsigned int nkwargin, ...);
partial_status Partial_call(Partial * pobj, void * ret, unsigned int nargin, unsigned int nkwargin, ...);
void Partial_del(Partial * pobj);