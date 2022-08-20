#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestLargeObj(McrouterTestCase):
    config_list = './mcrouter/test/test_named_handles_list.json'
    config_obj = './mcrouter/test/test_named_handles_obj.json'
    config_unknown = './mcrouter/test/test_unknown_name_handle.json'

    def setUp(self):
        self.mc1 = self.add_server(Memcached())
        self.mc2 = self.add_server(Memcached())

    def test_named_handles_list(self):
        mcrouter = self.add_mcrouter(self.config_list)
        # NullRoute returns NOT_STORED
        self.assertFalse(mcrouter.set('test', 'value'))
        self.assertEqual(self.mc1.get('test'), 'value')
        self.assertEqual(self.mc2.get('test'), 'value')

    def test_named_handles_obj(self):
        mcrouter = self.add_mcrouter(self.config_obj)
        # NullRoute returns NOT_STORED
        self.assertFalse(mcrouter.set('test2', 'value'))
        self.assertEqual(self.mc1.get('test2'), 'value')
        self.assertEqual(self.mc2.get('test2'), 'value')


class TestUnknownNamedHandle(McrouterTestCase):
    config_unknown = './mcrouter/test/test_unknown_name_handle.json'
    def test_unknown_named_handles(self):
        mcrouter = self.add_mcrouter(self.config_unknown)
        self.assertFalse(self._is_mcrouter_running(mcrouter))
        mcrouter.dump()
        log = mcrouter.get_log()
        expectedError = False
        if 'Unknown type or Missing Name Handle : D' in log:
            expectedError = True
        self.assertTrue(expectedError)

