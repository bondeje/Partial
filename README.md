# Partial
An implementation of partials (see e.g. Python [functools.partial](https://docs.python.org/3/library/functools.html#:~:text=functools.partial,Roughly%20equivalent%20to%3A)) for the C language as a high-level wrapper of [libffi](https://sourceware.org/libffi/) to account for all the ABIs. 

This is very much in progress. It is tested for basic numeric and character data running GCC 8.1 (Windows/MSYS2-MinGW64) and 9.4 (Linux/Ubuntu) using shared builds of libffi 3.4.4. On 64-bit machines only.

This does not use any of libffi's closure mechanisms because of the typical case of having to write a wrapper function for the closure calls. If using from an interpreted language, that makes sense, but since the target is C, this is adding a level of complexity not needed.

Warning, that this will require some knowledge/distinction about the ABIs in use for the functions being wrapped in the partial, but for now, this uses the defaults for the target systems.

## Dependencies

Depends on [libffi](https://sourceware.org/libffi/). You will need to install and configure the libffi library on your machine for the appropriate build and host machine. Then link as appropriate using the `ffi.h` and `ffitarget.h` headers.

### notes on configuring libffi

For Linux, the configuration is pretty straightforward. Run

`./configure`

in the unzipped libffi distribution directory to get the shared library.

For Windows, however, there is typically some finessing. For MSVC or clang, try to follow the instructions for [libffi on github](https://github.com/libffi/libffi). I did not get this to work for MSVC, but I'm more accustomed to GCC. For GCC, I highly suggest using an MSYS2 environment with GCC from MinGW64. 

The default configuration for libffi (at least in 3.4.4 that I tried) is wrong for GCC on MinGW64. For some reason, it defaults to x86_64-pc-msys/mingw64. The pc configuration will screw up the build and test of libffi. I had to specifically configure build and host machines:

`./configure --build=x86_64-w64-mingw64 --host=x86_64-w64-mingw64`

After I figured that out, no problems.

## Use

Declare a `Partial` structure and a buffer:

```
Partial partial_obj;
unsigned char buffer[estimated buffer size to hold memory of input arguments];
```

For any function that satisfies the limitations below, make 4 calls:

```
Partial_init(Partial * p, partial_abi ABI, function pointer, char * format, unsigned char * buffer, size_t buffer_size, unsigned int flags)


// bind pairs of values to function call. index specifies which argument is set and value is the corresponding value. PARTIAL_SENTINEL to end sequence
Partial_bind(Partial * p, index_0, value_0, index_1, value_1, ..., PARTIAL_SENTINEL);

// call the function. For output_type = `void` omit the LHS.
// fill in non-bound values from left to right to use in call.
Partial_call(Partial * p, void * return_value, val_0, val_1, ...);

// if statically allocating the Partial object and buffer, this is unnecessary, but if you're not sure whether any part was allocated, use this anyway.
Partial_del(Partial * p);
```

List of accepted types and their format specifiers

| type                  | specifier | notes |
| --------------------- | --------- | ----- |
| void                  | %v        | should only use for return value |
| bool                  | %b        | |
| char                  | %c        | |
| unsigned char         | %cu       | WARNING: this might change to be consistent with sscanf/printf |
| short                 | %hd       | |
| unsigned short        | %hu       | |
| int                   | %d        | |
| unsigned int          | %u        | |
| long                  | %ld       | |
| unsigned long         | %lu       | |
| long long             | %lld      | |
| unsigned long long    | %llu      | |
| size_t                | %zu       | may only work on Windows for MinGW64 |
| float                 | %f        | |
| double                | %lf       | |
| long double           | %LF       | On systems without long double, defaults to double (libffi) |
| void *                | %p        | Any object pointer |

### Example
```
#include <stdio.h>
#include "partial.h"

double add_int_double(int a, double b) {
    return a + b;
}

int main() {
    double result = 2.0;
    
    // static allocation example
    Partial p;
    size_t buffer_size = sizeof(int) + sizeof(double);
    unsigned char buffer[sizeof(int) + sizeof(double)]; 

    // initialize partial using function "add_int_double" that uses buffer of size buffer_size (there is buffer overflow checking) to potentially store int and double arguments for execution
    Partial_init(&p, PARTIAL_DEFAULT_ABI, FUNC_CAST(add_int_double), "%lf=%d%lf", buffer, buffer_size, 0);

    // bind values to arguments as positions starting from 0. In this case 2.345e-1 is bound to parameter 1 (b)
    Partial_bind(&p, 1, 2.345e-1, PARTIAL_SENTINEL);

    // call function of Partial with arguments int and double
    Partial_call(&p, &result, -1);
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