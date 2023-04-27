#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestLoggingRoute(McrouterTestCase):
    config_mc = './mcrouter/test/test_logging_route_mc.json'
    config = './mcrouter/test/test_logging_route_server.json'
    extra_args = ['--retain-source-ip', '--enable-logging-route']

    def setUp(self):
        self.mc = self.add_server(Memcached())
        self.mcrouter_mc = self.add_mcrouter(
            self.config_mc, extra_args=self.extra_args, bg_mcrouter=True)
        self.mcrouter = self.add_mcrouter(
            self.config, extra_args=self.extra_args)

    def test_basic(self):
        key = 'foo'
        value = 'value'
        self.mcrouter.set(key, value)
        self.assertEqual(self.mcrouter.get(key), value)
        self.assertEqual(self.mc.get(key), value)
