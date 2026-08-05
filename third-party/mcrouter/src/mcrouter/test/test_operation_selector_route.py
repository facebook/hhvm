#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

from carbon.carbon_result.thrift_types import Result
from mcrouter.test.MCProcess import MockMemcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestOperationSelectorRoute(McrouterTestCase):
    config = "./mcrouter/test/test_operation_selector_route.json"
    extra_args = []

    def setUp(self):
        self.memcached_get = self.add_server(MockMemcached())
        self.memcached_set = self.add_server(MockMemcached())
        self.memcached_delete = self.add_server(MockMemcached())

        mcrouter = self.add_mcrouter(
            self.config, extra_args=self.extra_args, enable_thrift=True
        )
        self.client = mcrouter.get_thrift_client()

    def test_get(self):
        self.assertTrue(self.memcached_get.set("key_get", "val_get"))
        reply = self.client.mcGet(b"key_get")
        self.assertEqual(Result.FOUND, reply.result)
        self.assertEqual(b"val_get", bytes(reply.value))

    def test_set(self):
        reply = self.client.mcSet(b"key_set", b"val_set")
        self.assertEqual(Result.STORED, reply.result)
        self.assertEqual("val_set", self.memcached_set.get("key_set"))

    def test_delete(self):
        self.assertTrue(self.memcached_delete.set("key_del", "val_del"))
        reply = self.client.mcDelete(b"key_del")
        self.assertEqual(Result.DELETED, reply.result)
        self.assertFalse(self.memcached_delete.get("key_del"))
