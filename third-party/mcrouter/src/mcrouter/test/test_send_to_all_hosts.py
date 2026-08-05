#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

from carbon.carbon_result.thrift_types import Result
from mcrouter.test.MCProcess import MockMemcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestSendToAllHosts(McrouterTestCase):
    config = "./mcrouter/test/test_send_to_all_hosts.json"
    extra_args = []

    def setUp(self):
        self.cacheA = self.add_server(MockMemcached())
        self.cacheB1 = self.add_server(MockMemcached())
        self.cacheB2 = self.add_server(MockMemcached())

        self.cacheA.set("ccw", "cacheA")
        self.cacheB1.set("ccw", "cacheB1")
        self.cacheB2.set("ccw", "cacheB2")

    def test_regular_request(self):
        mcrouter = self.add_mcrouter(
            self.config, extra_args=self.extra_args, enable_thrift=True
        ).get_thrift_client()
        reply = mcrouter.mcSet(b"test", b"val")
        self.assertEqual(Result.STORED, reply.result)
        self.assertEqual(self.cacheA.get("test"), "val")
        self.assertIsNone(self.cacheB1.get("test"))
        self.assertIsNone(self.cacheB2.get("test"))

    def test_set(self):
        self.assertIsNone(self.cacheB1.get("aaa"))
        self.assertIsNone(self.cacheB2.get("aaa"))
        mcrouter = self.add_mcrouter(
            self.config, extra_args=self.extra_args, enable_thrift=True
        ).get_thrift_client()
        reply = mcrouter.mcSet(b"aaa", b"val")
        self.assertEqual(Result.STORED, reply.result)
        self.assertEqual(self.cacheA.get("aaa"), "val")
        self.assertEqual(self.cacheB1.get("aaa"), "val")
        self.assertEqual(self.cacheB2.get("aaa"), "val")

    def test_delete(self):
        self.assertTrue(self.cacheA.set("aaa", "val"))
        self.assertTrue(self.cacheB1.set("aaa", "val"))
        self.assertTrue(self.cacheB2.set("aaa", "val"))
        mcrouter = self.add_mcrouter(
            self.config, extra_args=self.extra_args, enable_thrift=True
        ).get_thrift_client()
        reply = mcrouter.mcDelete(b"aaa")
        self.assertEqual(Result.DELETED, reply.result)
        self.assertIsNone(self.cacheA.get("aaa"))
        self.assertIsNone(self.cacheB1.get("aaa"))
        self.assertIsNone(self.cacheB2.get("aaa"))
