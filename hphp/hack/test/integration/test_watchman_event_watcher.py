#!/usr/bin/env python3
from __future__ import absolute_import, division, print_function, unicode_literals

import os
import random
import re
import shutil
import socket
import string
import subprocess
import sys
import time
import unittest

import common_tests
from hh_paths import watchman_event_watcher
from libfb.py.testutil import flaky


def err_print_ln(msg):
    print("{0}\n".format(msg), file=sys.stderr)
    return


def cat_file(filename):
    with open(filename, "r") as f:
        return f.read()


def run_watcher(daemonize=False, enable_watchman_event_watcher=True):
    # Creates a decorator with named arguments. Runs the watcher
    # and retrieves the socket name. Hands these as named arguments
    # to the test function.
    def wrap(fn):
        def f(self=None):
            # We have to do test case setup here instead of in the class method
            # 'setup' because we want to recreate the hg repo on Flaky retries.
            shutil.rmtree(self.repo_dir, ignore_errors=True)
            random_name = "".join(
                random.choices(string.ascii_uppercase + string.digits, k=8)
            )
            self.repo_dir = os.path.join(self.base_tmp_dir, random_name)
            print(
                "copying %s to %s", [self.template_repo, self.repo_dir], file=sys.stderr
            )
            shutil.copytree(self.template_repo, self.repo_dir)
            self.write_watchman_config("{}")
            if enable_watchman_event_watcher:
                self.write_enable_watchman_event_watcher_file()
            self.init_hg_repo()
            self.write_new_file_and_commit("foo1", "hello", "foo1 contents")
            sockname = self.get_sockname()
            if daemonize:
                proc = self.run_watchman_event_watcher(daemonize=True)
                log_file = self.wait_init_get_log_file(proc)
                self.assertIn(
                    ".watchman_event_watcher_log",
                    log_file,
                    "expect extension in filename",
                )
                try:
                    return fn(
                        self, log_file=log_file, sockname=sockname, starter_process=proc
                    )
                except Exception:
                    err_print_ln("Test threw exception.")
                    err_print_ln("See also event watcher logs:")
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
                    err_print_ln("test threw exception.")
                    err_print_ln("See also event watcher logs:")
                    proc.kill()
                    logs = proc.stderr.read().decode("UTF-8")
                    err_print_ln(logs)
                    raise

        return f

    return wrap


class CustodietTests(common_tests.CommonTestDriver, unittest.TestCase):

    template_repo = "hphp/hack/test/integration/data/simple_repo"

    @classmethod
    def setUpClass(cls):
        super().setUpClass(cls.template_repo)

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()

    def setUp(self) -> None:
        pass

    def tearDown(self) -> None:
        pass

    def check_call(self, cmd, timeout=None):
        subprocess.check_call(
            cmd, cwd=self.repo_dir, env=self.test_env, timeout=timeout
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
        cmd = ["hg", "commit", "-m", msg]
        self.check_call(cmd)

    def write_watchman_config(self, contents):
        with open(os.path.join(self.repo_dir, ".watchmanconfig"), "w") as f:
            f.write(contents)

    def write_enable_watchman_event_watcher_file(self):
        with open(
            os.path.join(self.repo_dir, ".hh_enable_watchman_event_watcher"), "w"
        ) as f:
            f.write("")

    def init_hg_repo(self):
        cmd = ["hg", "init"]
        self.check_call(cmd)
        with open(os.path.join(self.repo_dir, ".hg", "hgrc"), "w") as f:
            # Must enable fsmonitor extension to see State enter/leave events
            f.write("[extensions]\n")
            f.write("fsmonitor =\n")
            f.write("hgevents =\n")
        cmd = ["hg", "add"]
        self.check_call(cmd)
        self.hg_commit("starting")
        self.check_call(cmd)

    def run_watchman_event_watcher(self, daemonize=False):
        cmd = [watchman_event_watcher, self.repo_dir]
        if daemonize:
            cmd.append("--daemonize")
        return self.open_subprocess(cmd)

    def get_sockname(self):
        cmd = [watchman_event_watcher, "--get-sockname", self.repo_dir]
        output = subprocess.check_output(
            cmd, cwd=self.repo_dir, env=self.test_env, timeout=5
        )
        sockname = output.decode("UTF-8")
        return sockname

    # Read a line from the file, ignoring all lines in 'ignore'
    def poll_line(self, f, retry_wait=0.1, retries=100, retry_eof=False, ignore=None):
        while retries > 0:
            line = f.readline()
            if retry_eof and not line:
                err_print_ln("ignoring EOF. retrying.")
                time.sleep(retry_wait)
                retries -= 1
                continue
            elif ignore and any(x in line.strip() for x in ignore):
                err_print_ln("ignoring from ignore list. retrying.")
                time.sleep(retry_wait)
                retries -= 1
                continue
            else:
                result = line.strip()
                err_print_ln("poll_line result: {0}".format(result))
                return result
        raise TimeoutError

    def connect_socket(self, sockname, retry_wait=0.1, retries=100):
        while retries > 0:
            try:
                sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                sock.connect(sockname)
                return (sock, retries)
            except IOError:
                err_print_ln("failed to connect to socket. retrying.")
                time.sleep(retry_wait)
                retries -= 1
                continue
        raise TimeoutError

    def connect_socket_get_output(
        self,
        sockname,
        # If auto_close is False, keeps the socket open and returns
        # a (result, socket) pair
        auto_close=True,
        ignore_unknown=False,
        retry_wait=0.1,
        retries=100,
    ):
        # Connect to the socket and read a line.
        # If connection fails, retry.
        # If result is 'unknown' and ignore_unknown is True, then retry
        # with a fresh connection.
        # This is distinct from the 'ignore' list in poll_line, which is
        # repeatedly polling from the same socket.
        while retries > 0:
            (sock, retries) = self.connect_socket(
                sockname, retry_wait=retry_wait, retries=retries
            )
            f = sock.makefile()
            # TODO: Don't double-count retries. Meh.
            result = self.poll_line(f, retry_wait=retry_wait, retries=retries)
            f.close()
            if ignore_unknown and result == "unknown":
                sock.close()
                time.sleep(retry_wait)
                retries -= 1
                continue
            elif auto_close:
                sock.close()
                return result
            else:
                return (result, sock)
        raise TimeoutError

    def write_new_file_and_commit(self, filename, content, commit_msg):
        with open(os.path.join(self.repo_dir, filename), "w") as f:
            f.write(content)
        self.check_call(["hg", "add"])
        self.hg_commit(commit_msg)

    # Wait for this process to finish. Get the path to the log file.
    def wait_init_get_log_file(self, proc):
        outs, errs = proc.communicate(timeout=5)
        msg = errs.decode(encoding="UTF-8")
        self.assertIn("Spawning daemon", msg, "spawning message")
        pattern = re.compile(".+Its logs will go to: ([^\n]+)\n")
        m = pattern.match(msg)
        return m.group(1)

    @flaky
    @run_watcher(daemonize=True)
    def test_sockname(self, log_file=None, sockname=None, starter_process=None):
        with open(log_file, "r") as f:
            initialized_msg = self.poll_line(f)
            self.assertIn("initialized", initialized_msg, "initialized message")
            self.assertIn(sockname, initialized_msg, "sockname")
        return

    @flaky
    @run_watcher(daemonize=False, enable_watchman_event_watcher=False)
    def test_always_get_settled_message_when_watcher_disabled(
        self, sockname=None, watcher_process=None
    ):
        result = self.connect_socket_get_output(sockname)
        # Initially, watcher doesn't know the repo's state
        self.assertIn(
            "settled",
            result,
            "With the event watcher disabled, it should always return a settled message",
        )
        return

    @flaky
    @run_watcher(daemonize=False)
    def test_one_update_on_socket(self, sockname=None, watcher_process=None):
        result = self.connect_socket_get_output(sockname)
        # Initially, watcher doesn't know the repo's state
        self.assertIn("unknown", result, "state enter")
        update_proc = self.open_subprocess(["hg", "update", ".~1"])
        # We catch the repo in a mid-update state.
        # This is kind of racy, but that's what the ignore_unknown=True is fore
        result = self.connect_socket_get_output(
            sockname, retry_wait=0.05, ignore_unknown=True
        )
        self.assertIn("mid_update", result, "catch the watcher in mid-update state")
        update_proc.communicate()
        # Give the watchman subscription time to catch up.
        time.sleep(10)
        result = self.connect_socket_get_output(sockname, ignore_unknown=True)
        self.assertIn("settled", result, "settled after update finishes.")
        return

    @flaky
    @run_watcher(daemonize=False)
    def test_notify_waiting_client(self, sockname=None, watcher_process=None):
        # After getting the 'unknown' message, the client should get the 'settled'
        # notification after the repo settles.
        (sock, _) = self.connect_socket(sockname)
        f = sock.makefile()
        result = self.poll_line(f)
        self.assertIn("unknown", result, "state enter")
        update_proc = self.open_subprocess(["hg", "update", ".~1"])
        update_proc.communicate()
        # The same socket above should now have a settled message
        result = self.poll_line(f)
        self.assertIn(
            "settled",
            result,
            "Previously connected client gets notified of repo settling",
        )
        f.close()
        sock.close()
        return

    @flaky
    @run_watcher(daemonize=False)
    def test_notify_waiting_client_after_midupdate(
        self, sockname=None, watcher_process=None
    ):
        # Same as test above, but the client connected to the watcher while
        # it was in mid_update state.
        (sock, _) = self.connect_socket(sockname)
        f = sock.makefile()
        result = self.poll_line(f)
        self.assertIn("unknown", result, "state enter")
        f.close()
        sock.close()
        update_proc = self.open_subprocess(["hg", "update", ".~1"])
        # Catch the watcher in mid-update state.
        (result, sock) = self.connect_socket_get_output(
            sockname, auto_close=False, ignore_unknown=True
        )
        self.assertIn("mid_update", result, "catch repo in midupdate")
        update_proc.communicate()
        # The same socket above should now have a settled message
        f = sock.makefile()
        result = self.poll_line(f)
        self.assertIn(
            "settled",
            result,
            "Previously connected client gets notified of repo settling",
        )
        f.close()
        sock.close()
        return
