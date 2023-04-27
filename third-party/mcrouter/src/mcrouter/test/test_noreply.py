#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestNoReplyBase(McrouterTestCase):
    config = './mcrouter/test/test_noreply.json'

    def setUp(self):
        # The order here must corresponds to the order of hosts in the .json
        self.mc = self.add_server(self.make_memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(self.config)


class TestNoReply(TestNoReplyBase):
    def test_set_noreply(self):
        mcrouter = self.get_mcrouter()
        self.assertTrue(mcrouter.set("key", "value", noreply=True))
        self.assertTrue(self.eventually_get(key="key", expVal="value"))

    def test_add_replace_noreply(self):
        mcrouter = self.get_mcrouter()
        self.assertTrue(mcrouter.add("key", "value", noreply=True))
        self.assertTrue(self.eventually_get(key="key", expVal="value"))
        self.assertTrue(mcrouter.replace("key", "value1", noreply=True))
        self.assertTrue(self.eventually_get(key="key", expVal="value1"))

    def test_delete_noreply(self):
        mcrouter = self.get_mcrouter()
        self.assertTrue(mcrouter.set("key", "value"))
        self.assertTrue(self.eventually_get(key="key", expVal="value"))
        self.assertTrue(mcrouter.delete("key", noreply=True))
        self.assertFalse(self.mc.get("key"))

    def test_touch_noreply(self):
        mcrouter = self.get_mcrouter()
        self.assertTrue(mcrouter.set("key", "value"))
        self.assertTrue(self.eventually_get(key="key", expVal="value"))
        self.assertTrue(mcrouter.touch("key", 100, noreply=True))
        self.assertTrue(self.eventually_get(key="key", expVal="value"))

    def test_arith_noreply(self):
        mcrouter = self.get_mcrouter()
        self.assertTrue(mcrouter.set("arith", "1"))
        self.assertTrue(self.eventually_get(key="arith", expVal="1"))

        self.assertTrue(mcrouter.incr("arith", noreply=True))
        self.assertTrue(self.eventually_get(key="arith", expVal="2"))

        self.assertTrue(mcrouter.decr("arith", noreply=True))
        self.assertTrue(self.eventually_get(key="arith", expVal="1"))


class TestNoReplyAppendPrepend(TestNoReplyBase):
    def __init__(self, *args, **kwargs):
        super(TestNoReplyAppendPrepend, self).__init__(*args, **kwargs)
        self.use_mock_mc = True

    def test_affix_noreply(self):
        mcrouter = self.get_mcrouter()
        self.assertTrue(mcrouter.set("key", "value"))
        self.assertTrue(self.eventually_get(key="key", expVal="value"))

        self.assertTrue(mcrouter.append("key", "123", noreply=True))
        self.assertTrue(self.eventually_get(key="key", expVal="value123"))

        self.assertTrue(mcrouter.prepend("key", "456", noreply=True))
        self.assertTrue(self.eventually_get(key="key", expVal="456value123"))
