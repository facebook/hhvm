#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import random

from mcrouter.test.MCProcess import MockMemcached, MockMemcachedThrift
from mcrouter.test.MCProcess import MockMemcachedDual
from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import DeadServer
from mcrouter.test.mock_servers import SleepServer


def randstring(n):
    s = "0123456789abcdef"
    ans = ""
    for _ in range(n):
        ans += random.choice(s)
    return ans


class TestMcrouterSanityMock(McrouterTestCase):
    config = './mcrouter/test/test_ascii.json'

    def make_memcached(self):
        return MockMemcached()

    def setUp(self):
        mc_ports = [
            11510, 11511, 11512, 11513,
            11520, 11521, 11522, 11523,
            11530, 11531, 11532, 11533,
            11541]
        mc_gut_port = 11540
        tmo_port = 11555

        # have to do these before starting mcrouter
        self.mcs = [self.add_server(self.make_memcached(), logical_port=port)
                    for port in mc_ports]
        self.mc_gut = self.add_server(self.make_memcached(),
                                      logical_port=mc_gut_port)
        self.mcs.append(self.mc_gut)

        self.add_server(SleepServer(), logical_port=tmo_port)

        for port in [65532, 65522]:
            self.add_server(DeadServer(), logical_port=port)

        self.mcrouter = self.add_mcrouter(self.config)

    def data(self, n):
        """ generate n random (key, value) pairs """
        prefixes = ['foo:', 'bar:', 'baz:', 'wc:', 'lat:']
        keys = [random.choice(prefixes) + randstring(random.randint(3, 10))
                for i in range(n)]
        keys = list(set(keys))
        vals = [randstring(random.randint(3, 10)) for i in range(len(keys))]
        return zip(keys, vals)

    def test_server_error_message(self):
        # Test involes trying to get a key that triggers a server error
        m = self.mcrouter
        exp = b"SERVER_ERROR returned error msg with binary data \xdd\xab\r\n"
        bad_command = 'set __mockmc__.trigger_server_error 0 0 1\r\n0\r\n'

        self.assertEqual(m.issue_command(bad_command), exp.decode())

    def test_reject_policy(self):
        # Test the reject policy
        m = self.mcrouter
        exp = "SERVER_ERROR reject\r\n"
        bad_command = 'set rej:foo 0 0 3\r\nrej\r\n'

        self.assertEqual(m.issue_command(bad_command), exp)

    def test_metaget_age(self):
        self.mcrouter.set("key", "value")
        # Allow 5 seconds for super slow environments.
        self.assertLess(int(self.mcrouter.metaget("key")['age']), 5)
        self.mcrouter.set("unknown_age", "value")
        self.assertEqual(self.mcrouter.metaget("unknown_age")['age'], "unknown")


class TestCaretSanityMock(TestMcrouterSanityMock):
    config = './mcrouter/test/test_caret.json'


class TestThriftSanityMock(TestMcrouterSanityMock):
    config = './mcrouter/test/test_thrift.json'

    def make_memcached(self):
        return MockMemcachedThrift()


class TestDualCaretSanityMock(TestMcrouterSanityMock):
    config = './mcrouter/test/test_caret.json'

    def make_memcached(self):
        return MockMemcachedDual(mcrouterUseThrift=False)
