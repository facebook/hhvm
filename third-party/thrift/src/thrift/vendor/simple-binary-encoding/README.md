Simple Binary Encoding (SBE)
============================

[![Javadocs](https://www.javadoc.io/badge/uk.co.real-logic/sbe-tool.svg)](https://www.javadoc.io/doc/uk.co.real-logic/sbe-tool)
[![GitHub](https://img.shields.io/github/license/real-logic/simple-binary-encoding.svg)](https://github.com/real-logic/simple-binary-encoding/blob/master/LICENSE)

[![Actions Status](https://github.com/real-logic/simple-binary-encoding/workflows/Continuous%20Integration/badge.svg)](https://github.com/real-logic/simple-binary-encoding/actions)
[![CodeQL Status](https://github.com/real-logic/simple-binary-encoding/workflows/CodeQL/badge.svg)](https://github.com/real-logic/simple-binary-encoding/actions)

[SBE](https://github.com/FIXTradingCommunity/fix-simple-binary-encoding) is an OSI layer 6 presentation for 
encoding and decoding binary application messages for low-latency financial applications. This repository contains 
the reference implementations in Java, C++, Golang, C#, and Rust.

More details on the design and usage of SBE can be found on the [Wiki](https://github.com/real-logic/simple-binary-encoding/wiki).

An XSD for SBE specs can be found
[here](https://github.com/real-logic/simple-binary-encoding/blob/master/sbe-tool/src/main/resources/fpl/sbe.xsd). Please address questions about the specification to the [SBE FIX community](https://github.com/FIXTradingCommunity/fix-simple-binary-encoding).

For the latest version information and changes see the [Change Log](https://github.com/real-logic/simple-binary-encoding/wiki/Change-Log) with **downloads** at [Maven Central](http://search.maven.org/#search%7Cga%7C1%7Csbe). 

The Java and C++ SBE implementations work very efficiently with the [Aeron](https://github.com/real-logic/aeron)
messaging system for low-latency and high-throughput communications. The Java SBE implementation has a dependency on
[Agrona](https://github.com/real-logic/agrona) for its buffer implementations. Commercial support is available from
[sales@real-logic.co.uk](mailto:sales@real-logic.co.uk?subject=SBE).

Binaries
--------
Binaries and dependency information for Maven, Ivy, Gradle, and others can be found at 
[http://search.maven.org](http://search.maven.org/#search%7Cga%7C1%7Csbe).

Example for Maven:

```xml
<dependency>
    <groupId>uk.co.real-logic</groupId>
    <artifactId>sbe-all</artifactId>
    <version>${sbe.tool.version}</version>
</dependency>
```

Build
-----

Build the project with [Gradle](http://gradle.org/) using this [build.gradle](https://github.com/real-logic/simple-binary-encoding/blob/master/build.gradle) file.

Full clean build:

    $ ./gradlew

Run the Java examples

    $ ./gradlew runJavaExamples

Distribution
------------
Jars for the executable, source, and javadoc for the various modules can be found in the following directories:

    sbe-benchmarks/build/libs
    sbe-samples/build/libs
    sbe-tool/build/libs
    sbe-all/build/libs

An example to execute a Jar from command line using the 'all' jar which includes the Agrona dependency:

    java -Dsbe.generate.ir=true -Dsbe.target.language=Cpp -Dsbe.target.namespace=sbe -Dsbe.output.dir=include/gen -Dsbe.errorLog=yes -jar sbe-all/build/libs/sbe-all-${SBE_TOOL_VERSION}.jar my-sbe-messages.xml

C++ Build using CMake
---------------------
NOTE: Linux, Mac OS, and Windows only for the moment. See
[FAQ](https://github.com/real-logic/simple-binary-encoding/wiki/Frequently-Asked-Questions).
Windows builds have been tested with Visual Studio Express 12.

For convenience, the `cppbuild` script does a full clean, build, and test of all targets as a Release build.

    $ ./cppbuild/cppbuild

If you are comfortable using CMake, then a full clean, build, and test looks like:

    $ mkdir -p cppbuild/Debug
    $ cd cppbuild/Debug
    $ cmake ../..
    $ cmake --build . --clean-first
    $ ctest

__Note__: The C++ build includes the C generator. Currently, the C generator is a work in progress.

Golang Build
------------

First build using Gradle to generate the SBE jar and then use it to generate the golang code for testing.

    $ ./gradlew
    $ ./gradlew generateGolangCodecs

For convenience on Linux, a gnu Makefile is provided that runs some tests and contains some examples.

    $ cd gocode
    # make # test, examples, bench

Users of golang generated code should see the [user
documentation](https://github.com/real-logic/simple-binary-encoding/wiki/Golang-User-Guide).

Developers wishing to enhance the golang generator should see the [developer
documentation](https://github.com/real-logic/simple-binary-encoding/blob/master/gocode/README.md)

C# Build
--------
Users of CSharp generated code should see the [user documentation](https://github.com/real-logic/simple-binary-encoding/wiki/Csharp-User-Guide).

Developers wishing to enhance the CSharp generator should see the [developer documentation](https://github.com/real-logic/simple-binary-encoding/blob/master/csharp/README.md)

Rust Build
------------
The SBE Rust generator will produce 100% safe rust crates (no `unsafe` code will be generated).  Generated crates do
not have any dependencies on any libraries (including no SBE libraries). If you don't yet have Rust installed 
see [Rust: Getting Started](https://www.rust-lang.org/learn/get-started)

Generate the Rust codecs

    $ ./gradlew generateRustCodecs

Run the Rust test from Gradle

    $ ./gradlew runRustTests

Or run test directly with `Cargo`

    $ cd rust
    $ cargo test

License (See LICENSE file for full license)
-------------------------------------------
Copyright 2013-2024 Real Logic Limited.  
Copyright 2017 MarketFactory Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
