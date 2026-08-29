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

class PythonAsyncProcessorFactoryCustomerApiCTest:
    def __init__(self, unit_test: unittest.TestCase) -> None: ...
    async def test_unary_rpc_round_trips(self) -> None: ...
    async def test_unary_rpc_round_trips_through_composed_factory(self) -> None: ...
