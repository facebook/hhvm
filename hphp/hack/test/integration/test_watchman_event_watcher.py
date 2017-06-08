#!/usr/bin/env python3
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import os
import re
import shutil
import subprocess
import sys
import time
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

    def run_watchman_event_watcher(self, daemonize=False):
        cmd = [watchman_event_watcher, self.repo_dir]
        if daemonize:
            cmd.append('--daemonize')
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

    # Wait for this process to finish. Get the path to the log file.
    def wait_init_get_log_file(self, proc):
        outs, errs = proc.communicate(timeout=5)
        msg = errs.decode(encoding='UTF-8')
        self.assertIn(
            'Spawning daemon',
            msg,
            'spawning message')
        pattern = re.compile('.+Its logs will go to: ([^\n]+)\n')
        m = pattern.match(msg)
        return m.group(1)

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

    # Repeatedly poll for a newline from file f.
    def poll_line(self, f, timeout=20):
        while(timeout > 0):
            line = f.readline()
            if not line:
                time.sleep(1)
                timeout -= 1
                continue
            else:
                return line
        raise TimeoutError

    def test_one_hg_update_daemonized(self):
        self.write_watchman_config('{}')
        self.init_hg_repo()
        self.write_new_file_and_commit("foo1", "hello", "foo1 contents")
        c_proc = self.run_watchman_event_watcher(daemonize=True)
        log_file = self.wait_init_get_log_file(c_proc)
        self.assertIn(
            '.watchman_event_watcher_log',
            log_file,
            'expect extension in filename')
        with open(log_file, 'r') as f:
            initialized_msg = self.poll_line(f)
            self.assertIn(
                'initialized',
                initialized_msg,
                'initialized message')
            self.check_call(['hg', 'update', '.~1'])
            state_enter = self.poll_line(f)
            state_leave = self.poll_line(f)
            sentinel_file = self.poll_line(f)
            self.assertIn('State_enter hg.update', state_enter, 'state enter')
            self.assertIn('State_leave hg.update', state_leave, 'state leave')
            self.assertIn('Changes', sentinel_file, 'has changes')
            self.assertIn(
                'updatestate',
                sentinel_file,
                'changes includes updatestate')
