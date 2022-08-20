#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import errno
import re
import socket
import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import ConnectionErrorServer
from mcrouter.test.mock_servers import CustomErrorServer
from mcrouter.test.mock_servers import SleepServer


class TestMcrouterForwardedErrors(McrouterTestCase):
    config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    get_cmd = 'get test_key\r\n'
    gat_cmd = 'gat 999 test_key\r\n'
    set_cmd = 'set test_key 0 0 3\r\nabc\r\n'
    delete_cmd = 'delete test_key\r\n'
    append_cmd = 'append test_key 0 0 3\r\nabc\r\n'
    prepend_cmd = 'prepend test_key 0 0 3\r\nabc\r\n'
    touch_cmd = 'touch test_key 3600\r\n'
    server_errors = [
        'SERVER_ERROR out of order',
        'SERVER_ERROR timeout',
        'SERVER_ERROR connection timeout',
        'SERVER_ERROR connection error',
        'SERVER_ERROR 307 busy',
        'SERVER_ERROR 302 try again',
        'SERVER_ERROR unavailable',
        'SERVER_ERROR bad value',
        'SERVER_ERROR aborted',
        'SERVER_ERROR local error',
        'SERVER_ERROR remote error',
        'SERVER_ERROR waiting'
    ]
    client_errors = [
        'CLIENT_ERROR bad command',
        'CLIENT_ERROR bad key',
        'CLIENT_ERROR bad flags',
        'CLIENT_ERROR bad exptime',
        'CLIENT_ERROR bad lease_id',
        'CLIENT_ERROR bad cas_id',
        'CLIENT_ERROR malformed request',
        'CLIENT_ERROR out of memory'
    ]

    def setUp(self):
        self.server = self.add_server(CustomErrorServer())

    # server returned: SERVER_ERROR
    def test_server_replied_server_error_for_set(self):
        cmd = self.set_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.server_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    def test_server_replied_server_error_for_get(self):
        cmd = self.get_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.server_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual('END\r\n', res)
            mcrouter.terminate()

    def test_server_replied_server_error_for_get_with_no_miss_on_error(self):
        # With --disable-miss-on-get-errors, errors should be forwarded
        # to client
        cmd = self.get_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.server_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config,
                    extra_args=['--disable-miss-on-get-errors'])
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    def test_server_replied_server_error_for_gat(self):
        # gat command should behave as get on output
        cmd = self.gat_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.server_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual('END\r\n', res)
            mcrouter.terminate()

    def test_server_replied_server_error_for_gat_with_no_miss_on_error(self):
        # With --disable-miss-on-get-errors, errors should be forwarded
        # to client. gat command should behave as get on output.
        cmd = self.gat_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.server_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config,
                    extra_args=['--disable-miss-on-get-errors'])
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    def test_server_replied_server_error_for_append(self):
        cmd = self.append_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.server_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    def test_server_replied_server_error_for_prepend(self):
        cmd = self.prepend_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.server_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    def test_server_replied_server_error_for_delete(self):
        cmd = self.delete_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.server_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual('NOT_FOUND\r\n', res)
            mcrouter.terminate()

    def test_server_replied_server_error_for_touch(self):
        cmd = self.touch_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.server_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    def test_server_replied_server_error_for_delete_with_no_asynclog(self):
        # With --asynclog-disable, errors should be forwarded to client
        cmd = self.delete_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.server_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config,
                    extra_args=['--asynclog-disable'])
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    # server returned: CLIENT_ERROR
    def test_server_replied_client_error_for_set(self):
        cmd = self.set_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.client_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    def test_server_replied_client_error_for_get(self):
        cmd = self.get_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.client_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual('END\r\n', res)
            mcrouter.terminate()

    def test_server_replied_client_error_for_get_with_no_miss_on_error(self):
        # With --disable-miss-on-get-errors, errors should be forwarded
        # to client
        cmd = self.get_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.client_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config,
                    extra_args=['--disable-miss-on-get-errors'])
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    def test_server_replied_client_error_for_gat(self):
        # gat command should behave as get on output
        cmd = self.gat_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.client_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual('END\r\n', res)
            mcrouter.terminate()

    def test_server_replied_client_error_for_gat_with_no_miss_on_error(self):
        # With --disable-miss-on-get-errors, errors should be forwarded
        # to client. gat command should behave as get on output
        cmd = self.gat_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.client_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config,
                    extra_args=['--disable-miss-on-get-errors'])
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    def test_server_replied_client_error_for_delete(self):
        cmd = self.delete_cmd
        self.server.setExpectedBytes(len(cmd))
        for error in self.client_errors:
            self.server.setError(error)
            mcrouter = self.add_mcrouter(self.config)
            res = mcrouter.issue_command(cmd)
            self.assertEqual(error + '\r\n', res)
            mcrouter.terminate()

    def test_early_server_reply(self):
        value_len = 1024 * 1024 * 4
        value = 'a' * value_len
        cmd_header = 'set test:key 0 0 {}\r\n'.format(value_len)
        cmd = cmd_header + value + '\r\n'
        error = 'SERVER_ERROR out of memory'
        self.server.setError(error)
        # reply before reading the value part of request
        self.server.setExpectedBytes(len(cmd), len(cmd_header))
        # sleep for one second to ensure that there's a gap between
        # request write and reply read events in mcrouter
        self.server.setSleepAfterReply(1)
        mcrouter = self.add_mcrouter(self.config,
                                     extra_args=['--server-timeout', '2000'])
        res = mcrouter.issue_command(cmd)
        self.assertEqual(error + '\r\n', res)


class TestMcrouterGeneratedErrors(McrouterTestCase):
    config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    get_cmd = 'get test_key\r\n'
    gat_cmd = 'gat 999 test_key\r\n'
    set_cmd = 'set test_key 0 0 3\r\nabc\r\n'
    delete_cmd = 'delete test_key\r\n'

    def getMcrouter(self, server, args=()):
        self.add_server(server)
        return self.add_mcrouter(self.config, extra_args=args)

    # mcrouter generated: timeout
    def test_timeout_set(self):
        mcrouter = self.getMcrouter(SleepServer())
        res = mcrouter.issue_command(self.set_cmd)
        self.assertEqual('SERVER_ERROR Reply timeout\r\n', res)

    def test_timeout_get(self):
        mcrouter = self.getMcrouter(SleepServer())
        res = mcrouter.issue_command(self.get_cmd)
        self.assertEqual('END\r\n', res)

    def test_timeout_gat(self):
        mcrouter = self.getMcrouter(SleepServer())
        res = mcrouter.issue_command(self.gat_cmd)
        self.assertEqual('END\r\n', res)

    def test_timeout_delete(self):
        mcrouter = self.getMcrouter(SleepServer())
        res = mcrouter.issue_command(self.delete_cmd)
        self.assertEqual('NOT_FOUND\r\n', res)

    # mcrouter generated: connection error
    def test_connection_error_set(self):
        mcrouter = self.getMcrouter(ConnectionErrorServer())
        res = mcrouter.issue_command(self.set_cmd)
        print(res)
        self.assertTrue(
            re.match(
                'SERVER_ERROR (connection error|remote error'
                '|Failed to read|AsyncSocketException)',
                res
            )
        )

    def test_connection_error_get(self):
        mcrouter = self.getMcrouter(ConnectionErrorServer())
        res = mcrouter.issue_command(self.get_cmd)
        self.assertEqual('END\r\n', res)

    def test_connection_error_gat(self):
        mcrouter = self.getMcrouter(ConnectionErrorServer())
        res = mcrouter.issue_command(self.gat_cmd)
        self.assertEqual('END\r\n', res)

    def test_connection_error_delete(self):
        mcrouter = self.getMcrouter(ConnectionErrorServer())
        res = mcrouter.issue_command(self.delete_cmd)
        self.assertEqual('NOT_FOUND\r\n', res)

    # mcrouter generated: TKO
    def test_tko_set(self):
        mcrouter = self.getMcrouter(SleepServer(),
                args=['--timeouts-until-tko', '1'])
        res = mcrouter.issue_command(self.set_cmd)
        res = mcrouter.issue_command(self.set_cmd)
        self.assertTrue(
            re.match('SERVER_ERROR Server unavailable. Reason: .*', res))

    def test_tko_get(self):
        mcrouter = self.getMcrouter(SleepServer(),
                args=['--timeouts-until-tko', '1'])
        res = mcrouter.issue_command(self.set_cmd)
        res = mcrouter.issue_command(self.get_cmd)
        self.assertEqual('END\r\n', res)

    def test_tko_gat(self):
        mcrouter = self.getMcrouter(SleepServer(),
                args=['--timeouts-until-tko', '1'])
        res = mcrouter.issue_command(self.set_cmd)
        res = mcrouter.issue_command(self.gat_cmd)
        self.assertEqual('END\r\n', res)

    def test_tko_delete(self):
        mcrouter = self.getMcrouter(SleepServer(),
                args=['--timeouts-until-tko', '1'])
        res = mcrouter.issue_command(self.set_cmd)
        res = mcrouter.issue_command(self.delete_cmd)
        self.assertEqual('NOT_FOUND\r\n', res)

    # mcrouter generated: bad key
    def test_bad_key_set(self):
        mcrouter = self.getMcrouter(Memcached())
        cmd = 'set test.key' + ('x' * 10000) + ' 0 0 3\r\nabc\r\n'
        res = mcrouter.issue_command(cmd)
        self.assertEqual('CLIENT_ERROR bad key\r\n', res)

    def test_bad_key_get(self):
        mcrouter = self.getMcrouter(Memcached())
        cmd = 'get test.key' + ('x' * 10000) + '\r\n'
        res = mcrouter.issue_command(cmd)
        self.assertEqual('CLIENT_ERROR bad key\r\n', res)

    def test_bad_key_delete(self):
        mcrouter = self.getMcrouter(Memcached())
        cmd = 'delete test.key' + ('x' * 10000) + '\r\n'
        res = mcrouter.issue_command(cmd)
        self.assertEqual('CLIENT_ERROR bad key\r\n', res)

    def test_bad_key_append(self):
        mcrouter = self.getMcrouter(Memcached())
        cmd = 'append test.key' + ('x' * 10000) + ' 0 0 3\r\nabc\r\n'
        res = mcrouter.issue_command(cmd)
        self.assertEqual('CLIENT_ERROR bad key\r\n', res)

    def test_bad_key_prepend(self):
        mcrouter = self.getMcrouter(Memcached())
        cmd = 'prepend test.key' + ('x' * 10000) + ' 0 0 3\r\nabc\r\n'
        res = mcrouter.issue_command(cmd)
        self.assertEqual('CLIENT_ERROR bad key\r\n', res)

    # mcrouter generated: remote error
    def test_remote_error_command_not_supported(self):
        mcrouter = self.getMcrouter(Memcached())
        cmd = 'flush_all\r\n'
        res = mcrouter.issue_command(cmd)
        self.assertEqual('SERVER_ERROR Command disabled\r\n', res)


class TestMcrouterParseError(McrouterTestCase):
    config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    get_cmd = 'get test_key\r\n'

    def get_mcrouter(self, server, args):
        self.add_server(server)
        return self.add_mcrouter(self.config, extra_args=args)

    def connect(self, addr):
        while True:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect(addr)
                return sock
            except Exception as e:
                if e.errno == errno.ECONNREFUSED:
                    pass
                else:
                    raise

    # Test that server properly handles parsing errors.
    def test_parse_error(self):
        mcrouter = self.get_mcrouter(SleepServer(),
                                     ['--disable-miss-on-get-errors',
                                      '--server-timeout', '3000'])
        port = mcrouter.getport()
        addr = ('localhost', port)
        sock = self.connect(addr)
        fd = sock.makefile()
        # First send a normal request that will timeout followed by
        # mallformed request that will cause parsing error.

        req1 = "{}{}".format(self.get_cmd, 'get\r\n')
        sock.sendall(req1.encode())
        # Make sure the buffer is flushed.
        time.sleep(1)
        # Now send another normal request, it shouldn't be processed, since
        # server cannot parse further.
        sock.sendall(self.get_cmd.encode())

        self.assertEqual('SERVER_ERROR Reply timeout', fd.readline().strip())
        self.assertEqual('CLIENT_ERROR malformed request',
                          fd.readline().strip())

        # Check that mcrouter is still alive.
        self.assertTrue(mcrouter.is_alive())
        res = mcrouter.issue_command(self.get_cmd)
        self.assertEqual('SERVER_ERROR Reply timeout\r\n', res)
