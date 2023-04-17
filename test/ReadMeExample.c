#include <stdio.h>
#include "partial.h"

double add_int_double(int a, double b) {
    return a + b;
}

int main() {
    // static allocation example
    Partial p;
    // in general this must account for promotions.
    size_t buffer_size = sizeof(int) + sizeof(double);
    unsigned char buffer[sizeof(int) + sizeof(double)]; 

    // initialize partial using function "add_int_double" that uses buffer of size buffer_size (there is buffer overflow checking) to potentially store int and double arguments for execution
    Partial(&p, add_int_double, buffer, buffer_size, int, double);

    // bind values to arguments as positions starting from 0. In this case 2.345e-1 is bound to parameter 1 (b)
    Partial_bind(&p, 1, 2.345e-1, PARTIAL_SENTINEL);

    // fill in remainder values in order of remaining argument list. Since b is already set, first value filled in is a
    Partial_fill(&p, -1, PARTIAL_SENTINEL);

    // call function of Partial with arguments int and double
    double result = Partial_call(double, &p, int, double);
    printf("result of calculation: %lf\n", result);
    return 0;
}

//stdout
//result of calculation: -0.7655