#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include "partial.h"

#define all_objects_SIG "%v=%c%hhu%hd%hu%d%u%ld%lu%lld%llu%zu%f%lf%LF%p%s"
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
    printf("\ntesting all_objects...\n");
    my_pvoid = (void*)my_string;
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    Partial_init(&p, -1, FUNC_CAST(all_objects), all_objects_SIG, buffer, buffer_size, 0);
    Partial_bind_npairs(&p, 8, 0, CHAR_MIN, 2, SHRT_MIN, 4, INT_MIN, 6, LONG_MIN, 8, LLONG_MIN, 10, SIZE_MAX, 12, DBL_MAX, 14, my_pvoid);
    Partial_call(&p, NULL, 8, 0, UCHAR_MAX, USHRT_MAX, UINT_MAX, ULONG_MAX, ULLONG_MAX, FLT_MAX, LDBL_MAX, my_string);
    printf("...done\n");
}

#define bool_func_SIG "%b=%b{my_bool=false}%b{other_bool=true}"
bool bool_func(bool my_bool, bool other_bool) {
    return my_bool || other_bool;
}

void test_bool_func(void) {
    printf("\ntesting bool_func...\n");
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

#define short_func_SIG "%hd=%hd{my_short=-2}"
short short_func(short my_short) {
    return my_short+1;
}

void test_short_func(void) {
    printf("\ntesting short_func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    short result = SHRT_MIN;
    Partial_init(&p, -1, FUNC_CAST(short_func), short_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("short_func: call without args: %hd\n", result);
    Partial_call(&p, &result, 1, 0, 0);
    printf("short_func: call with args - 0: %hd\n", result);
    Partial_call(&p, &result, 0, 1, "my_short", 1);
    printf("short_func: call with kwargs - my_short = 1: %hd\n", result);
    printf("...done\n");
}

#define int_func_SIG "%d=%d{my_int=-2}"
int int_func(int my_int) {
    return my_int+1;
}

void test_int_func(void) {
    printf("\ntesting int_func...\n");
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
    printf("int_func: call with kwargs - my_int = 1: %d\n", result);
    printf("...done\n");
}

#define long_func_SIG "%ld=%ld{my_long=-2}"
long long_func(long my_long) {
    return my_long+1;
}

void test_long_func(void) {
    printf("\ntesting long_func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    long result = LONG_MIN;
    Partial_init(&p, -1, FUNC_CAST(long_func), long_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("long_func: call without args: %ld\n", result);
    Partial_call(&p, &result, 1, 0, 0);
    printf("long_func: call with args - 0: %ld\n", result);
    Partial_call(&p, &result, 0, 1, "my_long", 1);
    printf("long_func: call with kwargs - my_long = 1: %ld\n", result);
    printf("...done\n");
}

#define longlong_func_SIG "%lld=%lld{my_longlong=-2}"
long long longlong_func(long long my_longlong) {
    return my_longlong+1;
}

void test_longlong_func(void) {
    printf("\ntesting longlong_func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    long long result = LLONG_MIN;
    Partial_init(&p, -1, FUNC_CAST(longlong_func), longlong_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("longlong_func: call without args: %lld\n", result);
    Partial_call(&p, &result, 1, 0, 0LL);
    printf("longlong_func: call with args - 0: %lld\n", result);
    Partial_call(&p, &result, 0, 1, "my_longlong", 1LL);
    printf("longlong_func: call with kwargs - my_longlong = 1: %lld\n", result);
    printf("...done\n");
}

#define size_t_func_SIG "%zu=%zu{my_size_t=0}"
size_t size_t_func(size_t my_size_t) {
    return my_size_t+1;
}

void test_size_t_func(void) {
    printf("\ntesting size_t_func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    size_t result = SIZE_MAX;
    Partial_init(&p, -1, FUNC_CAST(size_t_func), size_t_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("size_t_func: call without args: %zu\n", result);
    Partial_call(&p, &result, 1, 0, (size_t)1);
    printf("size_t_func: call with args - 0: %zu\n", result);
    Partial_call(&p, &result, 0, 1, "my_size_t", (size_t)2);
    printf("size_t_func: call with kwargs - my_size_t = 2: %zu\n", result);
    printf("...done\n");
}

#define float_func_SIG "%f=%f{my_float=-1e4}"
float float_func(float my_float) {
    return my_float*1.1f;
}

void test_float_func(void) {
    printf("\ntesting float_func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    float result = FLT_MAX;
    Partial_init(&p, -1, FUNC_CAST(float_func), float_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("float_func: call without args: %f\n", result);
    Partial_call(&p, &result, 1, 0, 1.2e3);
    printf("float_func: call with args - 0: %f\n", result);
    Partial_call(&p, &result, 0, 1, "my_float", 4.8e5);
    printf("float_func: call with kwargs - my_float = 4.8e5: %f\n", result);
    printf("...done\n");
}

#define double_func_SIG "%lf=%lf{my_double=-1e4}"
double double_func(double my_double) {
    return my_double*1.1;
}

void test_double_func(void) {
    printf("\ntesting double_func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    double result = DBL_MAX;
    Partial_init(&p, -1, FUNC_CAST(double_func), double_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("double_func: call without args: %lf\n", result);
    Partial_call(&p, &result, 1, 0, 1.2e3);
    printf("double_func: call with args - 0: %lf\n", result);
    Partial_call(&p, &result, 0, 1, "my_double", 4.8e5);
    printf("double_func: call with kwargs - my_double = 4.8e5: %lf\n", result);
    printf("...done\n");
}

#define longdouble_func_SIG "%LF=%LF{my_longdouble=-1e4L}"
long double longdouble_func(long double my_longdouble) {
    return my_longdouble*1.1;
}

void test_longdouble_func(void) {
    printf("\ntesting longdouble_func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    long double result = DBL_MAX;
    Partial_init(&p, -1, FUNC_CAST(longdouble_func), longdouble_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("longdouble_func: call without args: %LF\n", result);
    Partial_call(&p, &result, 1, 0, 1.2e3L);
    printf("longdouble_func: call with args - 0: %LF\n", result);
    Partial_call(&p, &result, 0, 1, "my_longdouble", 4.8e5L);
    printf("longdouble_func: call with kwargs - my_longdouble = 4.8e5L: %LF\n", result);
    printf("...done\n");
}

#define pvoid_func_SIG "%p=%p{myp=NULL}"
char * pvoid_func(char * myp) {
    return myp;
}

void test_pvoid_func(void) {
    printf("\ntesting pvoid_func...\n");
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

#define cstr_func_SIG "%s=%s{myp=no_result}"
char * cstr_func(char * myp) {
    return myp;
}

void test_cstr_func(void) {
    printf("\ntesting cstr_func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    char def[20] = "no_result";
    char kwarg_res[20] = "keyword success";
    char * result = &def[0];
    Partial_init(&p, -1, FUNC_CAST(cstr_func), cstr_func_SIG, buffer, buffer_size, 0);
    Partial_call(&p, &result, 0, 0);
    printf("cstr_func: call without args: %s\n", result);
    Partial_call(&p, &result, 1, 0, my_string);
    printf("cstr_func: call with args - my_string: %s\n", result);
    Partial_call(&p, &result, 0, 1, "myp", &kwarg_res[0]);
    printf("cstr_func: call with kwargs - myp = kwarg_res: %s\n", result);
    printf("...done\n");
}

#define VOIDPFUNC_ARG 1

void my_default_voidpfunc(int a) {
    printf("in my_default_voidpfunc. Input: %d\n", a);
}

void my_voidpfunc(int a) {
    printf("in my_voidpfunc. Input: %d\n", a);
}

#define voidpfunc_func_SIG "%vf=%vf{my_func}"
voidpfunc voidpfunc_func(voidpfunc my_func) {
    return my_func;
}

void test_voidpfunc_func(void) {
    printf("\ntesting voidpfunc_func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    voidpfunc result = my_default_voidpfunc;
    Partial_init(&p, -1, FUNC_CAST(voidpfunc_func), voidpfunc_func_SIG, buffer, buffer_size, 0);
    //Partial_call(&p, &result, 0, 0);
    //printf("voidpfunc_func: call without args: %p, result of call(%d): \n\t", (void*)result, VOIDPFUNC_ARG);
    //result(VOIDPFUNC_ARG);
    Partial_call(&p, &result, 1, 0, my_voidpfunc);
    printf("voidpfunc_func: call with args (my_voidpfunc): %p, result of call(%d): \n\t", (void*)result, VOIDPFUNC_ARG);
    result(VOIDPFUNC_ARG);
    Partial_call(&p, &result, 0, 1, "my_func", my_voidpfunc);
    printf("cstr_func: call with kwargs - my_func = my_voidpfunc: %p, result of call(%d): \n\t", (void*)result, VOIDPFUNC_ARG);
    result(VOIDPFUNC_ARG);
    printf("...done\n");
}


#define PVOIDPFUNC_ARG my_pvoid

void * my_default_pvoidpfunc(void * a) {
    printf("in my_default_pvoidpfunc. Input: %p\n", a);
    return a;
}

void * my_pvoidpfunc(void * a) {
    printf("in my_pvoidpfunc. Input: %p\n", a);
    return a;
}

#define pvoidpfunc_func_SIG "%pf=%pf{my_pfunc}"
pvoidpfunc pvoidpfunc_func(pvoidpfunc my_pfunc) {
    return my_pfunc;
}

void test_pvoidpfunc_func(void) {
    printf("\ntesting pvoidpfunc_func...\n");
    size_t buffer_size = 1024;
    unsigned char buffer[1024] = {'\0'};
    Partial p;
    pvoidpfunc result = my_default_pvoidpfunc;
    Partial_init(&p, -1, FUNC_CAST(pvoidpfunc_func), pvoidpfunc_func_SIG, buffer, buffer_size, 0);
    //Partial_call(&p, &result, 0, 0);
    //printf("voidpfunc_func: call without args: %p, result of call(%p): \n\t", (void*)result, PVOIDPFUNC_ARG);
    //result(PVOIDPFUNC_ARG);
    Partial_call(&p, &result, 1, 0, my_pvoidpfunc);
    printf("voidpfunc_func: call with args (my_pvoidpfunc): %p, result of call(%p): \n\t", (char*)result, PVOIDPFUNC_ARG);
    result(PVOIDPFUNC_ARG);
    Partial_call(&p, &result, 0, 1, "my_pfunc", my_pvoidpfunc);
    printf("cstr_func: call with kwargs - my_pfunc = my_pvoidpfunc: %p, result of call(%p): \n\t", (char*)result, PVOIDPFUNC_ARG);
    result(PVOIDPFUNC_ARG);
    printf("...done\n");
}

int main() {
    
    test_all_objects();
    test_bool_func();
    test_short_func();
    test_int_func();
    test_long_func();
    test_longlong_func();
    test_size_t_func();
    test_float_func();
    test_double_func();
    test_longdouble_func();
    test_pvoid_func();
    test_cstr_func();
    test_voidpfunc_func();
    test_pvoidpfunc_func();
    
    return 0;
}