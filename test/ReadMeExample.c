#include <stdio.h>
#include "partial.h"

double add_int_double(int a, double b) {
    printf("received values: %d, %lf\n", a, b);
    return a + b;
}

int main() {
    double result = 2.0;
    
    // static allocation example
    Partial p;
    // in general this must account for promotions.
    size_t buffer_size = sizeof(int) + sizeof(double);
    unsigned char buffer[sizeof(int) + sizeof(double)]; 

    // initialize partial using function "add_int_double" that uses buffer of size buffer_size (there is buffer overflow checking) to potentially store int and double arguments for execution
    printf("init status: %d\n", Partial_init(&p, PARTIAL_DEFAULT_ABI, FUNC_CAST(add_int_double), "%lf=%d%lf", buffer, buffer_size, 0));

    // bind values to arguments as positions starting from 0. In this case 2.345e-1 is bound to parameter 1 (b)
    Partial_bind(&p, 1, 2.345e-1, PARTIAL_SENTINEL);

    // call function of Partial with arguments int and double
    printf("call status: %d\n", Partial_call(&p, &result, -1));
    printf("result of calculation: %lf\n", result);
    return 0;
}

//stdout
//result of calculation: -0.7655