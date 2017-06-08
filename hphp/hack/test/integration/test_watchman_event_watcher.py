#!/usr/bin/env python3
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import os
import re
import shutil
import socket
import subprocess
import sys
import time
import unittest

from libfb.testutil import flaky

import common_tests

from hh_paths import watchman_event_watcher


def err_print_ln(msg):
    print('{0}\n'.format(msg), file=sys.stderr)
    return


def cat_file(filename):
    with open(filename, 'r') as f:
        return f.read()


def run_watcher(daemonize=False):
    # Creates a decorator with named arguments. Runs the watcher
    # and retrieves the socket name. Hands these as named arguments
    # to the test function.
    def wrap(fn):
        def f(self=None):
            self.write_watchman_config('{}')
            self.init_hg_repo()
            self.write_new_file_and_commit("foo1", "hello", "foo1 contents")
            sockname = self.get_sockname()
            if daemonize:
                proc = self.run_watchman_event_watcher(daemonize=True)
                log_file = self.wait_init_get_log_file(proc)
                self.assertIn(
                    '.watchman_event_watcher_log',
                    log_file,
                    'expect extension in filename')
                try:
                    return fn(
                        self,
                        log_file=log_file,
                        sockname=sockname,
                        starter_process=proc,
                    )
                except Exception:
                    err_print_ln('Test threw exception.')
                    err_print_ln('See also event watcher logs:')
                    logs = cat_file(log_file)
                    err_print_ln(logs)
                    raise
            else:
                try:
                    proc = self.run_watchman_event_watcher(daemonize=False)
                    result = fn(self, sockname=sockname, watcher_process=proc)
                    proc.kill()
                    return result
                except Exception:
                    err_print_ln('test threw exception.')
                    err_print_ln('See also event watcher logs:')
                    proc.kill()
                    logs = proc.stderr.read().decode('UTF-8')
                    err_print_ln(logs)
                    raise
        return f
    return wrap


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

    def open_subprocess(self, cmd):
        return subprocess.Popen(
            cmd,
            cwd=self.repo_dir,
            stderr=subprocess.PIPE,
            close_fds=True,
            env=self.test_env,
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
        return self.open_subprocess(cmd)

    def get_sockname(self):
        cmd = [watchman_event_watcher, '--get-sockname', self.repo_dir]
        output = subprocess.check_output(
            cmd,
            cwd=self.repo_dir,
            env=self.test_env,
            timeout=5
        )
        sockname = output.decode('UTF-8')
        return sockname

    # Repeatedly poll for a newline from file f.
    def poll_line(self, f, retry_wait=0.1, retries=100):
        while(retries > 0):
            line = f.readline()
            if not line:
                time.sleep(retry_wait)
                retries -= 1
                continue
            else:
                result = line.strip()
                err_print_ln('poll_line result: {0}'.format(result))
                return result
        raise TimeoutError

    def connect_socket_get_output(
        self, sockname,
        ignore_unknown=False,
        retry_wait=0.1,
        retries=100
    ):
        while(retries > 0):
            try:
                sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                sock.connect(sockname)
                f = sock.makefile()
                result = self.poll_line(f)
                f.close()
                sock.close()
                if ignore_unknown and (result == 'unknown' or result == 'unknown\n'):
                    err_print_ln('ignoring unknown result. retrying.')
                    time.sleep(retry_wait)
                    retries -= 1
                    continue
                return result
            except IOError:
                err_print_ln('failed to connect to socket. retrying.')
                time.sleep(retry_wait)
                retries -= 1
                continue
        raise TimeoutError

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

    @flaky
    @run_watcher(daemonize=True)
    def test_one_hg_update_daemonized(
        self,
        log_file=None,
        sockname=None,
        starter_process=None
    ):
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

    @flaky
    @run_watcher(daemonize=True)
    def test_sockname(
        self,
        log_file=None,
        sockname=None,
        starter_process=None
    ):
        with open(log_file, 'r') as f:
            initialized_msg = self.poll_line(f)
            self.assertIn(
                'initialized',
                initialized_msg,
                'initialized message')
            self.assertIn(
                sockname,
                initialized_msg,
                'sockname')
        return

    @flaky
    @run_watcher(daemonize=False)
    def test_one_update_on_socket(self, sockname=None, watcher_process=None):
        result = self.connect_socket_get_output(sockname)
        # Initially, watcher doesn't know the repo's state
        self.assertIn('unknown', result, 'state enter')
        update_proc = self.open_subprocess(['hg', 'update', '.~1'])
        # We catch the repo in a mid-update state.
        # This is kind of racy, but that's what the ignore_unknown=True is fore
        result = self.connect_socket_get_output(
            sockname,
            ignore_unknown=True)
        self.assertIn('mid_update', result, 'state enter')
        update_proc.communicate()
        # Give the watchman subscription time to catch up.
        time.sleep(10)
        result = self.connect_socket_get_output(
            sockname,
            ignore_unknown=True)
        self.assertIn('settled', result, 'state enter')
        return
