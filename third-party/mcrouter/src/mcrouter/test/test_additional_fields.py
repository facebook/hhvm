#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestAdditionalFields(McrouterTestCase):
    config = './mcrouter/test/test_basic_caret.json'

    def setUp(self):
        self.mc = self.add_server(Memcached())
        self.mcrouter = self.add_mcrouter(self.config)

    def test_basic(self):
        self.assertTrue(self.mcrouter.set("abc", "def"))
        self.assertEqual(self.mcrouter.get("abc"), "def")
