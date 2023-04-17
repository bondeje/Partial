#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "partial.h"

#define PARTIAL_VOID_S "v"
#define PARTIAL_BOOL_S "b"
#define PARTIAL_CHAR_S "c"
#define PARTIAL_UCHAR_S "cu"
#define PARTIAL_SHORT_S "hd"
#define PARTIAL_USHORT_S "hu"
#define PARTIAL_INT_S "d"
#define PARTIAL_UINT_S "u"
#define PARTIAL_LONG_S "ld"
#define PARTIAL_ULONG_S "lu"
#define PARTIAL_FLT_S "f"
#define PARTIAL_DBL_S "lf"
#define PARTIAL_LLONG_S "lld"
#define PARTIAL_ULLONG_S "llu"
#define PARTIAL_SIZE_T_S "zu"
#define PARTIAL_PVOID_S "p"
#define PARTIAL_VOIDPVOID_S "pv"
#define PARTIAL_PVOIDPVOID_S "pp"

#define MAX_TYPE_SIZE 4

#define TYPE_DELIMITER '%'
#define VALID_STR_TOKENS "bcuhdlfzpv"

// because promoted buffer sizes are different
// https://stackoverflow.com/questions/1255775/default-argument-promotions-in-c-function-calls
static unsigned int PARTIAL_BUFFER_SIZES[] = {   0, /* void */
                                       sizeof(int), /* bool, promoted */
                                       sizeof(int), /* char, promoted */
                              sizeof(unsigned int), /* unsigned char, promoted */
                                       sizeof(int), /* short, promoted */
                              sizeof(unsigned int), /* unsigned short, promoted */
                                       sizeof(int), /* int */
                              sizeof(unsigned int), /* unsigned int */
                                  sizeof(long int), /* long int */
                         sizeof(unsigned long int), /* unsigned long int */
                                    sizeof(double), /* float */
                                    sizeof(double), /* double */
                             sizeof(long long int), /* long long int */
                    sizeof(unsigned long long int), /* unsigned long long int */
                                     sizeof(void*), /* void* */
                                sizeof(void (*)()), /* function pointer to void */
                               sizeof(void* (*)())}; /* function pointer to void pointer */

// need to have platform specific handling for when there are different cases
enum partial_status Partial_copy_buffer(Partial * pobj, size_t index, size_t size, va_list *args_p) {
    switch (size) {
        case sizeof(int): {
            struct int_buf {
                unsigned char x[sizeof(int)];
            } buf = va_arg(*args_p, struct int_buf);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        /*
        case sizeof(long int): {
            struct ll_buf {
                unsigned char x[sizeof(int)];
            } buf = va_arg(*args_p, struct ll_buf);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        */
        case sizeof(double): {
            struct dbl_buf {
                unsigned char x[sizeof(int)];
            } buf = va_arg(*args_p, struct dbl_buf);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        /*
        case sizeof(long long int): {
            struct ll_buf {
                unsigned char x[sizeof(int)];
            } buf = va_arg(*args_p, struct ll_buf);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case sizeof(void*): {
            struct pvoid_buf {
                unsigned char x[sizeof(int)];
            } buf = va_arg(*args_p, struct pvoid_buf);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case sizeof(void(*)()): {
            struct dbl_buf {
                unsigned char x[sizeof(int)];
            } buf = va_arg(*args_p, struct dbl_buf);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case sizeof(void*(*)()): {
            struct dbl_buf {
                unsigned char x[sizeof(int)];
            } buf = va_arg(*args_p, struct dbl_buf);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        */
    }
}

static enum partial_status Partial_get_type_size(char * specifier, unsigned int * size, unsigned int * type) {
    if (!strcmp(specifier, PARTIAL_BOOL_S)) {
        *type = PARTIAL_BOOL;
    } else if (!strcmp(specifier, PARTIAL_CHAR_S)) {
        *type = PARTIAL_CHAR;
    } else if (!strcmp(specifier, PARTIAL_UCHAR_S)) {
        *type = PARTIAL_UCHAR;
    } else if (!strcmp(specifier, PARTIAL_SHORT_S)) {
        *type = PARTIAL_SHORT;
    } else if (!strcmp(specifier, PARTIAL_USHORT_S)) {
        *type = PARTIAL_USHORT;
    } else if (!strcmp(specifier, PARTIAL_INT_S)) {
        *type = PARTIAL_INT;
    } else if (!strcmp(specifier, PARTIAL_UINT_S)) {
        *type = PARTIAL_UINT;
    } else if (!strcmp(specifier, PARTIAL_LONG_S)) {
        *type = PARTIAL_LONG;
    } else if (!strcmp(specifier, PARTIAL_ULONG_S)) {
        *type = PARTIAL_ULONG;
    } else if (!strcmp(specifier, PARTIAL_FLT_S)) {
        *type = PARTIAL_FLT;
    } else if (!strcmp(specifier, PARTIAL_DBL_S)) {
        *type = PARTIAL_DBL;
    } else if (!strcmp(specifier, PARTIAL_LLONG_S)) {
        *type = PARTIAL_LLONG;
    } else if (!strcmp(specifier, PARTIAL_ULLONG_S)) {
        *type = PARTIAL_ULLONG;
    } else if (!strcmp(specifier, PARTIAL_SIZE_T_S)) {
        *type = PARTIAL_SIZE_T;
    } else if (!strcmp(specifier, PARTIAL_PVOID_S)) {
        *type = PARTIAL_PVOID;
    } else if (!strcmp(specifier, PARTIAL_VOIDPVOID_S)) {
        *type = PARTIAL_VOIDPVOID;
    } else if (!strcmp(specifier, PARTIAL_PVOIDPVOID_S)) {
        *type = PARTIAL_PVOIDPVOID;
    } else {
        return PARTIAL_VALUE_ERROR;
    }
    *size = PARTIAL_BUFFER_SIZES[*type];
    return PARTIAL_SUCCESS;
}

static unsigned short Partial_parse_format(char * format, unsigned int * byte_loc, unsigned int * type) {
    char buf[MAX_TYPE_SIZE] = {'\0'};
    *byte_loc = 0;
    byte_loc++;
    unsigned short cur = 0;
    bool found_percent = false;
    unsigned short narg = 0;
    while (*format != '\0' && narg < MAX_PARTIAL_NARG) {
        if (found_percent) {
            if (strchr(VALID_STR_TOKENS, *format)) {
                buf[cur++] = *format;
            } else {
                buf[cur++] = '\0';
                if (cur > 1) { // a specifier was found...
                    unsigned int size = 0;
                    if (!Partial_get_type_size(buf, &size, type)) {
                        narg++;
                        *byte_loc = *(byte_loc-1) + size;
                    } else {
                        return PARTIAL_VALUE_ERROR; // malformed format string
                    }
                }
                cur = 0;
                found_percent = false;
            }
        } else {
            if (*format == TYPE_DELIMITER) {
                found_percent = true;
                cur = 0;
            }
        }
        format++;
    }
    return narg;
}

void Partial_init(Partial * pobj, void* (*func)(), char * format, unsigned char * call_stack_buffer, size_t buffer_size, unsigned int * types, unsigned short nparam, unsigned int flags) {
    if (!pobj) {
        return;
    }
    if (format) {
        if (!(pobj->narg = Partial_parse_format(format, pobj->byte_loc, pobj->type))) {
            return;
        }
    } else {
        if (!nparam || nparam > MAX_PARTIAL_NARG) {
            pobj->narg = 0;
            return;
        }
        for (unsigned int i = 0; i < nparam; i++) {
            pobj->type[i] = types[i];
            pobj->byte_loc[i+1] = pobj->byte_loc[i] + PARTIAL_BUFFER_SIZES[types[i]];
        }
        if (pobj->byte_loc[nparam] > buffer_size) {
            pobj->narg = 0;
            return;
        }

    }
    pobj->func = func;
    pobj->buffer = call_stack_buffer;
    pobj->buffer_size = buffer_size;
    for (int i = 0; i < buffer_size; i++) {
        pobj->buffer[i] = '\0';
    }
    
    // include check against buffer size here
    pobj->argset = 0;
    pobj->flags = 0;
}

// bind arguments to Partial object with pairs (unsigned int, value) when unsigned int == PARTIAL_SENTINEL, va_list terminates 
enum partial_status Partial_bind(Partial * pobj, ...) {
    va_list args;
    va_start(args, pobj);
    unsigned int index = va_arg(args, unsigned int);
    while (index < MAX_PARTIAL_NARG && index != PARTIAL_SENTINEL) {
        if (index < pobj->narg) {
            Partial_copy_buffer(pobj, index, pobj->byte_loc[index+1] - pobj->byte_loc[index], &args);
            pobj->argset |= (1 << index); // mark the argument as set
        } else {
            printf("Bad index in Partial_set, cannot copy buffer. narg: %u, index: %u\n", pobj->narg, index);
            return PARTIAL_VALUE_ERROR;
        }
        index = va_arg(args, unsigned int);
    }
    va_end(args);
    return PARTIAL_SUCCESS;
}

// fill stack buffer with remaining arguments in function call. va_list terminates when all arguments are occupied
enum partial_status Partial_fill(Partial * pobj, ...) {
    //printf("Partial_fill called\n");
    if (!pobj) {
        return PARTIAL_VALUE_ERROR;
    }
    va_list args;
    va_start(args, pobj);
    unsigned short arg_ct = 0;
    size_t flag = 1;

    printf("\n");
    while (arg_ct < pobj->narg) {
        if (!(flag & pobj->argset)) {
            Partial_copy_buffer(pobj, arg_ct, pobj->byte_loc[arg_ct+1] - pobj->byte_loc[arg_ct], &args);
        }
        arg_ct++;
        flag <<= 1;
    }
    va_end(args);
    return PARTIAL_SUCCESS;
}

float sum2(int a, int b) {
    return a + b;
}
int main() {
    Partial p;
    p.func = (void*(*)())sum2;
    float a = BUILD_CALL_S(float, &p, 2);
}
