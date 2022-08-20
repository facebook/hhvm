#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import time

from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import CustomErrorServer
from mcrouter.test.mock_servers import MockServer


class TestAsciiError(McrouterTestCase):
    config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    extra_args = []

    def setUp(self):
        self.add_server(CustomErrorServer(len('test_error'), 'ERROR\r\nEND'))
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_ascii_error(self):
        resp = self.mcrouter.get('test_error')
        self.assertEqual(None, resp)

class ExtraValueServer(MockServer):
    """A server that responds normally to probes.
       But for get request it will respond with a proper reply plus some
       additional reply"""
    def __init__(self, expected_key):
        super(ExtraValueServer, self).__init__()
        self.expected_key = expected_key
        self.expected_bytes = len('get ' + expected_key + '\r\n')

    def runServer(self, client_socket, client_address):
        # Read first char to see if it's a probe or a get request.
        char = client_socket.recv(1).decode()
        if char.lower() == 'v':
            # Read the remaing part of version request.
            client_socket.recv(len('ERSION\r\n'))
            client_socket.send(b'VERSION test\r\n')
            # Read the first char of get request.
            client_socket.recv(1)
        # Read remaining part of get request.
        client_socket.recv(self.expected_bytes - 1)
        extra_value = 'testValue'
        payload = (
            'VALUE {key} 0 {size}\r\n{value}\r\nEND\r\n'
            # extra reply to tko this server
            'VALUE test2 0 1\r\nV\r\nEND\r\n'.format(
                key=self.expected_key,
                size=len(extra_value),
                value=extra_value,
            )
        )
        client_socket.send(payload.encode())

class TestAsciiExtraData(McrouterTestCase):
    config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    extra_args = ['--probe-timeout-initial', '100']

    def setUp(self):
        self.add_server(ExtraValueServer('test'))
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_ascii_extra_data(self):
        # Send initial get.
        resp = self.mcrouter.get('test')
        self.assertEqual('testValue', resp)
        # Send another get, that will fail because of TKO.
        resp = self.mcrouter.get('test')
        self.assertEqual(None, resp)
        # Allow mcrouter some time to recover.
        time.sleep(1)
        # Send another get request that should succeed.
        resp = self.mcrouter.get('test')
        self.assertEqual('testValue', resp)
