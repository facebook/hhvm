#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import MockServer

class TimeoutServer(MockServer):
    """A server that responds to requests with 'END' after reading expected
    amount of bytes and waiting for timeout seconds"""
    def __init__(self, expected_key, timeout):
        super(TimeoutServer, self).__init__()
        self.expected_bytes = len("get \r\n")
        self.expected_bytes += len(expected_key)
        self.timeout = timeout
        self.seenRequests = 0

    def runServer(self, client_socket, client_address):
        while not self.is_stopped():
            client_socket.recv(self.expected_bytes)
            self.seenRequests = self.seenRequests + 1
            time.sleep(self.timeout)
            client_socket.send(b'END\r\n')

    def getSeenRequests(self):
        return self.seenRequests


class TestServerStatsOutstandingRequests(McrouterTestCase):
    config = './mcrouter/test/test_max_shadow_requests.json'
    extra_args = ['-t', '1000000', '--target-max-shadow-requests', '2']

    def setUp(self):
        # The order here must corresponds to the order of hosts in the .json
        self.add_server(Memcached())
        self.timeoutServer = TimeoutServer('test', 0.5)
        self.add_server(self.timeoutServer)
        self.mcrouter = self.add_mcrouter(
            self.config, extra_args=self.extra_args
        )

    def test_max_shadow_requests(self):
        for _ in range(0, 10):
            self.mcrouter.get('test')

        time.sleep(1.5)
        self.assertEqual(self.timeoutServer.getSeenRequests(), 2)
