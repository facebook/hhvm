#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestServiceInfo(McrouterTestCase):
    config = './mcrouter/test/test_service_info.json'

    def setUp(self):
        self.mc1 = self.add_server(Memcached())
        self.mc2 = self.add_server(Memcached())
        self.mcrouter = self.add_mcrouter(self.config)

    def test_route_format(self):
        ports = [self.mc1.port, self.mc2.port]
        # Test a key 'abc,def' with a comma
        route = self.mcrouter.get("__mcrouter__.route(set,abc,def)")
        parts = route.split("\r\n")
        self.assertEqual(len(parts), 2)
        for i, part in enumerate(parts):
            host, port = part.split(":")
            self.assertEqual(host, "127.0.0.1")
            self.assertEqual(port, str(ports[i]))

    def test_hostid(self):
        hostid = self.mcrouter.get("__mcrouter__.hostid")
        self.assertEqual(str(int(hostid)), hostid)
        self.assertEqual(hostid, self.mcrouter.get("__mcrouter__.hostid"))

    def _check_route_handles(self, op):
        # Test a key 'abc,def' with a comma
        cmd = "__mcrouter__.route_handles({},abc,def)".format(op)
        rh = self.mcrouter.get(cmd)
        self.assertTrue("root" in rh)
        self.assertTrue("127.0.0.1" in rh)
        self.assertTrue(str(self.mc1.port) in rh)
        self.assertTrue(str(self.mc2.port) in rh)

    def test_route_handles(self):
        self._check_route_handles("get")
        self._check_route_handles("set")
        self._check_route_handles("delete")
