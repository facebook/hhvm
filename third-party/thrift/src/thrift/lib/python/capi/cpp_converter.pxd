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

"""
    Note these functions are considered INTERNAL and no longer recommended for end use.
    Please use the python_capi converter cython module, imported like:
        from cimport <py3_namespace>.<module_name>.thrift_converter cimport (
            <TypePythonName>_convert_to_cpp,
            <TypePythonName>_from_cpp,
        )
    Using the converter module requires only a single import cython dep:
        "//path/to/package:<thrift_module_name>-python_capi-types-converter"
"""


cdef extern from "thrift/lib/python/capi/cpp_converter.h" namespace "apache::thrift::python::capi":
    # raises python error on failure
    cdef object cpp_to_python[ThriftT, NamespaceTag](const ThriftT&)
    # preserves python error raised on failure (e.g., OverflowError).
    # Prefer this to python_to_cpp_throws
    cdef ThriftT python_to_cpp[ThriftT, NamespaceTag](object) except *
    # throws C++ std::runtime_error on failure, which cython translates
    cdef ThriftT python_to_cpp_throws[ThriftT, NamespaceTag](object) except +
