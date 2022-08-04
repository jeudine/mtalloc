# mtalloc
A lightweight library to efficiently allocate memory in multithreaded applications.


## Usage

Clone this repository (`git clone git@github.com:jeudine/mtalloc.git`) or add it to your project as a submodule (`git submodule add git@github.com:jeudine/mtalloc.git`). You can then run `make` to create `mtalloc.a`. Simply include `src/mtalloc.h` to your source code and link `mtalloc.a`.

Before creating your threads, you need to initialize an `mtm` variable using `mtm_init()`. This variable can then be safely used in all your threads to allocate memory. Don't forget to destroy it at the end using `mtm_destroy()`.

## Test

A comparison between the standard malloc and mtalloc is implemented in `test\`. You can use `make` to build the program and then run it with the desired number of threads.
