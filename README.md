# hardy-remix

## Hardy Remix (HARE)

An investigation of American Fuzzy Lop++ (AFL) as a fuzzer

<img align="right" src="https://upload.wikimedia.org/wikipedia/commons/a/a4/Conejillo_de_indias.jpg" alt="American Fuzzy Lop">

## Run Tests

### Build Binaries

`make all`

### Execute AFL

`afl-fuzz -i test/input05/ -o test/output05/ -D dist/source05_best.bin @@`

### Crash Processing

 * [afl-crash-analyzer](https://github.com/floyd-fuh/afl-crash-analyzer) - another crash analyzer for AFL.
 * [fuzzer-utils](https://github.com/ThePatrickStar/fuzzer-utils) - a set of scripts for the analysis of results.
 * [atriage](https://github.com/Ayrx/atriage) - a simple triage tool.
 * [afl-kit](https://github.com/kcwu/afl-kit) - afl-cmin on Python.
 * [AFLize](https://github.com/d33tah/aflize) - a tool that automatically generates builds of debian packages suitable for AFL.
 * [afl-fid](https://github.com/FoRTE-Research/afl-fid) - a set of tools for working with input data.

## Test Files

## TEST RESULTS

| Filename     | Description                                      |
| :----------- | :----------------------------------------------- |
| source01_*.c | Read input and print to stdout                   |
| source02_*.c | Print command line argument to stdout            |
| source03_*.c | Compare input to a "password" file               |
| source04_*.c | Read input, authenticate, and read file argument |
| source05_*.c | Get filename from argv[1], read, and print it    |

NOTE: All source files should have a 'bad', 'better', and 'best' version.
