#include <stdlib.h>
#include <stdbool.h>
#include "partial.h"

#define MAX_TYPE_SIZE 1024

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   //define something for Windows (32-bit and 64-bit, this part is common)
   #define VA_ARG_REF_TYPE __int64
   #ifdef _WIN64
      #define my_va_arg(va_list_p, type) ((MAX_TYPE_SIZE > sizeof(VA_ARG_REF_TYPE) || (MAX_TYPE_SIZE & (MAX_TYPE_SIZE - 1)) != 0) ? \
                        **(struct PartialBuffer_MAX**)((*va_list_p += sizeof(VA_ARG_REF_TYPE)) - sizeof(VA_ARG_REF_TYPE)) : \
                        *(struct PartialBuffer_MAX* )((*va_list_p += sizeof(VA_ARG_REF_TYPE)) - sizeof(VA_ARG_REF_TYPE)))
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __linux__
    #include <stdint.h>
    #define VA_ARG_REF_TYPE int64_t
    #define my_va_arg(va_list_p, type) ((MAX_TYPE_SIZE > sizeof(VA_ARG_REF_TYPE) || (MAX_TYPE_SIZE & (MAX_TYPE_SIZE - 1)) != 0) ? \
                        **(struct PartialBuffer_MAX**)((va_list_p += sizeof(VA_ARG_REF_TYPE)) - sizeof(VA_ARG_REF_TYPE)) : \
                        *(struct PartialBuffer_MAX* )((va_list_p += sizeof(VA_ARG_REF_TYPE)) - sizeof(VA_ARG_REF_TYPE)))
#else
#   error "unhandled platform"
#endif

#define RESIZE_REALLOC(result, elem_type, obj, num)                                 \
{ /* encapsulate to ensure temp_obj can be reused */                                \
elem_type* temp_obj = (elem_type*) realloc(obj, sizeof(elem_type) * (num));      \
if (temp_obj) {                                                                     \
    obj = temp_obj;                                                                 \
    result = true;                                                                  \
} else {                                                                            \
    result = false;                                                                 \
}                                                                                   \
}

Partial * Partial_new_(void* (*func)(), unsigned int * sizes, unsigned short nparam, unsigned int flags) {
    if (!func || !sizes) {
        return NULL;
    }
    Partial * pobj = (Partial *) malloc(sizeof(Partial));
    if (!pobj) {
        return NULL;
    }
    size_t buffer_size = 0;
    for (unsigned short i = 0; i < nparam; i++) {
        buffer_size += (sizes[i] < MIN_VA_ARG_SIZE ? MIN_VA_ARG_SIZE : sizes[i]);
    }

    unsigned char * buffer = (unsigned char *) malloc(buffer_size);
    if (!buffer) {
        free(pobj);
        return NULL;
    }

    flags |= HEAP_BUFFER_FLAG;
    Partial_init(pobj, func, buffer, buffer_size, sizes, nparam, flags);
    return pobj;
}

void Partial_init(Partial * pobj, void* (*func)(), unsigned char * buffer, size_t buffer_size, unsigned int * sizes, unsigned short nparam, unsigned int flags) {
    //printf("calling Partial_init\n");
    if (!pobj) {
        return;
    } else if (nparam > MAX_PARTIAL_NARG) {
        //printf("too many arguments passes to initialization, max %d\n", MAX_PARTIAL_NARG);
        pobj->narg = 0;
        return;
    }
    pobj->narg = 0;
    pobj->argset = 0;
    pobj->func = func;
    pobj->buffer = buffer;
    pobj->buffer_size = buffer_size;
    for (int i = 0; i < buffer_size; i++) {
        pobj->buffer[i] = '\0';
    }
    pobj->byte_loc[0] = 0;
    for (unsigned short i = 0; i < nparam; i++) {
        // this next line when caching the object sizes accounts for all the default integer promotions. note that this does not fix the float->double promotion, which is why float does not work
        unsigned int size = (sizes[i] < MIN_VA_ARG_SIZE ? MIN_VA_ARG_SIZE : sizes[i]);
        if (size > MAX_TYPE_SIZE) {
            pobj->narg = 0;
            return;
        }
        pobj->byte_loc[i+1] = pobj->byte_loc[i] + size;
        //printf("size of index %hu: %u\n", i, pobj->byte_loc[i+1] - pobj->byte_loc[i]);
    }
    //printf("\tsizes consumed\n");
    for (unsigned int i = nparam + 1; i <= (int)MAX_PARTIAL_NARG; i++) {
        pobj->byte_loc[i] = pobj->byte_loc[i-1];
    }
    //printf("\tremainder of byte_locs filled in\n");
    // include check against buffer size here
    pobj->flags = flags;
    if (pobj->byte_loc[MAX_PARTIAL_NARG] > pobj->buffer_size) {
        if (!(pobj->flags & HEAP_BUFFER_FLAG)) {
            //printf("insufficient buffer size, setting error flag on Partial: have %zu, need %u\n", pobj->buffer_size, pobj->byte_loc[MAX_PARTIAL_NARG]);
            pobj->narg = 0;
            return;
        } else {
            bool result = true;
            RESIZE_REALLOC(result, unsigned char, pobj->buffer, pobj->byte_loc[MAX_PARTIAL_NARG]);
            if (result) {
                pobj->buffer_size = pobj->byte_loc[MAX_PARTIAL_NARG];
            } else {
                pobj->narg = 0;
                return;
            }
        }
    }
    pobj->narg = nparam;
}

// need to have platform specific handling for when there are different cases
static enum partial_status Partial_copy_buffer(Partial * pobj, size_t index, size_t size, va_list *args_p) {
    //printf("calling Partial_copy_buffer\n");
    if (!pobj) {
        return PARTIAL_VALUE_ERROR;
    }
    switch (size) { // special cases for common types for portability
        // consider case 3: to indicate a float
        case 4: {
            POLL_VA_ARG(buf, *args_p, 4);// look at the VS code hints to see how va_args expands
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case 8: {
            POLL_VA_ARG(buf, *args_p, 8);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case 12: {
            POLL_VA_ARG(buf, *args_p, 12);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case 16: {
            POLL_VA_ARG(buf, *args_p, 16);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        default: { // the non-portable or special struct types
            // this might have to get a macro distinguishing systems for porting

            // this works on Windows 10 with MinGW-64 running GCC 8.1.0
            struct PartialBuffer_MAX {
                unsigned char x[MAX_TYPE_SIZE];
            } buf = my_va_arg(args_p, PartialBUffer_MAX);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
    }
    //printf("Partial_copy_buffer success\n");
    return PARTIAL_SUCCESS;
}

enum partial_status Partial_bind(Partial * pobj, ...) {
    //printf("Calling Partial_bind\n");
    va_list args;
    va_start(args, pobj);
    unsigned int index = va_arg(args, unsigned int);
    while (index != PARTIAL_SENTINEL && index < pobj->narg) {
        //printf("\tbinding at index %u\n", index);
        Partial_copy_buffer(pobj, index, pobj->byte_loc[index+1] - pobj->byte_loc[index], &args);
        pobj->argset |= (1 << index); // mark the argument as set
        index = va_arg(args, unsigned int);
    }
    va_end(args);
    //printf("Partial_bind success\n");
    return PARTIAL_SUCCESS;
}

enum partial_status Partial_fill(Partial * pobj, ...) {
    //printf("Calling Partial_fill\n");
    if (!pobj) {
        return PARTIAL_VALUE_ERROR;
    }
    va_list args;
    va_start(args, pobj);
    unsigned short arg_ct = 0;
    size_t flag = 1;

    //printf("\n");
    while (arg_ct < pobj->narg) {
        if (!(flag & pobj->argset)) {
            Partial_copy_buffer(pobj, arg_ct, pobj->byte_loc[arg_ct+1] - pobj->byte_loc[arg_ct], &args);
        }
        arg_ct += 1;
        flag <<= 1;
    }
    va_end(args);
    //printf("Partial_fill success\n");
    return PARTIAL_SUCCESS;
}

void Partial_del(Partial * pobj) {
    free(pobj->buffer);
    pobj->buffer = NULL;
    free(pobj);
}