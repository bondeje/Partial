# Partial
An implementation of partial-function application (see e.g. Python [functools.partial](https://docs.python.org/3/library/functools.html#:~:text=functools.partial,Roughly%20equivalent%20to%3A)) for the C language as a high-level wrapper of [libffi](https://sourceware.org/libffi/) to account for all the ABIs. 

## Features
- Build as you go (curry-like) function execution environment
    - Once a `Partial` is initialized, the arguments can be bound/updated in any order and any number of times for future calls.
    - Similar to currying which typically does one argument at a time and will produce a new function object in languages with first-class functions.
    - Similar to closures in the sense that the `Partial` captures the environment for a function call; most of the stack frame is held in a buffer. Note that true closures would also hold the local variables in the function call, `Partial` does not.
- Configure function calls to accept default arguments
- Configure function calls to accept keyword argument assignment
- Cross-platform* written in C99
    - Ultimately limited by the completeness of the underlying `libffi`.
- Optional build flag `PARTIAL_PYTHON_STYLE` to change the behavior to more approximate that of Python's `functools.partial`
    - Limits/simplifies public API to just new/call and 
- `Partial`s can be built and executed without any heap allocations (no `malloc` required).

\* This is the intention, though testing is currently limited to Linux (Ubuntu) and Windows 10 (MSYS2/MinGW64) compiled with GCC. Some limitations might be present in `libffi`, but their [list of supported platforms](https://sourceware.org/libffi/) is quite extensive.

### Status and implementation notes

This is very much in progress. It is tested for basic numeric and character data running GCC 8.1 (Windows/MSYS2-MinGW64) and 9.4 (Linux/Ubuntu) using shared builds of libffi 3.4.4. On 64-bit machines only.

This does not use any of libffi's closure mechanisms because of the typical case of having to write a wrapper function for the closure calls. If using from an interpreted language, that makes sense, but since the target is C, this is adding a level of complexity not needed.

Warning, that this will require some knowledge/distinction about the ABIs in use for the functions being wrapped in the partial, but for now, this uses the defaults for the target systems.

The ability to create and execute `Partial`s without heap allocation means `Partial` objects include enough memory for `PARTIAL_MAX_NARG` arguments. By default `PARTIAL_MAX_NARG` is only 16, but can be set by recompiling with a larger value. In the future, a build flag will switch to a more dynamic allocation of memory that is not this restrictive, but will require more heap allocations.

The `PARTIAL_PYTHON_STYLE` implementation is present, but not tested so it is largely left out of the details below.

## API

```
/// @brief Allocate a new Partial object on the heap. It must be destroyed with a call to Partial_del. For static allocation, see Partial_init.
/// @param abi ABI to follow for the function call. See libffi document for details. Use FFI_DEFAULT_ABI or -1 for platform default.
/// @param func Function to be stored for later execution. To avoid compiler warnings, wrap in FUNC_CAST(func).
/// @param format Signature specification for the function call. See the function format specification.
/// @param nargin Number of positional arguments to initially bind to the function call.
/// @param nkwargin Number of keyword arguments to initially bind to the function call.
/// @param ...Variadic with at least as many values as nargin + nkwargin. Provide the positional arguments first, then key-value pairs.
/// @return partial_status code indicating success (generally a 0) or a code describing the failure. 
Partial * Partial_new(partial_abi abi, FUNC_PROTOTYPE(func), char * format, unsigned int nargin, unsigned int nkwargin, ...);

/// @brief Call the input Partial object.
/// @param pobj Pointer to the Partial object to be called.
/// @param ret Pointer to memory where the partial-function application return is stored. Up to user to ensure enough space is allocated.
/// @param nargin Number of positional arguments to complete the function call.
/// @param nkwargin Nubmer of keyword arguments to complete the function call.
/// @param ...Variadic with at least as many values as nargin + nkwargin. Provide the positional arguments first, then key-value pairs.
/// @return partial_status code indicating success (generally a 0) or a code describing the failure. Note that if a failure occurs at any point, ret is unchanged.
partial_status Partial_call(Partial * pobj, void * ret, unsigned int nargin, unsigned int nkwargin, ...);

/// @brief Clean up/free portions of the Partial object that have been allocated on the heap. If the Partial object was created with ALLOCED_BUFFER_FLAG, a call to Partial_del is required.
/// @param pobj - Pointer to the Partial object to be destroyed.
void Partial_del(Partial * pobj);

/// @brief Static initialization of a Partial object and allocation of certain dynamic components depending on flags.
/// @param pobj Pointer to an allocated Partial object.
/// @param abi ABI to follow for the function call. See libffi document for details. Use FFI_DEFAULT_ABI or -1 for platform default.
/// @param func Function to be stored for later execution. To avoid compiler warnings, wrap in FUNC_CAST(func).
/// @param format Signature specification for the function call. See the function format specification.
/// @param buffer Memory buffer to be used to store part of the stack frame in the function call.
/// @param buffer_size Size of the provided memory buffer.
/// @param flags flags indicating how to initialize or allocate dynamic features of a Partial object.
/// @return partial_status code indicating success (generally a 0) or a code describing the failure.
partial_status Partial_init(Partial * pobj, partial_abi abi, FUNC_PROTOTYPE(func), char * format, unsigned char * buffer, size_t buffer_size, unsigned int flags);

/// @brief Bind a number of arguments to an already initialized Partial object.
/// @param pobj Pointer to an initialized Partial object.
/// @param nargin Number of positional arguments to bind to the function call.
/// @param ...Variadic with at least as many values as nargin.
/// @return partial_status code indicating success (generlly a 0) or a code describing the failure. In the event of failure, some arguments may have been bound.
partial_status Partial_bind_nargs(Partial * pobj, unsigned int nargin, ...);

/// @brief Bind a number of keyword arguments to an already initialized Partial object.
/// @param pobj Pointer to an initialized Partial object.
/// @param nkwargin Number of keyword arguments to bind to the function call.
/// @param ...Variadic with at least as many values as nkwargin.
/// @return partial_status code indicating success (generlly a 0) or a code describing the failure. In the event of failure, some arguments may have been bound.
partial_status Partial_bind_nkwargs(Partial * pobj, unsigned int nkwargin, ...);
```

### Available except with PARTIAL_PYTHON_STYLE set

```
/// @brief Bind a number of arguments to an already initialized Partial object in (argument index, argument value) pairs
/// @param pobj Pointer to an initialized Partial object.
/// @param nargin Number of (index, value) pairs to bind to the function call.
/// @param ...Variadic with at least as many values as 2*nargin. The first and every other argument thereafter must be an unsigned integer indicating the position of the argument (starting at 0)
/// @return partial_status code indicating success (generlly a 0) or a code describing the failure. In the event of failure, some arguments may have been bound.
partial_status Partial_bind_npairs(Partial * pobj, unsigned int nargin, ...);

/// @brief Populate (fill in) a number of arguments to an already initialized Partial object in (argument index, argument value) pairs
/// @param pobj Pointer to an initialized Partial object.
/// @param nargin Number of (index, value) pairs to populate in the function call.
/// @param ...Variadic with at least as many values as 2*nargin. The first and every other argument thereafter must be an unsigned integer indicating the position of the argument (starting at 0)
/// @return partial_status code indicating success (generlly a 0) or a code describing the failure. In the event of failure, some arguments may have been populated.
partial_status Partial_fill_npairs(Partial * pobj, unsigned int nargin, ...);

/// @brief Populate (fill in) a number of arguments to an already initialized Partial object.
/// @param pobj Pointer to an initialized Partial object.
/// @param nargin Number of positional arguments to populate in the function call.
/// @param ...Variadic with at least as many values as nargin.
/// @return partial_status code indicating success (generlly a 0) or a code describing the failure. In the event of failure, some arguments may have been populated.
partial_status Partial_fill_nargs(Partial * pobj, unsigned int nargin, ...);

/// @brief Populate (fill in) a number of keyword arguments to an already initialized Partial object.
/// @param pobj Pointer to an initialized Partial object.
/// @param nkwargin Number of keyword arguments to populated in the function call.
/// @param ...Variadic with at least as many values as nkwargin.
/// @return partial_status code indicating success (generlly a 0) or a code describing the failure. In the event of failure, some arguments may have been populated.
partial_status Partial_fill_nkwargs(Partial * pobj, unsigned int nkwargin, ...);
```

### partial_status codes

| partial_status                        | (alias) description |
| ------------------------------------- | ----------- |
| `PARTIAL_SUCCESS` (0)                 | no issues detected, expect OK |
| `PARTIAL_FAILURE`                     | unspecified issue detected |
| `PARTIAL_BAD_TYPEDEF`                 | (FFI_BAD_TYPEDEF) |
| `PARTIAL_BAD_ABI`                     | (FFI_BAD_ABI) - unknown/inappropriate ABI |
| `PARTIAL_BAD_ARGTYPE`                 | (FFI_BAD_ARGTYPE) - error in use of argument type(s) |
| `PARTIAL_MALLOC_FAILURE`              | `malloc` failure |
| `PARTIAL_REALLOC_FAILURE`             | `realloc` failure |
| `PARTIAL_INIT_FAILED`                 | initialization of partial failed |
| `PARTIAL_BAD_FORMAT`                  | failed parsing of function signature format specification |
| `PARTIAL_INCOMPLETE_SPEC`             | incomplete function signature format specification provided |
| `PARTIAL_INSUFFICIENT_BUFFER_SIZE`    | static buffer provided is insufficient to hold required data for call stack |
| `PARTIAL_UNSUPPORTED_TYPE`            | format specifier in function signature specification is not understood, not accepted |
| `PARTIAL_DEFAULT_STRING_TOO_LARGE`    | default string larger than `PARTIAL_MAX_DEFAULT_SIZE` (can be set at compile time) |
| `PARTIAL_COPY_FAILURE`                | failure to copy argument value to call stack |
| `PARTIAL_VALUE_ERROR`                 | invalid argument value passed to function |
| `PARTIAL_KEY_ERROR`                   | invalid keyword passed to function |
| `PARTIAL_TYPE_ERROR`                  | invalid type detected in format specification |
| `PARTIAL_INSUFFICIENT_ARGS`           | not enough arguments were provided for function call |
| `PARTIAL_CANNOT_FILL_BOUND_ARG`       | an argument set with `Partial_new` or `Partial_bind*` cannot <br/> be overwritten by a call from `Partial_call`, `Partial_fill_npairs`, <br/> or `Partial_fill_nkwargs` |
| `PARTIAL_NOT_YET_IMPLEMENTED`         | function call is not yet implemented |

### Function signature format specification

A string that follows type specifications borrowed from the `*printf, *scanf` family of functions.

The format is a cstr that follows the pattern below where <...> delineates optional portions. All other symbols outside of the specifiers and string names (specifiers, keywords, default_values) are required. Whitespace is ignored.

"return_specifier=arg0_specifier<{<arg0_keyword><=arg0_default_value>}><arg1_specifier<{<arg1_keyword><=arg1_default_value>}>...<argN-1_specifier<{<argN-1_keyword><=argN-1_default_value>}>>"

#### Description for format specification

An appropriate return specifier is always required. `%v` is provided for `void`/ignored return functions.

At least one input argument is required. Current default maximum value for N is 16.

Keywords are optional for all arguments.

Default values are optional for all arguments, but for several types, specifically those that do not support a way to represent the default as a string that can be decoded, there may not be or are limited in defaults.

### List of accepted types and their format specifiers

| type                  | specifier | notes |
| --------------------- | --------- | ----- |
| void                  | %v        | should only use for return value, no defaults, no keywords |
| bool                  | %b        | acceptable defaults are `true` and `false` |
| char                  | %c        | |
| signed char           | %hhi      | |
| unsigned char         | %hhu      | |
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
| void *                | %p        | Any object pointer. Only valid default is `NULL` |
| cstr (char*)          | %s        | special case for handling char * inputs that need <br/> defaults. If no default is needed, best to use void *. <br/> Note that because of the way buffer/format handling is done,<br/> the cstr default value lifetime is the partial object,<br/> NOT the life of a string literal/string used as format |
| void (*)()            | %vf       | functions without a return. Does not currently accept defaults |
| void * (*)()          | %pf       | functions returning pointers. Does not currently accept defaults |

## Simple Use

This is a contrived use case purely to show the capabilities of `Partial`s. It is not necessarily a <b> good </b> use case of partial-function application

```
// target function to be called and its formatted SIGNATURE for use with Partial

void * get_attribute(Object * obj, char * attr) {
    /* 
    get attribute named "attr" from Object instance obj, return as void *
    */
    return pointer_to_attribute_value;
}
/*
Signature identifies that get_attribute returns a pointer and accepts a pointer and a cstr as inputs. The cstr can be assigned with the keyword argument "attr" and also defaults to the attribute "name"
*/
#define get_attribute_SIGNATURE "%p=%p%s{attr=name}"

Object o = {//initialize Object instance};

```

Create a Partial, which binds the object `o` to a call to `get_attribute`:

```
// -1 for first argument is default ABI
Partial * o_attr = Partial_new(-1, FUNC_CAST(get_attribute), get_attribute_SIGNATURE, 1, 0, &o);

// Since we used "Partial_new", the Partial is heap-allocated and the subsequent calls to Partial_call below may occur in any scope until the terminal call to Partial_del.

void * attr; // allocate for return
Partial_call(o_attr, &attr, 0, 0); // attr now contains the "name" attribute of o.

Partial_call(o_attr, &attr, 1, 0, "size"); // attr now contains the "size" attribute of o;

Partial_call(o_attr, &attr, 0, 1, "attr", "address"); // attr now contains the "address" attribute of o;

Partial_del(o_attr); // clean up

```

For setting values after `Partial_new` or `Partial_init` but before `Partial_call`, there are functions of the prototype: `Partial_[action]_n[param type]` where the `[action]` values are "bind" or "fill" and `[param type]` are "pairs", "args", or "kwargs". The semantics of the `[action]` values are such that "bind" will set values defined by parameters and set them as "bound", i.e. they can only be overwritten by another bind, while "fill" will set values only on non-"bound arguments". The `[param type]` values mean:

"kwargs" - the variadic is filled with pairs of (cstr keywords, value) where cstr must be a keyword that was set in the format to a call to `Partial_new` or `Partial_init`.

"pairs" - the variadic is filled with pairs of (index, value) where index is the <b>input</b> argument index starting from 0 and value is the value to be filled/bound.

"args" - the variadic is just a list of values, which will be copied based on the semantics of the `[action]`. Note that unlike "kwargs" and "pairs", the argument location is not specified. In the case of `[action]=bind`, the values are filled from left to right starting at input argument index 0. In the case of `[action]=fill`, the values are filled from left to right in non-"bound" arguments.

`Partial_new` follows the semantics of `Partial_bind_*`, i.e. `Partial_new` is effectively like calling `Partial_init` following by `Partial_bind_*` for the positional and keyword arguments, but with the added benefit of memory management on the heap.

`Partial_call` follows the semantics of `Partial_fill_*`

Note that the above is explicitly different from the simple application of `functools.partial` in Python. Python's partial-function application treats the initial arguments as arguments with a purely delayed call, meaning calling the partial is exactly calling the underlying function with all the positional and keyword arguments from creation and the call in the proper Python-legal order all at once, specifically all keyword arguments override positional arguments. In the current implementation, arguments are set in the order they are bound or filled, specifically positional arguments can override keyword arguments.

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
result of calculation: -0.7655
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

- Function pointers as arguments
- Arbitrary structures as arguments passed by value