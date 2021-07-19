# hardy-remix

## Hardy Remix (HARE)

An investigation of American Fuzzy Lop++ as a fuzzer

<img align="right" src="https://upload.wikimedia.org/wikipedia/commons/a/a4/Conejillo_de_indias.jpg" alt="American Fuzzy Lop">

## Run Tests

### Build Binaries

`make all`

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
