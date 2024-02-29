Overview
========

There is now a
[user guide](https://github.com/real-logic/simple-binary-encoding/wiki/Golang-User-Guide)
and this document is for development of the SBE golang generator.

Code Layout
-----------
The Java code that performs the generation of golang code is
[here](https://github.com/real-logic/simple-binary-encoding/tree/master/sbe-tool/src/main/java/uk/co/real_logic/sbe/generation/golang).

Golang code used for testing resides in the top-level
[gocode directory](https://github.com/real-logic/simple-binary-encoding/tree/master/gocode).

Building and testing
--------------------
At some point the golang build will be better integrated into
gradle. For the time being some instructions/procedures are encoded
into a gnu Makefile in the top level gocode directory

To match golang's requirement on directory structure and file layout,
the build environment, example and test code are structured into a top
level gocode directory hierarchy.

Code is generated into this structure with pre-existing test code in place.

For the example, the code is generated into a library and the matching
example code lives in it's own directory at the same level. For
example, the example-schema generates the baseline library code into
`gocode/src/baseline` and example code lives in `gocode/src/example-schema`.

To use this layout you should `set GOPATH=/path/to/gocode` or use the
supplied Makefile which does this for you. For the tests you will need
to not have `GOPATH` set or use the supplied Makefile which does this
for you.


Available make targets are:

```example``` will generate golang for the example schema (and
extensions) and run some test code that goes with it.

```test``` will generate golang for some test schemas and run some
test code for them.

```bench``` will run some benchmarking code based on the Car example.


Design choices
--------------
Most of the design choice rationale is contained in the user guide
however, some design decisions are based around the structure of
sbe-tool itself.

sbe-tool parses the XML into an internal representation (IR) and then
passes this to the language specific generator. It is this IR which a
generator processes.

Roadmap
=======
 * Windows developer support (currently tested on Linux/MacOS)
 * Further Unicode support
 * Testing/Bug fixes
