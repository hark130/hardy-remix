# hardy-remix

## Hardy Remix (HARE)

An investigation of three fuzzers: American Fuzzy Lop++ (AFL), Honggfuzz, and Radamsa

<img align="right" src="https://upload.wikimedia.org/wikipedia/commons/a/a4/Conejillo_de_indias.jpg" alt="American Fuzzy Lop">

## AFL++

### DOCKER CONTAINERS

AFL++ can be installed natively (which seems like such a pain that I skipped it) or run using their Docker image.  I eventually realized I needed a bit more functionality than was included by default.

#### AFL++ Docker Container

The `docker run` examples in this README will use the `hare_afl` Docker image as the "IMAGE".  The procedures for the base AFL++ Docker image are listed below.

```
docker pull aflplusplus/aflplusplus:latest  # Just use aflplusplus/aflplusplus as the docker run IMAGE (see: man docker-run)
docker image list  # Verify you see "aflplusplus/aflplusplus   latest"
```

#### HARE Docker Container

HARE does some things that needs additional functionality and I got tired of repeating these steps manually.  I wrote a Dockerfile that essentially adds Radamsa and syslog to `aflplusplus/aflplusplus:latest`.

```
docker build devops/docker/HARE_AFL/ --tag hare_afl:latest  # Just use hare_afl:latest as the docker run IMAGE (see: man docker-run)
docker image list  # Verify you see "hare_afl                  latest"
```

### RUN AFL++

1. From the host OS

```
# Build the HARE AFL Docker container (see: "HARE Docker Container" above)
docker run -ti --rm --mount type=tmpfs,destination=/ramdisk -e AFL_TMPDIR=/ramdisk -v `pwd`:/HARE hare_afl:latest  # Start the container
```

2. From the HARE AFL Docker container

```
cd /HARE  # Use the hardy-remix directory as the working directory
make all  # Build *all* the binaries
mkdir test/afl/input01/  # Input directory with test cases
mkdir test/afl/output01/  # Output directory for fuzzer findings
echo -n "some_file.txt" > test/afl/input01/test_input.txt  # Create a test case
afl-fuzz -D -i test/afl/input01/ -o test/afl/output01/ dist/<BINARY TO FUZZ>.bin @@
```

3. Watch it run

NOTE:  Regarding `<BINARY TO FUZZ>`... there are multiple binaries to fuzz.  Hopefully, the binaries are obviously named.  The higher the "source??" number, the more mature the code is.  The "source08" code represents the ultimate goal of this research: a "lite" Linux daemon to fuzz.  I recommend focusing on the "test_harness" files.  The "best" binaries should be error/crash/BUG free.  The "bad" binaries should have BUGs for the fuzzer to find.  TLDR... You're probably looking for a `dist` binary that matches `source08_test_harness_b*_AFL_*.bin`.

## AFL++ RESOURCES

### AFL-Utils

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

## HONGGFUZZ

## RADAMSA

## HARE TEST FILES

| Filename     | Description                                                              |
| :----------- | :----------------------------------------------------------------------- |
| source01_*.c | Read input and print to stdout                                           |
| source02_*.c | Print command line argument to stdout                                    |
| source03_*.c | Compare input to a "password" file                                       |
| source04_*.c | Read input, authenticate, and read file argument                         |
| source05_*.c | Get filename from argv[1], read, and print it                            |
| source06_*.c | Get filename from argv[1], read, and print it w/ Sanitizers              |
| source07_*.c | Get filename from argv[1], read, and print it w/ AFL test harness & ASAN |
| source08_*.c | Launch a "lite" Linux daemon that moves/renames/logs a given file        |

NOTE: All source files should have a 'bad' and 'best' version.
