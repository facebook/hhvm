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


class TestWarmup2(McrouterTestCase):
    config = "./mcrouter/test/test_warmup2.json"
    enable_thrift_frontend = False

    def setUp(self):
        self.mc_warm = self.add_server(MockMemcached())
        self.mc_cold = self.add_server(MockMemcached())
        self.mcrouter = self.add_mcrouter(
            self.config, enable_thrift=self.enable_thrift_frontend
        )

    def test_warmup_get(self):
        k = "key"
        v = "value"
        self.assertTrue(self.mc_warm.set(k, v, exptime=1000))
        self.assertEqual(self.mcrouter.get(k), v)
        # warmup request is async
        time.sleep(1)
        self.assertTrue(self.mc_cold.get(k), v)
        cold_exptime = int(self.mc_cold.metaget(k)["exptime"])
        warm_exptime = int(self.mc_warm.metaget(k)["exptime"])
        self.assertAlmostEqual(cold_exptime, time.time() + 1000, delta=20)
        self.assertAlmostEqual(warm_exptime, time.time() + 1000, delta=20)

        self.assertTrue(self.mc_warm.delete(k))
        self.assertEqual(self.mcrouter.get(k), v)
        self.assertTrue(self.mc_cold.delete(k))
        self.assertIsNone(self.mcrouter.get(k))

        k = "key2"
        v = "value2"
        key_set_time = int(time.time())
        self.assertTrue(self.mc_warm.set(k, v, exptime=100))
        key_after_set_time = int(time.time())
        self.assertEqual(self.mcrouter.get(k), v)
        # warmup request is async
        time.sleep(1)
        self.assertTrue(self.mc_cold.get(k), v)
        cold_exptime = int(self.mc_cold.metaget(k)["exptime"])
        warm_exptime = int(self.mc_warm.metaget(k)["exptime"])

        self.assertIn(warm_exptime, range(key_set_time + 100, key_after_set_time + 101))

        # you should keep warm consistent by yourself
        self.assertTrue(self.mcrouter.delete(k))
        self.assertEqual(self.mc_warm.get(k), v)
        self.assertEqual(self.mcrouter.get(k), v)

    def test_warmup_lease_get(self):
        k = "key"
        v = "value"

        ret = self.mcrouter.leaseGet(k)
        self.assertEqual(ret["value"], "")
        self.assertIsNotNone(ret["token"])
        ret["value"] = v
        self.assertTrue(self.mcrouter.leaseSet(k, ret))
        ret = self.mcrouter.leaseGet(k)
        self.assertEqual(ret["value"], v)
        self.assertIsNone(ret["token"])

        k = "key2"
        v = "value2"
        self.assertTrue(self.mc_warm.set(k, v, exptime=1000))
        ret = self.mcrouter.leaseGet(k)
        self.assertEqual(ret["value"], v)
        self.assertIsNone(ret["token"])
        # warmup request is async
        time.sleep(1)
        ret = self.mc_cold.leaseGet(k)
        self.assertEqual(ret["value"], v)
        self.assertIsNone(ret["token"])
        cold_exptime = int(self.mc_cold.metaget(k)["exptime"])
        warm_exptime = int(self.mc_warm.metaget(k)["exptime"])
        self.assertAlmostEqual(cold_exptime, time.time() + 1000, delta=20)
        self.assertAlmostEqual(warm_exptime, time.time() + 1000, delta=20)

    def test_warmup_metaget(self):
        k = "key"
        v = "value"

        self.assertEqual(len(self.mcrouter.metaget(k)), 0)

        key_set_time = int(time.time())
        self.assertTrue(self.mc_warm.set(k, v, exptime=100))
        key_after_set_time = int(time.time())

        self.assertIn(
            int(self.mcrouter.metaget(k)["exptime"]),
            range(key_set_time + 100, key_after_set_time + 105),
        )
        self.assertTrue(self.mc_warm.delete(k))
        self.assertEqual(len(self.mcrouter.metaget(k)), 0)
        self.assertTrue(self.mc_cold.set(k, v, exptime=100))
        self.assertIn(
            int(self.mcrouter.metaget(k)["exptime"]),
            range(key_set_time + 100, key_after_set_time + 105),
        )


class TestWarmup2AppendPrependTouch(TestWarmup2):
    enable_thrift_frontend = True

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def test_warmup_append_prepend(self):
        key = b"key"
        value = b"value"
        suffix = b"suffix"
        prefix = b"prefix"
        client = self.mcrouter.get_thrift_client()

        # make sure appends and prepends go to cold route
        self.assertEqual(Result.STORED, client.mcSet(key, value, exptime=1000).result)
        self.assertEqual(self.mc_cold.get("key"), "value")
        self.assertEqual(Result.STORED, client.mcAppend(key, suffix).result)
        self.assertEqual(self.mc_cold.get("key"), "valuesuffix")
        self.assertEqual(Result.STORED, client.mcPrepend(key, prefix).result)
        self.assertEqual(self.mc_cold.get("key"), "prefixvaluesuffix")

    def test_warmup_touch(self):
        key = b"key"
        value = b"value"
        client = self.mcrouter.get_thrift_client()

        # make sure touch requests go to cold route
        self.assertEqual(Result.STORED, client.mcSet(key, value, exptime=1000).result)
        self.assertEqual(self.mc_cold.get("key"), "value")
        self.assertEqual(Result.TOUCHED, client.mcTouch(key, exptime=0).result)
        self.assertEqual(self.mcrouter.metaget("key")["exptime"], "0")
