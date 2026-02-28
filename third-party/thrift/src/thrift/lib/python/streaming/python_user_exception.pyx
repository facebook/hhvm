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

from cython.operator cimport dereference as deref
from libcpp.string cimport string
from libcpp.utility cimport move as cmove
from libcpp.memory cimport make_unique

from folly.iobuf cimport from_unique_ptr


cdef class PythonUserException(Exception):
    def __init__(self, type_: str, reason: str, buf: IOBuf) -> None:
        self._cpp_obj = make_unique[cPythonUserException](
            <string>type_.encode('UTF-8'),
            <string>reason.encode('UTF-8'),
            cmove(buf._ours)
        )

cdef IOBuf extractPyUserExceptionIOBuf(cFollyExceptionWrapper& ew):
    cdef unique_ptr[cIOBuf] buf = extractBufFromPythonUserException(ew)
    if not buf:
        return None

    return from_unique_ptr(cmove(buf))
    
