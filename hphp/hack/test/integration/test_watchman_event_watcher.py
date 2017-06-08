from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import os
import shutil
import subprocess
import sys
import unittest

import common_tests

from hh_paths import watchman_event_watcher


class CustodietTests(common_tests.CommonTestDriver, unittest.TestCase):

    template_repo = 'hphp/hack/test/integration/data/simple_repo'

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()

    def setUp(self):
        print(
            "copying %s to %s",
            [self.template_repo, self.repo_dir],
            file=sys.stderr)
        shutil.copytree(self.template_repo, self.repo_dir)

    def tearDown(self):
        pass

    def check_call(self, cmd, timeout=None):
        subprocess.check_call(
            cmd,
            cwd=self.repo_dir,
            env=self.test_env,
            timeout=timeout,
        )

    def hg_commit(self, msg):
        cmd = ['hg', 'commit', '-m', msg]
        self.check_call(cmd)

    def write_watchman_config(self, contents):
        with open(os.path.join(self.repo_dir, '.watchmanconfig'), 'w') as f:
            f.write(contents)

    def init_hg_repo(self):
        cmd = ['hg', 'init']
        self.check_call(cmd)
        with open(os.path.join(self.repo_dir, '.hg', 'hgrc'), 'w') as f:
            # Must enable fsmonitor extension to see State enter/leave events
            f.write('[extensions]\n')
            f.write('fsmonitor =\n')
        cmd = ['hg', 'add']
        self.check_call(cmd)
        self.hg_commit('starting')
        self.check_call(cmd)

    def run_watchman_event_watcher(self):
        cmd = [watchman_event_watcher, self.repo_dir]
        return subprocess.Popen(
            cmd,
            stderr=subprocess.PIPE,
            close_fds=True,
        )

    def write_new_file_and_commit(self, filename, content, commit_msg):
        with open(os.path.join(self.repo_dir, filename), 'w') as f:
            f.write(content)
        self.check_call(['hg', 'add'])
        self.hg_commit(commit_msg)

    def test_one_hg_update(self):
        self.write_watchman_config('{}')
        self.init_hg_repo()
        self.write_new_file_and_commit("foo1", "hello", "foo1 contents")
        c_proc = self.run_watchman_event_watcher()
        c_stderr = c_proc.stderr
        initialized_msg = c_stderr.readline().decode(encoding='UTF-8')
        self.assertIn(
            'initialized',
            initialized_msg,
            'initialized message')
        self.check_call(['hg', 'update', '.~1'])
        state_enter = c_stderr.readline().decode(encoding='UTF-8')
        state_leave = c_stderr.readline().decode(encoding='UTF-8')
        sentinel_file = c_stderr.readline().decode(encoding='UTF-8')
        c_proc.kill()
        self.assertIn('State_enter hg.update', state_enter, 'state enter')
        self.assertIn('State_leave hg.update', state_leave, 'state leave')
        self.assertIn('Changes', sentinel_file, 'has changes')
        self.assertIn(
            'updatestate',
            sentinel_file,
            'changes includes updatestate')
