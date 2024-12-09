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


# DO_BEFORE(satishvk, 20250130): Remove flags related to abstract types after launch.
# When you remove the flag, these tests will fail. Remove them as well.
class AbstractTypesFlagsTest(unittest.TestCase):
    def test_abstract_types_always_generated(self) -> None:
        from thrift.python.test.abstract_types_flags.thrift_abstract_types import (  # noqa
            TestStruct,
        )

    def test_abstract_types_disabled(self) -> None:
        from thrift.python.test.abstract_types_flags.thrift_abstract_types import (
            TestStruct as TestStructAbstract,
        )
        from thrift.python.test.abstract_types_flags.thrift_mutable_types import (
            TestStruct as TestStructMutable,
        )
        from thrift.python.test.abstract_types_flags.thrift_types import (
            TestStruct as TestStructImmutable,
        )

        self.assertFalse(issubclass(TestStructImmutable, TestStructAbstract))
        self.assertFalse(issubclass(TestStructMutable, TestStructAbstract))
        self.assertNotIsInstance(TestStructImmutable(), TestStructAbstract)
        self.assertNotIsInstance(TestStructMutable(), TestStructAbstract)
