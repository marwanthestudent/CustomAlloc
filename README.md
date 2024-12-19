## Description

An implementation of malloc and free. Implemented a library that interacts with the operating system to perform heap management for a user process. This project is built in C.

## Building and Running the Code

The code compiles into four shared libraries and ten test programs. To build the code, change to your top level assignment directory and type:
```
make
```
Once you have the library, you can use it to override the existing malloc by using
LD_PRELOAD. The following example shows running the ffnf test using the First Fit shared library:
```
$ env LD_PRELOAD=lib/libmalloc-ff.so tests/ffnf
```

To run the other heap management schemes replace libmalloc-ff.so with the appropriate library:
```
Best-Fit: libmalloc-bf.so
First-Fit: libmalloc-ff.so
Next-Fit: libmalloc-nf.so
Worst-Fit: libmalloc-wf.so
```
## Features

1. Splitting and coalescing of free blocks. If two free blocks are adjacent they
are combined. If a free block is larger than the requested size then the block is split into two.
2. Four heap management strategies: First Fit, Next Fit, Worst Fit, Best Fit.
3. Counters exist in the code for tracking of the following events:

* Number of times the user calls malloc successfully
* Number of times the user calls free successfully
* Number of times we reuse an existing block
* Number of times we request a new block
* Number of times we split a block
* Number of times we coalesce blocks
* Number blocks in free list
* Total amount of memory requested
* Maximum size of the heap

The code will print the statistics ( THESE STATS ARE FAKE) upon exit and should look like similar to:
```
mallocs: 8
frees: 8
reuses: 1
grows: 5
splits: 1
coalesces: 1
blocks: 5
requested: 7298
max heap: 4096
```

4. Test programs are provided to demonstrate the memory allocator. They are located in the tests
directory.
5. realloc and calloc.

## Notes

You will see an extra malloc of 1024 bytes in your statistics. This is the system allocating memory for the printf statement.

## Debugging
To run the debugger: 
```
$ gdb ./tests/ffnf
...
(gdb) set exec-wrapper env LD_PRELOAD=./lib/libmalloc-ff.so
(gdb) run
...
(gdb) where
```
