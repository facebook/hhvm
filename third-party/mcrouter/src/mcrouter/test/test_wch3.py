#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


from collections import defaultdict

from mcrouter.test.mock_servers import MockServer
from mcrouter.test.McrouterTestCase import McrouterTestCase


class EchoServer(MockServer):
    """A server that responds to get requests with its port number.
    """

    def runServer(self, client_socket, client_address):
        while not self.is_stopped():
            cmd = client_socket.recv(1000)
            if not cmd:
                return
            if cmd.decode().startswith('get'):
                reply = "VALUE hit 0 {size}\r\n{payload}\r\nEND\r\n".format(
                    size=len(str(self.port)),
                    payload=self.port
                )
                client_socket.send(reply.encode())


class TestWCH3(McrouterTestCase):
    config = './mcrouter/test/test_wch3.json'
    extra_args = []

    def setUp(self):
        for _ in range(8):
            self.add_server(EchoServer())

        self.mcrouter = self.add_mcrouter(
            self.config,
            '/test/A/',
            extra_args=self.extra_args)

    def test_wch3(self):
        valid_ports = []
        for i in [1, 2, 4, 5, 6, 7]:
            valid_ports.append(self.get_open_ports()[i])
        invalid_ports = []
        for i in [0, 3]:
            invalid_ports.append(self.get_open_ports()[i])
        request_counts = defaultdict(int)
        n = 20000
        for i in range(0, n):
            key = 'someprefix:{}:|#|id=123'.format(i)
            resp = int(self.mcrouter.get(key))
            respB = int(self.mcrouter.get('/test/B/' + key))
            respC = int(self.mcrouter.get('/test/C/' + key))
            self.assertEqual(resp, respB)
            self.assertEqual(resp, respC)
            request_counts[resp] += 1
            self.assertTrue(resp in valid_ports)
            self.assertTrue(resp not in invalid_ports)
        # Make sure that the fraction of keys to a server are what we expect
        # within a tolerance
        expected_fractions = {
            0: 0,
            1: 1,
            2: 1,
            3: 0.0,
            4: 0.5,
            5: 1,
            6: 0.3,
            7: 0.5
        }
        tolerance = 0.075
        total_weight = sum(expected_fractions.values())
        for i, weight in expected_fractions.items():
            expected_frac = weight / total_weight
            port = int(self.get_open_ports()[i])
            measured_frac = request_counts[port] / float(n)
            if expected_frac > 0:
                self.assertAlmostEqual(
                    measured_frac,
                    expected_frac,
                    delta=tolerance
                )
            else:
                self.assertEqual(measured_frac, 0.0)
