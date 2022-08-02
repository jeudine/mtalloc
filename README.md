# mtalloc
A lightweight library to efficiently allocate memory in multithreaded applications.


## Usage

Add `src\mtalloc.c` and `src\mtalloc.h` to your sources.

Before creating your threads, you need to initialize an `mtm` variable using `mtm_init()`. This variable can then be safely used in all your threads to allocate memory. Don't forget to destroy it at the end using `mtm_destroy()`.

## Test

A comparison between the standard malloc and mtalloc is implemented in `test\`. You can use `make` to compile the program and then execute it with the desired number of threads.
