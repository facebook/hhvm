## Purpose of dso_test
This is a simple zend extension solely for testing, taken from
    http://devzone.zend.com/303
and renamed "dso_test".

This extension is intended to be compiled as a DSO, not statically linked
into hhvm, and then loaded into hhvm using the hhvm extensions mechanism.

## Zend Extensions for Testing DSOs
This zend extension has these requirements:

1. It is just for testing;

2. It is to be loaded into HHVM as a DSO;

3. It is to be built independently from the main build tree,
following the standard recipie for building
a zend extension for HHVM, namely: `hphpize && cmake . && make`

It is tempting (and easy) to build the DSO as part of the main root-level
all-encompassing cmake/make, but that doesn't provide an environment to
satisfy requirement (3), and will burden production hhvm binary with
test only zend extensions.

There is some monkey-business in `../CMakeLists.txt` to prevent the normal
cmake recursion from visiting this directory when discovering how
to build statically loaded zend extensions.

`../CMakeLists.txt` also knows how to build the DSO in this directory.

Because of the way that cmake/make is invoked recursively in this directory,
you will see a line in the root level build progress report
that will report 100% done when the root level build is only (say) 77% done.
Don't fret; the 100% is just the report from the recursive make
rapidly doing its relatively trivial work.

### Build:

    bash ../../../tools/hphpize/hphpize
    cmake .
    make

### Examine products:

    nm -u dso_test.so | c++filt

