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

from hh_paths import hh_client, recorder_cat, server_driver, turntable


def boxed_string(content):
    return (
        "\n============================\n" +
        content +
        "\n============================\n")

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

    def stop_server(self):
        logs = self.get_server_logs()
        (_, _, retcode) = self.proc_call([
            hh_client,
            'stop',
            self.repo_dir
        ])
        self.assertEqual(retcode, 0, "See also server logs: " + boxed_string(logs))

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
            '(Loaded_saved_state /tmp.*/foo with 0 dirtied files)',
            "See also recorder daemon logs:" + boxed_string(recorder_log))

    def fresh_start_with_recorder_on(self):
        """
        Writes the load config that will have recorder on when server starts,
        starts the hack server, returns paths of recording and recorder's log
        """
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
        self.assertIn('recorder_log', recording_matcher.group(2))
        return (
            os.path.realpath(recording_matcher.group(1)),
            os.path.realpath(recording_matcher.group(2)))

    # Start up a hack server and run `server_driver --test-case <case_num>` on it.
    # Returns the subprocess, path to the recording file, and recorder logs
    # This doesn't kill the server_driver process, so its persistent connection
    # with the Hack server is intentionally left alive maintaining the server's
    # IDE state.
    def run_server_driver_case(self, case_num):
        recording_path, recorder_log_path = self.fresh_start_with_recorder_on()
        proc = self.proc_create([
            server_driver,
            '--test-case',
            str(case_num),
            self.repo_dir
        ], {})
        finished = proc.stderr.readline()
        if finished.strip() != "Finished":
            proc.send_signal(signal.SIGINT)
            (driver_out, driver_err) = proc.communicate(None)
            driver_err = finished + driver_err
            self.assertEqual(
                finished.strip(),
                "Finished",
                "See also server_driver stderr: " + boxed_string(driver_err))
        with open(recorder_log_path) as f:
            recorder_log = f.read()
        # Running a new server instance after shutting down the current one
        # will overwrite the recording's, So we want to return the real
        # absolute path here.
        recording = os.path.realpath(recording_path)
        return proc, recording, recorder_log

    # Run the entire record on the repo using the turntable.
    # Keeps the turntable spinning even after reaching the end of the cecord
    # so that the persisent connection is maintained (thus returns that
    # still-running process).
    def spin_record(self, recording):
        proc = self.proc_create([
            turntable,
            '--recording',
            recording,
            self.repo_dir
        ], {})
        for line in proc.stderr:
            if line.strip() == "Played back an event":
                continue
            elif line.strip() == "End of recording...waiting for termination":
                break
            else:
                self.fail("Read unexpected line: " + line)
        else:
            self.fail("Read no lines from turntable stderr")
        return proc

    def test_server_driver_case_zero(self):
        server_driver, recording, recorder_log = self.run_server_driver_case(0)
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
            recording
        ])
        self.assertEqual(
            retcode,
            0,
            "See also recorder_cat stderr:" + boxed_string(err))
        self.assert_match_regexes(
            out.strip(),
            [
                '(Loaded_saved_state /tmp.*/foo with 0 dirtied files)',
                '(HandleServerCommand STATUS)',
                '(HandleServerCommand INFER_TYPE)'
            ],
            "See also recorder daemon logs:" + boxed_string(recorder_log))

    # Creates a recoridng using server_driver case 1, then spins the recording
    # on a new Hack instance with the turntable
    def test_server_driver_case_one_and_turntable(self):
        server_driver, recording, recorder_log = self.run_server_driver_case(1)
        self.check_cmd(["{root}foo_1.php:3:26,37: "
                        "Undefined variable: $missing_var (Naming[2050])"])
        # Stop the server and the server_driver before spinning up a new server
        # on which we will playback the recording.
        self.stop_server()
        server_driver.send_signal(signal.SIGINT)
        server_driver.wait()
        # We can't wait for grandchild pids, i.e. the recorder daemon,
        # so just give it a little time.
        time.sleep(1)
        self.check_cmd(["No errors!"])
        turntable_proc = self.spin_record(recording)
        # Before spinning the record, we get no errors. Afterwards, we get
        # the errors introduced by the record.
        self.check_cmd(["{root}foo_1.php:3:26,37: Undefined "
                        "variable: $missing_var (Naming[2050])"])
        turntable_proc.send_signal(signal.SIGINT)

    def test_modified_file_playback(self):
        recording_path, recorder_log_path = self.fresh_start_with_recorder_on()
        with open(os.path.join(self.repo_dir, 'foo_2.php'), 'w') as f:
            f.write("""
            <?hh
            function g(): int {
                return 'a';
            }
            """)
        self.check_cmd([
            "{root}foo_2.php:4:24,26: Invalid return type (Typing[4110])",
            "  {root}foo_2.php:3:27,29: This is an int",
            "  {root}foo_2.php:4:24,26: It is incompatible with a string"])
        # Stop the server to end the recorder daemon, give it a couple seconds
        self.stop_server()
        time.sleep(2)
        self.check_cmd(["No errors!"])
        turntable_proc = self.spin_record(recording_path)
        self.check_cmd([
            "{root}foo_2.php:4:24,26: Invalid return type (Typing[4110])",
            "  {root}foo_2.php:3:27,29: This is an int",
            "  {root}foo_2.php:4:24,26: It is incompatible with a string"])
        turntable_proc.send_signal(signal.SIGINT)
