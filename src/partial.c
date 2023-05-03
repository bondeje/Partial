/*
TODO: need to figure out a way to pass c-str defaults into char * values. 
    Probably have to make a special case, but then need an extra buffer, or add
     to the existing buffer in pobj to store the default string. In the latter 
    case, default cstr argument will not work in a static implementation. The 
    only other alternative is to have a global buffer for string defaults, but
    that sounds like an aweful idea.

TODO: implement the aliasing and the corresponding keyword lookup
*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#ifdef DEVELOPMENT
    #include <stdio.h>
#endif

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
#include "partial.h"

#define DEFAULT_DYN_BUFFER_SIZE 64

#define ALLOCED_BUFFER_FLAG     DYNAMIC_BUFFER_FLAG
#define ALLOCED_PARTIAL_FLAG    2

#ifdef PARTIAL_PYTHON_STYLE
#define PARTIAL_DEFAULT_FLAGS ALLOCED_BUFFER_FLAG | ALLOCED_PARTIAL_FLAG | PYTHON_STYLE
#else
#define PARTIAL_DEFAULT_FLAGS ALLOCED_BUFFER_FLAG | ALLOCED_PARTIAL_FLAG
#endif

#define FORMAT_SENTINEL '\0'
#define FORMAT_TYPE_START '%'
#define FORMAT_KEYWORD_SENTINEL '}'
#define FORMAT_KEYWORD_START '{'
#define FORMAT_RETURN_SENTINEL '='
#define FORMAT_DEFAULT_START FORMAT_RETURN_SENTINEL
#define FORMAT_DEFAULT_SENTINEL FORMAT_KEYWORD_SENTINEL
#define WHITESPACE " \t\n\v\f\r"

#define PARTIAL_FORMAT_BUFFER_SIZE 5

#define PARTIAL_ARG_START_INDEX 1

struct PartialType {
    ffi_type * ftp;
    size_t size;
    char * fmt_code;
    char * name;
    unsigned short id;
};

typedef struct VA_LIST {
    va_list args;
} VA_LIST;


typedef void (*voidpfunc)();
typedef void* (*pvoidpfunc)();

static bool partial_initialized = false;

#define PARTIAL_N_TYPES 20

#define PARTIAL_VOID         0
#define PARTIAL_BOOL         1
#define PARTIAL_CHAR         2
#define PARTIAL_UCHAR        3
#define PARTIAL_SHORT        4
#define PARTIAL_USHORT       5
#define PARTIAL_INT          6
#define PARTIAL_UINT         7
#define PARTIAL_LONG         8
#define PARTIAL_ULONG        9
#define PARTIAL_LLONG       10
#define PARTIAL_ULLONG      11
#define PARTIAL_SIZE_T      12
#define PARTIAL_FLOAT       13
#define PARTIAL_DOUBLE      14
#define PARTIAL_LDOUBLE     15
#define PARTIAL_PVOID       16
#define PARTIAL_VOIDPFUNC   17
#define PARTIAL_PVOIDPFUNC  18
#define PARTIAL_CSTRING     19

PartialType partial_types[PARTIAL_N_TYPES];

static partial_status Partial_global_init() {
    for (unsigned int i = 0; i < PARTIAL_N_TYPES; i++) {
        switch (i) {
            case PARTIAL_VOID: {
                partial_types[i] = (PartialType) {&ffi_type_void, 0,"v", "void", PARTIAL_VOID};
                break;
            }
            case PARTIAL_BOOL: {
                partial_types[i] = (PartialType) {&ffi_type_schar, sizeof(char), "b", "bool", PARTIAL_BOOL};
                break;
            }
            case PARTIAL_CHAR: {
                partial_types[i] = (PartialType) {&ffi_type_schar, sizeof(char), "c", "char", PARTIAL_CHAR};
                break;
            }
            case PARTIAL_UCHAR: {
                partial_types[i] = (PartialType) {&ffi_type_uchar, sizeof(unsigned char), "cu", "unsigned char", PARTIAL_UCHAR};
                break;
            }
            case PARTIAL_SHORT: {
                partial_types[i] = (PartialType) {&ffi_type_sshort, sizeof(short), "hd", "short", PARTIAL_SHORT};
                break;
            }
            case PARTIAL_USHORT: {
                partial_types[i] = (PartialType) {&ffi_type_ushort, sizeof(unsigned short), "hu", "unsigned short", PARTIAL_USHORT};
                break;
            }
            case PARTIAL_INT: {
                partial_types[i] = (PartialType) {&ffi_type_sint, sizeof(int), "d", "int", PARTIAL_INT};
                break;
            }
            case PARTIAL_UINT: {
                partial_types[i] = (PartialType) {&ffi_type_uint, sizeof(unsigned int), "u", "unsigned int", PARTIAL_UINT};
                break;
            }
            case PARTIAL_LONG: {
                partial_types[i] = (PartialType) {&ffi_type_slong, sizeof(long), "ld", "long", PARTIAL_LONG};
                break;
            }
            case PARTIAL_ULONG: {
                partial_types[i] = (PartialType) {&ffi_type_ulong, sizeof(unsigned long), "lu", "unsigned long", PARTIAL_ULONG};
                break;
            }
            case PARTIAL_LLONG: {
                partial_types[i] = (PartialType) {&ffi_type_sint64, sizeof(long long), "lld", "long long", PARTIAL_LLONG};
                break;
            }
            case PARTIAL_ULLONG: {
                partial_types[i] = (PartialType) {&ffi_type_uint64, sizeof(unsigned long long),    "llu", "unsigned long long", PARTIAL_ULLONG};
                break;
            }
            case PARTIAL_SIZE_T: {
// TODO: need a better way to distinguish how size_t should be represented
#if SIZE_MAX == 2147483647
                partial_types[i] = (PartialType) {&ffi_type_uint32, sizeof(size_t), "zu", "size_t", PARTIAL_SIZE_T};
#else
                partial_types[i] = (PartialType) {&ffi_type_uint64, sizeof(size_t), "zu", "size_t", PARTIAL_SIZE_T};
#endif
                break;
            }
            case PARTIAL_FLOAT: {
                partial_types[i] = (PartialType) {&ffi_type_float, sizeof(float), "f", "float", PARTIAL_FLOAT};
                break;
            }
            case PARTIAL_DOUBLE: {
                partial_types[i] = (PartialType) {&ffi_type_double, sizeof(double), "lf", "double", PARTIAL_DOUBLE};
                break;
            }
            case PARTIAL_LDOUBLE: {
                partial_types[i] = (PartialType) {&ffi_type_longdouble, sizeof(long double), "LF", "long double", PARTIAL_LDOUBLE};
                break;
            }
            case PARTIAL_PVOID: {
                partial_types[i] = (PartialType) {&ffi_type_pointer, sizeof(void*), "p", "void *", PARTIAL_PVOID};
                break;
            }
            case PARTIAL_VOIDPFUNC: {
                // stand-in...this isn't standard C, but libffi apparently does not have a suitable alternative
                partial_types[i] = (PartialType) {&ffi_type_pointer, sizeof(voidpfunc), "vf", "void (*)()", PARTIAL_VOIDPFUNC};
                break;
            }
            case PARTIAL_PVOIDPFUNC: {
                // stand-in...this isn't standard C, but libffi apparently does not have a suitable alternative
                partial_types[i] = (PartialType) {&ffi_type_pointer, sizeof(pvoidpfunc), "pf", "void * (*)()", PARTIAL_PVOIDPFUNC};
                break;
            }
            case PARTIAL_CSTRING: {
                partial_types[i] = (PartialType) {&ffi_type_pointer, sizeof(char*), "s", "char *", PARTIAL_CSTRING};
                break;
            }
            default: {
                return PARTIAL_UNSUPPORTED_TYPE;
            }
        }
    }
    partial_initialized = true;
    return PARTIAL_SUCCESS;
}

static inline bool Format_is_whitespace(char c) {
    return strchr(WHITESPACE, c) != NULL;
}

static inline void Format_skip_whitespace(char ** cpp) {
    while (Format_is_whitespace(**cpp)) {
        (*cpp)++;
    }
}

static inline char * Format_get_type_code_end(char ** cpp) {
    char c = **cpp;
    /*
    valid type codes can end with
    1) whitespace,
    2) start of another type code, '%'
    3) return sentinel, '='
    4) start of a keyword or default, '{'
    5) end of c-string format sentinel, '\0'

    FORMAT_SENTINEL indicates cannot move pointer forward, but unlike keyword and default, this does not indicate an error
    */
    while (c != FORMAT_TYPE_START && c != FORMAT_RETURN_SENTINEL && c != FORMAT_KEYWORD_START && c != FORMAT_SENTINEL && !strchr(WHITESPACE, c)) {
        c = *(++(*cpp));
    }
    return *cpp;
}

static inline char * Format_get_keyword_end(char ** cpp) {
    char c = **cpp;
    /*
    valid keyword alias can end with
    1) whitespace,
    2) keyword sentinel, '}'
    3) start of a default spec, '='

    FORMAT_SENTINEL indicates cannot move pointer forward
    */
    while (c != FORMAT_SENTINEL && c != FORMAT_KEYWORD_SENTINEL && c != FORMAT_DEFAULT_START && !strchr(WHITESPACE, c)) {
        c = *(++(*cpp));
    }
    if (c == FORMAT_SENTINEL) {
        return NULL;
    }
    return *cpp;
}

static inline char * Format_get_default_end(char ** cpp) {
    char c = **cpp;
    /*
    valid default arguments can end with
    1) whitespace,
    2) default sentinel, '}'

    FORMAT_SENTINEL indicates cannot move pointer forward
    */
    while (c != FORMAT_SENTINEL && c != FORMAT_DEFAULT_SENTINEL && !strchr(WHITESPACE, c)) {
        c = *(++(*cpp));
    }
    if (c == FORMAT_SENTINEL) {
        return NULL;
    }
    return *cpp;
}

// end points to the character after the format code
// probably replace with a map once that is implemented
// TODO: make a generic string compare like this where one is a c-str and another is an incomplete c-str/c++ iterator style char array
static partial_status Partial_set_type(Partial * pobj, unsigned int index, size_t * buf_locp, char * start, char * end) {
    if (!start || !end) {
        pobj->status = PARTIAL_BAD_FORMAT;
        return pobj->status;
    }
    for (unsigned int i = 0; i < PARTIAL_N_TYPES; i++) {
        char * typep = start, * fmtp = partial_types[i].fmt_code;
        while (*fmtp != '\0' && typep != end && *fmtp == *typep) {
            fmtp++;
            typep++;
        }
        if (*fmtp == '\0' && typep == end) {
            pobj->args[index].type = &partial_types[i];
            pobj->args[index].buf_loc = *buf_locp;
            if (index) {
                *buf_locp += pobj->args[index].type->size;  // don't save return type to buffer
                pobj->narg++;                               // don't save 
            }
#ifdef DEVELOPMENT
    printf("type found: %s\n", pobj->args[index].type->name);
#endif
            return pobj->status;
        }
    }
    pobj->status = PARTIAL_UNSUPPORTED_TYPE;
    return pobj->status;
}

// TODO: still need to store the keyword somewhere
static partial_status Partial_set_alias(Partial * pobj, unsigned int index, char * start) {
    if (!start) {
        return (pobj->status = PARTIAL_BAD_FORMAT);
    }
#ifdef DEVELOPMENT
    printf("alias found: %s\n", start);
#endif
    if (KeywordMap_add(&pobj->map, start, index)) {
        pobj->status = PARTIAL_KEY_ERROR;
    }
    return pobj->status;
}

// TODO:
// if buffer is already present or allocated, handle buffer size errors or resizing if necessary
static partial_status Partial_set_default(Partial * pobj, unsigned int index, char * start, char * end) {
    if (!start || !end) {
        pobj->status = PARTIAL_BAD_FORMAT;
        return pobj->status;
    }
    if (end-start >= PARTIAL_MAX_DEFAULT_SIZE) {
        pobj->status = PARTIAL_DEFAULT_STRING_TOO_LARGE;
        return pobj->status;
    }
#ifdef DEVELOPMENT
    printf("default found: %s\n", start);
#endif

    // copy default into temporary buffer c-string
    char buffer[PARTIAL_MAX_DEFAULT_SIZE];
    unsigned int i = 0; // because of check above, i cannot be >= PARTIAL_MAX_DEFAULT_SIZE
    while (start != end) {
        buffer[i] = *start;
        i++;
        start++;
    }
    buffer[i] = '\0';

    // need to handle special case of c-str and void *
    // might be better to use a switch statement
    if (pobj->args[index].type->id == PARTIAL_PVOID) {
        if (strcmp(buffer, "NULL")) { 
            return (pobj->status = PARTIAL_TYPE_ERROR);
        }
        void * v = NULL;
        memcpy(pobj->buffer + pobj->args[index].buf_loc, &v, sizeof(void*));
    } else if (pobj->args[index].type->id == PARTIAL_BOOL) {
        bool default_bool = false;
        if (!strcmp(buffer, "true")) {
            default_bool = true;
        } else if (!strcmp(buffer, "false")) {
            default_bool = false;
        } else {
            return (pobj->status = PARTIAL_TYPE_ERROR);
        }
        memcpy(pobj->buffer + pobj->args[index].buf_loc, &default_bool, sizeof(bool));
    } else if (pobj->args[index].type->id == PARTIAL_CSTRING) {
        memcpy(pobj->buffer + pobj->args[index].buf_loc, &start, sizeof(char*));
    } else {
        // set format code for sscanf
        char format[PARTIAL_FORMAT_BUFFER_SIZE];
        i = 0;
        format[i] = '%';
        i++;
        if (pobj->args[index].type->id == PARTIAL_UCHAR) { // special case of unsigned char until i a better use case is defined.
            format[i] = 'c';
            i++;
        } else {
            char * f = pobj->args[index].type->fmt_code;
            while (*f != '\0') {
                format[i] = *f;
                f++;
                i++;
                if (i >= PARTIAL_FORMAT_BUFFER_SIZE) {
                    return (pobj->status = PARTIAL_INSUFFICIENT_BUFFER_SIZE);
                }
            }
        }
        format[i] = '\0';

        sscanf(buffer, format, (void*)(pobj->buffer + pobj->args[index].buf_loc)); // not sure this is going to work
    }
    return pobj->status;
}

// do this only once
static partial_status Partial_buffer_format(Partial * pobj, unsigned int index, size_t * buf_loc, char * format) {
#ifdef DEVELOPMENT
    printf("adding format to buffer\n");
#endif
    size_t N = strlen(format) + 1; // number of bytes needed to store the format
    if (pobj->buffer_size >= N || pobj->flags & ALLOCED_BUFFER_FLAG) {
#ifdef DEVELOPMENT
        printf("sufficient space in buffer\n");
#endif
        if (pobj->buffer_size < N) {
            if (pobj->buffer_size) {
                unsigned char * buffer = NULL;
                buffer = (unsigned char *) realloc(pobj->buffer, N);
                if (!buffer) {
                    return (pobj->status = PARTIAL_REALLOC_FAILURE);
                }
                pobj->buffer = buffer;
            } else {
                pobj->buffer = (unsigned char *) malloc(N);
                if (!pobj->buffer) {
                    return (pobj->status = PARTIAL_MALLOC_FAILURE);
                }
            }
            pobj->buffer_size = N;
        } 
        size_t i = 0;
        while (*format != '\0') {
            pobj->buffer[i] = (unsigned char)(*format);
            i++;
            format++;
        }
        pobj->buffer[i] = '\0';

#ifdef DEVELOPMENT
        printf("shifting buf_locations by %zu to accommodate format in buffer:\n", N);
#endif
        i = 0;
        while (i < index) {
#ifdef DEVELOPMENT
            printf("shifting %s location from %zu -> %zu\n", pobj->args[i].type->name, pobj->args[i].buf_loc, pobj->args[i].buf_loc + N);
#endif
            pobj->args[i].buf_loc += N;
            i++;
        }
        *buf_loc += N;
    } else {
        pobj->status = PARTIAL_INSUFFICIENT_BUFFER_SIZE;
    }
    
    return pobj->status;
}

// for now the return portion is required
// TODO: now that partial_status is held in the object, this should return the buffer size required and the buffer allocation check should move to the Partial_init function
static size_t Partial_parse_format(Partial * pobj, char * format) {
    char *cp = format;
    unsigned int index = 0;
    size_t buf_loc = 0;
    bool done = false;
    bool alias_found = false;
    bool buffered_format = false;
    while (!done && *cp != FORMAT_SENTINEL) {
        switch (*cp) {
            case FORMAT_TYPE_START: {
                cp++;
                Format_skip_whitespace(&cp);
                char * start = cp;
                pobj->status = Partial_set_type(pobj, index, &buf_loc, start, Format_get_type_code_end(&cp));
                if (pobj->status != PARTIAL_SUCCESS) {
                    return pobj->status;
                }
                index++;
#ifdef DEVELOPMENT
                printf("%zu\n", buf_loc);
#endif
                break;
            }
            case FORMAT_SENTINEL: {
                if (!pobj->narg) {
                    pobj->status = PARTIAL_BAD_FORMAT;
                    return pobj->status;
                }
                done = true;
                break;
            }
            case FORMAT_KEYWORD_START: {
#ifdef DEVELOPMENT
                printf("found keyword alias\n");
#endif
                // add keyword alias
                if (index == 1) { // FORMAT_KEYWORD_START found after return argument. invalid format
                    return PARTIAL_BAD_FORMAT;
                }
                cp++;
                Format_skip_whitespace(&cp);
                char * start = (char*)pobj->buffer + (cp - format);
                char * end = (char*)pobj->buffer + (Format_get_keyword_end(&cp) - format);
                if (end == start) { // just a default, no keyword
                    break;
                }
                if (!alias_found) {
                    KeywordMap_init(&pobj->map, PARTIAL_ALIAS_MAP_SIZE);
                    alias_found = true;
                    if (!buffered_format) {
                        pobj->status = Partial_buffer_format(pobj, index, &buf_loc, format);
                        if (pobj->status == PARTIAL_SUCCESS) {
                            buffered_format = true;
                        } else {
                            return pobj->status;
                        }
                    }
                }
                *end = '\0'; // do I even need end?
                pobj->status = Partial_set_alias(pobj, index - 1, start);
                if (pobj->status != PARTIAL_SUCCESS) {
                    return pobj->status;
                }
                
                break;
            }
            case FORMAT_DEFAULT_START: { // also covers RETURN_SENTINEL
                cp++;
                if (index != 1) { // index == 1 mean sentinel for return type, which cannot have a default value. maybe do not need found_return variable    
                    Format_skip_whitespace(&cp);
                    char * start = cp;
                    // if default is c-str, use pointer to place in format after it is buffered
                    if (pobj->args[index-1].type->id == PARTIAL_CSTRING) {
                        if (!buffered_format) {
                            pobj->status = Partial_buffer_format(pobj, index, &buf_loc, format);
                            if (pobj->status == PARTIAL_SUCCESS) {
                                buffered_format = true;
                            } else {
                                return pobj->status;
                            }
                        }
                        // TODO: need to set default from format string in buffer...not from format string in input
                        start = (char *)pobj->buffer + (start - format);
                        char * end = (char *)pobj->buffer + (Format_get_default_end(&cp) - format);
                        *end = '\0'; // change the string in the buffer. Do I even need the end value?
                        pobj->status = Partial_set_default(pobj, index - 1, start, end);
                        
                    } else {
                        pobj->status = Partial_set_default(pobj, index - 1, start, Format_get_default_end(&cp));
                    }
                    if (pobj->status != PARTIAL_SUCCESS) {
                        return pobj->status;
                    }
                }
                break;
            }
            case FORMAT_KEYWORD_SENTINEL: { // also covers FORMAT_DEFAULT_SENTINEL
                cp++;
                break;
            }
            default: { // only other acceptable part of format is whitespace
                if (!Format_is_whitespace(*cp)) {
                    pobj->status = PARTIAL_BAD_FORMAT;
                    return pobj->status;
                }
                Format_skip_whitespace(&cp);
                break;
            }
        }
    }



    return buf_loc;
}


// TODO: check that pobj-narg only counts input arguments
partial_status Partial_init(Partial * pobj, partial_abi abi, FUNC_PROTOTYPE(func), char * format, unsigned char * buffer, size_t buffer_size, unsigned int flags) {
    if (!pobj) {
        return PARTIAL_VALUE_ERROR;
    }
    pobj->status = PARTIAL_SUCCESS;
    if (!partial_initialized) {
        Partial_global_init();
        if (!partial_initialized) {
            pobj->status = PARTIAL_INIT_FAILED;
            return pobj->status;
        }
    }
    if (!format || !func || !(buffer || (flags & ALLOCED_BUFFER_FLAG))) {
        pobj->status = PARTIAL_VALUE_ERROR;
        return pobj->status;
    }

    pobj->narg = 0;
    pobj->argset = 0;
    pobj->func = func;
    if (abi < FFI_FIRST_ABI || abi > FFI_DEFAULT_ABI) {
        abi = FFI_DEFAULT_ABI;
    }
    pobj->ABI = abi;

    pobj->flags = flags;
    if (!(flags & ALLOCED_BUFFER_FLAG)) {
        pobj->buffer = buffer;
        pobj->buffer_size = buffer_size;
        for (size_t i = 0; i < pobj->buffer_size; i++) {
            pobj->buffer[i] = '\0';
        }
    } else {
        pobj->buffer = NULL;
        pobj->buffer_size = 0;
    }

    size_t buf_loc = Partial_parse_format(pobj, format);

    // TODO: fix the logic here. some of the if-then checks are redundant since malloced are set to NULL
    if (pobj->flags & ALLOCED_BUFFER_FLAG) {
        if (pobj->buffer) {
            if (pobj->buffer_size < buf_loc) {
                unsigned char * new_buffer = (unsigned char *) realloc(pobj->buffer, buf_loc);
                if (!new_buffer) {
                    pobj->status = PARTIAL_MALLOC_FAILURE;
                    return pobj->status;
                }
                for (size_t i = pobj->buffer_size; i < buf_loc; i++) {
                    new_buffer[i] = '\0';
                }
                pobj->buffer = new_buffer;
                pobj->buffer_size = buf_loc;
            }
        } else {
            pobj->buffer = (unsigned char *) malloc(buf_loc);
            if (!pobj->buffer) {
                pobj->flags &= ~ALLOCED_BUFFER_FLAG; // do not free a buffer that has not been malloced. turn off the flag
                pobj->status = PARTIAL_MALLOC_FAILURE;
                return pobj->status;
            }
            for (size_t i = 0; i < buf_loc; i++) {
                pobj->buffer[i] = '\0';
            }
            pobj->buffer_size = buf_loc;
        }
    } else {
        if (pobj->buffer_size < buf_loc) {
            pobj->status = PARTIAL_INSUFFICIENT_BUFFER_SIZE;
            return pobj->status;
        }
    }

    return pobj->status;
}

static partial_status Partial_copy_pair(Partial * pobj, unsigned int arg_index, VA_LIST * args) {
    arg_index += PARTIAL_ARG_START_INDEX;
#ifdef DEVELOPMENT
    printf("copying to buffer values: ");
#endif
    switch (pobj->args[arg_index].type->id) {
        case PARTIAL_VOID: { // void, do nothing
            return pobj->status;
            break;
        }
        case PARTIAL_BOOL: {
            bool b = (bool) va_arg(args->args, int);
#ifdef DEVELOPMENT
            printf("%s, ", b ? "true" : "false");
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &b, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_CHAR: {
            char c = (char) va_arg(args->args, int);
#ifdef DEVELOPMENT
            printf("%c, ", c);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &c, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_UCHAR: {
            unsigned char cu = (unsigned char) va_arg(args->args, unsigned int);
#ifdef DEVELOPMENT
            printf("%c, ", cu);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &cu, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_SHORT: {
            short hd = (short) va_arg(args->args, int);
#ifdef DEVELOPMENT
            printf("%hd, ", hd);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &hd, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_USHORT: {
            unsigned short hu = (unsigned short) va_arg(args->args, unsigned int);
#ifdef DEVELOPMENT
            printf("%hu, ", hu);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &hu, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_INT: {
            int d = va_arg(args->args, int);
#ifdef DEVELOPMENT
            printf("%d, ", d);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &d, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_UINT: {
            unsigned int u = va_arg(args->args, unsigned int);
#ifdef DEVELOPMENT
            printf("%u, ", u);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &u, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_LONG: {
            long ld = va_arg(args->args, long);
#ifdef DEVELOPMENT
            printf("%ld, ", ld);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &ld, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_ULONG: {
            unsigned long lu = va_arg(args->args, unsigned long);
#ifdef DEVELOPMENT
            printf("%lu, ", lu);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &lu, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_LLONG: {
            long long lld = va_arg(args->args, long long);
#ifdef DEVELOPMENT
            printf("%lld, ", lld);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &lld, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_ULLONG: {
            unsigned long long llu = va_arg(args->args, unsigned long long);
#ifdef DEVELOPMENT
            printf("%llu, ", llu);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &llu, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_SIZE_T: {
            size_t zu = va_arg(args->args, size_t);
#ifdef DEVELOPMENT
            printf("%zu, ", zu);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &zu, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_FLOAT: {
            float f = (float) va_arg(args->args, double);
#ifdef DEVELOPMENT
            printf("%f, ", f);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &f, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_DOUBLE: {
            double lf = va_arg(args->args, double);
#ifdef DEVELOPMENT
            printf("%lf, ", lf);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &lf, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_LDOUBLE: {
            long double llf = va_arg(args->args, long double);
#ifdef DEVELOPMENT
            printf("%LF, ", llf);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &llf, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_PVOID: {
            void * p = va_arg(args->args, void *);
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &p, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_VOIDPFUNC: {
            voidpfunc vf = va_arg(args->args, voidpfunc);
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &vf, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_PVOIDPFUNC: {
            pvoidpfunc pf = va_arg(args->args, pvoidpfunc);
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &pf, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_CSTRING: {
            char * pc = va_arg(args->args, char *);
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &pc, pobj->args[arg_index].type->size);
            break;
        }
        default: {
            pobj->status = PARTIAL_UNSUPPORTED_TYPE;
            break;
        }
    }
#ifdef DEVELOPMENT
    printf("\n");
#endif
    return pobj->status;
}

/* 
semantics:
Partial_copy_pair copies the argument into place no matter what (we want this behavior from keyword arguments and default values)
Partial_fill_pair copies the argument into place, but only if it is not bound.
Partial_bind_pair copies the argument into place not matter what AND sets it as bound, i.e. cannot be filled in
*/

// index < pobj->narg must be satisfied by caller
static inline partial_status Partial_fill_pair(Partial * pobj, unsigned int index, VA_LIST * arg) {
    if (pobj->argset & (1 << index)) {
        pobj->status = PARTIAL_CANNOT_FILL_BOUND_ARG;
    } else {
        pobj->status = Partial_copy_pair(pobj, index, arg);
    }
    return pobj->status;
}

// index < pobj->narg must be satisfied by caller
static inline partial_status Partial_bind_pair(Partial * pobj, unsigned int index, VA_LIST * arg) {
    if ((pobj->status = Partial_copy_pair(pobj, index, arg)) == PARTIAL_SUCCESS) {
        pobj->argset |= (1 << index);
    }
    return pobj->status;
}

static partial_status vPartial___npairs(Partial * pobj, partial_status (*f_assign)(Partial *, unsigned int, VA_LIST *), unsigned int nargin, VA_LIST * pairs) {
    if (!pobj || !f_assign) {
        return PARTIAL_VALUE_ERROR;
    }
    unsigned int i = 0;
    nargin = nargin > pobj->narg ? pobj->narg : nargin;
    while (i < nargin && (pobj->status == PARTIAL_SUCCESS)) {
        unsigned int index = va_arg(pairs->args, unsigned int);
        if (index < pobj->narg) {
            pobj->status = f_assign(pobj, index, pairs); // want to exit on trying to fill a bound argument here
        } else {
            pobj->status = PARTIAL_KEY_ERROR;
        }
        i++;
    }
    return pobj->status;
}

partial_status Partial_bind_npairs(Partial * pobj, unsigned int nargin, ...) {
    if (!pobj) {
        return PARTIAL_VALUE_ERROR;
    }
    VA_LIST pairs;
    va_start(pairs.args, nargin);
    pobj->status = vPartial___npairs(pobj, Partial_bind_pair, nargin, &pairs);
    va_end(pairs.args);
    return pobj->status;
}

partial_status Partial_fill_npairs(Partial * pobj, unsigned int nargin, ...) {
    if (!pobj) {
        return PARTIAL_VALUE_ERROR;
    }
    VA_LIST pairs;
    va_start(pairs.args, nargin);
    pobj->status = vPartial___npairs(pobj, Partial_fill_pair, nargin, &pairs);
    va_end(pairs.args);
    return pobj->status;
}

static partial_status vPartial___nargs(Partial * pobj, partial_status (*f_assign)(Partial *, unsigned int, VA_LIST *), unsigned int nargin, VA_LIST * args) {
    if (!pobj || !f_assign) {
        return PARTIAL_VALUE_ERROR;
    }
    unsigned int i = 0, j = 0;
    nargin = nargin > pobj->narg ? pobj->narg : nargin;
    while (i < nargin && j < pobj->narg && (pobj->status == PARTIAL_SUCCESS)) {
        while (j < pobj->narg && f_assign(pobj, j, args) == PARTIAL_CANNOT_FILL_BOUND_ARG) { // here we want to ignore this "error" as a warning
            j++;
        }
        j++;
        i++;
    }
    if (pobj->status == PARTIAL_CANNOT_FILL_BOUND_ARG) {
        pobj->status = PARTIAL_SUCCESS;
    }
    return pobj->status;
}

partial_status Partial_bind_nargs(Partial * pobj, unsigned int nargin, ...) {
    if (!pobj) {
        return PARTIAL_VALUE_ERROR;
    }
    VA_LIST args;
    va_start(args.args, nargin);
    pobj->status = vPartial___nargs(pobj, Partial_bind_pair, nargin, &args);
    va_end(args.args);
    return pobj->status;
}

partial_status Partial_fill_nargs(Partial * pobj, unsigned int nargin, ...) {
    if (!pobj) {
        return PARTIAL_VALUE_ERROR;
    }
    VA_LIST args;
    va_start(args.args, nargin);
    pobj->status = vPartial___nargs(pobj, Partial_fill_pair, nargin, &args);
    va_end(args.args);
    return pobj->status;
}

static partial_status vPartial___nkwargs(Partial * pobj, partial_status (*f_assign)(Partial *, unsigned int, VA_LIST *), unsigned int nkwargin, VA_LIST * kwargs) {
    if (!pobj || !f_assign) {
        return PARTIAL_VALUE_ERROR;
    }
    unsigned int i = 0, j = 0;
    nkwargin = nkwargin > pobj->narg ? pobj->narg : nkwargin;
    while (i < nkwargin && (pobj->status == PARTIAL_SUCCESS)) {
        j = KeywordMap_get(&pobj->map, va_arg(kwargs->args, char *));
        if (j < pobj->narg) {
            pobj->status = f_assign(pobj, j, kwargs);
        } else {
            pobj->status = PARTIAL_KEY_ERROR;
        }
        i++;
    }
    return pobj->status;
}

partial_status Partial_bind_nkwargs(Partial * pobj, unsigned int nkwargin, ...) {
    VA_LIST kwargs;
    va_start(kwargs.args, nkwargin);
    pobj->status = vPartial___nkwargs(pobj, Partial_bind_pair, nkwargin, &kwargs);
    va_end(kwargs.args);
    return pobj->status;
}

partial_status Partial_fill_nkwargs(Partial * pobj, unsigned int nkwargin, ...) {
    VA_LIST kwargs;
    va_start(kwargs.args, nkwargin);
    pobj->status = vPartial___nkwargs(pobj, Partial_fill_pair, nkwargin, &kwargs);
    va_end(kwargs.args);
    return pobj->status;
}

// since ffi_abi always starts at 0, <0 will indicate to use default. user only needs to go below FFI_FIRST_ABI
Partial * Partial_new(partial_abi abi, FUNC_PROTOTYPE(func), char * format, unsigned int nargin, unsigned int nkwargin, ...) {
    Partial * pobj = (Partial *) malloc(sizeof(Partial));
    if (!pobj) {
        return NULL;
    }
    
    pobj->status = Partial_init(pobj, abi, func, format, NULL, 0, PARTIAL_DEFAULT_FLAGS);

    VA_LIST args;
    va_start(args.args, nkwargin);
    if (pobj->status == PARTIAL_SUCCESS && nargin) {
        pobj->status = vPartial___nargs(pobj, Partial_bind_pair, nargin, &args);
    }
    if (pobj->status == PARTIAL_SUCCESS && nkwargin) {
        pobj->status = vPartial___nkwargs(pobj, Partial_bind_pair, nkwargin, &args);
    }
    va_end(args.args);

    if (pobj->status != PARTIAL_SUCCESS) {
        Partial_del(pobj);
        pobj = NULL;
    }

    return pobj;
}

static partial_status vPartial_call(Partial * pobj, void * ret, unsigned int nargin, unsigned int nkwargin, VA_LIST * args) {
    if (!pobj) {
        return PARTIAL_VALUE_ERROR;
    } else if (!ret && (pobj->args[0].type->id != PARTIAL_VOID)) {
        return (pobj->status = PARTIAL_VALUE_ERROR);
    }
    if (pobj->status == PARTIAL_SUCCESS && nargin) {
        pobj->status = vPartial___nargs(pobj, Partial_fill_pair, nargin, args);
    }
    if (pobj->status == PARTIAL_SUCCESS && nkwargin) {
        pobj->status = vPartial___nkwargs(pobj, Partial_fill_pair, nkwargin, args);
    }

    unsigned int i = 0;
    ffi_type * ret_type = pobj->args[0].type->ftp;
    ffi_type * arg_types[PARTIAL_MAX_NARG];
    void * arg_values[PARTIAL_MAX_NARG];
    while (i < pobj->narg) {
        arg_values[i] = (void*)(pobj->buffer + pobj->args[i+PARTIAL_ARG_START_INDEX].buf_loc);
        arg_types[i] = pobj->args[i+PARTIAL_ARG_START_INDEX].type->ftp; // since i++ occurs before...DO NOT include PARTIAL_ARG_START_INDEX
        i++;
    }

#ifdef DEVELOPMENT
    printf("finished copying values...\n");
#endif

    ffi_cif cif;
    pobj->status = ffi_prep_cif(&cif, pobj->ABI, pobj->narg, ret_type, arg_types);
#ifdef DEVELOPMENT
    printf("prepped cif\n");
#endif
    if (pobj->status != PARTIAL_SUCCESS) {
#ifdef DEVELOPMENT
        printf("ffi_prep_cif not ok: %d\n", pobj->status);
#endif
        return pobj->status;
    }
    
    ffi_call(&cif, pobj->func, ret, arg_values);
    return pobj->status;
}

partial_status Partial_call(Partial * pobj, void * ret, unsigned int nargin, unsigned int nkwargin, ...) {
    if (!pobj) {
        return PARTIAL_VALUE_ERROR;
    }
    VA_LIST args;
    va_start(args.args, nkwargin);
    pobj->status = vPartial_call(pobj, ret, nargin, nkwargin, &args);
    va_end(args.args);
    return pobj->status;
}

void Partial_del(Partial * pobj) {
    if (pobj->flags & ALLOCED_BUFFER_FLAG) {
        free(pobj->buffer);
    }
    pobj->buffer = NULL;
    if (pobj->flags & ALLOCED_PARTIAL_FLAG) {
        free(pobj);
    }
}