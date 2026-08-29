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

from libcpp.memory cimport make_shared, shared_ptr, static_pointer_cast
from libcpp.utility cimport move as cmove
from libcpp.vector cimport vector as cvector
from testing.base_service_only.thrift_clients import BaseService
from testing.base_service_only.thrift_services import BaseServiceInterface
from thrift.python.client import get_client
from thrift.python.server import ThriftServer

from thrift.python.server_impl.async_processor cimport (
    cAsyncProcessorFactory,
    AsyncProcessorFactory,
)
from thrift.python.server_impl.python_async_processor cimport PythonAsyncProcessorFactory
from thrift.python.types cimport ServiceInterface as cServiceInterface


cdef extern from "thrift/lib/cpp2/async/MultiplexAsyncProcessor.h" namespace "apache::thrift":
    cdef cppclass cMultiplexAsyncProcessorFactory "apache::thrift::MultiplexAsyncProcessorFactory"(cAsyncProcessorFactory):
        cMultiplexAsyncProcessorFactory(
            cvector[shared_ptr[cAsyncProcessorFactory]] processorFactories,
        ) except +


class Handler(BaseServiceInterface):
    async def theAnswer(self) -> int:
        return 42


cdef AsyncProcessorFactory compose_processor_factory(
    PythonAsyncProcessorFactory factory,
):
    cdef shared_ptr[cAsyncProcessorFactory] inner_factory = factory._cpp_obj
    cdef cvector[shared_ptr[cAsyncProcessorFactory]] factories
    cdef AsyncProcessorFactory composed_factory = (
        AsyncProcessorFactory.__new__(AsyncProcessorFactory)
    )
    factories.push_back(inner_factory)
    composed_factory._cpp_obj = static_pointer_cast[
        cAsyncProcessorFactory,
        cMultiplexAsyncProcessorFactory,
    ](
        make_shared[cMultiplexAsyncProcessorFactory](cmove(factories))
    )
    return composed_factory


async def round_trip(AsyncProcessorFactory factory):
    server = ThriftServer(factory, ip="::1")
    serve_task = asyncio.create_task(server.serve())
    try:
        address = await asyncio.wait_for(server.get_address(), timeout=5.0)
        assert address.ip is not None
        assert address.port is not None
        async with get_client(
            BaseService,
            host=str(address.ip),
            port=address.port,
        ) as client:
            response = await asyncio.wait_for(client.theAnswer(), timeout=5.0)
        server.stop()
        await asyncio.wait_for(serve_task, timeout=5.0)
        return response
    except BaseException:
        server.stop()
        await asyncio.gather(serve_task, return_exceptions=True)
        raise


cdef class PythonAsyncProcessorFactoryCustomerApiCTest:
    """Exercises Cython APIs that customers call directly."""

    def __cinit__(self, object unit_test):
        self.ut = unit_test

    async def test_unary_rpc_round_trips(self):
        # GIVEN
        cdef cServiceInterface handler = Handler()
        cdef PythonAsyncProcessorFactory factory = (
            PythonAsyncProcessorFactory.create(handler)
        )
        expected = 42

        # WHEN
        actual = await round_trip(factory)

        # THEN
        self.ut.assertEqual(expected, actual)

    async def test_unary_rpc_round_trips_through_composed_factory(self):
        # GIVEN
        cdef cServiceInterface handler = Handler()
        cdef PythonAsyncProcessorFactory factory = (
            PythonAsyncProcessorFactory.create(handler)
        )
        composed_factory = compose_processor_factory(factory)
        expected = 42

        # WHEN
        actual = await round_trip(composed_factory)

        # THEN
        self.ut.assertEqual(expected, actual)
