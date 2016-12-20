from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import mini_state_test_driver
import os
import unittest
import re
import signal
import sys
import time

from hh_paths import hh_client, recorder_cat, server_driver


def boxed_string(content):
    return "\n============\n" + content + "\n===========\n"

# Use the MiniStateTestDriver to set up the custom saved mini state
# Note: We don't use this driver to drive any of the CommonTests since
# this driver is only useful for testing hh_record
# and not the different kinds of saved states.


class HhRecordTestDriver(mini_state_test_driver.MiniStateTestDriver):

    def write_local_conf(self):
        with open(os.path.join(self.repo_dir, 'hh.conf'), 'w') as f:
            f.write(r"""
# some comment
use_mini_state = true
use_watchman = true
start_with_recorder_on = true
""")

class HhRecordTests(HhRecordTestDriver, unittest.TestCase):
    template_repo = 'hphp/hack/test/integration/data/simple_repo'

    def assert_match_regexes(self, doc, regexes, msg):
        for regex in regexes:
            self.assertRegex(
                doc,
                regex,
                msg)

    def test_record_with_no_dirty_files(self):
        self.write_load_config()
        self.run_check()
        logs = self.get_server_logs()
        self.assertIn('start_with_recorder_on = true', logs)
        self.assertIn('About to spawn recorder daemon', logs)
        self.assertIn('Successfully loaded mini-state', logs)
        self.assertIn('Sending Loaded_saved_state debug event', logs)
        recording_matcher = re.search(
            'About to spawn recorder daemon. Output will go to (.+)\. Logs to (.+)\.',
            logs)
        self.assertIsNotNone(recording_matcher)
        self.assertIn('recorder_out', recording_matcher.group(1))
        self.assertIn('recorder_log', recording_matcher.group(2))
        (_, _, retcode) = self.proc_call([
            hh_client,
            'stop',
            self.repo_dir
        ])
        self.assertEqual(retcode, 0)
        # We can't wait for grandchild pids, i.e. the recorder daemon,
        # so just give it a little time.
        time.sleep(5)
        out, err, retcode = self.proc_call([
            recorder_cat,
            recording_matcher.group(1)
        ])
        with open(recording_matcher.group(2)) as f:
            recorder_log = f.read()
        self.assertEqual(retcode, 0, "See also recorder_cat stderr:\n" + err)
        self.assertRegex(
            out.strip(),
            '(Loaded_saved_state /tmp/.........../foo with 0 dirtied files)',
            "See also recorder daemon logs:" + boxed_string(recorder_log))

    def test_server_driver_case_zero(self):
        self.write_load_config()
        self.run_check()
        logs = self.get_server_logs()
        self.assertIn('start_with_recorder_on = true', logs)
        self.assertIn('Successfully loaded mini-state', logs)
        recording_matcher = re.search(
            'About to spawn recorder daemon. Output will go to (.+)\. Logs to (.+)\.',
            logs)
        self.assertIsNotNone(recording_matcher)
        self.assertIn('recorder_out', recording_matcher.group(1))
        (_, err, retcode) = self.proc_call([
            server_driver,
            '0',
            self.repo_dir
        ])
        self.assertEqual(
            retcode,
            0,
            "See also server_driver stderr:" + boxed_string(err))
        (_, _, retcode) = self.proc_call([
            hh_client,
            'stop',
            self.repo_dir
        ])
        self.assertEqual(retcode, 0)
        # We can't wait for grandchild pids, i.e. the recorder daemon,
        # so just give it a little time.
        time.sleep(4)
        out, err, retcode = self.proc_call([
            recorder_cat,
            recording_matcher.group(1)
        ])
        with open(recording_matcher.group(2)) as f:
            recorder_log = f.read()
        self.assertEqual(
            retcode,
            0,
            "See also recorder_cat stderr:" + boxed_string(err))
        self.assert_match_regexes(
            out.strip(),
            [
                '(Loaded_saved_state /tmp/.........../foo with 0 dirtied files)',
                '(HandleServerCommand STATUS)',
                '(HandleServerCommand INFER_TYPE)'
            ],
            "See also recorder daemon logs:" + boxed_string(recorder_log))
