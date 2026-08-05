#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

from unittest.mock import MagicMock, patch

from carbon.carbon_result.thrift_types import Result
from facebook.memcache.thrift.Memcache.thrift_types import (
    McAppendReply,
    McPrependReply,
    McTouchReply,
)
from mcrouter.facebook.test.thrift_test_client import ThriftTestClient
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

    def test_append_prepend(self):
        key = b"append-prepend-key"
        value = b"value\x00"
        suffix = b"suffix\xff"
        prefix = b"prefix\x01"

        append_miss_reply = self.client.mcAppend(key, suffix)
        self.assertEqual(Result.NOTSTORED, append_miss_reply.result)

        prepend_miss_reply = self.client.mcPrepend(key, prefix)
        self.assertEqual(Result.NOTSTORED, prepend_miss_reply.result)

        set_reply = self.client.mcSet(key, value)
        self.assertEqual(Result.STORED, set_reply.result)

        append_reply = self.client.mcAppend(key, suffix)
        self.assertEqual(Result.STORED, append_reply.result)

        prepend_reply = self.client.mcPrepend(key, prefix)
        self.assertEqual(Result.STORED, prepend_reply.result)

        get_reply = self.client.mcGet(key)
        self.assertEqual(Result.FOUND, get_reply.result)
        self.assertEqual(prefix + value + suffix, bytes(get_reply.value))

    def test_touch(self):
        key = b"touch-key"
        value = b"touch-value"

        self.assertEqual(Result.STORED, self.client.mcSet(key, value).result)
        self.assertEqual(Result.TOUCHED, self.client.mcTouch(key, exptime=100).result)

        get_reply = self.client.mcGet(key)
        self.assertEqual(Result.FOUND, get_reply.result)
        self.assertEqual(value, bytes(get_reply.value))

        self.assertEqual(
            Result.NOTFOUND,
            self.client.mcTouch(b"missing-key", exptime=100).result,
        )
        self.assertEqual(Result.TOUCHED, self.client.mcTouch(key, exptime=-1).result)
        self.assertEqual(Result.NOTFOUND, self.client.mcGet(key).result)

    def test_touch_request_fields(self):
        client = ThriftTestClient("::1", 1)
        rpc_client = MagicMock()
        rpc_client.mcTouch.return_value = McTouchReply(result=Result.TOUCHED)
        context_manager = MagicMock()
        context_manager.__enter__.return_value = rpc_client

        with patch.object(client, "_getSRClient", return_value=context_manager):
            client.mcTouch(b"touch-key\x00", exptime=123)

        touch_request = rpc_client.mcTouch.call_args.args[0]
        self.assertEqual(b"touch-key\x00", bytes(touch_request.key))
        self.assertEqual(123, touch_request.exptime)

    def test_append_prepend_request_fields(self):
        client = ThriftTestClient("::1", 1)
        rpc_client = MagicMock()
        rpc_client.mcAppend.return_value = McAppendReply(result=Result.STORED)
        rpc_client.mcPrepend.return_value = McPrependReply(result=Result.STORED)
        context_manager = MagicMock()
        context_manager.__enter__.return_value = rpc_client

        with patch.object(client, "_getSRClient", return_value=context_manager):
            client.mcAppend(
                b"append-key",
                b"append-value\x00",
                exptime=123,
                flags=456,
            )
            client.mcPrepend(
                b"prepend-key",
                b"prepend-value\xff",
                exptime=789,
                flags=1011,
            )

        append_request = rpc_client.mcAppend.call_args.args[0]
        self.assertEqual(b"append-key", bytes(append_request.key))
        self.assertEqual(b"append-value\x00", bytes(append_request.value))
        self.assertEqual(123, append_request.exptime)
        self.assertEqual(456, append_request.flags)

        prepend_request = rpc_client.mcPrepend.call_args.args[0]
        self.assertEqual(b"prepend-key", bytes(prepend_request.key))
        self.assertEqual(b"prepend-value\xff", bytes(prepend_request.value))
        self.assertEqual(789, prepend_request.exptime)
        self.assertEqual(1011, prepend_request.flags)
