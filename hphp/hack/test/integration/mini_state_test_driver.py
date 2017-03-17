from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import json
import os
import shlex
import shutil
import tempfile

import common_tests

from hh_paths import hh_server, hh_client


def write_echo_json(f, obj):
    f.write("echo %s\n" % shlex.quote(json.dumps(obj)))


class MiniStateTestDriver(common_tests.CommonTestDriver):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        # we create the state in a different dir from the one we run our tests
        # on, to verify that the saved state does not depend on any absolute
        # paths
        init_dir = os.path.join(cls.base_tmp_dir, 'init')
        shutil.copytree(cls.template_repo, init_dir)
        cls.saved_state_dir = tempfile.mkdtemp()
        cls.save_command(init_dir)
        shutil.rmtree(init_dir)

    @classmethod
    def tearDownClass(cls):
        # super().tearDownClass()
        shutil.rmtree(cls.saved_state_dir)

    @classmethod
    def saved_state_path(cls):
        return os.path.join(cls.saved_state_dir, 'foo')

    @classmethod
    def save_command(cls, init_dir):
        stdout, stderr, retcode = cls.proc_call([
            hh_server,
            '--check', init_dir,
            '--save-mini', cls.saved_state_path()
        ])
        if retcode != 0:
            raise Exception('Failed to save! stdout: "%s" stderr: "%s"' %
                            (stdout, stderr))

    def write_local_conf(self):
        with open(os.path.join(self.repo_dir, 'hh.conf'), 'w') as f:
            f.write(r"""
# some comment
use_mini_state = true
use_watchman = true
watchman_subscribe = true
""")

    def write_hhconfig(self, script_name):
        with open(os.path.join(self.repo_dir, '.hhconfig'), 'w') as f:
            f.write(r"""
# some comment
assume_php = false
load_mini_script = %s
auto_namespace_map = {"Herp": "Derp\\Lib\\Herp"}
""" % os.path.join(self.repo_dir, script_name))

    def write_watchman_config(self):
        with open(os.path.join(self.repo_dir, '.watchmanconfig'), 'w') as f:
            f.write('{}')

        os.mkdir(os.path.join(self.repo_dir, '.hg'))

    def write_load_config(self, *changed_files):
        with open(os.path.join(self.repo_dir, 'server_options.sh'), 'w') as f:
            f.write("#! /bin/sh\n")
            os.fchmod(f.fileno(), 0o700)
            write_echo_json(
                f,
                {
                    'state': self.saved_state_path(),
                    'corresponding_base_revision': '1',
                    'is_cached': True,
                    'deptable': self.saved_state_path() + '.sql',
                })
            write_echo_json(
                f,
                {
                    'changes': changed_files,
                })
            os.fchmod(f.fileno(), 0o700)

        self.write_local_conf()
        self.write_hhconfig('server_options.sh')
        self.write_watchman_config()

    def run_check(self, stdin=None, options=None):
        options = [] if options is None else options
        root = self.repo_dir + os.path.sep
        return self.proc_call(
            [
                hh_client,
                'check',
                '--retries',
                '20',
                self.repo_dir
            ] + list(map(lambda x: x.format(root=root), options)),
            stdin=stdin)

    def check_cmd(self, expected_output, stdin=None, options=None):
        (output, err, _) = self.run_check(stdin, options)
        logs = self.get_server_logs()
        self.assertIn('Using watchman', logs)
        self.assertIn('Successfully loaded mini-state', logs)
        root = self.repo_dir + os.path.sep
        self.assertCountEqual(
            map(lambda x: x.format(root=root), expected_output),
            output.splitlines())
        return err

    def assertEqualString(self, first, second, msg=None):
        root = self.repo_dir + os.path.sep
        second = second.format(root=root)
        self.assertEqual(first, second, msg)
