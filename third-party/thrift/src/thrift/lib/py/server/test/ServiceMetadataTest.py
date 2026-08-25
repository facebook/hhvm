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


import unittest

from apache.thrift.metadata.thrift_types import ThriftServiceMetadataResponse
from thrift.python.serializer import deserialize, Protocol
from thrift.server.TCppServer import _ProcessorAdapter
from thrift.server.test.slots_throwing_service import SlotsThrowingService


class Handler(SlotsThrowingService.Iface):
    def throwUserException(self):
        pass

    def throwUncaughtException(self, msg):
        return 0


class ServiceMetadataTest(unittest.TestCase):
    def test_get_service_metadata_returns_valid_bytes(self):
        handler = Handler()
        processor = SlotsThrowingService.Processor(handler)
        adapter = _ProcessorAdapter(processor)
        result = adapter.get_service_metadata()
        self.assertIsNotNone(result)
        self.assertIsInstance(result, bytes)
        self.assertGreater(len(result), 0)

    def test_metadata_contains_service_functions(self):
        handler = Handler()
        processor = SlotsThrowingService.Processor(handler)
        adapter = _ProcessorAdapter(processor)
        serialized = adapter.get_service_metadata()
        self.assertIsNotNone(serialized)
        response = deserialize(
            ThriftServiceMetadataResponse, serialized, Protocol.BINARY
        )
        service_info = response.context.service_info
        self.assertIn("SlotsThrowingService", service_info.name)
        func_names = {f.name for f in service_info.functions}
        self.assertIn("throwUserException", func_names)
        self.assertIn("throwUncaughtException", func_names)
