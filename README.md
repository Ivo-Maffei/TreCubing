# Description
This is a simple program to test the speed of the different components used to build a modular-exponentiation-based TLP.
It accompanies the paper "Time-Lock Puzzles via Cubing" by Ivo Maffei and Andrew W. Roscoe.

This project provides naive and insecure implementations, as it purpose was only to provide rough time estimates.


# Dependencies
This program relies on the following libraries (other versions might work, but were not tested):
  - [GMP](https://gmplib.org) version 6.3
  - [OpenSSL](https://openssl-library.org/) version 3.4
  - [GNU's Argp](https://www.gnu.org/software/libc/manual/html_node/Argp.html)
  - [GNU Make](https://www.gnu.org/software/make/) version 3.81
  - [GCC](https://gcc.gnu.org/) version 14.2

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
Test different primitives for Time-Lock Puzzles via Cubing and outputs the test results in the
file provided.

  -n, --iterations=nIters    Specify the number of indipendent iterations to
                             run (default: 100)
  -p, --primesize=pSize      Specify the (approximate) size in bits for the
                             modolus to use (default: test all valid sizes)
  -s, --securityParam=secpar If non-zero, this specifies that we are using a
                             prime power modulo whose base has this bitsize

 Select one or more of the following 5 if you don't want to test all methods:
      --clean                Clean the output file before writing to it
  -c, --cubing               Test the cubing/cube root performance
      --delayThorp           Test the performance of the whole delay/open
                             method using Thorp as FPE
  -d, --delay                Test the performance of the whole delay/open
                             method (using both-ends encryption)
  -e, --encryption           Test the stream cipher encryption performance
  -f, --fpe[=fpeScheme]      Test the performance of the specified FPE methods
                             as a comma separated list (if no list is provided,
                             all methods are tested). Valid FPE schemes are:
                             'thorp', 'swapornot'
  -x, --hashing              Test the hashing performance

  -C, --chain=nChain         Specify how many delays to chain together when
                             testing the whole delay process. If this option is
                             omitted, we test values: 10 and 100
  -m, --moduli               Test the performance of different moduli creations

  -R, --rounds=nRounds       Specify the number of rounds to use when testing
                             FPE schemes. If this option is omitted, we test
                             the values: 100, 0.5*primesize and primesize

  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to ivo.maffei@uni.lu.
```