#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include "partial.h"

char * my_string = "I am the very model of a modern major general";
void * my_pvoid = NULL;

#define SUM_OBJECT_SIZES (sizeof(char) + sizeof(unsigned char) + sizeof(char*) + sizeof(void*) + sizeof(short) + sizeof(unsigned short) + sizeof(int) + sizeof(unsigned int) + sizeof(long) + sizeof(unsigned long) + sizeof(long long) + sizeof(unsigned long long) + sizeof(size_t) + sizeof(double) + sizeof(double) + sizeof(long double))*2

#define TEST_OBJECT_TYPES char, unsigned char, char*, void*, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, size_t, double, double, long double
float test_object_types(char c, unsigned char cu, char * s, void * p, short hd, unsigned short hu, int d, unsigned int u, long l, unsigned long lu, long long ll, unsigned long long llu, size_t zu, float flt, double dbl, long double ldbl) {
    printf("inputs:\nchar: %c(=%hhd)\nunsigned char: %hhu\nstring: %s\npointer: %p\nshort: %hd\nunsigned short: %hu\nint: %d\nunsigned int: %u\nlong: %ld\nunsigned long: %lu\nlong long: %lld\nunsigned long long: %llu\nsize_t: %zu\nfloat: %f\ndouble: %lf\nlong double: %Lf\n", c, c, cu, s, p, hd, hu, d, u, l, lu, ll, llu, zu, flt, dbl, ldbl);
    return flt;
}

int main() {
    size_t buffer_size = SUM_OBJECT_SIZES;
    unsigned char object_type_buffer[SUM_OBJECT_SIZES] = {'\0'};
    Partial p1;
    float f = FLT_MAX;
    Partial(&p1, test_object_types, object_type_buffer, buffer_size, TEST_OBJECT_TYPES);
    Partial_bind(&p1, 0, CHAR_MIN, 2, my_string, 4, SHRT_MIN, 6, INT_MIN, 8, LONG_MIN, 10, LLONG_MIN, 12, SIZE_MAX, 14, DBL_MAX, PARTIAL_SENTINEL);
    Partial_fill(&p1, UCHAR_MAX, my_pvoid, USHRT_MAX, UINT_MAX, ULONG_MAX, ULLONG_MAX, f, LDBL_MAX, PARTIAL_SENTINEL);

    printf("\n\ninputs:\nchar: %c(=%hhd)\nunsigned char: %hhu\nstring: %s\npointer: %p\nshort: %hd\nunsigned short: %hu\nint: %d\nunsigned int: %u\nlong: %ld\nunsigned long: %lu\nlong long: %lld\nunsigned long long: %llu\nsize_t: %zu\nfloat: %f\ndouble: %lf\nlong double: %Lf\n", CHAR_MIN, CHAR_MIN, UCHAR_MAX, my_string, my_pvoid, SHRT_MIN, USHRT_MAX, INT_MIN, UINT_MAX, LONG_MIN, ULONG_MAX, LLONG_MIN, ULLONG_MAX, SIZE_MAX, FLT_MAX, DBL_MAX, LDBL_MAX);
    float fresult = Partial_call(float, &p1, TEST_OBJECT_TYPES);
    printf("float output: %f\n", fresult);

    printf("sizeof(double) = %zu, sizeof(long double) = %zu\n", sizeof(double), sizeof(long double));
    return 0;
}