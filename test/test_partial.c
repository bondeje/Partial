#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include "partial.h"

char * my_string = "I am the very model of a modern major general";
void * my_pvoid; // set in main

#define SUM_OBJECT_SIZES (sizeof(char) + sizeof(unsigned char) + sizeof(char*) + sizeof(void*) + sizeof(short) + sizeof(unsigned short) + sizeof(int) + sizeof(unsigned int) + sizeof(long) + sizeof(unsigned long) + sizeof(long long) + sizeof(unsigned long long) + sizeof(size_t) + sizeof(double) + sizeof(double) + sizeof(long double))*2

#define TEST_OBJECT_TYPES char, unsigned char, char*, void*, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, size_t, double, double, long double
float test_object_types(char c, unsigned char cu, char * s, void * p, short hd, unsigned short hu, int d, unsigned int u, long l, unsigned long lu, long long ll, unsigned long long llu, size_t zu, float flt, double dbl, long double ldbl) {
    printf("inputs:\nchar: %c(=%hhd)\nunsigned char: %hhu\nstring: %s\npointer: %p\nshort: %hd\nunsigned short: %hu\nint: %d\nunsigned int: %u\nlong: %ld\nunsigned long: %lu\nlong long: %lld\nunsigned long long: %llu\nsize_t: %zu\nfloat: %f\ndouble: %lf\nlong double: %Lf\n", c, c, cu, s, p, hd, hu, d, u, l, lu, ll, llu, zu, flt, dbl, ldbl);

    printf("(%zu)char match: %s\n",sizeof(char), (c == CHAR_MIN ? "true" : "false"));
    printf("(%zu)unsigned char match: %s\n",sizeof(unsigned char), (cu == UCHAR_MAX ? "true" : "false"));
    printf("(%zu)string match: %s\n",sizeof(char*), (!strcmp(my_string, s) ? "true" : "false"));
    printf("(%zu)void* match: %s\n",sizeof(void*), (p == my_pvoid ? "true" : "false"));
    printf("(%zu)short match: %s\n",sizeof(short), (hd == SHRT_MIN ? "true" : "false"));
    printf("(%zu)unsigned short match: %s\n",sizeof(unsigned short), (hu == USHRT_MAX ? "true" : "false"));
    printf("(%zu)int match: %s\n",sizeof(int), (d == INT_MIN ? "true" : "false"));
    printf("(%zu)unsigned int match: %s\n",sizeof(unsigned int), (u == UINT_MAX ? "true" : "false"));
    printf("(%zu)long match: %s\n",sizeof(long), (l == LONG_MIN ? "true" : "false"));
    printf("(%zu)unsigned long match: %s\n",sizeof(unsigned long), (lu == ULONG_MAX ? "true" : "false"));
    printf("(%zu)long long match: %s\n",sizeof(long long), (ll == LLONG_MIN  ? "true" : "false"));
    printf("(%zu)unsigned long long match: %s\n",sizeof(unsigned long long), (llu == ULLONG_MAX ? "true" : "false"));
    printf("(%zu)size_t match: %s\n",sizeof(size_t), (zu == SIZE_MAX ? "true" : "false"));
    printf("(%zu)float match: %s\n",sizeof(float), (flt == FLT_MAX ? "true" : "false"));
    printf("(%zu)double match: %s\n",sizeof(double), (dbl == DBL_MAX ? "true" : "false"));
    printf("(%zu)long double match: %s\n",sizeof(long double), (ldbl == LDBL_MAX ? "true" : "false"));
    return flt;
}

void my_func(char a, int b, long c, long long d, float e, double f) {
    printf("my_func receives values:\n\t%c\n\t%d\n\t%ld\n\t%lld\n\t%f\n\t%lf\n", a, b, c, d, e, f);
    return;
}

void test_init(void) {
    Partial p;
    char format[256] = "%v=%c{ my_char = c }%d{my_int=-3}%ld{my_long=-380710293}%lld{my_longlong=-23807098475445393}%f{my_float=-1.3e4}%lf{my_double=2.34e56}"; 
    unsigned char buffer[256];
    printf("result of Partial_init: %d\n", Partial_init(&p, -1, FUNC_CAST(my_func), format, buffer, 256, 0));
    char a = 'c';
    int b = -3;
    long c = -380710293;
    long long d = -23807098475445393;
    float e = -1.3e4;
    double f = 2.34e56;
    Partial_bind(&p, 2, c, PARTIAL_SENTINEL);
    printf("sending values:\n\t%c\n\t%d\n\t%ld\n\t%lld\n\t%f\n\t%lf\n", a, b, c, d, e, f);
    printf("result of Partial_call: %d\n", Partial_call(&p, NULL, a, b, d, e, f));
    
}

int main() {
    test_init();

    /*
    my_pvoid = (void*)my_string;
    size_t buffer_size = SUM_OBJECT_SIZES;
    unsigned char object_type_buffer[SUM_OBJECT_SIZES] = {'\0'};
    Partial p1;
    float f = FLT_MAX;
    Partial(&p1, test_object_types, object_type_buffer, buffer_size, TEST_OBJECT_TYPES);
    Partial_bind(&p1, 0, CHAR_MIN, 2, my_string, 4, SHRT_MIN, 6, INT_MIN, 8, LONG_MIN, 10, LLONG_MIN, 12, SIZE_MAX, 14, DBL_MAX, PARTIAL_SENTINEL);
    Partial_fill(&p1, UCHAR_MAX, my_pvoid, USHRT_MAX, UINT_MAX, ULONG_MAX, ULLONG_MAX, f, LDBL_MAX, PARTIAL_SENTINEL);

    float fresult = Partial_call(float, &p1, TEST_OBJECT_TYPES);
    printf("float output: %f\n", fresult);

    printf("sizeof(double) = %zu, sizeof(long double) = %zu\n", sizeof(double), sizeof(long double));
    */
    return 0;
}