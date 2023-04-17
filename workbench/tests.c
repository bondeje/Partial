#include <stdio.h>
#include <float.h>
typedef struct {
    unsigned char x[3];
} CONFIG_FLOAT;

#define FLOAT float *)double

void f(float from_double) {
    printf("%f\n", from_double);
}

int main() {
    printf("max float: %f\n", FLT_MAX);
    double d = FLT_MAX;
    double * dp = &d;
    f(*dp);
    return 0;
}