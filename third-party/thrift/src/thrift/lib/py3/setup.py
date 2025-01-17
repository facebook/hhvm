#!/usr/bin/env python3
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

# The build_ext setuptools command is overridden such that we copy the first
# listed source - the CMake compiled Cython extension (shared library) into the
# location requested by setuptools as it builds the package.
# This allows us to benefit both from consistency in compiling against the
# thrift C++ sources and, from the installation logic embedded into Python's
# setuptools.

from distutils.file_util import copy_file

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext as _build_ext


class copy_cmake_built_libs_build_ext(_build_ext):
    def build_extension(self, ext):
        copy_file(
            ext.sources[0],
            self.get_ext_fullpath(ext.name),
            verbose=self.verbose,
            dry_run=self.dry_run,
        )


# Extension: filename created by cmake
extensions = [
    Extension("thrift.py3.common", sources=["thrift/py3/common.so"]),
    Extension("thrift.py3.types", sources=["thrift/py3/types.so"]),
    Extension("thrift.py3.exceptions", sources=["thrift/py3/exceptions.so"]),
    Extension("thrift.py3.serializer", sources=["thrift/py3/serializer.so"]),
    Extension("thrift.py3.client", sources=["thrift/py3/client.so"]),
    Extension("thrift.py3.server", sources=["thrift/py3/server.so"]),
    Extension("thrift.py3.ssl", sources=["thrift/py3/ssl.so"]),
    Extension("thrift.py3.reflection", sources=["thrift/py3/reflection.so"]),
]
setup(
    name="thrift",
    cmdclass={"build_ext": copy_cmake_built_libs_build_ext},
    version="0.0.1",
    packages=["thrift", "thrift.py3"],
    package_dir={"thrift": ".", "thrift.py3": "."},
    package_data={"": ["*.pxd", "*.h", "__init__.pyx", "*.py"]},
    zip_safe=False,
    ext_modules=extensions,
)
