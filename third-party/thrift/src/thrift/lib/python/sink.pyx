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

import asyncio

from folly cimport cFollyPromise
from folly.executor cimport get_executor

from cython.operator import dereference
from libcpp.memory cimport make_shared, make_unique, shared_ptr
from libcpp.utility cimport move
from thrift.python.util cimport toAsyncGenerator

# TODO: (pyamane) This is here just to validate the types are correct for now
cdef void generator(object iterator, cFollyPromise[optional[unique_ptr[cIOBuf]]] promise) noexcept:
  pass

cdef class ClientSink:
  @staticmethod
  cdef create(cClientSink[unique_ptr[cIOBuf], unique_ptr[cIOBuf]]&& client):
    inst = <ClientSink>ClientSink.__new__(ClientSink)
    inst._cpp_obj = make_shared[cClientSink[unique_ptr[cIOBuf], unique_ptr[cIOBuf]]](move(client))
    return inst

  def __init__(self):
    self._cpp_obj = make_shared[cClientSink[unique_ptr[cIOBuf], unique_ptr[cIOBuf]]]()

  async def sink(self, iterator):
    # TODO: (pyamane) This is here just to validate the types are correct for now
    dereference(self._cpp_obj).sink(
      toAsyncGenerator[unique_ptr[cIOBuf]](iterator, get_executor(), generator)
    )
    return "pass"
