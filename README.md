# hardy-remix

## Hardy Remix (HARE)

An investigation of American Fuzzy Lop++ (AFL) as a fuzzer

<img align="right" src="https://upload.wikimedia.org/wikipedia/commons/a/a4/Conejillo_de_indias.jpg" alt="American Fuzzy Lop">

## Run Tests

### Build Binaries

`make all`

### Execute AFL

#### AFL++

##### AFL++ Docker Container

```
docker pull aflplusplus/aflplusplus
docker run -ti --rm --mount type=tmpfs,destination=/ramdisk -e AFL_TMPDIR=/ramdisk -v `pwd`/hardy-remix:/HARE aflplusplus/aflplusplus
```

#### Execute AFL++

##### Base AFL++

```
echo -n "some_file.txt" > test/afl/input01/test_input.txt
afl-fuzz -D -i test/afl/input01/ -o test/afl/output01/ dist/source06_bad_AFL.bin @@
```

##### AFL++ w/ Test Harness

```
echo -n "some_file.txt" > test/afl/input02/test_input.txt
afl-fuzz -D -i test/afl/input02/ -o test/afl/output02/ dist/source07_test_harness_bad_AFL.bin @@
```

##### AFL++ w/ Test Harness and Sanitizer

```
echo -n "some_file.txt" > test/afl/input03/test_input.txt
afl-fuzz -D -i test/afl/input03/ -o test/afl/output03/ dist/source07_test_harness_bad_AFL_ASAN.bin @@
```

#### AFL-Utils

`afl-collect ../hardy-remix/test/output05_bad ../hardy-remix/test/test_collection/ -- ../hardy-remix/dist/source05_bad.bin @@`

`afl-vcrash ../hardy-remix/test/test_collection/ -- ../hardy-remix/dist/source05_bad.bin @@`

### Report/Crash Processing

#### Deployment, management, monitoring, reporting

 * [afl-utils](https://gitlab.com/rc0r/afl-utils) - a set of utilities for automatic processing/analysis of crashes and reducing the number of test cases.
 * [afl-other-arch](https://github.com/shellphish/afl-other-arch) - is a set of patches and scripts for easily adding support for various non-x86 architectures for AFL.
 * [afl-trivia](https://github.com/bnagy/afl-trivia) - a few small scripts to simplify the management of AFL.
 * [afl-monitor](https://github.com/reflare/afl-monitor) - a script for monitoring AFL.
 * [afl-manager](https://github.com/zx1340/afl-manager) - a web server on Python for managing multi-afl.
 * [afl-remote](https://github.com/block8437/afl-remote) - a web server for the remote management of AFL instances.
 * [afl-extras](https://github.com/fekir/afl-extras) - shell scripts to parallelize afl-tmin, startup, and data collection.

#### Crash Reporting

 * [afl-crash-analyzer](https://github.com/floyd-fuh/afl-crash-analyzer) - another crash analyzer for AFL.
 * [fuzzer-utils](https://github.com/ThePatrickStar/fuzzer-utils) - a set of scripts for the analysis of results.
 * [atriage](https://github.com/Ayrx/atriage) - a simple triage tool.
 * [afl-kit](https://github.com/kcwu/afl-kit) - afl-cmin on Python.
 * [AFLize](https://github.com/d33tah/aflize) - a tool that automatically generates builds of debian packages suitable for AFL.
 * [afl-fid](https://github.com/FoRTE-Research/afl-fid) - a set of tools for working with input data.

## Test Files

## TEST RESULTS

| Filename     | Description                                                              |
| :----------- | :----------------------------------------------------------------------- |
| source01_*.c | Read input and print to stdout                                           |
| source02_*.c | Print command line argument to stdout                                    |
| source03_*.c | Compare input to a "password" file                                       |
| source04_*.c | Read input, authenticate, and read file argument                         |
| source05_*.c | Get filename from argv[1], read, and print it                            |
| source06_*.c | Get filename from argv[1], read, and print it w/ Sanitizers              |
| source07_*.c | Get filename from argv[1], read, and print it w/ AFL test harness & ASAN |

NOTE: All source files should have a 'bad' and 'best' version.
