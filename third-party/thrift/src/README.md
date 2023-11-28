Facebook Thrift
---------------

[![Support Ukraine](https://img.shields.io/badge/Support-Ukraine-FFD500?style=flat&labelColor=005BBB)](https://opensource.fb.com/support-ukraine)
[![Linux Build Status](
  https://github.com/facebook/fbthrift/actions/workflows/getdeps_linux.yml/badge.svg)](
  https://github.com/facebook/fbthrift/actions/workflows/getdeps_linux.yml)
[![macOS Build Status](
  https://github.com/facebook/fbthrift/actions/workflows/getdeps_mac.yml/badge.svg)](
  https://github.com/facebook/fbthrift/actions/workflows/getdeps_mac.yml)
[![Windows Build Status](
  https://github.com/facebook/fbthrift/actions/workflows/getdeps_windows.yml/badge.svg)](
  https://github.com/facebook/fbthrift/actions/workflows/getdeps_windows.yml)
<img src="thrift/website/static/img/logo.svg"
     alt="Facebook Thrift Logo" align="right" width="20%"/>

Thrift is a serialization and RPC framework for service communication. Thrift enables these features in all major languages, and there is strong support for C++, Python, Hack, and Java. Most services at Facebook are written using Thrift for RPC, and some storage systems use Thrift for serializing records on disk.

Facebook Thrift is not a distribution of [Apache Thrift](https://thrift.apache.org/). This is an evolved internal branch of Thrift that Facebook re-released to open source community in February 2014. Facebook Thrift was originally released closely tracking Apache Thrift but is now evolving in new directions. In particular, the compiler was rewritten from scratch and the new implementation features a fully asynchronous Thrift server. Read more about these improvements in the [ThriftServer documentation](https://github.com/facebook/fbthrift/blob/main/thrift/doc/languages/cpp/cpp2.md).

You can also learn more about this project in the original Facebook Code [blog post](https://code.facebook.com/posts/1468950976659943/under-the-hood-building-and-open-sourcing-fbthrift/).

Table of Contents
=========================

* [About Thrift](#about-thrift)
  * [A Code Generator](#a-code-generator)
  * [A Serialization Framework](#a-serialization-framework)
  * [An RPC Framework](#an-rpc-framework)
* [Building](#building)
  * [Dependencies](#dependencies)
  * [Build](#build)
  * [Thrift Files](#thrift-files)
* [C++ Static Reflection](#c-static-reflection)
* [C++ Server Metrics](#c-server-metrics)

## About Thrift
At a high level, Thrift is three major things:

### A Code Generator

Thrift has a code generator which generates data structures that can be serialized using Thrift, and client and server stubs for RPC, in different languages.

### A Serialization Framework

Thrift has a set of protocols for serialization that may be used in different languages to serialize the generated structures created from the code generator.

### An RPC Framework

Thrift has a framework to frame messages to send between clients and servers and to call application-defined functions when receiving messages in different languages.

There are several key goals for these components:
* Ease of use:
  Thrift takes care of the boilerplate of serialization and RPC and enables the developer to focus on the schema of the system's serializable types and on the interfaces of the system's RPC services.

* Cross-language support:
  Thrift enables intercommunication between different languages. For example, a Python client communicating with a C++ server.

* Performance:
  Thrift structures and services enable fast serialization and deserialization, and its RPC protocol and frameworks are designed with performance as a feature.

* Backwards compatibility:
  Thrift allows fields to be added to and removed from serializable types in a manner that preserves backward and forward compatibility.

## Building

### Dependencies

On Linux or MacOS (with [Homebrew](https://brew.sh/) installed) you can install
system dependencies to avoid building them:

    # Clone the repo
    git clone https://github.com/facebook/fbthrift
    # Install dependencies
    cd fbthrift
    ./build/fbcode_builder/getdeps.py install-system-deps --recursive fbthrift

On other platforms or if on Linux and without system dependencies `getdeps.py`
will mostly download and build them for you during the build step.

Some of the dependencies `getdeps.py` uses and installs are:

**System**:
[Boost](https://www.boost.org),
[CMake](https://cmake.org),
[OpenSSLv1.0.2g](https://www.openssl.org),
[PThreads](https://computing.llnl.gov/tutorials/pthreads),
[Python](https://www.python.org/), and
[Zlib](https://zlib.net)

**External**:
[{fmt}](https://github.com/fmtlib/fmt),
[GFlags](https://github.com/gflags/gflags),
[GLog](https://github.com/google/glog), and
[GTest and GMock](https://github.com/google/googletest)

**Facebook**:
[Fizz](https://github.com/facebookincubator/fizz),
[Folly](https://github.com/facebook/folly),
[Wangle](https://github.com/facebook/wangle), and
[Zstd](https://github.com/facebook/zstd)

The Thrift compiler only depends on Boost, CMake and {fmt}.

### Build

    # Clone the repo
    git clone https://github.com/facebook/fbthrift
    cd fbthrift
    # Build, using system dependencies if available
    ./build/fbcode_builder/getdeps.py --allow-system-packages build fbthrift

`getdeps.py` will invoke cmake etc and put output in its scratch area (you can see in logs, and can override with `--scratch-path`):

* `installed/fbthrift/bin/thrift1`: The Thrift compiler binary to generate client and
  server code.
* `installed/fbthrift/lib/libthriftcpp2.a`: Library for clients and servers.

If you want to invoke `cmake` again to iterate, there is an helpful `run_cmake.py` output in the scratch `build/fbthrift` directory.

CMake options:

* `THRIFT_COMPILER_ONLY`:
  specifies whether to build only the Thrift compiler (OFF by default)
* `enable_tests`:
  specifies whether to enable tests

### Thrift Files

When using thrift and the CMake build system, include: `ThriftLibrary.cmake` in
your project. This includes the following macro to help building Thrift files:

    thrift_library(
      #file_name
      #services
      #language
      #options
      #file_path
      #output_path
    )

This generates a library called `file_name-<language>`. That is, for
`Test.thrift` compiled as cpp2, it will generate the library `Test-cpp2`.
This should be added as a dependency to any source or header file that contains
an include to generated code.

## C++ Static Reflection

Information regarding C++ Static Reflection support can be found under the [static reflection library directory](thrift/lib/cpp2/reflection/), in the corresponding [`README` file](thrift/lib/cpp2/reflection/README.md).

## C++ Server Metrics

To collect runtime stats from a Thrift server, e.g. the number of active requests/connections, the C++ Thrift server supports an observer API that installs callbacks at a set of specific execution points in the server.

To expose collected metrics out of the server process, one way is to use `fb303` interfaces, see [fb303 Github repo](https://github.com/facebook/fb303).
