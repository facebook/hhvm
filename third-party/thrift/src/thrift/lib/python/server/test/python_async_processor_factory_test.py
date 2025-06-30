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

# pyre-strict

import unittest

from parameterized import parameterized
from thrift.python.server import RpcKind
from thrift.python.test.python_async_processor_factory_test import (
    PythonAsyncProcessorFactoryCTest as CTests,
)


class PythonAsyncProcessorFactoryTest(unittest.TestCase):
    @parameterized.expand(
        [
            (
                b"with_resource_pool_single_request_single_response",
                RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE,
                (
                    b"CreateMethodMetadataResult(MethodMetadataMap(with_resource_pool_single_request_single_response=MethodMetadata("
                    b"executorType=ANY "
                    b"interactionType=NONE "
                    b"rpcKind=SINGLE_REQUEST_SINGLE_RESPONSE "
                    b"priority=NORMAL "
                    b"interactionName=NONE "
                    b"createsInteraction=false "
                    b"isWildcard=false"
                    b")))"
                ),
            ),
            (
                b"with_resource_pool_single_request_no_response",
                RpcKind.SINGLE_REQUEST_NO_RESPONSE,
                (
                    b"CreateMethodMetadataResult(MethodMetadataMap(with_resource_pool_single_request_no_response=MethodMetadata("
                    b"executorType=ANY "
                    b"interactionType=NONE "
                    b"rpcKind=SINGLE_REQUEST_NO_RESPONSE "
                    b"priority=NORMAL "
                    b"interactionName=NONE "
                    b"createsInteraction=false "
                    b"isWildcard=false"
                    b")))"
                ),
            ),
            (
                b"with_resource_pool_single_request_streaming_response",
                RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE,
                (
                    b"CreateMethodMetadataResult(MethodMetadataMap(with_resource_pool_single_request_streaming_response=MethodMetadata("
                    b"executorType=ANY "
                    b"interactionType=NONE "
                    b"rpcKind=SINGLE_REQUEST_STREAMING_RESPONSE "
                    b"priority=NORMAL "
                    b"interactionName=NONE "
                    b"createsInteraction=false "
                    b"isWildcard=false"
                    b")))"
                ),
            ),
        ],
    )
    def test_create_method_metadata(
        self,
        function_name: bytes,
        rpc_kind: RpcKind,
        expected: bytes,
    ) -> None:
        """
        Even though the service creator does not interact directly with
        CreateMethodMetadata, this test is in python because the cpp
        code is in a cython library and I wasn't able to successfully
        make the cpp test depend on a cython_library target.
        """
        CTests(self).test_create_method_metadata(
            function_name,
            rpc_kind,
            expected,
        )
