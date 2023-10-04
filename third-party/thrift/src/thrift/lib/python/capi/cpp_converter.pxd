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


cdef extern from "thrift/lib/python/capi/cpp_converter.h" namespace "apache::thrift::python::capi":
    # raises python error on failure
    cdef object cpp_to_python[ThriftT](const ThriftT&)
    # preserves python error raised on failure (e.g., OverflowError).
    # Prefer this to python_to_cpp_throws
    cdef ThriftT python_to_cpp[ThriftT](object) except *
    # throws C++ std::runtime_error on failure, which cython translates
    cdef ThriftT python_to_cpp_throws[ThriftT](object) except +
