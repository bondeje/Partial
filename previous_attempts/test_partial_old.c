#include <stdlib.h> // maybe not necessary
#include <stddef.h> // size_t
#include <stdarg.h> // va_list, etc.
#include <stdio.h>
#include <string.h> // memcpy

/*
TODO:
need to handle cases of buffer overflow
Need to test cases of very different data types

//rebuild this so that I use a string input like printf to specify function signature
"%{output_type}=%{type parameter 1}%{type parameter 2}" where all white space is ignored

basically have a mapping from acceptable parameter types in the string to casting functions for the pointers

*/

// should match the number of TYPE_LISTN
#ifndef MAX_PARTIAL_NARG
#define MAX_PARTIAL_NARG 16
#endif // MAX_PARTIAL_NARG

#define MIN_VA_ARG_SIZE sizeof(int)

#define CONCAT2(A,B) A##B
#define CONCAT(A,B) CONCAT2(A,B)
#define UNIQUE_VAR_NAME(BASENAME) CONCAT(BASENAME,__LINE__)

typedef struct {
    unsigned char x[3];
} PARTIAL_FLOAT;

#define GET_SIZE(type) sizeof(type)

#define  TYPE_LIST1(TYPE) sizeof(TYPE)
#define  TYPE_LIST2(TYPE, ...) sizeof(TYPE), TYPE_LIST1(__VA_ARGS__)
#define  TYPE_LIST3(TYPE, ...) sizeof(TYPE), TYPE_LIST2(__VA_ARGS__)
#define  TYPE_LIST4(TYPE, ...) sizeof(TYPE), TYPE_LIST3(__VA_ARGS__)
#define  TYPE_LIST5(TYPE, ...) sizeof(TYPE), TYPE_LIST4(__VA_ARGS__)
#define  TYPE_LIST6(TYPE, ...) sizeof(TYPE), TYPE_LIST5(__VA_ARGS__)
#define  TYPE_LIST7(TYPE, ...) sizeof(TYPE), TYPE_LIST6(__VA_ARGS__)
#define  TYPE_LIST8(TYPE, ...) sizeof(TYPE), TYPE_LIST7(__VA_ARGS__)
#define  TYPE_LIST9(TYPE, ...) sizeof(TYPE), TYPE_LIST8(__VA_ARGS__)
#define TYPE_LIST10(TYPE, ...) sizeof(TYPE), TYPE_LIST9(__VA_ARGS__)
#define TYPE_LIST11(TYPE, ...) sizeof(TYPE), TYPE_LIST10(__VA_ARGS__)
#define TYPE_LIST12(TYPE, ...) sizeof(TYPE), TYPE_LIST11(__VA_ARGS__)
#define TYPE_LIST13(TYPE, ...) sizeof(TYPE), TYPE_LIST12(__VA_ARGS__)
#define TYPE_LIST14(TYPE, ...) sizeof(TYPE), TYPE_LIST13(__VA_ARGS__)
#define TYPE_LIST15(TYPE, ...) sizeof(TYPE), TYPE_LIST14(__VA_ARGS__)
#define TYPE_LIST16(TYPE, ...) sizeof(TYPE), TYPE_LIST15(__VA_ARGS__)

#define GET_SIZE_LIST(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME
#define SIZE_LIST(...) GET_SIZE_LIST(__VA_ARGS__, TYPE_LIST16, TYPE_LIST15, TYPE_LIST14, TYPE_LIST13, \
                                                  TYPE_LIST12, TYPE_LIST11, TYPE_LIST10, TYPE_LIST9,  \
                                                  TYPE_LIST8, TYPE_LIST7, TYPE_LIST6, TYPE_LIST5,     \
                                                  TYPE_LIST4, TYPE_LIST3, TYPE_LIST2, TYPE_LIST1, UNUSED)(__VA_ARGS__)

#define MAKE_PARTIAL_ARG_LIST1(pobjp, type) *(type*)((pobjp)->buffer + (pobjp)->byte_loc[0])
#define MAKE_PARTIAL_ARG_LIST2(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST1(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[1])
#define MAKE_PARTIAL_ARG_LIST3(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST2(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[2])
#define MAKE_PARTIAL_ARG_LIST4(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST3(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[3])
#define MAKE_PARTIAL_ARG_LIST5(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST4(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[4])
#define MAKE_PARTIAL_ARG_LIST6(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST5(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[5])
#define MAKE_PARTIAL_ARG_LIST7(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST6(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[6])
#define MAKE_PARTIAL_ARG_LIST8(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST7(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[7])
#define MAKE_PARTIAL_ARG_LIST9(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST8(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[8])
#define MAKE_PARTIAL_ARG_LIST10(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST9(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[9])
#define MAKE_PARTIAL_ARG_LIST11(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST10(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[10])
#define MAKE_PARTIAL_ARG_LIST12(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST11(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[11])
#define MAKE_PARTIAL_ARG_LIST13(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST12(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[12])
#define MAKE_PARTIAL_ARG_LIST14(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST13(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[13])
#define MAKE_PARTIAL_ARG_LIST15(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST14(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[14])
#define MAKE_PARTIAL_ARG_LIST16(pobjp, type, ...) MAKE_PARTIAL_ARG_LIST15(pobjp, __VA_ARGS__), *(type*)((pobjp)->buffer + (pobjp)->byte_loc[15])

#define SIZE_BUFFER(n) \
typedef struct {\
    unsigned char x[n];\
} CONCAT(buffer_, n);

/*
should try to see if I can make an array of different buffer sizes and just select based on the array position
*/

SIZE_BUFFER(4)
SIZE_BUFFER(8)
SIZE_BUFFER(12)
SIZE_BUFFER(16)
SIZE_BUFFER(20)
SIZE_BUFFER(24)
SIZE_BUFFER(28)
SIZE_BUFFER(32)
SIZE_BUFFER(36)
SIZE_BUFFER(38)
SIZE_BUFFER(42)
SIZE_BUFFER(44)
SIZE_BUFFER(48)
SIZE_BUFFER(52)
SIZE_BUFFER(56)
SIZE_BUFFER(60)
SIZE_BUFFER(64)

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

#define partial(pobjp, func, buffer, buffer_size, ...) \
size_t UNIQUE_VAR_NAME(sizes)[NARG(__VA_ARGS__)+1] = {0, SIZE_LIST(__VA_ARGS__)};\
Partial_init(pobjp, (void * (*)())func, buffer, buffer_size, UNIQUE_VAR_NAME(sizes), NARG(__VA_ARGS__));

void * PARTIAL_VOID = NULL;

#define GET_ARG_LIST_MACRO(n) CONCAT(MAKE_PARTIAL_ARG_LIST, n)
#define Partial_call(type, pobjp, ...) ((type (*)())(pobjp)->func)(GET_ARG_LIST_MACRO(NARG(__VA_ARGS__))(pobjp, __VA_ARGS__))
/*
// for testing two
#define Partial_call(type, pobjp, ...) ((type (*)())((pobjp)->func))(MAKE_PARTIAL_ARG_LIST2(pobjp, __VA_ARGS__))
*/

typedef struct Partial {
    void* (*func)();
    size_t argset;
    size_t byte_loc[MAX_PARTIAL_NARG+1];
    size_t buf_size;
    unsigned int flags; // for tracking memory allocations
    unsigned short narg;
    unsigned char * buffer;
} Partial;

void Partial_init(Partial * pobj, void* (*func)(), unsigned char * call_stack_buffer, size_t buffer_size, size_t * sizes, unsigned short nparam) {
    if (!pobj) {
        return;
    } else if (nparam >= MAX_PARTIAL_NARG) {
        printf("too many arguments passes to initialization, max %d\n", MAX_PARTIAL_NARG);
        pobj->narg = 0;
        return;
    }
    pobj->func = func;
    pobj->buffer = call_stack_buffer;
    pobj->buf_size = buffer_size;
    for (int i = 0; i < buffer_size; i++) {
        pobj->buffer[i] = '\0';
    }
    pobj->narg = 0;
    size_t nBytes = 0;
    for (int i = 0; i <= nparam; i++) {
        size_t next_size = (sizes[i] && sizes[i] < MIN_VA_ARG_SIZE ? MIN_VA_ARG_SIZE : sizes[i]);
        if (nBytes + next_size > buffer_size) {
            printf("ran out of buffer space\n");
            return; // pobj->narg = 0 is an error indicator
        }
        nBytes += next_size;
        pobj->byte_loc[i] = nBytes;
    }
    // include check against buffer size here
    pobj->narg = nparam;
    pobj->argset = 0;
    pobj->flags = 0;
}

void Partial_copy_buffer(Partial * pobj, size_t index, size_t size, va_list *args_p) {
    switch (size) {
        case 4: {
            buffer_4 buf = va_arg(*args_p, buffer_4);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case 8: {
            buffer_8 buf = va_arg(*args_p, buffer_8);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case 12: {
            buffer_12 buf = va_arg(*args_p, buffer_12);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case 16: {
            buffer_16 buf = va_arg(*args_p, buffer_16);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case 20: {
            buffer_20 buf = va_arg(*args_p, buffer_20);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case 24: {
            buffer_24 buf = va_arg(*args_p, buffer_24);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case 28: {
            buffer_28 buf = va_arg(*args_p, buffer_28);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
        case 32: {
            buffer_32 buf = va_arg(*args_p, buffer_32);
            memcpy(pobj->buffer + pobj->byte_loc[index], &buf, size);
            break;
        }
    }
}

// set values in pairs (index of argument, value of argument)
Partial * Partial_set(Partial * pobj, int narg, ...) {
    va_list args;
    va_start(args, narg);
    for (int i = 0; i < narg; i++) {
        unsigned int index = va_arg(args, unsigned int);
        if (index < pobj->narg) {
            Partial_copy_buffer(pobj, index, pobj->byte_loc[index+1] - pobj->byte_loc[index], &args);
            pobj->argset |= (1 << index); // mark the argument as set
        } else {
            printf("Bad index in Partial_set, cannot copy buffer. narg: %u, index: %u\n", pobj->narg, index);
            return NULL;
        }
    }
    va_end(args);
    return pobj;
}

Partial * Partial_fill(Partial * pobj, ...) {
    //printf("Partial_fill called\n");
    if (!pobj) {
        printf("null Partial passed to Partial_fill\n");
        return NULL;
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
        arg_ct += 1;
        flag <<= 1;
    }
    va_end(args);
    return pobj;
}

int sum2(int a, int b) {
    printf("called with %d, %d\n", a, b);
    return a + b;
    //return 0;
}

int sum3(int a, int b, int c) {
    printf("called with %d, %d, %d\n", a, b, c);
    return a + b + c;
    //return 0;
}

double sum2d(double a, double b) {
    printf("called with %lf, %lf\n", a, b);
    return a + b;
    //return 0;
}

float sum2f(float a, float b) {
    printf("called with %f, %f\n", a, b);
    return a + b;
    //return 0;
}

long long sum2ll(long long a, long long b) {
    printf("called with %lld, %lld\n", a, b);
    return a + b;
    //return 0;
}

short sum2s(short a, short b) {
    printf("called with %hd, %hd\n", a, b);
    return a + b;
    //return 0;
}

int sum2c(char a, char b) {
    printf("called with %c, %c\n", a, b);
    return a + b;
    //return 0;
}

typedef struct {
    float f;
} float_struct;

void print_float(float_struct * fs) {
    printf("%f\n", fs->f);
}


int main() {
    unsigned char buf[128];
    
    Partial p;
    
    partial(&p, sum3, buf, 64, int, int, int);
    Partial_set(&p, 2, 1, 4, 2, 3);
    Partial_fill(&p, 2);
    int out = Partial_call(int, &p, int, int, int);
    printf("return value = %d\n", out);

    partial(&p, sum2, buf, 64, int, int);
    Partial_set(&p, 1, 0, 3);
    Partial_fill(&p, 2);
    out = Partial_call(int, &p, int, int);
    printf("return value = %d\n", out);

    Partial_fill(&p, 5);
    out = Partial_call(int, &p, int, int);
    printf("\nreturn value = %d\n", out);

    Partial_set(&p, 1, 1, 4);
    out = Partial_call(int, &p, int, int);
    printf("return value = %d\n", out);
    

    Partial p2;

    partial(&p2, sum2d, buf, 64, double, double);
    double d = 3.141592;
    /*
    printf("locations\n");
    for (int i = 0; i < 3; i++) {
        printf("%zu ", p2.byte_loc[i]);
    }
    printf("\n");
    printf("double buffer before\n");
    for (int i = 0; i < 64; i++) {
        printf("%x ", p2.buffer[i]);
    }
    printf("\n");
    */
    Partial_set(&p2, 1, 0, d);
    Partial_fill(&p2, 1.5898345);
    /*
    printf("double buffer after\n");
    for (int i = 0; i < 64; i++) {
        printf("%x ", p2.buffer[i]);
    }
    printf("\n");
    */
    double outd = Partial_call(double, &p2, double, double);
    printf("return value = %lf\n", outd);

    Partial p3;

    partial(&p3, sum2f, buf, 64, double, double);
    float f = 3.141592;
    /*
    printf("locations\n");
    for (int i = 0; i < 3; i++) {
        printf("%zu ", p2.byte_loc[i]);
    }
    printf("\n");
    printf("float buffer before\n");
    for (int i = 0; i < 64; i++) {
        printf("%x ", p3.buffer[i]);
    }
    printf("\n");
    */
    Partial_set(&p3, 1, 1, f);
    Partial_fill(&p3, 1.5898345);
    /*
    printf("float buffer after\n");
    for (int i = 0; i < 64; i++) {
        printf("%x ", p3.buffer[i]);
    }
    printf("\n");
    */
    float outf = Partial_call(float, &p3, double, double);
    printf("return value = %f\n", outf);

    Partial p4;
    partial(&p4, sum2c, buf, 64, char, char);
    char c = 'a';
    Partial_set(&p4, 1, 1, c);
    Partial_fill(&p4, 'b');
    int outc = Partial_call(int, &p4, char, char);
    printf("%d + %d = %d: return value = %d\n", 'a', 'b', 'a' + 'b', outc);

    Partial p5;
    partial(&p5, sum2ll, buf, 64, long long, long long);
    long long l = 1982723453019827;
    Partial_set(&p5, 1, 1, l);
    Partial_fill(&p5, 5);
    long long outll = Partial_call(long long, &p5, long long, long long);
    printf("return value = %lld\n",  outll);

    Partial p6;
    partial(&p6, sum2s, buf, 64, short, short);
    short s = 15;
    Partial_set(&p6, 1, 1, s);
    Partial_fill(&p6, 6);
    short outs = Partial_call(short, &p6, short, short);
    printf("return value = %hd\n",  outs);

    // test 
    float_struct fs = {1.453};
    void * fp = (void*)&fs;
    print_float(fp);

    return 0;
}