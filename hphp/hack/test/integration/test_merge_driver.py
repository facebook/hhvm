from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import json
import os
import shutil
import subprocess
import unittest

import common_tests


class MergeDriverTests(common_tests.CommonTestDriver, unittest.TestCase):

    template_repo = 'hphp/hack/test/integration/data/repo_with_merge_driver'

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()

    def write_local_conf(self):
        with open(os.path.join(self.repo_dir, 'hh.conf'), 'w') as f:
            f.write(
                r"""
# some comment
use_mini_state = true
use_watchman = true
# Can't use subscriptions yet due to deadlock. Fix is coming
watchman_subscribe = false
lazy_decl = true
lazy_parse = true
lazy_init = true
enable_fuzzy_search = false
"""
            )

    # Write our custom merge driver.
    # In order for the hh_client launched by mercurial to
    # find the correct hh_server (correct in the sense that
    # it's the same hh_server as the one launched within this
    # python test), we need to dump out test_env which includes
    # the HH_TMPDIR.
    def write_merge_driver(self):
        test_env_json = json.dumps(self.test_env)
        with open(os.path.join(
                self.repo_dir,
                'scripts',
                'mergedriver_test_env.json'), 'w') as f:
            f.write(test_env_json)

    def setUp(self):
        shutil.copytree(self.template_repo, self.repo_dir)
        self.write_merge_driver()

    def tearDown(self):
        pass

    def write_hgrc(self):
        with open(os.path.join(self.repo_dir, '.hg', 'hgrc'), 'w') as f:
            f.write(
                r"""
[extensions]
mergedriver =

[experimental]
mergedriver = python:scripts/mergedriver.py
"""
            )

    def check_call(self, cmd):
        subprocess.check_call(cmd, cwd=self.repo_dir, env=self.test_env)

    def init_hg_repo(self):
        cmd = ['hg', 'init']
        self.check_call(cmd)
        cmd = ['hg', 'add']
        self.check_call(cmd)
        cmd = ['hg', 'commit', '-m', 'starting']
        self.check_call(cmd)
        self.write_hgrc()

    def hg_commit(self, msg):
        cmd = ['hg', 'commit', '-m', msg]
        self.check_call(cmd)

    foo_1_start = r"""
        <?hh
        function f() {
            return 1;
        }
"""

    foo_1_first_append = r"""
        function returns_int() {
            return 5;
        }
"""

    foo_1_second_append = r"""
        function returns_string() {
            return "hello";
        }
"""

    def write_foo_1_and_commit(self, content, commit_msg):
        with open(os.path.join(self.repo_dir, 'foo_1.php'), 'w') as f:
            f.write(content)
        self.hg_commit(commit_msg)

    def test_mergedriver_finishes_quickly(self):
        self.write_local_conf()
        self.init_hg_repo()
        self.write_foo_1_and_commit(self.foo_1_start, "starting")
        self.write_foo_1_and_commit(
            self.foo_1_start + self.foo_1_first_append, "first append")
        self.write_foo_1_and_commit(
            self.foo_1_start + self.foo_1_first_append + self.foo_1_second_append,
            "second append")
        self.check_cmd(['No errors!'])
        # Backing out the first append will trigger 3-way merge logic, firing
        # the merge driver, which calls Hack build
        cmd = ['hg', 'backout', '.^', '--no-commit']
        self.check_call(cmd)
