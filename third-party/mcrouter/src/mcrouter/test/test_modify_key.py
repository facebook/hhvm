#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestModifyKey(McrouterTestCase):
    config = './mcrouter/test/test_modify_key.json'
    extra_args = []

    def setUp(self):
        self.mc = self.add_server(Memcached())
        self.mcr = self.add_mcrouter(
            self.config, '/a/a/', extra_args=self.extra_args)

    def test_modify_key(self):
        self.assertTrue(self.mcr.set("key", "value"))
        self.assertIsNone(self.mc.get("key"))
        self.assertEqual(self.mc.get("/a/b/foo:key"), "value")

        self.assertTrue(self.mcr.set("foo:bar", "value2"))
        self.assertEqual(self.mc.get("/a/b/foo:bar"), "value2")

        self.assertTrue(self.mcr.set("/*/*/foo:bar", "value3"))
        time.sleep(1)
        self.assertEqual(self.mc.get("/a/b/foo:bar"), "value3")
        self.assertEqual(self.mc.get("/*/*/foo:bar"), "value3")
        self.assertEqual(self.mc.get("foo:bar"), "value3")

        self.assertTrue(self.mcr.set("/a/a/foo:bar", "value4"))
        self.assertEqual(self.mc.get("/a/b/foo:bar"), "value4")

        self.assertTrue(self.mcr.set("/a/a/o:", "value5"))
        self.assertEqual(self.mc.get("/a/b/foo:o:"), "value5")

        self.assertTrue(self.mcr.set("/b/c/key", "value6"))
        self.assertEqual(self.mc.get("/b/c/foo:key"), "value6")

        self.assertTrue(self.mcr.set("/c/d/123", "value7"))
        self.assertEqual(self.mc.get("123"), "value7")

        self.assertFalse(self.mcr.set("/c/d/", "value"))
        self.assertFalse(self.mcr.set("/d/e/", "value"))

        self.assertTrue(self.mcr.set("/d/e/123", "value8"))
        self.assertEqual(self.mc.get("123"), "value8")

        self.assertTrue(self.mcr.set("/e/f/akey", "value9"))
        self.assertEqual(self.mc.get("/e/f/bar:"), "value9")

        self.assertTrue(self.mcr.set("/e/f/mykeys", "value10"))
        self.assertEqual(self.mc.get("/e/f/bar:ys"), "value10")

        self.assertTrue(self.mcr.set("/e/f/key", "value11"))
        self.assertEqual(self.mc.get("/e/f/bar:key"), "value11")

        self.assertTrue(self.mcr.set("/e/f/bar:key", "value12"))
        self.assertEqual(self.mc.get("/e/f/bar:key"), "value12")

        self.assertTrue(self.mcr.set("/f/g/akey", "value13"))
        self.assertEqual(self.mc.get("/f/g/bar:akey"), "value13")

        self.assertTrue(self.mcr.set("/g/h/akey", "value14"))
        self.assertEqual(self.mc.get("/a/b/bar:"), "value14")

        self.assertTrue(self.mcr.set("/g/h/mykeys", "value15"))
        self.assertEqual(self.mc.get("/a/b/bar:ys"), "value15")

        self.assertTrue(self.mcr.set("/g/h/key", "value16"))
        self.assertEqual(self.mc.get("/a/b/bar:key"), "value16")

        self.assertTrue(self.mcr.set("/h/i/mykeys", "value17"))
        self.assertEqual(self.mc.get("bar:ys"), "value17")

        self.assertTrue(self.mcr.set("/h/i/hi", "value18"))
        self.assertEqual(self.mc.get("bar:hi"), "value18")

        self.assertTrue(self.mcr.set("/h/i/keys", "value19"))
        self.assertEqual(self.mc.get("bar:"), "value19")

        self.assertFalse(self.mcr.set("/i/j/", "value"))

        self.assertTrue(self.mcr.set("/i/j/keys", "value20"))
        self.assertEqual(self.mc.get("keys"), "value20")

        self.assertTrue(self.mcr.set("/j/k/foo.sup", "value21"))
        self.assertEqual(self.mc.get("/j/k/bar.sup"), "value21")

        self.assertTrue(self.mcr.set("/d/w/foo.sup", "value21"))
        self.assertEqual(self.mc.get("/d/w/foo.sup:bar"), "value21")

        self.assertTrue(self.mcr.set("/e/w/akey", "value14"))
        self.assertEqual(self.mc.get("/e/w/bar:foo"), "value14")

        self.assertTrue(self.mcr.set("/e/w/mykeys", "value15"))
        self.assertEqual(self.mc.get("/e/w/bar:ysfoo"), "value15")

        self.assertTrue(self.mcr.set("/f/w/akey", "value14"))
        self.assertEqual(self.mc.get("bar:foo"), "value14")

        # reverts to prefix append if replace not present
        self.assertTrue(self.mcr.set("/j/k/baz.sup", "value22"))
        self.assertEqual(self.mc.get("/j/k/bar.baz.sup"), "value22")

        # same as above, but with a suffix
        self.assertTrue(self.mcr.set("/j/l/baz.sup", "value22"))
        self.assertEqual(self.mc.get("/j/l/bar.baz.supmoot"), "value22")
