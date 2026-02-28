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

# cython: c_string_type=unicode, c_string_encoding=ascii

from collections import namedtuple
from cython.operator cimport dereference as deref, preincrement as inc
from libc.stdint cimport uint32_t
from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "thrift/compiler/diagnostic.h" namespace "apache::thrift::compiler":
    cpdef enum class DiagnosticLevel "apache::thrift::compiler::diagnostic_level":
        error
        warning
        info
        debug

    cdef cppclass diagnostic:
        DiagnosticLevel level()
        string file()
        int lineno()
        string message()
        string str()

    cdef cppclass diagnostic_results:
        vector[diagnostic] diagnostics()

cdef extern from "thrift/compiler/source_location.h" namespace "apache::thrift::compiler":
    cdef cppclass source_manager:
        source_manager() except +

cdef extern from "thrift/compiler/compiler.h" namespace "apache::thrift::compiler":
    cpdef enum class CompileRetcode "apache::thrift::compiler::compile_retcode":
        success
        failure

    cdef struct compile_result:
        CompileRetcode retcode
        diagnostic_results detail

    cdef compile_result compile(vector[string], source_manager) except +


DiagnosticMessage = namedtuple(
    'DiagnosticMessage',
    ['level', 'filename', 'lineno', 'message']
)

def thrift_compile(vector[string] args):
    s_mgr = source_manager()
    result = compile(args, s_mgr)

    py_messages = []
    it = result.detail.diagnostics().const_begin()
    end = result.detail.diagnostics().const_end()
    while it != end:
        py_messages.append(
            DiagnosticMessage(
                deref(it).level(),
                deref(it).file(),
                deref(it).lineno(),
                deref(it).message(),
            )
        )
        inc(it)
    return result.retcode, py_messages
