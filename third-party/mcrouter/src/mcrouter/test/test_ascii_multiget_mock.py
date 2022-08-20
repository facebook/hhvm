#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import MockMemcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestMcrouterAsciiMultigetSanityMock(McrouterTestCase):
    config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    extra_args = ['--disable-miss-on-get-errors']

    def setUp(self):
        self.add_server(MockMemcached(), logical_port=12345)
        self.mcrouter = self.add_mcrouter(self.config, None, self.extra_args)

    def test_multiget_sanity(self):
        m = self.mcrouter

        self.assertTrue(m.set('test:multiget:1', 'A'))
        # Test multiget with one miss.
        self.assertEqual(m.get(['test:multiget:1', 'test:multiget:2']),
                          {'test:multiget:1': 'A',
                           'test:multiget:2': None})
        # Test multiget with one timeout.
        self.assertEqual(m.get(['test:multiget:1', '__mockmc__.want_timeout']),
                          'SERVER_ERROR timeout')
