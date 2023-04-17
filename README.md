# Partial
An implementation of partials (see e.g. Python functools.partial) for the C language.

This is very much in progress. It is tested for simple objects with arguments of types with sizes 1, 2, 4, 8, 12, and 16 on Windows with MinGW running GCC 8.1.

## Use

Declary a `Partial` structure:

```
Partial partial_obj;
```

For any function that satisfies the limitations below, make 4 calls:

```
Partial(Partial * p, function pointer, unsigned char * buffer, size_t buffer_size, arg_type_0, arg_type_1, ..., arg_type_N); // 0 <= N <= 15...the numbers are the "indices" used below

// bind pairs of values to function call. index specifies which argument is set and value is the corresponding value. PARTIAL_SENTINEL to end sequence
Partial_bind(Partial * p, index_0, value_0, index_1, value_1, ..., PARTIAL_SENTINEL);

// fill in non-bound values from left to right to use in call.
Partial_fill(Partial * p, value, value, ..., PARTIAL_SENTINEL);

// call the function. For output_type = `void` omit the LHS.
output_type result = Partial_call(output_type, Partial * p, arg_type_0, arg_type_1, ..., arg_type_N);
```

For any given `Partial` object, once a value is bound, it currently cannot be unbound. `Partial_bind` may be called multiple times on the same object and overwrite bound values.

`Partial_fill` may be called multiple times on the same `Partial` object. Each subsequent call will override values. It will never override already bound values.

## (Known) Limitations

Most of the limitations below are due to the underlying mechanisms relying on variadics, for which the default promotions really screw up buffering. If the C preprocessor would have the ability to distinguish at least the basic types or even between int vs float vs anything else, all these limitations could be in principle taken into account.

- Only up to 16 arguments are handled. If you absolutely need more, may whatever god you pray to have mercy on your soul.
- `float` types will not work. The default promotions prevent use of this common type as it is utterly indistinguishable by the preprocessor from other types that are not promoted. Suggested work-around is to create a wrapper function that takes double and casts down to float.
- All non-integer types `A` where `sizeof(A) < sizeof(int)`. For the same reason as `float`, but here, a design choice was made to either be compatible with integer types of size smaller than `int/unsigned int` or small non-integer types. Note that `bool` and `char` being promoted to integer types work fine. In the future, I might make a flag to invert this behavior since there is probably a workaround that I have not thought of for ints.

## not yet tested

- The full gamut of possible function pointers.
- C++ types, especially template types

## Example
```
double add_int_double(int a, double b) {
    return a + b;
}

int main() {
    // static allocation example
    Partial p;
    // in general this must account for promotions.
    size_t buffer_size = sizeof(int) + sizeof(double);
    char buffer[sizeof(int) + sizeof(double)]; 

    // initialize partial using function "add_int_double" that uses buffer of size buffer_size (there is buffer overflow checking) to potentially store int and double arguments for execution
    partial(&p, add_int_double, buffer, buffer_size, int, double);

    // bind values to arguments as positions starting from 0. In this case 2.345e-1 is bound to parameter 1 (b)
    Partial_bind(&p, 1, 2.345e-1, PARTIAL_SENTINEL);

    // fill in remainder values in order of remaining argument list. Since b is already set, first value filled in is a
    Partial_fill(&p, -1, PARTIAL_SENTINEL);

    // call function of Partial with arguments int and double
    double result = Partial_call(double, &p, int, double);
    printf("result of calculation: %lf\n", result);
    return 0;
}

\\stdout
result of calculation: -0.7655
```