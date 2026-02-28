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
        from thrift.python.test.special_cases.thrift_types import (
            TestPropertyAsField as TestPropertyAsFieldImmutable,
        )

        def make_abstract(
            t: TestPropertyAsField,
        ) -> TestPropertyAsField:
            return t

        class incorrect_type:
            pass

        # pyre-ignore[9]: break_unless_used_with_renamed_built_in_property is declared to have type `incorrect_type` but is used as type `str`
        _: incorrect_type = (  # noqa: F841
            make_abstract(
                TestPropertyAsFieldImmutable()
            ).break_unless_used_with_renamed_built_in_property
        )
        __: str = (  # noqa: F841
            make_abstract(
                TestPropertyAsFieldImmutable()
            ).break_unless_used_with_renamed_built_in_property
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

    def test_field_has_same_name_as_a_keyword(self) -> None:
        # The purpose of this is to ensure that the Python parses the imported module
        # below, which will cause the failure below, if not implemented appropriately.
        from thrift.python.test.special_cases.thrift_types import (  # noqa: F401
            # Import TestKeywordAsField only to point to the thrift struct that is
            # defined in a manner to cause the error above.
            TestKeywordAsField,  # noqa: F401
        )

    def test_enum_named_as_python_keyword(self) -> None:
        # Test that enums named after Python keywords are properly escaped.
        # "from" is a Python reserved keyword; the generated code must use "from_"
        # as the class name to avoid syntax errors.
        from thrift.python.test.special_cases.thrift_enums import from_

        # Verify the enum works correctly
        self.assertEqual(from_.VALUE.value, 1)

    def test_typedef_named_as_python_keyword(self) -> None:
        from thrift.python.test.special_cases.thrift_types import and_  # noqa: F401
