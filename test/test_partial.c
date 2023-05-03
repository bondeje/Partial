#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include "partial.h"

char * my_string = "I am the very model of a modern major general";
void * my_pvoid; // set in main

#define all_objects_SIG "%v=%c%cu%hd%hu%d%u%ld%lu%lld%llu%zu%f%lf%LF%p%s"
void all_objects(char c, unsigned char cu, short hd, unsigned short hu, int d, unsigned int u, long ld, unsigned long lu, long long lld, unsigned long long llu, size_t zu, float flt, double dbl, long double ldbl, void * p, char * s) {
    printf("inputs:\nchar: %c(=%hhd)\nunsigned char: %hhu\nshort: %hd\nunsigned short: %hu\nint: %d\nunsigned int: %u\nlong: %ld\nunsigned long: %lu\nlong long: %lld\nunsigned long long: %llu\nsize_t: %zu\nfloat: %f\ndouble: %lf\nlong double: %Lf\npointer: %p\nstring: %s\n", c, c, cu, hd, hu, d, u, ld, lu, lld, llu, zu, flt, dbl, ldbl, p, s);

    printf("(%zu)char match: %s\n",sizeof(char), (c == CHAR_MIN ? "true" : "false"));
    printf("(%zu)unsigned char match: %s\n",sizeof(unsigned char), (cu == UCHAR_MAX ? "true" : "false"));
    printf("(%zu)short match: %s\n",sizeof(short), (hd == SHRT_MIN ? "true" : "false"));
    printf("(%zu)unsigned short match: %s\n",sizeof(unsigned short), (hu == USHRT_MAX ? "true" : "false"));
    printf("(%zu)int match: %s\n",sizeof(int), (d == INT_MIN ? "true" : "false"));
    printf("(%zu)unsigned int match: %s\n",sizeof(unsigned int), (u == UINT_MAX ? "true" : "false"));
    printf("(%zu)long match: %s\n",sizeof(long), (ld == LONG_MIN ? "true" : "false"));
    printf("(%zu)unsigned long match: %s\n",sizeof(unsigned long), (lu == ULONG_MAX ? "true" : "false"));
    printf("(%zu)long long match: %s\n",sizeof(long long), (lld == LLONG_MIN  ? "true" : "false"));
    printf("(%zu)unsigned long long match: %s\n",sizeof(unsigned long long), (llu == ULLONG_MAX ? "true" : "false"));
    printf("(%zu)size_t match: %s\n",sizeof(size_t), (zu == SIZE_MAX ? "true" : "false"));
    printf("(%zu)float match: %s\n",sizeof(float), (flt == FLT_MAX ? "true" : "false"));
    printf("(%zu)double match: %s\n",sizeof(double), (dbl == DBL_MAX ? "true" : "false"));
    printf("(%zu)long double match: %s\n",sizeof(long double), (ldbl == LDBL_MAX ? "true" : "false"));
    printf("(%zu)void* match: %s\n",sizeof(void*), (p == my_pvoid ? "true" : "false"));
    printf("(%zu)string match: %s\n",sizeof(char*), (!strcmp(my_string, s) ? "true" : "false"));
}

void test_all_objects(void) {
    my_pvoid = (void*)my_string;
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    Partial_init(&p, -1, FUNC_CAST(all_objects), all_objects_SIG, buffer, buffer_size, 0);
    Partial_bind_npairs(&p, 8, 0, CHAR_MIN, 2, SHRT_MIN, 4, INT_MIN, 6, LONG_MIN, 8, LLONG_MIN, 10, SIZE_MAX, 12, DBL_MAX, 14, my_pvoid, PARTIAL_SENTINEL);
    Partial_call(&p, NULL, 8, 0, UCHAR_MAX, USHRT_MAX, UINT_MAX, ULONG_MAX, ULLONG_MAX, FLT_MAX, LDBL_MAX, my_string);
}

int main() {
    test_all_objects();
    
    return 0;
}