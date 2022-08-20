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

from folly.executor cimport get_executor

import asyncio

cdef public api void cancelAsyncGenerator(object generator):
    asyncio.get_event_loop().create_task(
        generator.aclose()
    )

cdef class ClientBufferedStream:
    """
    Base class for all ClientBufferedStream object
    """
    def __cinit__(ClientBufferedStream self, RpcOptions options):
        self._executor = get_executor()
        self._rpc_options = options

    def __aiter__(self):
        return self

    def __anext__(self):
        loop = asyncio.get_event_loop()
        future = loop.create_future()
        future.set_exception(RuntimeError("Not implemented"))
        return future

cdef class ResponseAndClientBufferedStream:
    def __iter__(self):
        yield None  # response
        yield None  # stream

cdef class ServerStream:
    """
    Base class for all Stream object
    """
    pass

cdef class ResponseAndServerStream:
    pass

cdef class ServerPublisher:
    pass
