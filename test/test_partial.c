#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include "partial.h"

#define all_objects_SIG "%v=%c%cu%hd%hu%d%u%ld%lu%lld%llu%zu%f%lf%LF%p%s"
char * my_string = "I am the very model of a modern major general";
void * my_pvoid; // set in main
void all_objects(char c, unsigned char cu, short hd, unsigned short hu, int d, unsigned int u, long ld, unsigned long lu, long long lld, unsigned long long llu, size_t zu, float flt, double dbl, long double ldbl, void * p, char * s) {
    //printf("inputs:\nchar: %c(=%hhd)\nunsigned char: %hhu\nshort: %hd\nunsigned short: %hu\nint: %d\nunsigned int: %u\nlong: %ld\nunsigned long: %lu\nlong long: %lld\nunsigned long long: %llu\nsize_t: %zu\nfloat: %f\ndouble: %lf\nlong double: %Lf\npointer: %p\nstring: %s\n", c, c, cu, hd, hu, d, u, ld, lu, lld, llu, zu, flt, dbl, ldbl, p, s);

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
    printf("\ntesting all objects...\n");
    my_pvoid = (void*)my_string;
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    Partial_init(&p, -1, FUNC_CAST(all_objects), all_objects_SIG, buffer, buffer_size, 0);
    Partial_bind_npairs(&p, 8, 0, CHAR_MIN, 2, SHRT_MIN, 4, INT_MIN, 6, LONG_MIN, 8, LLONG_MIN, 10, SIZE_MAX, 12, DBL_MAX, 14, my_pvoid, PARTIAL_SENTINEL);
    Partial_call(&p, NULL, 8, 0, UCHAR_MAX, USHRT_MAX, UINT_MAX, ULONG_MAX, ULLONG_MAX, FLT_MAX, LDBL_MAX, my_string);
    printf("...done\n");
}

#define bool_func_SIG "%b=%b{my_bool=false}%b{other_bool=true}"
bool bool_func(bool my_bool, bool other_bool) {
    return my_bool || other_bool;
}

void test_bool_func(void) {
    printf("\ntesting bool func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    bool result;
    Partial_init(&p, -1, FUNC_CAST(bool_func), bool_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("bool_func: call without args: %s\n", result ? "true" : "false");
    Partial_call(&p, &result, 1, 0, true);
    printf("bool_func: call with 1 arg - true: %s\n", result ? "true" : "false");
    Partial_call(&p, &result, 2, 0, false, false);
    printf("bool_func: call with 2 args - false, false: %s\n", result ? "true" : "false");
    Partial_call(&p, &result, 0, 2, "my_bool", false, "other_bool", false);
    printf("bool_func: call with kwargs - my_bool = false, my_other_bool = false: %s\n", result ? "true" : "false");
    Partial_call(&p, &result, 0, 1, "my_other_bool", false);
    printf("bool_func: call with bad kwargs - my_other_bool = false: %s\n", result ? "true" : "false");
    printf("...done\n");
}

#define int_func_SIG "%d=%d{my_int=-2}"
int int_func(int my_int) {
    return my_int+1;
}

void test_int_func(void) {
    printf("\ntesting int func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    int result = INT_MIN;
    Partial_init(&p, -1, FUNC_CAST(int_func), int_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("int_func: call without args: %d\n", result);
    Partial_call(&p, &result, 1, 0, 0);
    printf("int_func: call with args - 0: %d\n", result);
    Partial_call(&p, &result, 0, 1, "my_int", 1);
    printf("int_func: call with kwargs - a = 0: %d\n", result);
    printf("...done\n");
}

#define pvoid_func_SIG "%p=%p{myp=NULL}"
char * pvoid_func(char * myp) {
    return myp;
}

void test_pvoid_func(void) {
    printf("\ntesting pvoid func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    char def[20] = "no_result";
    char kwarg_res[20] = "keyword success";
    char * result = &def[0];
    Partial_init(&p, -1, FUNC_CAST(pvoid_func), pvoid_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("pvoid_func: call without args: %s\n", result);
    Partial_call(&p, &result, 1, 0, my_string);
    printf("pvoid_func: call with args - my_string: %s\n", result);
    Partial_call(&p, &result, 0, 1, "myp", &kwarg_res[0]);
    printf("pvoid_func: call with kwargs - myp = kwarg_res: %s\n", result);
    printf("...done\n");
}

int main() {
    test_all_objects();
    test_bool_func();
    test_int_func();
    test_pvoid_func();
    
    return 0;
}