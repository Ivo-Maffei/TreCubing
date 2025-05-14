# Description
This is a simple program to test the speed of the different components used to build a modular-exponentiation-based TLP.
It accompanies the paper "Time-Lock Puzzles via Cubing" by Ivo Maffei and Andrew W. Roscoe.

This project provides naive and insecure implementations, as it purpose was only to provide rough time estimates.

See the file in `tests output` for an example output of the program.

# Dependencies
This program relies on the following libraries (other versions might work, but were not tested):
  - [GMP](https://gmplib.org) version 6.3
  - [OpenSSL](https://openssl-library.org/) version 3.4
  - [GNU's Argp](https://www.gnu.org/software/libc/manual/html_node/Argp.html)
  - [GNU Make](https://www.gnu.org/software/make/) version 3.81
  - [GCC](https://gcc.gnu.org/) version 14.2

*WARNING*: some parts of the code are optimised for running on machines storing values in little endian order. Big endian is not currently supported. 

# Installation
  1. Install all the dependencies above. (On MacOS you can use [homebrew](https://brew.sh/).)
  2. Edit the `makefile` to choose your C compiler and the include paths.
  3. Use the `.patch` files to augment GMP with a faster (i.e., no window precomputation) modular exponentiation function.
  4. Compile with `make`.
  5. Run with `./trecubing`.

# Usage
You can view a list of various options by running `./trecubing --help`.
Here is an example of its output.
```
Usage: trecubing [OPTION...] [FILENAME|stdout]
Test different primitives for Time-Lock Puzzles via Cubing and outputs the test
results in the file provided.

  -n, --iterations=nIters    Specify the number of indipendent iterations to
                             run (default: 100)
  -p, --primesize=pSize      Specify the (approximate) size in bits for the
                             modolus to use (default: test all valid sizes)
  -s, --securityParam=secpar Specify the bit-size of the based used for moduli
                             using prime powers or product of primes powers
                             (default: 128 bits)

 Select one or more of the following options if you don't want to test all
 methods:
  -c, --cubing               Test the cubing/cube root performance
      --clean                Clean the output file before writing to it
  -e, --encryption           Test the stream cipher encryption performance
  -m, --moduli               Test the performance of prime power modulo
                             creations
  -x, --hashing              Test the hashing performance

  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```