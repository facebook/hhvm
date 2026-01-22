# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Do not call directly, use cmake
#
# Cython requires source files in a specific structure, the structure is
# created as tree of links to the real source files.

import os
import sys

import Cython
from Cython.Build import cythonize
from Cython.Compiler import Options
from setuptools import Extension, setup

Options.fast_fail = True

# Configure Cython include path from environment to find dependency .pxd files
# CMake sets CYTHON_INCLUDE_PATH with paths to folly, boost, and build directories
# This is needed for both --api-only and build_ext phases
cython_include_path = os.environ.get("CYTHON_INCLUDE_PATH", "")
include_dirs = []
if cython_include_path:
    include_dirs = [p for p in cython_include_path.split(":") if p]

if "--api-only" in sys.argv:
    if include_dirs:
        # Create CompilationOptions with include_path
        # This allows Cython to find folly .pxd files during compilation
        compilation_options = Options.CompilationOptions(
            Options.default_options,
            include_path=include_dirs,
        )
    else:
        compilation_options = None

    # Invoke cython compiler directly instead of calling cythonize().
    # Generating *_api.h files only requires first stage of compilation
    # # from cython source -> cpp source.
    Cython.Compiler.Main.compile(
        "thrift/python/_types.pyx",
        options=compilation_options,
        full_module_name="thrift.python.types",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/python/server/python_async_processor.pyx",
        options=compilation_options,
        full_module_name="thrift.python.server.python_async_processor",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/python/server/request_context.pyx",
        options=compilation_options,
        full_module_name="thrift.python.server.request_context",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/python/server/interceptor/service_interceptor.pyx",
        options=compilation_options,
        full_module_name="thrift.python.server.interceptor.service_interceptor",
        cplus=True,
        language_level=3,
    )
    # Compile server_impl modules (OSS compatibility shim)
    Cython.Compiler.Main.compile(
        "thrift/python/server_impl/python_async_processor.pyx",
        options=compilation_options,
        full_module_name="thrift.python.server_impl.python_async_processor",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/python/server_impl/request_context.pyx",
        options=compilation_options,
        full_module_name="thrift.python.server_impl.request_context",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/python/server_impl/interceptor/service_interceptor.pyx",
        options=compilation_options,
        full_module_name="thrift.python.server_impl.interceptor.service_interceptor",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/py3/_stream.pyx",
        options=compilation_options,
        full_module_name="thrift.py3.stream",
        cplus=True,
        language_level=3,
    )
    # Compile streaming modules to generate *_api.h headers
    Cython.Compiler.Main.compile(
        "thrift/python/streaming/sink.pyx",
        options=compilation_options,
        full_module_name="thrift.python.streaming.sink",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/python/streaming/bidistream.pyx",
        options=compilation_options,
        full_module_name="thrift.python.streaming.bidistream",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/python/streaming/py_promise.pyx",
        options=compilation_options,
        full_module_name="thrift.python.streaming.py_promise",
        cplus=True,
        language_level=3,
    )
else:
    python_lib_idx = sys.argv.index("--libpython")
    python_lib = sys.argv[python_lib_idx + 1][len("lib") : -len(".so")]
    del sys.argv[python_lib_idx : python_lib_idx + 2]

    # Library search paths from CMakeLists (passed via LIBRARY_DIRS env var)
    lib_search_paths = os.environ.get("LIBRARY_DIRS", "").split(":")
    lib_search_paths = [p for p in lib_search_paths if p]  # Filter empty strings

    # All C++ dependencies consolidated into libthrift_python_cpp.so
    # Extensions only need to link to thrift_python_cpp + system libs
    dynamic_libs = [
        "thrift_python_cpp",  # Contains all thrift/folly/wangle/fizz code
        "ssl",
        "crypto",
        "pthread",
        "aio",
        "glog",
        "gflags",
        "event",
        "lzma",
        "snappy",
        "sodium",
        "unwind",
    ]

    extra_link_args = []

    # Read LDFLAGS from environment
    ldflags_str = os.environ.get("LDFLAGS", "")
    if ldflags_str:
        import shlex

        extra_link_args.extend(shlex.split(ldflags_str))

    # RPATH for runtime library resolution
    # Use $ORIGIN so auditwheel can properly bundle libraries into the wheel.
    # $ORIGIN/.libs is where auditwheel places bundled shared libraries.
    # We also add $ORIGIN for libraries in the same directory as the extension.
    extra_link_args.append("-Wl,-rpath,$ORIGIN/.libs")
    extra_link_args.append("-Wl,-rpath,$ORIGIN")

    common_options = {
        "language": "c++",
        "include_dirs": include_dirs,
        "library_dirs": lib_search_paths,  # Tell linker where to find dynamic libraries
        "libraries": dynamic_libs + [python_lib],
        "extra_compile_args": ["-std=c++20", "-fcoroutines"],
        "extra_link_args": extra_link_args,
    }

    exts = [
        # thrift.python extension modules
        Extension(
            "thrift.python.adapter",
            sources=["thrift/python/adapter.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.client.async_client_factory",
            sources=["thrift/python/client/async_client_factory.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.client.async_client",
            sources=["thrift/python/client/async_client.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.client.omni_client",
            sources=["thrift/python/client/omni_client.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.client.request_channel",
            sources=["thrift/python/client/request_channel.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.client.ssl",
            sources=["thrift/python/client/ssl.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.client.sync_channel_factory",
            sources=["thrift/python/client/sync_channel_factory.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.client.sync_client_factory",
            sources=["thrift/python/client/sync_client_factory.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.client.sync_client",
            sources=["thrift/python/client/sync_client.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.common",
            sources=["thrift/python/common.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.converter",
            sources=["thrift/python/converter.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.exceptions",
            sources=["thrift/python/exceptions.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.mutable_containers",
            sources=["thrift/python/mutable_containers.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.mutable_exceptions",
            sources=["thrift/python/mutable_exceptions.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.mutable_serializer",
            sources=["thrift/python/mutable_serializer.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.mutable_typeinfos",
            sources=["thrift/python/mutable_typeinfos.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.mutable_types",
            sources=["thrift/python/mutable_types.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.protocol",
            sources=["thrift/python/protocol.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.serializer",
            sources=["thrift/python/serializer.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.server",
            sources=[
                "thrift/python/server.pyx",
                "thrift/python/server/PythonAsyncProcessor.cpp",
                "thrift/python/server/PythonAsyncProcessorFactory.cpp",
                "thrift/python/server/event_handler.cpp",
                "thrift/python/server/interceptor/PythonServiceInterceptor.cpp",
                "thrift/python/std_libcpp.cpp",
            ],
            define_macros=[("__PYX_ENUM_CLASS_DECL", "")],
            **common_options,
        ),
        # thrift.python.streaming extension modules
        # Each .pyx file generates to generated/*.cpp (via build_dir="generated")
        # Handwritten .cpp files (e.g., Sink.cpp, bidi_stream.cpp) are also compiled
        Extension(
            "thrift.python.streaming.stream",
            sources=["thrift/python/streaming/stream.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.streaming.py_promise",
            sources=["thrift/python/streaming/py_promise.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.streaming.python_user_exception",
            sources=[
                "thrift/python/streaming/python_user_exception.pyx",
                "thrift/python/streaming/PythonUserException.cpp",
            ],
            **common_options,
        ),
        Extension(
            "thrift.python.streaming.sink",
            sources=[
                "thrift/python/streaming/sink.pyx",
                "thrift/python/streaming/Sink.cpp",
            ],
            **common_options,
        ),
        Extension(
            "thrift.python.streaming.bidistream",
            sources=[
                "thrift/python/streaming/bidistream.pyx",
                "thrift/python/streaming/bidi_stream.cpp",
            ],
            **common_options,
        ),
        Extension(
            "thrift.python.types",
            sources=["thrift/python/_types.pyx"],
            **common_options,
        ),
        # Additional thrift.python extension modules
        Extension(
            "thrift.python.any.serializer",
            sources=["thrift/python/any/serializer.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.conformance.universal_name",
            sources=["thrift/python/conformance/universal_name.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.mutable_converter",
            sources=["thrift/python/mutable_converter.pyx"],
            **common_options,
        ),
        # NOTE: thrift.python.server.* submodule extensions are NOT built here.
        # They would conflict with the thrift.python.server extension module (.so).
        # All imports use thrift.python.server_impl.* instead (see server.pyx, py3/server.pyx).
        # thrift.python.server_impl extension modules
        Extension(
            "thrift.python.server_impl.async_processor",
            sources=["thrift/python/server_impl/async_processor.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.server_impl.python_async_processor",
            sources=["thrift/python/server_impl/python_async_processor.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.server_impl.request_context",
            sources=["thrift/python/server_impl/request_context.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.server_impl.interceptor.server_module",
            sources=["thrift/python/server_impl/interceptor/server_module.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.server_impl.interceptor.service_interceptor",
            sources=["thrift/python/server_impl/interceptor/service_interceptor.pyx"],
            **common_options,
        ),
        # thrift.py3 extension modules
        Extension(
            "thrift.py3.exceptions",
            sources=["thrift/py3/exceptions.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.py3.serializer",
            sources=["thrift/py3/serializer.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.py3.server",
            sources=[
                "thrift/py3/server.pyx",
                "thrift/python/server/event_handler.cpp",
            ],
            **common_options,
        ),
        Extension(
            "thrift.py3.stream",
            sources=["thrift/py3/_stream.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.py3.types",
            sources=["thrift/py3/types.pyx"],
            **common_options,
        ),
        # Additional thrift.py3 extension modules
        Extension(
            "thrift.py3.builder",
            sources=["thrift/py3/builder.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.py3.client",
            sources=["thrift/py3/client.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.py3.converter",
            sources=["thrift/py3/converter.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.py3.metadata",
            sources=["thrift/py3/metadata.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.py3.reflection",
            sources=["thrift/py3/reflection.pyx"],
            **common_options,
        ),
    ]

    packages = [
        "thrift",
        "thrift.python",
        "thrift.python.any",
        "thrift.python.client",
        "thrift.python.conformance",
        "thrift.python.server_impl",
        "thrift.python.server_impl.interceptor",
        # "thrift.python.streaming",  # DISABLED FOR NOW
        "thrift.py3",
        "thrift.lib",
        "thrift.lib.python",
        "apache.thrift.metadata",
        # Folly Python bindings bundled into thrift wheel
        # (folly doesn't build a wheel yet)
        "folly",
    ]

    # Test-only package (directory created by test symlinks)
    if os.environ.get("THRIFT_BUILD_TESTS", "0") == "1":
        packages.append("thrift.lib.python.client")

    setup(
        name="thrift",
        version="0.0.1",
        packages=packages,
        package_data={"": ["*.pxd", "*.h", "*.so"]},
        setup_requires=["cython"],
        zip_safe=False,
        ext_modules=cythonize(
            exts,
            compiler_directives={"language_level": 3},
            build_dir="generated",
            include_path=include_dirs,  # Allow Cython to find folly .pxd files
        ),
    )
