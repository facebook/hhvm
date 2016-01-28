from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import common_tests
import os
import subprocess
import sys
import unittest

from hh_paths import hh_server, hh_client
from utils import ensure_output_contains

class TestSaveRestore(common_tests.CommonSaveStateTests, unittest.TestCase):

    @classmethod
    def save_command(cls, init_dir):
        cls.proc_call([
            hh_server,
            '--check', init_dir,
            '--save', os.path.join(cls.tmp_dir, 'foo'),
        ])

    def write_local_conf(self):
        with open(os.path.join(self.repo_dir, 'hh.conf'), 'w') as f:
            f.write(r"""
# some comment
use_mini_state = false
""")

    def write_load_config(self, *changed_files):
        """
        repo_dir: Repository to run hh_server on
        saved_state_path: Path to file containing saved server state
        changed_files: list of strings
        """

        with open(os.path.join(self.repo_dir, 'server_options.sh'), 'w') as f:
            f.write(r"""
#! /bin/sh
echo %s
""" % self.saved_state_path())
            os.fchmod(f.fileno(), 0o700)
            for fn in changed_files:
                f.write("echo %s\n" % fn)
            os.fchmod(f.fileno(), 0o700)

        with open(os.path.join(self.repo_dir, '.hhconfig'), 'w') as f:
            # we can't just write 'echo ...' inline because Hack server will
            # be passing this command some command-line options
            f.write(r"""
# some comment
assume_php = false
load_script = %s
""" % os.path.join(self.repo_dir, 'server_options.sh'))

        self.write_local_conf()

    def check_cmd(self, expected_output, stdin=None, options=[]):
        root = self.repo_dir + os.path.sep
        (output, err, _) = self.proc_call([
            hh_client,
            'check',
            '--retries',
            '20',
            self.repo_dir
            ] + list(map(lambda x: x.format(root=root), options)),
            stdin=stdin)
        self.assertCountEqual(
            map(lambda x: x.format(root=root), expected_output),
            output.splitlines())
        return err

    def test_server_output(self):
        self.write_load_config()
        server_proc = self.start_hh_server()
        ensure_output_contains(server_proc.stderr,
                'Load state found at %s.' % self.saved_state_path())

    def test_options_cmd(self):
        """
        Make sure we are invoking the server_options_cmd with the right flags
        """
        args_file = os.path.join(self.tmp_dir, 'cmd_args')
        with open(os.path.join(self.repo_dir, 'server_options.sh'), 'w') as f:
            f.write(r"""
#! /bin/sh
echo "$1" > {out}
echo "$2" >> {out}
            """.format(out=args_file))
            os.fchmod(f.fileno(), 0o700)

        with open(os.path.join(self.repo_dir, '.hhconfig'), 'w') as f:
            f.write(r"""
# some comment
assume_php = false
load_script = %s
            """ % os.path.join(self.repo_dir, 'server_options.sh'))

        self.check_cmd(['No errors!'])

        (version, _, _) = self.proc_call([
            hh_server,
            '--version'
        ])

        with open(args_file) as f:
            self.assertEqual(f.read().splitlines(), [self.repo_dir, version])
