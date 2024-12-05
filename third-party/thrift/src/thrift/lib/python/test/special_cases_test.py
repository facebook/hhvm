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


class SpecialCasesTest(unittest.TestCase):
    def test_field_named_property(self) -> None:
        # The purpose of this is to ensure that the Python parses the imported module
        # below, which will cause the failure below, if not implemented appropriately.
        #   TypeError: 'property' object is not callable
        from thrift.python.test.special_cases.thrift_abstract_types import (  # noqa: F401
            # Import TestPropertyAsField only to point to the thrift struct that is
            # defined in a manner to cause the error above.
            TestPropertyAsField,  # noqa: F401
        )

    def test_field_named_register(self) -> None:
        # The purpose of this is to ensure that the Python parses the imported module
        # below, which will cause the failure below, if not implemented appropriately.
        #   TypeError: 'property' object is not callable
        from thrift.python.test.special_cases.thrift_types import (  # noqa: F401
            # Import TestRegisterAsField only to point to the thrift struct that is
            # defined in a manner to cause the error above.
            TestRegisterAsField,  # noqa: F401
        )
