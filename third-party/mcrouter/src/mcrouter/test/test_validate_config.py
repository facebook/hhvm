#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import socket
from subprocess import Popen
import time
import unittest

from mcrouter.test.config import McrouterGlobals

# Test validate-config parameter
class TestValidateConfig(unittest.TestCase):
    valid_config = 'mcrouter/test/test_ascii.json'
    invalid_config = 'mcrouter/test/invalid_config.json'
    extra_args = []

    def try_mcrouter(self, config: str) -> int:
        listen_sock = socket.socket()
        listen_sock.listen(100)
        cmd = McrouterGlobals.preprocessArgs([
            McrouterGlobals.binPath('mcrouter'),
            '-L', '/tmp/test.log',
            '--config', 'file:' + config,
            '--listen-sock-fd', str(listen_sock.fileno()),
            '--stats-logging-interval', '0',
            '--config-dump-root', '',
            '--validate-config',
        ] + self.extra_args)
        proc = Popen(cmd)
        for _i in range(20):
            if proc.poll() is not None:
                break
            time.sleep(1)

        ret = proc.returncode

        if proc is not None and proc.poll() is None:
            proc.terminate()
        listen_sock.close()

        return ret

    def test_valid_config(self) -> None:
        ret = self.try_mcrouter(self.valid_config)
        self.assertEqual(ret, 0)

    def test_invalid_config(self) -> None:
        ret = self.try_mcrouter(self.invalid_config)
        self.assertNotEqual(ret, 0)
        self.assertNotEqual(ret, None)
