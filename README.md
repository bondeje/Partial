# Partial
An implementation of partials (see e.g. Python functools.partial) for the C language as a high-level wrapper of [libffi](https://sourceware.org/libffi/) to account for all the ABIs. 

This does not use any of libffi's closure mechanisms because of the typical case of having to write a wrapper function for the closure calls. If using from an interpreted language, that makes sense, but since the target is C, this is adding a level of complexity not needed.

This is very much in progress. It is tested for basic numeric and character data running GCC 8.1 (Windows/MSYS2-MinGW64) and 9.4 (Linux/Ubuntu) using shared builds of libffi 3.4.4.

Warning, that this will require some knowledge/distinction about the ABIs in use for the functions being wrapped in the partial, but for now, this uses the defaults for the target systems.

## Use

Declare a `Partial` structure and a buffer:

```
Partial partial_obj;
unsigned char buffer[estimated buffer size to hold memory of input arguments];
```

For any function that satisfies the limitations below, make 3 calls:

```
Partial_init(Partial * p, function pointer, char * format, unsigned char * buffer, size_t buffer_size, unsigned int flags)


// bind pairs of values to function call. index specifies which argument is set and value is the corresponding value. PARTIAL_SENTINEL to end sequence
Partial_bind(Partial * p, index_0, value_0, index_1, value_1, ..., PARTIAL_SENTINEL);

// call the function. For output_type = `void` omit the LHS.
// fill in non-bound values from left to right to use in call.
Partial_call(void * return_value, Partial * p, val_0, val_1, ...);
```

### Example
```
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
    Partial_init(&p, FUNC_CAST(add_int_double), "%lf=%d%lf", buffer, buffer_size, int, double);

    // bind values to arguments as positions starting from 0. In this case 2.345e-1 is bound to parameter 1 (b)
    Partial_bind(&p, 1, 2.345e-1, PARTIAL_SENTINEL);

    // call function of Partial with arguments int and double
    double result = Partial_call(double, &p, int, double);
    printf("result of calculation: %lf\n", result);
    return 0;
}

//stdout
//result of calculation: -0.7655
```

For any given `Partial` object, once a value is bound, it currently cannot be unbound. `Partial_bind` may be called multiple times on the same object and overwrite bound values.

## not yet tested

- The full gamut of possible function pointers.
- C++ types, especially template types