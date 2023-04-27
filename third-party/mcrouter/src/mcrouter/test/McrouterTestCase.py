#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import os
import unittest
import time

from mcrouter.test.MCProcess import Mcrouter, Memcached, MockMemcached


class McrouterTestCase(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.use_mock_mc = False

    def ensureClassVariables(self):
        if 'open_servers' not in self.__dict__:
            self.open_servers = []
        if 'open_ports' not in self.__dict__:
            self.open_ports = []
        if 'open_ports_sr' not in self.__dict__:
            self.open_ports_sr = []

    @classmethod
    def wait_for_file(cls, path, *, retries=0, interval=0.25):
        interval = interval if interval > 0 else 0.25
        while True:
            if os.path.exists(path):
                return True
            retries -= 1
            if retries < 0:
                return False
            time.sleep(interval)

    @classmethod
    def wait_noempty_dir(cls, root, *, retries=0, interval=1):
        while True:
            file_count = 0
            for _, _, files in os.walk(root):
                file_count += len(files)
            if file_count > 0:
                return True
            retries -= 1
            if retries < 0:
                return False
            time.sleep(interval)

    def add_server(self, server, logical_port=None, sr_routing=False):
        self.ensureClassVariables()
        server.ensure_connected()
        self.open_servers.append(server)
        if sr_routing:
                self.open_ports_sr.append(server.get_secondary_port())
        else:
            if server.getsslport() is not None:
                self.open_ports.append(server.getsslport())
            else:
                self.open_ports.append(server.getport())

        if logical_port:
            if 'port_map' not in self.__dict__:
                self.port_map = {}
            if logical_port in self.port_map:
                raise Exception("logical_port %d was already used"
                                % logical_port)
            self.port_map[logical_port] = server.getport()

        return server

    def add_mcrouter(self, config, route=None, extra_args=None,
                     replace_map=None, bg_mcrouter=False, replace_ports=True,
                     flavor=None, sr_mock_smc_config=None, base_dir=None):
        self.ensureClassVariables()
        substitute_ports = None
        if replace_ports:
            substitute_ports = (self.open_ports
                                if 'port_map' not in self.__dict__
                                else self.port_map)

        mcrouter = Mcrouter(config,
                            substitute_config_ports=substitute_ports,
                            substitute_config_smc_ports=self.open_ports_sr,
                            default_route=route,
                            extra_args=extra_args,
                            replace_map=replace_map,
                            flavor=flavor,
                            sr_mock_smc_config=sr_mock_smc_config,
                            base_dir=base_dir)
        mcrouter.ensure_connected()

        if bg_mcrouter:
            self.open_ports.append(mcrouter.getport())

        if 'open_mcrouters' not in self.__dict__:
            self.open_mcrouters = []
        self.open_mcrouters.append(mcrouter)
        return mcrouter

    def make_memcached(self):
        return MockMemcached() if self.use_mock_mc else Memcached()

    def get_open_ports(self):
        self.ensureClassVariables()
        return self.open_ports

    def tearDown(self):
        # Stop mcrouters first to close connections to servers
        # (some mock severs might be blocked on recv() calls)
        if 'open_mcrouters' in self.__dict__:
            for mcr in self.open_mcrouters:
                mcr.terminate()

        if 'open_servers' in self.__dict__:
            for server in self.open_servers:
                server.terminate()

    def eventually_get(self, key, expVal, retries=20):
        start_time = time.time()
        interval = 0.5
        while retries > 0:
            if (self.mc.get(key) == expVal):
                return True
            time.sleep(interval)
            retries -= 1
        return False

    def assert_eventually_true(self, condition):
        retry = 0
        max_retry = 6 # don't increase too much, at worst with 6 it takes ~1 minute
        sleep_time = 1
        result = False
        while retry < max_retry:
            if (condition()):
                result = True
                break
            retry += 1
            time.sleep(sleep_time)
            sleep_time *= 2
        self.assertTrue(result)


    def _is_mcrouter_running(self, mcrouter):
        try:
            return bool(mcrouter.stats())

        except Exception as e:
            self.assertIsInstance(e, ConnectionResetError)
            return False
