#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.McrouterTestCase import McrouterTestCase
import tempfile
import os
import time
from string import Template
import threading
import queue

class CarbonLookasideTmpConfig():
    routeConfigFile = """
{
  "pools": {
    "A": {
      "servers": [ "127.0.0.1:12345" ]
    }
  },
  "route": {
    "type": "CarbonLookasideRoute",
    "prefix": "$TEMPLATE_PREFIX",
    "ttl": $TEMPLATE_TTL,
    "ttl_unit_ms": $TEMPLATE_TTL_UNIT_MS,
    "flavor": "$TEMPLATE_FILENAME",
    "lease_settings": {
      "enable_leases": $TEMPLATE_LEASE_ENABLE,
      "initial_wait_interval_ms": $TEMPLATE_LEASE_WAIT_INTERVAL,
      "num_retries": $TEMPLATE_LEASE_NUM_RETRIES,
    },
    "child": [
      "PoolRoute|A"
    ]
  }
}
"""
    routeLatencyConfigFile = """
{
  "pools": {
    "A": {
      "servers": [ "127.0.0.1:12345" ]
    }
  },
  "route": {
    "type": "CarbonLookasideRoute",
    "prefix": "$TEMPLATE_PREFIX",
    "ttl": $TEMPLATE_TTL,
    "flavor": "$TEMPLATE_FILENAME",
    "lease_settings": {
      "enable_leases": $TEMPLATE_LEASE_ENABLE,
      "initial_wait_interval_ms": $TEMPLATE_LEASE_WAIT_INTERVAL,
      "num_retries": $TEMPLATE_LEASE_NUM_RETRIES,
    },
    "child": {
      "type": "LatencyInjectionRoute",
      "child": "PoolRoute|A",
      "before_latency_ms": $TEMPLATE_BEFORE_LATENCY,
      "after_latency_ms": $TEMPLATE_AFTER_LATENCY,
    }
  }
}
"""
    clientConfigFile = """
{
  "pools": {
    "A": {
      "servers": [ "127.0.0.1:${TEMPLATE_PORT}" ]
    }
  },
  "route": "PoolRoute|A"
}
"""
    flavorConfigFile = """
{
  "options": {
    "asynclog_disable": "true",
    "config": "${TEMPLATE_FILENAME}",
    "failures_until_tko": "10",
    "num_proxies": "4",
    "probe_delay_initial_ms": "5000",
    "scuba_sample_period": "0",
    "server_timeout_ms": "250"
  }
}
"""

    def cleanup(self):
        if not self.tmpRouteFile:
            os.remove(self.tmpRouteFile)
        if not self.tmpClientFile:
            os.remove(self.tmpClientFile)
        if not self.tmpFlavorFile:
            os.remove(self.tmpFlavorFile)

    def __init__(self, prefix, ttl, port, lease_enable='false', lease_interval=0,
            lease_num_retries=0, latency_before=0, latency_after=0,
            has_ms_ttl='false'):
        # Client file configuration
        with tempfile.NamedTemporaryFile(mode='w', delete=False) as self.tmpClientFile:
            clientDict = {'TEMPLATE_PORT': port}
            src = Template(self.clientConfigFile)
            result = src.substitute(clientDict)
            self.tmpClientFile.write(result)
        # Flavor file configuration
        with tempfile.NamedTemporaryFile(mode='w', delete=False) as self.tmpFlavorFile:
            flavorDict = {'TEMPLATE_FILENAME': 'file:' + self.tmpClientFile.name}
            src = Template(self.flavorConfigFile)
            result = src.substitute(flavorDict)
            self.tmpFlavorFile.write(result)
        # Route file configuration
        with tempfile.NamedTemporaryFile(mode='w', delete=False) as self.tmpRouteFile:
            routeDict = {'TEMPLATE_PREFIX': prefix,
                         'TEMPLATE_TTL': ttl,
                         'TEMPLATE_TTL_UNIT_MS': has_ms_ttl,
                         'TEMPLATE_FILENAME': 'file:' + self.tmpFlavorFile.name,
                         'TEMPLATE_LEASE_ENABLE': lease_enable,
                         'TEMPLATE_LEASE_WAIT_INTERVAL': lease_interval,
                         'TEMPLATE_LEASE_NUM_RETRIES': lease_num_retries,
                         'TEMPLATE_BEFORE_LATENCY': latency_before,
                         'TEMPLATE_AFTER_LATENCY': latency_after}
            if latency_before or latency_after:
                src = Template(self.routeLatencyConfigFile)
            else:
                src = Template(self.routeConfigFile)
            result = src.substitute(routeDict)
            self.tmpRouteFile.write(result)

    def getFileName(self):
        return self.tmpRouteFile.name


class TestCarbonLookasideRouteBasic(McrouterTestCase):
    prefix = "CarbonLookaside"
    ttl = 120
    extra_args = []

    def setUp(self):
        self.mc = self.add_server(self.make_memcached())
        self.tmpConfig = CarbonLookasideTmpConfig(self.prefix, self.ttl,
                                                  self.mc.getport())
        self.config = self.tmpConfig.getFileName()
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def tearDown(self):
        self.tmpConfig.cleanup()

    def test_carbonlookaside_basic(self):
        n = 20
        # Insert 20 items into memcache
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.set(key, 'value'))
        # Get the 20 items from memcache, they will be set in
        # carbonlookaside
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')
        # Query carbonlookaside directly with the configured prefix
        # that the items have indeed been stored.
        for i in range(0, n):
            key = '{}someprefix:{}:|#|id=123'.format(self.prefix, i)
            self.assertTrue(self.mc.get(key))
        # Query the items through mcrouter and check that they are there
        # This query will be fed from carbonlookaside.
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')

    def test_carbonlookaside_larger(self):
        n = 200
        # Insert 200 items into memcache
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.set(key, 'value'))
        # Get the 200 items from memcache, they will be set in
        # carbonlookaside
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')
        # Query carbonlookaside directly with the configured prefix
        # that the items have indeed been stored.
        for i in range(0, n):
            key = '{}someprefix:{}:|#|id=123'.format(self.prefix, i)
            self.assertTrue(self.mc.get(key))
        # Query the items through mcrouter and check that they are there
        # This query will be fed from carbonlookaside.
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')


class TestCarbonLookasideRouteExpiry(McrouterTestCase):
    prefix = "CarbonLookaside"
    ttl = 2
    extra_args = []

    def setUp(self):
        self.mc = self.add_server(self.make_memcached())
        self.tmpConfig = CarbonLookasideTmpConfig(self.prefix, self.ttl,
                                                  self.mc.getport())
        self.config = self.tmpConfig.getFileName()
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def tearDown(self):
        self.tmpConfig.cleanup()

    def test_carbonlookaside_ttl_expiry(self):
        n = 20
        # Insert 20 items into memcache
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.set(key, 'value'))
        # Get the 20 items from memcache, they will be set in
        # carbonlookaside
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')
        # Query carbonlookaside directly with the configured prefix
        # that the items have indeed been stored.
        for i in range(0, n):
            key = '{}someprefix:{}:|#|id=123'.format(self.prefix, i)
            self.assertTrue(self.mc.get(key))
        time.sleep(3)
        # Query carbonlookaside directly and check they have expired
        for i in range(0, n):
            key = '{}someprefix:{}:|#|id=123'.format(self.prefix, i)
            self.assertFalse(self.mc.get(key))


class TestCarbonLookasideRouteNoExpiry(McrouterTestCase):
    prefix = "CarbonLookaside"
    ttl = 0
    extra_args = []

    def setUp(self):
        self.mc = self.add_server(self.make_memcached())
        self.tmpConfig = CarbonLookasideTmpConfig(self.prefix, self.ttl,
                                                  self.mc.getport())
        self.config = self.tmpConfig.getFileName()
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def tearDown(self):
        self.tmpConfig.cleanup()

    def test_carbonlookaside_ttl_no_expiry(self):
        n = 20
        # Insert 20 items into memcache
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.set(key, 'value', exptime=2))
        # Get the 20 items from memcache, they will be set in
        # carbonlookaside
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')
        time.sleep(4)
        # Items should have expired in memcache
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertFalse(self.mc.get(key))
        # Items still available in carbonlookaside through mcrouter
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key))


class TestCarbonLookasideRouteLeases(McrouterTestCase):
    prefix = "CarbonLookaside"
    ttl = 0
    extra_args = []

    def setUp(self):
        self.mc = self.add_server(self.make_memcached())
        self.tmpConfig = CarbonLookasideTmpConfig(self.prefix, self.ttl,
                                                  self.mc.getport(), 'true',
                                                  10, 10)
        self.config = self.tmpConfig.getFileName()
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def tearDown(self):
        self.tmpConfig.cleanup()

    def test_carbonlookaside_basic_leases(self):
        n = 20
        # Insert 20 items into memcache
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.set(key, 'value'))
        # Get the 20 items from memcache, they will be set in
        # carbonlookaside
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')
        # Query carbonlookaside directly with the configured prefix
        # that the items have indeed been stored.
        for i in range(0, n):
            key = '{}someprefix:{}:|#|id=123'.format(self.prefix, i)
            self.assertTrue(self.mc.get(key))
        # Query the items through mcrouter and check that they are there
        # This query will be fed from carbonlookaside.
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')

    def test_carbonlookaside_larger_leases(self):
        n = 200
        # Insert 200 items into memcache
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.set(key, 'value'))
        # Get the 200 items from memcache, they will be set in
        # carbonlookaside
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')
        # Query carbonlookaside directly with the configured prefix
        # that the items have indeed been stored.
        for i in range(0, n):
            key = '{}someprefix:{}:|#|id=123'.format(self.prefix, i)
            self.assertTrue(self.mc.get(key))
        # Query the items through mcrouter and check that they are there
        # This query will be fed from carbonlookaside.
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')


class TestCarbonLookasideRouteLeasesHotMiss(McrouterTestCase):
    prefix = "CarbonLookaside"
    ttl = 0
    extra_args = []

    def setUp(self):
        self.mc = self.add_server(self.make_memcached())
        self.tmpConfig = CarbonLookasideTmpConfig(self.prefix, self.ttl,
                                                  self.mc.getport(), 'true',
                                                  10, 3, 0, 5000)
        self.config = self.tmpConfig.getFileName()
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

        self.mcrouter2 = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def tearDown(self):
        self.tmpConfig.cleanup()

    def async_get(self, key, ret):
        ret.put(self.mcrouter.get(key))

    def test_carbonlookaside_basic_leases(self):
        # Add KV pair directly to MC to avoid backend latency and
        # carbonlookaside intereference
        key = 'auld_lang_syne'
        self.assertTrue(self.mc.set(key, 'value'))
        # Make an async request for the key. It will initially do a lease get
        # to carbonlookaside which will return a lease token. Note using a
        # latency route here to slow things down so there is time to provoke
        # a hot miss before backend responds and carbon lookaside does a lease
        # set.
        ret = queue.Queue()
        t = threading.Thread(target=self.async_get, args=(key, ret))
        t.start()
        # Ensure lease get has arrived at MC server before proceeding
        stats = self.mc.stats()
        while stats["cmd_lease_get"] == '0':
            stats = self.mc.stats()
        # Hot miss
        self.assertTrue(self.mcrouter2.get(key), 'value')
        stats = self.mc.stats()
        self.assertTrue(stats["cmd_lease_get"] == '5')

        # Now wait on the back end returning and the write to carbonLookaside
        # completing.
        t.join()
        self.assertTrue(ret.get())
        stats = self.mc.stats()

        # the lookaside sets dont block, so allow it to retry till set arrives
        # at the local MC server.
        retry = 0
        while stats["cmd_lease_set"] == '0' and retry < 3:
            time.sleep(1)
            retry += 1
        self.assertTrue(stats["cmd_lease_set"] == '1')


class TestCarbonLookasideRouteExpiryMsTTLBase(McrouterTestCase):
    prefix = "CarbonLookaside"
    ttl = 500
    has_ms_ttl = 'true'
    extra_args = []

    def setUp(self):
        self.mc = self.add_server(self.make_memcached())
        self.tmpConfig = CarbonLookasideTmpConfig(self.prefix, self.ttl,
                                                  self.mc.getport(), 'false', 0,
                                                  0, 0, 0, self.has_ms_ttl)
        self.config = self.tmpConfig.getFileName()
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def tearDown(self):
        self.tmpConfig.cleanup()

    def test_carbonlookaside_ttl_ms_expiry_config(self):
        k = "some_key"
        v = "some_value"
        self.assertTrue(self.mcrouter.set(k, v))


class TestCarbonLookasideRouteExpiryConfig100(TestCarbonLookasideRouteExpiryMsTTLBase):
    ttl = 100


class TestCarbonLookasideRouteExpiryConfig200(TestCarbonLookasideRouteExpiryMsTTLBase):
    ttl = 200


class TestCarbonLookasideRouteExpiryMsTTL(McrouterTestCase):
    prefix = "CarbonLookaside"
    ttl = 500
    n = 1
    has_ms_ttl = 'true'
    extra_args = []

    def setUp(self):
        self.mc = self.add_server(self.make_memcached())
        self.tmpConfig = CarbonLookasideTmpConfig(self.prefix, self.ttl,
                                                  self.mc.getport(), 'false', 0,
                                                  0, 0, 0, self.has_ms_ttl)
        self.config = self.tmpConfig.getFileName()
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def tearDown(self):
        self.tmpConfig.cleanup()

    def test_carbonlookaside_ttl_ms_expiry(self):
        # Align to 1 second boundary
        epoch_t1 = int(time.time())
        epoch_t2 = int(time.time())
        while (epoch_t1 == epoch_t2):
            time.sleep(0.1)
            epoch_t2 = int(time.time())

        # Insert n items into memcache
        for i in range(0, self.n):
            key = 'someprefix:{}:id=123'.format(i)
            self.assertTrue(self.mcrouter.set(key, 'value'))

        # Get the n items from memcache, they will be set in
        # carbonlookaside. Then delete them from memcache.
        for i in range(0, self.n):
            key = 'someprefix:{}:id=123'.format(i)
            self.assertTrue(self.mcrouter.get(key), 'value')
            self.assertTrue(self.mc.delete(key))

        # Check that the keys have been correctly set in carbonlookaside
        stats = self.mc.stats()
        self.assertEqual(int(stats["total_items"]), 2 * self.n)

        # Let item expire
        time.sleep(0.5)
        # Query carbonlookaside directly and check they have expired
        for i in range(0, self.n):
            key = '{}someprefix:{}:id=123'.format(self.prefix, i)
            self.assertFalse(self.mc.get(key), 'value')
