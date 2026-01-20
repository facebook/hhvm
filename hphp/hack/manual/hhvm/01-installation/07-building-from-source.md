# Building from Source

Building from source is advisable generally when you need features that exist in our source that are not in a [package](/hhvm/installation/introduction#prebuilt-packages). Otherwise, installing from a package is the easiest and most stable way to get up and running.

## Requirements

- An `x86_64` system
- Several GB of RAM
- GCC 7+ or clang
- we only actively support building on distributions we create binary packages for; your mileage may vary on other systems

We only support building with the bundled OCaml; you may need to uninstall
(or `brew unlink` on Mac) other ocamlc and ocamlbuild binaries before
building HHVM.

## Installing Build Dependencies

### Debian or Ubuntu

If you haven't already added our apt repositories (e.g. to install binary packages):

```
$ apt-get update
$ apt-get install software-properties-common apt-transport-https
$ apt-key adv --recv-keys --keyserver hkp://keyserver.ubuntu.com:80 0xB4112585D386EB94
```

To install the build dependencies:

```
$ add-apt-repository -s https://dl.hhvm.com/debian
# - or - #
$ add-apt-repository -s https://dl.hhvm.com/ubuntu

$ apt-get update
$ apt-get build-dep hhvm-nightly
```

### Other Distributions

It's best to obtain the dependency list from our nightly packaging system, to ensure you're using an
up-to-date list; to do this, search https://github.com/hhvm/packaging/ for `Build-Depends:`

## Downloading the HHVM source-code

```
git clone git://github.com/facebook/hhvm.git
cd hhvm
git submodule update --init --recursive
```

## Building HHVM

This will take a *long* time.

```
mkdir build
cd build
cmake -DMYSQL_UNIX_SOCK_ADDR=/var/run/mysqld/mysqld.sock ..
make -j [number_of_processor_cores] # eg. make -j 4
sudo make install
```

### Custom GCC

If you have built your own GCC, you will need to pass additional options to cmake:

```
-DCMAKE_C_COMPILER=/path/to/gcc -DCMAKE_CXX_COMPILER=/path/to/g++ -DSTATIC_CXX_LIB=On
```

## Running programs

The installed hhvm binary can be found in `/usr/local/bin`.

## Errors

If any errors occur, you may have to remove the `CMakeCache.txt` file in the checkout.

If your failure was on the `make` command, try to correct the error and run `make` again, it should restart from the point it stops. If the error persists, try to remove as explained above.

## Running Tests

If you want to run the regression tests, you will first need to install some locales.  These locales should be sufficient, although may be more than are actually needed:

```
  sudo locale-gen en_EN
  sudo locale-gen en_UK
  sudo locale-gen en_US
  sudo locale-gen en_GB
  sudo locale-gen de_DE
  sudo locale-gen fr_FR
  sudo locale-gen fa_IR
  sudo locale-gen zh_CN.utf8
  sudo locale-gen zh_CN
```

There are 2 families of regression tests. There are about 5000 tests in all. All tests should pass. It takes about 100 CPU minutes to run them all, but the test runner will run them in parallel, using 1 thread per core:

```
  pushd hphp
    test/run quick
    test/run slow
  popd
```
