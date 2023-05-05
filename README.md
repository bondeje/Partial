# Partial
An implementation of partials (see e.g. Python [functools.partial](https://docs.python.org/3/library/functools.html#:~:text=functools.partial,Roughly%20equivalent%20to%3A)) for the C language as a high-level wrapper of [libffi](https://sourceware.org/libffi/) to account for all the ABIs. 

This is very much in progress. It is tested for basic numeric and character data running GCC 8.1 (Windows/MSYS2-MinGW64) and 9.4 (Linux/Ubuntu) using shared builds of libffi 3.4.4. On 64-bit machines only.

This does not use any of libffi's closure mechanisms because of the typical case of having to write a wrapper function for the closure calls. If using from an interpreted language, that makes sense, but since the target is C, this is adding a level of complexity not needed.

Warning, that this will require some knowledge/distinction about the ABIs in use for the functions being wrapped in the partial, but for now, this uses the defaults for the target systems.

## Use

Declare a `Partial` structure and a buffer:

```
Partial partial_obj;
unsigned char buffer[estimated buffer size to hold memory of input arguments];
```

For any function that satisfies the limitations below, make 3 (+1 optional for purely static allocation) calls:

```
Partial_init(Partial * p, partial_abi ABI, function pointer, char * format, unsigned char * buffer, size_t buffer_size, unsigned int flags)


// bind pairs of values to function call. index specifies which argument is set and value is the corresponding value. PARTIAL_SENTINEL to end sequence
Partial_bind_npairs(Partial * p, N, index_0, value_0, index_1, value_1, ..., index_N-1, value_N-1);

/*
For any given `Partial` object, once a value is bound, it currently cannot be unbound. `Partial_bind` may be called multiple times on the same object and overwrite bound values.
*/

// call the function. For output_type = `void` omit the LHS.
// fill in non-bound values from left to right to use in call.
Partial_call(Partial * p, void * return_value, N, M, val_0, val_1, ..., val_N-1, key_0, kval_0, key_0, kval_0, ..., key_M-1, kval_M-1);

// if statically allocating the Partial object and buffer, this is unnecessary, but if you're not sure whether any part was allocated, use this anyway.
Partial_del(Partial * p);
```

For setting values after `Partial_new` or `Partial_init` but before `Partial_call`, there are functions of the prototype: `Partial_[action]_n[param type]` where the `[action]` values are "bind" or "fill" and `[param type]` are "pairs", "args", or "kwargs". The semantics of the `[action]` values are such that "bind" will set values defined by parameters and set them as "bound", i.e. they can only be overwritten by another bind, while "fill" will set values only on non-"bound arguments". The `[param type]` values mean:

"kwargs" - the variadic is filled with pairs of (cstr keywords, value) where cstr must be a keyword that was set in the format to a call to `Partial_new` or `Partial_init`.
"pairs" - the variadic is filled with pairs of (index, value) where index is the <b>input<b/> argument index starting from 0 and value is the value to be filled/bound.
"args" - the variadic is just a list of values, which will be copied based on the semantics of the `[action]`. Note that unlike "kwargs" and "pairs", the argument location is not specified. In the case of `[action]=bind`, the values are filled from left to right starting at input argument index 0. In the case of `[action]=fill`, the values are filled from left to right in non-"bound" arguments.

`Partial_new` follows the semantics of `Partial_bind_*`

`Partial_call` follows the semantics of `Partial_fill_*`

Note that the above is explicitly different from the simple application of `functools.partial` in Python.

```
#Python:
from functools import partial

def my_func(a, b, c, d):
    # my_func implementation

a = partial(my_func, 1, c=3)
#b = a(2,4)             # error in Python because d does not get assigned a value and c given 2x
c = a(2, d=4)           # my_func(1, 2, 3, 4)
d = a(2, c=5, d=4)      # my_func(1,2,5,4), repeat of c overwrites original in creation of partial

//C equivalent calls...slightly different behavior
char my_func_SIG = "%v=%d%d%d{c}%d{d}"; // format signature for my_func use in Partial*
void my_func(int a, int b, int c, int d)

Partial * p = Partial_new(-1, FUNC_CAST(my_func), my_func_SIG, 1, 1, 1, "c", 3);
Partial_call(p, NULL, 2, 0, 2, 4);              // unlike Python, this will call my_func(1, 2, 3, 4);
Partial_call(p, NULL, 1, 1, 2, "d", 4);         // this is explicitly my_func(1,2,3,4), same as Python
Partial_call(p, NULL, 1, 2, 2, "d", 4, "c", 5); // this will fail trying to fill in a bound value
Partial_del(p);

```

In general, there is currently no (easy) way to exactly replicate Python's `functools.partial` behavior, but I might add a configuration flag for Python style, which could add a lot more memory usage.

Specific example of `Partial_fill_nargs`:

```
void my_func(int a, int b, int c, int d);

Partial p;
Partial_init(&p, -1, FUNC_CAST(my_func), ...);
Partial_fill_nargs(&p, 2, 0, 1);                // sets a = 0, and b = 1 but neither are bound
Partial_bind_npairs(&p, 2, 0, 3, 2, 4);         // sets and binds a = 3, and c = 4.
Partial_fill_nargs(&p, 1, 2);                   // sets b = 2, but not bound
Partial_call(&p, NULL, 2, 0, 5, 6);             // calls my_func(3, 5, 4, 6); // overwrites b = 2 from previous call, because Partial_call internally uses Partial_fill_nargs
```

List of accepted types and their format specifiers

| type                  | specifier | notes |
| --------------------- | --------- | ----- |
| void                  | %v        | should only use for return value, no defaults, no keywords |
| bool                  | %b        | acceptable defaults are `true` and `false` |
| char                  | %c        | |
| unsigned char         | %cu       | WARNING: this might change to be consistent with <br/> sscanf/printf |
| short                 | %hd       | |
| unsigned short        | %hu       | |
| int                   | %d        | |
| unsigned int          | %u        | |
| long                  | %ld       | |
| unsigned long         | %lu       | |
| long long             | %lld      | |
| unsigned long long    | %llu      | |
| size_t                | %zu       | For Windows, may only work with MinGW64. <br/> See make files for appropriate flags in this case |
| float                 | %f        | |
| double                | %lf       | |
| long double           | %LF       | On systems without long double, defaults to double (due to libffi) |
| void *                | %p        | Any object pointer. In future, a %s specifier may be <br/> added for char * cstr to allow cstr default values <br/> Only valid default is NULL |
| cstr (char*)          | %s        | special case for handling char * inputs that need <br/> defaults. If no default is needed, best to use void *. <br/> Note that because of the way buffer/format handling is done<br/>, the cstr default value lifetime is the partial object,<br/> NOT the life of a string literal/string used as format |

### WARNINGS:

Very much not type safe. Due to heavy uses of variadics, the types must match the signature exactly and especially integer input literals should be either cast or defined with appropriate suffixes.

Due to OS and ABI differences, for example, if an untyped signed integer literal is passed into variadics, the following data types will result in UB:
- long (LINUX, *nix because long is 64-bit on these OSes)
- long long 
- size_t
- int64_t
- etc.

Additionally, due to calling conventions and passing via registers, on Windows this will be a problem for integers passed as part of variadic in first 4 arguments, but on Linux, this will only really be a problem if the integer is passed as the 7th or larger argument.

Fortunately should not be an issue for char/short since they are promoted to int in variadics anyway.

### Example

Add an int to a bound double.

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
    // in general this must account for promotions.
    size_t buffer_size = sizeof(int) + sizeof(double);
    unsigned char buffer[sizeof(int) + sizeof(double)]; 

    // initialize partial using function "add_int_double" that uses buffer of size buffer_size (there is buffer overflow checking) to potentially store int and double arguments for execution
    Partial_init(&p, PARTIAL_DEFAULT_ABI, FUNC_CAST(add_int_double), "%lf=%d%lf", buffer, buffer_size, 0)

    // bind values to arguments as positions starting from 0. In this case 2.345e-1 is bound to parameter 1 (b)
    Partial_bind_npairs(&p, 1, 1, 2.345e-1);

    // call function of Partial with arguments int and double
    printf("call status: %d\n", Partial_call(&p, &result, 1, 0, -1));
    printf("result of calculation: %lf\n", result);
    return 0;
}

//stdout
//result of calculation: -0.7655
```

## Dependencies

Depends on [libffi](https://sourceware.org/libffi/). You will need to install and configure the libffi library on your machine for the appropriate build and host machine. Then link as appropriate using the `ffi.h` and `ffitarget.h` headers.

### notes on configuring libffi

For Linux, the configuration is pretty straightforward. Run

`./configure`

in the unzipped libffi distribution directory to get the shared library.

For Windows, however, there is typically some finessing. For MSVC or clang, try to follow the instructions for [libffi on github](https://github.com/libffi/libffi). I did not get this to work for MSVC, but I'm more accustomed to GCC. For GCC, I highly suggest using an MSYS2 environment with GCC from MinGW64.

The default configuration for libffi (at least in 3.4.4 that I tried) is wrong for GCC on MinGW64 with x86_64 architecture. For some reason, it defaults to x86_64-pc-msys/mingw64, but the pc configuration will screw up the build and test of libffi. I had to specifically configure build and host machines:

`./configure --build=x86_64-w64-mingw64 --host=x86_64-w64-mingw64`

After I figured that out, no problems.

## not yet tested/wish list

- keyword argument binding/calling. Note that I'm probably going to re-define `Partial_call` to get this to work.
- The full gamut of possible function pointers.
- C++ types, especially template types, but they probably will not work out of the box.