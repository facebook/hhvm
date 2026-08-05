#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

import time

from carbon.carbon_result.thrift_types import Result
from mcrouter.test.MCProcess import MockMemcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestModifyKey(McrouterTestCase):
    config = "./mcrouter/test/test_modify_key.json"
    extra_args = []

    def setUp(self):
        self.mc = self.add_server(MockMemcached())
        self.mcr = self.add_mcrouter(
            self.config, "/a/a/", extra_args=self.extra_args, enable_thrift=True
        )
        self.client = self.mcr.get_thrift_client()

    def assertStored(self, key, value):
        reply = self.client.mcSet(key.encode(), value.encode())
        self.assertEqual(Result.STORED, reply.result)

    def assertInvalidEmptyKey(self, key):
        reply = self.client.mcSet(key.encode(), b"value")
        self.assertEqual(Result.LOCAL_ERROR, reply.result)
        self.assertEqual(
            "ModifyKeyRoute: invalid key: invalid key: missing", reply.message
        )

    def test_modify_key(self):
        self.assertStored("key", "value")
        self.assertIsNone(self.mc.get("key"))
        self.assertEqual(self.mc.get("/a/b/foo:key"), "value")

        self.assertStored("foo:bar", "value2")
        self.assertEqual(self.mc.get("/a/b/foo:bar"), "value2")

        self.assertStored("/*/*/foo:bar", "value3")
        time.sleep(1)
        self.assertEqual(self.mc.get("/a/b/foo:bar"), "value3")
        self.assertEqual(self.mc.get("/*/*/foo:bar"), "value3")
        self.assertEqual(self.mc.get("foo:bar"), "value3")

        self.assertStored("/a/a/foo:bar", "value4")
        self.assertEqual(self.mc.get("/a/b/foo:bar"), "value4")

        self.assertStored("/a/a/o:", "value5")
        self.assertEqual(self.mc.get("/a/b/foo:o:"), "value5")

        self.assertStored("/b/c/key", "value6")
        self.assertEqual(self.mc.get("/b/c/foo:key"), "value6")

        self.assertStored("/c/d/123", "value7")
        self.assertEqual(self.mc.get("123"), "value7")

        for key in ("/c/d/", "/d/e/"):
            self.assertInvalidEmptyKey(key)

        self.assertStored("/d/e/123", "value8")
        self.assertEqual(self.mc.get("123"), "value8")

        self.assertStored("/e/f/akey", "value9")
        self.assertEqual(self.mc.get("/e/f/bar:"), "value9")

        self.assertStored("/e/f/mykeys", "value10")
        self.assertEqual(self.mc.get("/e/f/bar:ys"), "value10")

        self.assertStored("/e/f/key", "value11")
        self.assertEqual(self.mc.get("/e/f/bar:key"), "value11")

        self.assertStored("/e/f/bar:key", "value12")
        self.assertEqual(self.mc.get("/e/f/bar:key"), "value12")

        self.assertStored("/f/g/akey", "value13")
        self.assertEqual(self.mc.get("/f/g/bar:akey"), "value13")

        self.assertStored("/g/h/akey", "value14")
        self.assertEqual(self.mc.get("/a/b/bar:"), "value14")

        self.assertStored("/g/h/mykeys", "value15")
        self.assertEqual(self.mc.get("/a/b/bar:ys"), "value15")

        self.assertStored("/g/h/key", "value16")
        self.assertEqual(self.mc.get("/a/b/bar:key"), "value16")

        self.assertStored("/h/i/mykeys", "value17")
        self.assertEqual(self.mc.get("bar:ys"), "value17")

        self.assertStored("/h/i/hi", "value18")
        self.assertEqual(self.mc.get("bar:hi"), "value18")

        self.assertStored("/h/i/keys", "value19")
        self.assertEqual(self.mc.get("bar:"), "value19")

        self.assertInvalidEmptyKey("/i/j/")

        self.assertStored("/i/j/keys", "value20")
        self.assertEqual(self.mc.get("keys"), "value20")

        self.assertStored("/j/k/foo.sup", "value21")
        self.assertEqual(self.mc.get("/j/k/bar.sup"), "value21")

        self.assertStored("/d/w/foo.sup", "value21")
        self.assertEqual(self.mc.get("/d/w/foo.sup:bar"), "value21")

        self.assertStored("/e/w/akey", "value14")
        self.assertEqual(self.mc.get("/e/w/bar:foo"), "value14")

        self.assertStored("/e/w/mykeys", "value15")
        self.assertEqual(self.mc.get("/e/w/bar:ysfoo"), "value15")

        self.assertStored("/f/w/akey", "value14")
        self.assertEqual(self.mc.get("bar:foo"), "value14")

        # reverts to prefix append if replace not present
        self.assertStored("/j/k/baz.sup", "value22")
        self.assertEqual(self.mc.get("/j/k/bar.baz.sup"), "value22")

        # same as above, but with a suffix
        self.assertStored("/j/l/baz.sup", "value22")
        self.assertEqual(self.mc.get("/j/l/bar.baz.supmoot"), "value22")
