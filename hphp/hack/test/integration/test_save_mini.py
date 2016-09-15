from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import json
import os
import shlex
import shutil
import stat
import subprocess
import tempfile
import time
import unittest

import common_tests
import hierarchy_tests

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
            write_echo_json(f, {
                'state': self.saved_state_path(),
                'is_cached': True,
                'deptable': self.saved_state_path() + '.deptable',
                })
            write_echo_json(f, {
                'changes': changed_files,
                })
            os.fchmod(f.fileno(), 0o700)

        self.write_local_conf()
        self.write_hhconfig('server_options.sh')
        self.write_watchman_config()

    def run_check(self, stdin=None, options=None):
        options = [] if options is None else options
        root = self.repo_dir + os.path.sep
        return self.proc_call([
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


class LazyDeclTestDriver(MiniStateTestDriver):
    def write_local_conf(self):
        with open(os.path.join(self.repo_dir, 'hh.conf'), 'w') as f:
            f.write(r"""
# some comment
use_mini_state = true
use_watchman = true
lazy_decl = true
""")

class MiniStateCommonTests(common_tests.CommonTests, MiniStateTestDriver,
        unittest.TestCase):
    pass

class LazyDeclCommonTests(common_tests.CommonTests, LazyDeclTestDriver,
        unittest.TestCase):
    pass

class MiniStateHierarchyTests(hierarchy_tests.HierarchyTests,
        MiniStateTestDriver, unittest.TestCase):
    pass

class LazyDeclHierarchyTests(hierarchy_tests.HierarchyTests,
        LazyDeclTestDriver, unittest.TestCase):
    def test_failed_decl(self):
        super().test_failed_decl()

class MiniStateTests(MiniStateTestDriver, unittest.TestCase):
    """
    Tests in this class are specific to saved state; would not make sense
    for them to run on a fresh init
    """
    template_repo = 'hphp/hack/test/integration/data/simple_repo'

    def test_no_state_found(self):
        error_msg = 'No such rev'
        with open(os.path.join(self.repo_dir, 'server_options.sh'), 'w') as f:
            f.write("#! /bin/sh\n")
            write_echo_json(f, {
                'error': error_msg,
                })
            os.fchmod(f.fileno(), 0o700)

        self.write_local_conf()
        self.write_hhconfig('server_options.sh')
        self.write_watchman_config()

        (output, _, _) = self.run_check()

        self.assertEqual(output.strip(), 'No errors!')

        logs = self.get_server_logs()
        self.assertIn('Could not load mini state', logs)
        self.assertIn(error_msg, logs)

    def test_get_changes_failure(self):
        error_msg = 'hg is not playing nice today'
        with open(os.path.join(self.repo_dir, 'server_options.sh'), 'w') as f:
            f.write("#! /bin/sh\n")
            write_echo_json(f, {
                'state': self.saved_state_path(),
                'is_cached': True,
                'deptable': self.saved_state_path() + '.deptable',
                })
            write_echo_json(f, {
                'error': error_msg,
                })
            os.fchmod(f.fileno(), 0o700)

        self.write_local_conf()
        self.write_hhconfig('server_options.sh')
        self.write_watchman_config()

        (output, _, _) = self.run_check()

        self.assertEqual(output.strip(), 'No errors!')

        logs = self.get_server_logs()
        self.assertIn('Could not load mini state', logs)
        self.assertIn(error_msg, logs)

    def test_hhconfig_change(self):
        """
        Start hh_server, then change .hhconfig and check that the server
        restarts itself
        """
        self.write_load_config()
        self.check_cmd(['No errors!'])
        with open(os.path.join(self.repo_dir, '.hhconfig'), 'w') as f:
            f.write(r"""
# some comment
assume_php = true
load_mini_script = %s
""" % os.path.join(self.repo_dir, 'server_options.sh'))

        # Server may take some time to kill itself.
        time.sleep(2)

        # this should start a new server
        self.check_cmd(['No errors!'])
        # check how the old one exited
        log_file = self.proc_call([
            hh_client, '--logname', self.repo_dir]
            )[0].strip() + '.old'
        with open(log_file) as f:
            logs = f.read()
            self.assertIn('.hhconfig changed in an incompatible way', logs)

    def test_watchman_timeout(self):
        self.write_load_config()

        with open(os.path.join(self.repo_dir, 'hh.conf'), 'a') as f:
            f.write(r"""
watchman_init_timeout = 1
""")

        with open(os.path.join(self.bin_dir, 'watchman'), 'w') as f:
            f.write(r"""sleep 2""")
            os.fchmod(f.fileno(), stat.S_IRWXU)

        self.run_check()
        # Stop the server, ensuring that its logs get flushed
        self.proc_call([hh_client, 'stop', self.repo_dir])
        self.assertIn('Watchman.Timeout', self.get_server_logs())
