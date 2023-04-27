#!/usr/bin/env python
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

import sys
from distutils.command.build_ext import build_ext as _build_ext
from distutils.core import Extension as _Extension, setup
from distutils.errors import CCompilerError, CompileError, DistutilsError


class Extension(_Extension):
    """Add an `optional` kwarg, to allow skipping on failure"""

    def __init__(self, name, sources, optional=False, **kwargs):
        _Extension.__init__(self, name, sources, **kwargs)
        self.optional = optional


class build_ext(_build_ext):
    """Build extensions, but skip on failure if optional is set"""

    def build_extensions(self):
        self.check_extensions_list(self.extensions)

        for ext in self.extensions:
            try:
                self.build_extension(ext)
            except (CCompilerError, DistutilsError, CompileError) as e:
                if not ext.optional:
                    raise
                self.warn('building extension "%s" failed: %s' % (ext.name, e))


fastprotomod = Extension(
    "thrift.protocol.fastproto",
    sources=["protocol/fastproto.cpp"],
    libraries=["thriftcpp2", "thriftprotocol", "folly"],
    extra_compile_args=["-std=c++1y"],
    optional=True,
)

version_info = sys.version_info
boost_python = "boost_python-py{}{}".format(version_info[0], version_info[1])

cppservermod = Extension(
    "thrift.server.CppServerWrapper",
    sources=["server/CppServerWrapper.cpp"],
    include_dirs=["../../../"],
    libraries=[boost_python, "thriftcpp2", "folly", "wangle"],
    extra_compile_args=["-std=c++1y", "-fno-strict-aliasing"],
    optional=True,
)

setup(
    name="Thrift",
    version="0.1",
    description="Thrift Python Libraries",
    author="Thrift Developers",
    author_email="thrift-dev@incubator.apache.org",
    url="http://incubator.apache.org/thrift/",
    license="Apache License 2.0",
    packages=[
        "thrift",
        "thrift.protocol",
        "thrift.transport",
        "thrift.server",
        "thrift.util",
    ],
    package_dir={"thrift": "."},
    ext_modules=[fastprotomod, cppservermod],
    cmdclass={"build_ext": build_ext},
)
