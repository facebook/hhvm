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

from folly cimport cFollyExecutor, cFollyPromise
from folly.async_generator cimport cAsyncGenerator

from thrift.py3.std_libcpp cimport optional

cdef extern from "thrift/lib/python/util.h" namespace "::thrift::python":
  cAsyncGenerator[TChunk] toAsyncGenerator[TChunk](
    object,
    cFollyExecutor*,
    void(*)(object, cFollyPromise[optional[TChunk]])
  )

cdef public api void cancelAsyncGenerator(object generator)
