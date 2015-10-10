from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import common_tests
import os
import subprocess
import unittest

from hh_paths import hh_server, hh_client
from utils import touch, proc_call, test_env

class TestSaveMiniState(common_tests.CommonSaveStateTests, unittest.TestCase):
    @classmethod
    def save_command(cls, init_dir):
        subprocess.call([
            hh_server,
            '--check', init_dir,
            '--save-mini', cls.saved_state_path()
        ], env=test_env)

    def write_local_conf(self):
        with open(os.path.join(self.repo_dir, 'hh.conf'), 'w') as f:
            f.write(r"""
# some comment
use_mini_state = true
""")

    def write_hhconfig(self, script_name):
        with open(os.path.join(self.repo_dir, '.hhconfig'), 'w') as f:
            f.write(r"""
# some comment
assume_php = false
load_mini_script = %s
""" % os.path.join(self.repo_dir, script_name))

    def write_load_config(self, *changed_files):
        with open(os.path.join(self.repo_dir, 'server_options.sh'), 'w') as f:
            f.write(r"""
#! /bin/sh
echo %s
""" % self.saved_state_path())
            os.fchmod(f.fileno(), 0o700)
            for fn in changed_files:
                f.write("echo %s\n" % fn)
            os.fchmod(f.fileno(), 0o700)

        self.write_local_conf()
        self.write_hhconfig('server_options.sh')

    def check_cmd(self, expected_output, stdin=None, options=None):
        options = [] if options is None else options
        root = self.repo_dir + os.path.sep
        output = proc_call([
            hh_client,
            'check',
            '--retries',
            '20',
            self.repo_dir
            ] + list(map(lambda x: x.format(root=root), options)),
            env={'HH_LOCALCONF_PATH': self.repo_dir},
            stdin=stdin)
        log_file = proc_call([hh_client, '--logname', self.repo_dir]).strip()
        with open(log_file) as f:
            logs = f.read()
            self.assertIn('Mini-state loading worker took', logs)
        self.assertCountEqual(
            map(lambda x: x.format(root=root), expected_output),
            output.splitlines())

    def test_no_state_found(self):
        with open(os.path.join(self.repo_dir, 'server_options.sh'), 'w') as f:
            # exit without printing anything
            f.write(r"#! /bin/sh")
            os.fchmod(f.fileno(), 0o700)

        self.write_local_conf()
        self.write_hhconfig('server_options.sh')

        output = proc_call([
            hh_client,
            'check',
            '--retries',
            '20',
            self.repo_dir
            ],
            env={'HH_LOCALCONF_PATH': self.repo_dir})

        self.assertEqual(output.strip(), 'No errors!')

        log_file = proc_call([hh_client, '--logname', self.repo_dir]).strip()
        with open(log_file) as f:
            logs = f.read()
            self.assertIn('Could not load mini state', logs)
