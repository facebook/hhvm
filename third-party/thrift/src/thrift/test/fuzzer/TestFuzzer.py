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

# pyre-unsafe

from __future__ import absolute_import, division, print_function, unicode_literals

import unittest

from fuzz import DerivedTestService, TestService
from thrift.util.fuzzer import Service


class TestServiceWrapper(unittest.TestCase):
    def testServiceLoadMethods(self):
        service = Service(None, None, TestService)
        service.load_methods()
        service_methods = service.get_methods()

        self.assertEqual(len(service_methods), 3)
        self.assertIsNotNone(service_methods["lookup"])
        self.assertIsNotNone(service_methods["nested"])
        self.assertIsNotNone(service_methods["listStruct"])

        method_args = service_methods["lookup"]["args_class"].thrift_spec

        # args_class has an additional None value in the beginning before the args
        self.assertEqual(len(method_args), 3)

        self.assertEqual(method_args[1][2], "root")
        self.assertEqual(method_args[2][2], "key")

        self.assertEqual(len(service_methods["lookup"]["thrift_exceptions"]), 2)

    def testServiceFilterMethods(self):
        service = Service(None, None, TestService)
        service.load_methods()
        service_methods = service.get_methods(["lookup", "nested"])

        self.assertEqual(len(service_methods), 2)
        self.assertIsNotNone(service_methods["lookup"])
        self.assertIsNotNone(service_methods["nested"])

    def testServiceExcludeIfaces(self):
        service = Service(None, None, TestService)
        service.load_methods(exclude_ifaces=[TestService.Iface])
        service_methods = service.get_methods()

        self.assertEqual(len(service_methods), 0)

    def testServiceInheritance(self):
        service = Service(None, None, DerivedTestService)
        service.load_methods()
        service_methods = service.get_methods()

        self.assertIn("lookup", service_methods)
