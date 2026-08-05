#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

from carbon.carbon_result.thrift_types import Result
from mcrouter.test.MCProcess import MockMemcachedDual
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestThriftClient(McrouterTestCase):
    config = "./mcrouter/test/test_thrift_gut.json"

    def setUp(self):
        self.add_server(MockMemcachedDual())
        mcrouter = self.add_mcrouter(self.config, enable_thrift=True)
        self.client = mcrouter.get_thrift_client()

    def test_set_get_delete(self):
        key = b"typed-key"
        value = b"typed-value\x00"
        flags = 42

        version_reply = self.client.mcVersion()
        self.assertEqual(Result.OK, version_reply.result)

        set_reply = self.client.mcSet(key, value, flags=flags)
        self.assertEqual(Result.STORED, set_reply.result)

        get_reply = self.client.mcGet(key)
        self.assertEqual(Result.FOUND, get_reply.result)
        self.assertEqual(value, bytes(get_reply.value))
        self.assertEqual(flags, get_reply.flags)

        miss_reply = self.client.mcGet(b"missing-key")
        self.assertEqual(Result.NOTFOUND, miss_reply.result)
        self.assertIsNone(miss_reply.value)

        delete_reply = self.client.mcDelete(key)
        self.assertEqual(Result.DELETED, delete_reply.result)

        deleted_reply = self.client.mcGet(key)
        self.assertEqual(Result.NOTFOUND, deleted_reply.result)
        self.assertIsNone(deleted_reply.value)
