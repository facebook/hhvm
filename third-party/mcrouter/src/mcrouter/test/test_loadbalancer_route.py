#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Mcrouter, MockMemcachedDual
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestLoadBalancerRoute(McrouterTestCase):
    config = "./mcrouter/test/test_loadbalancer_route.json"
    extra_args = []
    num_reqs = 20000
    delta = 0.02

    def setUp(self):
        self.mc = self.makeServers()
        for server in self.mc:
            self.add_server(server)
        self.mcrouter = self.add_mcrouter(self.config, extra_args=self.extra_args)

    def makeServers(self):
        ret = []
        for _ in range(8):
            ret.append(
                Mcrouter(
                    "./mcrouter/test/test_nullroute.json",
                    extra_args=["--server-load-interval-ms=50"],
                )
            )
        return ret

    def getServerCmdGetCount(self, mc):
        return int(mc.stats()["cmd_get_count"])

    def getExpectedRates(self):
        return [0.125] * 8

    def test_loadbalancer(self):
        for i in range(0, self.num_reqs):
            key = "someprefix:{}:|#|id=123".format(i)
            self.assertTrue(not self.mcrouter.get(key))

        self.assertGreater(int(self.mcrouter.stats()["cmd_get_count"]), 0)
        sum = 0
        rates = self.getExpectedRates()
        for i in range(8):
            count = self.getServerCmdGetCount(self.mc[i])
            print("server {} req count: {}".format(i, count))
            self.assertAlmostEqual(count / self.num_reqs, rates[i], delta=self.delta)
            sum += count
        self.assertEqual(sum, self.num_reqs)


class TestThriftLoadBalancerRoute(TestLoadBalancerRoute):
    config = "./mcrouter/test/test_loadbalancer_route_thrift.json"

    def makeServers(self):
        ret = []
        for _ in range(4):
            ret.append(MockMemcachedDual(extra_args=["-l", "20"]))
        for _ in range(4):
            ret.append(MockMemcachedDual(extra_args=["-l", "80"]))
        return ret

    def getServerCmdGetCount(self, mc):
        return int(mc.stats("__mockmc__")["cmd_get_count"])

    def getExpectedRates(self):
        return [0.2, 0.2, 0.2, 0.2, 0.05, 0.05, 0.05, 0.05]


class TestThriftLoadBalancerRoute2(TestThriftLoadBalancerRoute):
    delta = 0.1

    def makeServers(self):
        ret = []
        for _ in range(4):
            ret.append(MockMemcachedDual(extra_args=["-l", "100"]))
            ret.append(MockMemcachedDual(extra_args=["-l", "0"]))
        return ret

    def getExpectedRates(self):
        return [0, 0.25, 0, 0.25, 0, 0.25, 0, 0.25]


class TestThriftLoadBalancerRouteReturnLoadMiss(TestThriftLoadBalancerRoute):
    def makeServers(self):
        ret = []
        for _ in range(8):
            ret.append(MockMemcachedDual())
        return ret

    def getExpectedRates(self):
        return [0.125] * 8
