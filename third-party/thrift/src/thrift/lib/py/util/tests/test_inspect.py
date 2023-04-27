#!/usr/bin/env python3
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
from typing import Dict, List

import thrift.util.inspect as thrift_inspect
from thrift.test.child import ChildService
from thrift.test.parent import ParentService


class Test(unittest.TestCase):
    def test_service_hierarchy(self) -> None:
        self.assertEqual(
            [ParentService], thrift_inspect.get_service_module_hierarchy(ParentService)
        )
        self.assertEqual(
            [ChildService, ParentService],
            thrift_inspect.get_service_module_hierarchy(ChildService),
        )

        with self.assertRaises(TypeError) as ctx:
            thrift_inspect.get_service_module_hierarchy(unittest)
        self.assertIn("does not look like a thrift service module", str(ctx.exception))

    def test_list_service_functions(self) -> None:
        functions = thrift_inspect.list_service_functions(ParentService)
        self.assertEqual(
            ["getStatus() -> string"], self._functions_to_strings(functions)
        )

        functions = thrift_inspect.list_service_functions(ChildService)
        self.maxDiff = 4096
        self.assertEqual(
            [
                "doSomething("
                "message: string, "
                "input1: thrift.test.child.ttypes.SomeStruct, "
                "input2: thrift.test.child.ttypes.SomeStruct, "
                "e: thrift.test.child.ttypes.AnEnum, "
                "data: binary"
                ") -> thrift.test.child.ttypes.SomeStruct",
                "mightFail(message: string) -> i32",
                "shoutIntoTheWind(message: string) [oneway]",
            ],
            self._functions_to_strings(functions),
        )

    def test_list_all_functions(self) -> None:
        functions = thrift_inspect.list_all_functions(ParentService)
        self.assertEqual(
            ["getStatus() -> string"], self._functions_to_strings(functions)
        )

        functions = thrift_inspect.list_all_functions(ChildService)
        self.maxDiff = 4096
        self.assertEqual(
            [
                "doSomething("
                "message: string, "
                "input1: thrift.test.child.ttypes.SomeStruct, "
                "input2: thrift.test.child.ttypes.SomeStruct, "
                "e: thrift.test.child.ttypes.AnEnum, "
                "data: binary"
                ") -> thrift.test.child.ttypes.SomeStruct",
                "getStatus() -> string",
                "mightFail(message: string) -> i32",
                "shoutIntoTheWind(message: string) [oneway]",
            ],
            self._functions_to_strings(functions),
        )

    def test_get_function_info(self) -> None:
        fn_info = thrift_inspect.get_function_info(ChildService, "mightFail")
        self.assertEqual("mightFail(message: string) -> i32", str(fn_info))

        fn_info = thrift_inspect.get_function_info(ChildService, "getStatus")
        self.assertEqual("getStatus() -> string", str(fn_info))

        with self.assertRaises(ValueError):
            thrift_inspect.get_function_info(
                ChildService, "getStatus", search_parent_interfaces=False
            )

    def _functions_to_strings(
        self, functions: Dict[str, thrift_inspect.Function]
    ) -> List[str]:
        return [str(fn_info) for fn_name, fn_info in sorted(functions.items())]
