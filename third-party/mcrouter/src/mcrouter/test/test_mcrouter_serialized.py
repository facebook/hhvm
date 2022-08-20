#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import SleepServer
from mcrouter.test.mock_servers import ConnectionErrorServer

def wait_until(target_time, increment):
    curr_time = int(time.time())
    while (int(target_time) > curr_time):
        time.sleep(increment)
        curr_time = int(time.time())

class TestMigratedPoolsFailover(McrouterTestCase):
    config = './mcrouter/test/test_migrated_pools_failover.json'
    extra_args = []

    def setUp(self):
        self.a_new = self.add_server(Memcached())
        self.a_old = self.add_server(Memcached())
        self.b_new = self.add_server(Memcached())
        self.b_old = self.add_server(Memcached())

    def get_mcrouter(self, start_time):
        return self.add_mcrouter(
            self.config, extra_args=self.extra_args,
            replace_map={"START_TIME": start_time})

    def test_migrated_pools_failover(self):
        phase_1_time = int(time.time())
        phase_2_time = phase_1_time + 4  # start time
        phase_3_time = phase_2_time + 4
        phase_4_time = phase_3_time + 4

        mcr = self.get_mcrouter(phase_2_time)

        #set keys that should be deleted in later phases
        for phase in range(1, 5):
            self.a_old.set("get-key-" + str(phase), str(phase))
            self.a_new.set("get-key-" + str(phase), str(phase * 10))
            self.b_old.set("get-key-" + str(phase), str(phase * 100))
            self.b_new.set("get-key-" + str(phase), str(phase * 1000))

        # first we are in the old domain make sure all ops go to
        # the old host only
        # note: only run if we're still in phase 1
        if int(time.time()) < phase_2_time:
            self.assertEqual(mcr.get("get-key-1"), str(1))
            mcr.set("set-key-1", str(42))
            self.assertEqual(self.a_old.get("set-key-1"), str(42))

            self.a_old.terminate()
            self.assertEqual(mcr.get("get-key-1"), str(100))
            mcr.set("set-key-1", str(42))
            self.assertEqual(self.b_old.get("set-key-1"), str(42))
        else:
            self.a_old.terminate()

        # next phase (2)
        wait_until(phase_2_time, 0.5)
        # note: only run if we're still in phase 2
        if int(time.time()) < phase_3_time:
            self.assertEqual(mcr.get("get-key-2"), str(200))
            mcr.set("set-key-2", str(42))
            self.assertEqual(self.b_old.get("set-key-2"), str(42))

        # last phase (4)
        wait_until(phase_4_time + 1, 0.5)
        # gets/sets go to the new place
        self.assertEqual(mcr.get("get-key-3"), str(30))
        mcr.set("set-key-3", str(424242))
        self.assertEqual(self.a_new.get("set-key-3"), str(424242))

        self.a_new.terminate()
        self.assertEqual(mcr.get("get-key-3"), str(3000))


class TestSamePoolFailover(McrouterTestCase):
    config = './mcrouter/test/test_same_pool_failover.json'
    extra_args = []

    def setUp(self):
        self.add_server(Memcached(), 12345)

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_same_pool_failover(self):
        mcr = self.get_mcrouter()

        self.assertEqual(mcr.get('foobar'), None)
        self.assertTrue(mcr.set('foobar', 'bizbang'))
        self.assertEqual(mcr.get('foobar'), 'bizbang')
        mcr.delete('foobar')
        self.assertEqual(mcr.get('foobar'), None)


class TestGetFailover(McrouterTestCase):
    config = './mcrouter/test/test_get_failover.json'
    extra_args = []

    def setUp(self):
        self.gut = self.add_server(Memcached())
        self.wildcard = self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def failover_common(self, key):
        self.mcr = self.get_mcrouter()

        self.assertEqual(self.mcr.get(key), None)
        self.assertTrue(self.mcr.set(key, 'bizbang'))
        self.assertEqual(self.mcr.get(key), 'bizbang')

        # kill the main host so everything failsover to gut
        self.wildcard.terminate()

        self.assertEqual(self.mcr.get(key), None)
        self.assertTrue(self.mcr.set(key, 'bizbang-fail'))
        self.assertEqual(self.mcr.get(key), 'bizbang-fail')

    def test_get_failover(self):
        self.failover_common('testkey')
        # the failover should have set it with a much shorter TTL
        # so make sure that we can't get the value after the TTL
        # has expired
        time.sleep(4)
        self.assertEqual(self.mcr.get('testkey'), None)


class TestGetFailoverWithFailoverTag(TestGetFailover):
    config = './mcrouter/test/test_get_failover_with_failover_tag.json'

    def test_get_failover(self):
        key = 'testkey|#|extra=1'
        self.failover_common(key)

        # Verify the failover tag was appended
        fail_key = key + ":failover=1"
        self.assertEqual(self.mcr.get(key), 'bizbang-fail')
        self.assertEqual(self.gut.get(fail_key), 'bizbang-fail')


class TestLeaseGetFailover(McrouterTestCase):
    config = './mcrouter/test/test_get_failover.json'
    extra_args = []

    def setUp(self):
        self.gut = self.add_server(Memcached())
        self.wildcard = self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_lease_get_failover(self):
        mcr = self.get_mcrouter()

        get_res = {}
        get_res['testkey'] = mcr.leaseGet('testkey')
        get_res['testkey']['value'] = 'bizbang-lease'
        self.assertGreater(get_res['testkey']['token'], 0)
        self.assertTrue(mcr.leaseSet('testkey', get_res['testkey']))
        get_res['testkey'] = mcr.leaseGet('testkey')
        self.assertFalse(get_res['testkey']['token'])
        self.assertEqual(get_res['testkey']['value'], 'bizbang-lease')

        # kill the main host so everything failsover to mctestc00.gut
        self.wildcard.terminate()

        get_res['testkey'] = mcr.leaseGet('testkey')
        get_res['testkey']['value'] = 'bizbang-lease-fail'
        self.assertGreater(get_res['testkey']['token'], 0)
        self.assertTrue(mcr.leaseSet('testkey', get_res['testkey']))

        get_res['testkey'] = mcr.leaseGet('testkey')
        self.assertFalse(get_res['testkey']['token'])
        self.assertEqual(get_res['testkey']['value'], 'bizbang-lease-fail')

        # the failover should have set it with a much shorter TTL
        # so make sure that we can't get the value after the TTL
        # has expired
        time.sleep(4)
        get_res['testkey'] = mcr.leaseGet('testkey')
        self.assertGreater(get_res['testkey']['token'], 0)
        self.assertFalse(get_res['testkey']['value'])


class TestMetaGetFailover(McrouterTestCase):
    config = './mcrouter/test/test_get_failover.json'
    extra_args = []

    def setUp(self):
        self.gut = self.add_server(Memcached())
        self.wildcard = self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_metaget_failover(self):
        mcr = self.get_mcrouter()

        get_res = {}

        key_set_time = int(time.time())
        self.assertTrue(mcr.set('testkey', 'bizbang', exptime=100))
        key_after_set_time = int(time.time())

        get_res = mcr.metaget('testkey')
        self.assertIn(int(get_res['exptime']),
                      range(key_set_time + 100, key_after_set_time + 101))

        self.wildcard.terminate()

        self.assertTrue(mcr.set('testkey', 'bizbang-fail'))
        self.assertEqual(mcr.get('testkey'), 'bizbang-fail')
        get_res = mcr.metaget('testkey')
        self.assertAlmostEqual(int(get_res['exptime']),
                               int(time.time()) + 3,
                               delta=1)

        # the failover should have set it with a much shorter TTL
        # so make sure that we can't get the value after the TTL
        # has expired
        time.sleep(4)
        self.assertEqual(mcr.metaget('testkey'), {})
        self.assertEqual(mcr.get('testkey'), None)


class TestFailoverWithLimit(McrouterTestCase):
    config = './mcrouter/test/test_failover_limit.json'

    def setUp(self):
        self.gut = self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(self.config)

    def test_failover_limit(self):
        mcr = self.get_mcrouter()

        # first 12 requests should succeed (9.8 - 1 + 0.2 * 11 - 11 = 0)
        self.assertTrue(mcr.set('key', 'value.gut'))
        for _ in range(11):
            self.assertEqual(mcr.get('key'), 'value.gut')
        # now every 5th request should succeed
        for _ in range(10):
            for _ in range(4):
                self.assertIsNone(mcr.get('key'))
            self.assertEqual(mcr.get('key'), 'value.gut')


# this test behaves exactly like TestFailoverWithLimit test above because
# TKO errors are ignored in the ratelim calcualtions
class TestFailoverWithLimitWithTKO(McrouterTestCase):
    config = './mcrouter/test/test_failover_limit_error.json'
    extra_args = ['--timeouts-until-tko', '5']

    def setUp(self):
        self.gutA = self.add_server(ConnectionErrorServer())
        self.gutB = self.add_server(ConnectionErrorServer())
        self.gutC = self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_failover_limit(self):
        mcr = self.get_mcrouter()

        self.assertTrue(mcr.set('key', 'value.gut'))
        for _ in range(11):
            self.assertEqual(mcr.get('key'), 'value.gut')
        # now every 5th request should succeed
        for _ in range(10):
            for _ in range(4):
                self.assertIsNone(mcr.get('key'))
            self.assertEqual(mcr.get('key'), 'value.gut')


# Create two sleep servers which generates timeout errors
class TestFailoverWithLimitWithErrors(McrouterTestCase):
    config = './mcrouter/test/test_failover_limit_error.json'
    extra_args = ['--timeouts-until-tko', '5']

    def setUp(self):
        self.gutA = self.add_server(SleepServer())
        self.gutB = self.add_server(SleepServer())
        self.gutC = self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_failover_limit(self):
        mcr = self.get_mcrouter()

        # Each operation takes 3 token because first two tries would
        # fail due to timeout errors. So only 3 (one set and two get
        # operations would succeed before rate limiting kicks in)
        self.assertTrue(mcr.set('key', 'value.gut'))
        for _ in range(2):
            self.assertEqual(mcr.get('key'), 'value.gut')

        # all subsequest requests would fail until timeouts become
        # as TKOs
        for _ in range(18):
            self.assertIsNone(mcr.get('key'))

        # From here it should behave like the testcase above because
        # all destinations are declared TKO and ratelimiting is not
        # applicable to those
        for _ in range(10):
            self.assertEqual(mcr.get('key'), 'value.gut')
            for _ in range(4):
                self.assertIsNone(mcr.get('key'))


class TestFailoverWithLimitWithTKOAndErrors(McrouterTestCase):
    config = './mcrouter/test/test_failover_limit_error.json'
    extra_args = ['--timeouts-until-tko', '5']

    def setUp(self):
        self.gutA = self.add_server(SleepServer())
        self.gutB = self.add_server(ConnectionErrorServer())
        self.gutC = self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_failover_limit(self):
        mcr = self.get_mcrouter()

        self.assertTrue(mcr.set('key', 'value.gut'))
        # Each operation takes 3 token because first two tries would
        # fail due to timeout errors. So only 5 (one set and two get
        # operations would succeed before rate limiting kicks in)
        for _ in range(4):
            self.assertEqual(mcr.get('key'), 'value.gut')
        self.assertIsNone(mcr.get('key'))
        # All dests are TKO now, so now every 5th request should succeed
        for _ in range(10):
            self.assertEqual(mcr.get('key'), 'value.gut')
            for _ in range(4):
                self.assertIsNone(mcr.get('key'))
