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
from libc.stdint cimport int32_t
from libcpp.memory cimport shared_ptr, make_shared

cimport testing.types
import testing.types

def simulate_HardError(str errortext, int32_t code):
    cdef shared_ptr[testing.types.cHardError] c_inst
    c_inst = make_shared[testing.types.cHardError]()
    deref(c_inst).errortext_ref().assign(<bytes>errortext.encode('utf-8'))
    deref(c_inst).code_ref().assign(code)
    return testing.types.HardError._fbthrift_create(c_inst)

def simulate_UnusedError(str message):
    cdef shared_ptr[testing.types.cUnusedError] c_inst
    c_inst = make_shared[testing.types.cUnusedError]()
    deref(c_inst).message_ref().assign(<bytes>message.encode('utf-8'))
    return testing.types.UnusedError._fbthrift_create(c_inst)
