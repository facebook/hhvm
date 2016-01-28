from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import common_tests
import os
import time
import unittest

from hh_paths import hh_client

class TestFreshInit(common_tests.CommonSaveStateTests, unittest.TestCase):

    @classmethod
    def save_command(cls, init_dir):
        pass

    def write_load_config(self, *changed_files):
        with open(os.path.join(self.repo_dir, '.hhconfig'), 'w') as f:
            f.write("assume_php = false\n")

    def check_cmd(self, expected_output, stdin=None, options=(), retries=3):
        time.sleep(2)  # wait for Hack to catch up with file system changes

        root = self.repo_dir + os.path.sep
        (output, err, retcode) = self.proc_call([
            hh_client,
            'check',
            '--retries',
            '20',
            '--no-load',
            self.repo_dir
            ] + list(map(lambda x: x.format(root=root), options)),
            stdin=stdin)

        if retcode == 6 and retries > 0:
            # this sometimes happens and retrying helps
            return self.check_cmd(expected_output, stdin, options, retries - 1)
        self.assertIn(retcode, [0, 2])

        self.assertCountEqual(
            map(lambda x: x.format(root=root), expected_output),
            output.splitlines())
        return err
