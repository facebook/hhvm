#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import CustomErrorServer

class TestMissOnErrorArithOps(McrouterTestCase):
    config = './mcrouter/test/test_miss_on_error_arith_ops.json'
    extra_args = []

    def setUp(self):
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_behaviour(self):
        key = 'foo:test:key:1234'
        self.assertIsNone(self.mcrouter.set(key, 2))
        self.assertIsNone(self.mcrouter.incr(key, 2))
        self.assertIsNone(self.mcrouter.decr(key, 2))


class TestDisableMissOnErrorArithOps(McrouterTestCase):
    config = './mcrouter/test/test_miss_on_error_arith_ops.json'
    error = 'CLIENT_ERROR cannot increment or decrement non-numeric value'
    key = 'foo:amcrn'

    def setUp(self):
        self.mc = self.add_server(CustomErrorServer(error_message=self.error))
        extra_args = ['--disable-miss-on-arith-errors']
        self.mcrouter = self.add_mcrouter(self.config, extra_args=extra_args)

    def test_incr_error(self):
        reply = self.mcrouter.incr(self.key, 1)
        self.assertEqual(self.error, reply.rstrip())

    def test_decr_error(self):
        reply = self.mcrouter.decr(self.key, 2)
        self.assertEqual(self.error, reply.rstrip())
