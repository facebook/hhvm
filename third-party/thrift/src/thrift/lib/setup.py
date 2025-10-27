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

import sys

import Cython
from Cython.Build import cythonize
from Cython.Compiler import Options
from setuptools import Extension, setup

Options.fast_fail = True

if "--api-only" in sys.argv:
    # Invoke cython compiler directly instead of calling cythonize().
    # Generating *_api.h files only requires first stage of compilation
    # # from cython source -> cpp source.
    Cython.Compiler.Main.compile(
        "thrift/python/_types.pyx",
        full_module_name="thrift.python.types",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/python/server.pyx",
        full_module_name="thrift.python.server",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/python/_util.pyx",
        full_module_name="thrift.python.util",
        cplus=True,
        language_level=3,
    )
    Cython.Compiler.Main.compile(
        "thrift/py3/_stream.pyx",
        full_module_name="thrift.py3.stream",
        cplus=True,
        language_level=3,
    )
else:
    python_lib_idx = sys.argv.index("--libpython")
    python_lib = sys.argv[python_lib_idx + 1][len("lib") : -len(".so")]
    del sys.argv[python_lib_idx : python_lib_idx + 2]

    libs = [
        # fmt, folly, glog, openssl
        "crypto",
        "fmt",
        "folly",
        "folly_python_cpp",
        "glog",
        # Thrift
        "async",
        "concurrency",
        "rpcmetadata",
        "runtime",
        "thrift-core",
        "thriftcpp2",
        "thriftprotocol",
        "thrift_python_cpp",
        "transport",
        "thriftmetadata",
    ] + [python_lib]

    common_options = {
        "language": "c++",
        "libraries": libs,
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
                "thrift/python/server/flagged/EnableResourcePoolsForPython.cpp",
            ],
            define_macros=[("__PYX_ENUM_CLASS_DECL", "")],
            **common_options,
        ),
        Extension(
            "thrift.python.stream",
            sources=["thrift/python/stream.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.types",
            sources=["thrift/python/_types.pyx"],
            **common_options,
        ),
        Extension(
            "thrift.python.util",
            sources=["thrift/python/_util.pyx"],
            **common_options,
        ),
        # thrift.py3 extension modules
        Extension(
            "thrift.py3.common",
            sources=["thrift/py3/common.pyx"],
            **common_options,
        ),
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
            sources=["thrift/py3/server.pyx"],
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
    ]

    setup(
        name="thrift",
        version="0.0.1",
        packages=[
            "thrift",
            "thrift.python",
            "thrift.python.client",
            "thrift.py3",
            "apache.thrift.metadata",
        ],
        package_data={"": ["*.pxd", "*.h"]},
        setup_requires=["cython"],
        zip_safe=False,
        ext_modules=cythonize(exts, compiler_directives={"language_level": 3}),
    )
