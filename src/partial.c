#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>
#ifdef DEVELOPMENT
    #include <stdio.h>
#endif
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

#define FORMAT_SENTINEL '\0'
#define FORMAT_TYPE_START '%'
#define FORMAT_KEYWORD_SENTINEL '}'
#define FORMAT_KEYWORD_START '{'
#define FORMAT_RETURN_SENTINEL '='
#define FORMAT_DEFAULT_START FORMAT_RETURN_SENTINEL
#define FORMAT_DEFAULT_SENTINEL FORMAT_KEYWORD_SENTINEL
#define WHITESPACE " \t\n\v\f\r"

#define PARTIAL_ARG_START_INDEX 1

struct PartialType {
    ffi_type * ftp;
    size_t size;
    char * fmt_code;
    char * name;
    unsigned short id;
};

typedef void (*voidpfunc)();
typedef void* (*pvoidpfunc)();

static bool partial_initialized = false;

#define PARTIAL_N_TYPES 19

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
        return PARTIAL_BAD_FORMAT;
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
            *buf_locp += pobj->args[index].type->size;
            if (index) {
                pobj->narg++;
            }
#ifdef DEVELOPMENT
    printf("type found: %s\n", pobj->args[index].type->name);
#endif
            return PARTIAL_SUCCESS;
        }
    }
    return PARTIAL_UNSUPPORTED_TYPE;
}

// TODO:
static partial_status Partial_set_alias(Partial * pobj, unsigned int index, char * start, char * end) {
    if (!start || !end) {
        return PARTIAL_BAD_FORMAT;
    }
#ifdef DEVELOPMENT
    printf("alias found: ");
    while (start != end) {
        printf("%c", *start);
        start++;
    }
    printf("\n");
    return PARTIAL_SUCCESS;
#endif
    return PARTIAL_BAD_FORMAT;
}

// TODO:
// if buffer is already present or allocated, handle buffer size errors or resizing if necessary
static partial_status Partial_set_default(Partial * pobj, unsigned int index, char * start, char * end) {
    if (!start || !end) {
        return PARTIAL_BAD_FORMAT;
    }
#ifdef DEVELOPMENT
    printf("default found: ");
    while (start != end) {
        printf("%c", *start);
        start++;
    }
    printf("\n");
    return PARTIAL_SUCCESS;
#endif
    return PARTIAL_TYPE_ERROR;
}

// for now the return portion is required
static partial_status Partial_parse_format(Partial * pobj, char * format) {
    char *cp = format;
    unsigned int index = 0;
    size_t buf_loc = 0;
    bool done = false;
    while (!done && *cp != FORMAT_SENTINEL) {
        switch (*cp) {
            case FORMAT_TYPE_START: {
                cp++;
                Format_skip_whitespace(&cp);
                char * start = cp;
                partial_status found_type = Partial_set_type(pobj, index, &buf_loc, start, Format_get_type_code_end(&cp));
                if (found_type != PARTIAL_SUCCESS) {
                    return found_type;
                }
                index++;
                printf("%zu\n", buf_loc);
                break;
            }
            case FORMAT_SENTINEL: {
                if (!pobj->narg) {
                    return PARTIAL_BAD_FORMAT;
                }
                done = true;
                break;
            }
            case FORMAT_KEYWORD_START: {
                // add keyword alias
                if (index == 1) { // FORMAT_KEYWORD_START found after return argument. invalid format
                    return PARTIAL_BAD_FORMAT;
                }
                cp++;
                Format_skip_whitespace(&cp);
                char * start = cp;
                partial_status found_keyword = Partial_set_alias(pobj, index - 1, start, Format_get_keyword_end(&cp));
                if (found_keyword != PARTIAL_SUCCESS) {
                    return found_keyword;
                }
                break;
            }
            case FORMAT_DEFAULT_START: { // also covers RETURN_SENTINEL
                cp++;
                if (index != 1) { // index == 1 mean sentinel for return type, which cannot have a default value. maybe do not need found_return variable    
                    Format_skip_whitespace(&cp);
                    char * start = cp;
                    partial_status found_default = Partial_set_default(pobj, index - 1, start, Format_get_default_end(&cp));
                    if (found_default != PARTIAL_SUCCESS) {
                        return found_default;
                    }
                }
                break;
            }
            case FORMAT_KEYWORD_SENTINEL: { // also covers FORMAT_DEFAULT_SENTINEL
                cp++;
                break;
            }
            default: {
                Format_skip_whitespace(&cp);

                break;
            }
        }
    }

    // todo, manipulate buffer to fit types from format
    if (pobj->flags & ALLOCED_BUFFER_FLAG) {
        if (pobj->buffer) {
            if (pobj->buffer_size < buf_loc) {
                unsigned char * new_buffer = (unsigned char *) realloc(pobj->buffer, buf_loc);
                if (!new_buffer) {
                    return PARTIAL_MALLOC_FAILURE;
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
                pobj->flags &= ~ALLOCED_BUFFER_FLAG; // do not free a buffer that has not been malloced
                return PARTIAL_MALLOC_FAILURE;
            }
            for (size_t i = 0; i < buf_loc; i++) {
                pobj->buffer[i] = '\0';
            }
            pobj->buffer_size = buf_loc;
        }
    } else {
        if (pobj->buffer_size < buf_loc) {
            return PARTIAL_INSUFFICIENT_BUFFER_SIZE;
        }
    }

    return PARTIAL_SUCCESS;
}


// TODO: check that pobj-narg only counts input arguments
partial_status Partial_init(Partial * pobj, FUNC_PROTOTYPE(func), char * format, unsigned char * buffer, size_t buffer_size, unsigned int flags) {
    if (!partial_initialized) {
        Partial_global_init();
        if (!partial_initialized) {
            return PARTIAL_INIT_FAILED;
        }
    }
    if (!pobj || !format || !func || !(buffer || (flags & ALLOCED_BUFFER_FLAG))) {
        return PARTIAL_VALUE_ERROR;
    }

    pobj->narg = 0;
    pobj->argset = 0;
    pobj->func = func;
    pobj->ABI = FFI_DEFAULT_ABI;

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

    return Partial_parse_format(pobj, format);
}

static partial_status Partial_copy_value(Partial * pobj, unsigned int arg_index, va_list * args) {
    arg_index += PARTIAL_ARG_START_INDEX;
#ifdef DEVELOPMENT
    printf("copying to buffer values: ");
#endif
    switch (pobj->args[arg_index].type->id) {
        case PARTIAL_VOID: { // void, do nothing
            return PARTIAL_SUCCESS;
            break;
        }
        case PARTIAL_BOOL: {
            bool b = (bool) va_arg(*args, int);
#ifdef DEVELOPMENT
            printf("%s, ", b ? "true" : "false");
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &b, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_CHAR: {
            char c = (char) va_arg(*args, int);
#ifdef DEVELOPMENT
            printf("%c, ", c);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &c, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_UCHAR: {
            unsigned char cu = (unsigned char) va_arg(*args, unsigned int);
#ifdef DEVELOPMENT
            printf("%c, ", cu);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &cu, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_SHORT: {
            short hd = (short) va_arg(*args, int);
#ifdef DEVELOPMENT
            printf("%hd, ", hd);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &hd, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_USHORT: {
            unsigned short hu = (unsigned short) va_arg(*args, unsigned int);
#ifdef DEVELOPMENT
            printf("%hu, ", hu);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &hu, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_INT: {
            int d = va_arg(*args, int);
#ifdef DEVELOPMENT
            printf("%d, ", d);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &d, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_UINT: {
            unsigned int u = va_arg(*args, unsigned int);
#ifdef DEVELOPMENT
            printf("%u, ", u);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &u, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_LONG: {
            long ld = va_arg(*args, long);
#ifdef DEVELOPMENT
            printf("%ld, ", ld);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &ld, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_ULONG: {
            unsigned long lu = va_arg(*args, unsigned long);
#ifdef DEVELOPMENT
            printf("%lu, ", lu);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &lu, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_LLONG: {
            long long lld = va_arg(*args, long long);
#ifdef DEVELOPMENT
            printf("%lld, ", lld);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &lld, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_ULLONG: {
            unsigned long long llu = va_arg(*args, unsigned long long);
#ifdef DEVELOPMENT
            printf("%llu, ", llu);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &llu, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_SIZE_T: {
            size_t zu = va_arg(*args, size_t);
#ifdef DEVELOPMENT
            printf("%zu, ", zu);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &zu, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_FLOAT: {
            float f = (float) va_arg(*args, double);
#ifdef DEVELOPMENT
            printf("%f, ", f);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &f, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_DOUBLE: {
            double lf = va_arg(*args, double);
#ifdef DEVELOPMENT
            printf("%lf, ", lf);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &lf, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_LDOUBLE: {
            long double llf = va_arg(*args, long double);
#ifdef DEVELOPMENT
            printf("%LF, ", llf);
#endif
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &llf, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_PVOID: {
            void * p = va_arg(*args, void *);
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &p, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_VOIDPFUNC: {
            voidpfunc vf = va_arg(*args, voidpfunc);
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &vf, pobj->args[arg_index].type->size);
            break;
        }
        case PARTIAL_PVOIDPFUNC: {
            pvoidpfunc pf = va_arg(*args, pvoidpfunc);
            memcpy(pobj->buffer + pobj->args[arg_index].buf_loc, &pf, pobj->args[arg_index].type->size);
            break;
        }
        default: {
            return PARTIAL_UNSUPPORTED_TYPE;
            break;
        }
    }
#ifdef DEVELOPMENT
    printf("\n");
#endif
    return PARTIAL_SUCCESS;
}

partial_status Partial_bind(Partial * pobj, ...) {
    va_list args;
    va_start(args, pobj);
    int index = va_arg(args, int);
    while (index != PARTIAL_SENTINEL && !Partial_copy_value(pobj, index, &args)) {
        pobj->argset |= (1 << index);
        index = va_arg(args, int);
    }
    va_end(args);
    if (index != PARTIAL_SENTINEL) {
        return PARTIAL_COPY_FAILURE;
    }
    return PARTIAL_SUCCESS;
}

partial_status Partial_bind_args(Partial * pobj, ...) { // bind null-terminated list of arguments in order
    return PARTIAL_NOT_YET_IMPLEMENTED;
} 
partial_status Partial_bind_kwargs(Partial * pobj, ...) {// bind null-terminated list of keyword arguments
    return PARTIAL_NOT_YET_IMPLEMENTED;
}


partial_status Partial_call(void * ret, Partial * pobj, ...) {
    va_list args;
    va_start(args, pobj);
    unsigned short i = 0;
    ffi_type *argtypes[PARTIAL_MAX_NARG+1];
    argtypes[0] = pobj->args[0].type->ftp;
    void * argvalues[PARTIAL_MAX_NARG];
    //printf("nargin: %u\n", pobj->narg);
    while (i < pobj->narg) {
        if (!(pobj->argset & (1 << i))) {
            if (Partial_copy_value(pobj, i, &args)) {
                return PARTIAL_COPY_FAILURE;
            }
        }
        argvalues[i] = (void*)(pobj->buffer + pobj->args[i+PARTIAL_ARG_START_INDEX].buf_loc);
        i++;
        argtypes[i] = pobj->args[i].type->ftp; // since i++ occurs before...DO NOT include PARTIAL_ARG_START_INDEX
    }

    //printf("finished copying values...\n");

    ffi_cif cif;
    ffi_status cfi_stat = ffi_prep_cif(&cif, pobj->ABI, pobj->narg, argtypes[0], &argtypes[1]);
    if (cfi_stat != FFI_OK) {
        //printf("ffi_prep_cif not ok: %d\n", cfi_stat);
        return cfi_stat;
    }

    ffi_call(&cif, pobj->func, ret, &argvalues[0]);
    return cfi_stat;
    
    //return PARTIAL_SUCCESS;
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