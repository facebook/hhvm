from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import json
import os
import shlex
import stat
import time
import unittest

import common_tests
import hierarchy_tests

from hh_paths import hh_client

from mini_state_test_driver import MiniStateTestDriver

def write_echo_json(f, obj):
    f.write("echo %s\n" % shlex.quote(json.dumps(obj)))


class LazyInitTestDriver(MiniStateTestDriver):
    def write_local_conf(self):
        with open(os.path.join(self.repo_dir, 'hh.conf'), 'w') as f:
            f.write(
                r"""
# some comment
use_mini_state = true
use_watchman = true
watchman_subscribe_v2 = true
lazy_decl = true
lazy_parse = true
lazy_init = true
incremental_init = true
enable_fuzzy_search = false
"""
            )


class LazyInitCommonTests(
    common_tests.CommonTests, LazyInitTestDriver, unittest.TestCase
):
    pass


class LazyInitHeirarchyTests(
    hierarchy_tests.HierarchyTests, LazyInitTestDriver, unittest.TestCase
):
    pass


class MiniStateCommonTests(
    common_tests.CommonTests, MiniStateTestDriver, unittest.TestCase
):
    pass


class MiniStateHierarchyTests(
    hierarchy_tests.HierarchyTests, MiniStateTestDriver, unittest.TestCase
):
    pass


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
            write_echo_json(f, {'error': error_msg, })
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
            write_echo_json(
                f, {
                    'state': self.saved_state_path(),
                    'corresponding_base_revision': '1',
                    'is_cached': True,
                    'deptable': self.saved_state_path() + '.sql',
                }
            )
            write_echo_json(f, {'error': error_msg, })
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
            f.write(
                r"""
# some comment
assume_php = true
load_mini_script = %s
""" % os.path.join(self.repo_dir, 'server_options.sh')
            )

        # Server may take some time to kill itself.
        time.sleep(2)

        # this should start a new server
        self.check_cmd(['No errors!'])
        # check how the old one exited
        log_file = self.proc_call([
            hh_client, '--logname', self.repo_dir
        ])[0].strip() + '.old'
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
        self.assertIn('Watchman_sig.Types.Timeout', self.get_server_logs())
