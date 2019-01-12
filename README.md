
# Solc Fuzzing

Our motivation for fuzzing the Solidity compiler is multilayered. Various dependent systems allow compilation of potentially *malicous* Solidity source. Many blockchain explorers allow contract verification, such as [Etherscan](https://etherscan.io/contractsVerified) or [Etherchain](https://www.etherchain.org/tools/verifyContract). Two different approaches to fuzz solc are presented in the following sections:

   1. [AFL](#AFL)
   2. [Libfuzzer](#Libfuzzer)

## AFL

### Preparations
First of all, solc needs to be prepared for manual compilation as described in the [official documentation](https://solidity.readthedocs.io/en/v0.5.2/installing-solidity.html#building-from-source). Furthermore, AFL needs to be [installed](https://github.com/mirrorer/afl/blob/master/docs/INSTALL).


In order to make the project fuzzable, it has to be compiled with AFL's G++.
This can be achieved by adjusting scripts/build.sh:

```
cmake .. -DCMAKE_CXX_COMPILER=afl-clang++ -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${@:2}"
```

### Dataset

Using [Galactuzz](https://github.com/Ethermat/galactuzz), we are able to sync all verified contracts from [Etherscan](https://etherscan.io/contractsVerified) and use them as a data source for AFL.

Since the data set is huge, it is recommend to make use of corpus minimization techniques such as afl-cmin and afl-tmin (or both):

```
afl-cmin -i contracts -o fuzzing_testcases -- ./solc @@
```

### Fuzzing
File inputs can be passed directly to AFL:
```
afl-fuzz -i fuzzing_testcases -o fuzzing_out -- ./solc @@
```

## Libfuzzer


### Custom Code
Our fuzzing entrypoint relies on solc's testing functionality. Therefore, we can simply integrate the following code fragment to "test/tools/fuzzer.cpp":

```
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t) {
  string input = string((char *) Data);
  testCompiler(input, false);
  return 0;
}
```

### Building
Adjust scripts/build.sh, such that clang++ with fuzzing instrumentation flags is specified:
```
cmake .. -DUSE_CVC4=OFF -DUSE_Z3=OFF -DCMAKE_CXX_FLAGS="-g -fsanitize=address,fuzzer" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${@:2}"
```

Since we have dependencies to the testing functionality we need to build the project using:
```
./scripts/build.sh TESTS
```

### Dataset

Using [Galactuzz](https://github.com/Ethermat/galactuzz), we are able to sync all verified contracts from [Etherscan](https://etherscan.io/contractsVerified) and use them as a data source for libfuzzer, too.
