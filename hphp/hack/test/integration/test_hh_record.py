from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import mini_state_test_driver
import unittest
import signal
import sys
import time

from hh_paths import hh_record

# Use the MiniStateTestDriver to set up the custom saved mini state
# Note: We don't use this driver to drive any of the CommonTests since
# this driver is only useful for testing hh_record
# and not the different kinds of saved states.


class HhRecordTestDriver(mini_state_test_driver.MiniStateTestDriver):

    def start_hh_record(self):
        return self.proc_create([
            hh_record,
            self.repo_dir
        ], env={})


class HhRecordTests(HhRecordTestDriver, unittest.TestCase):
    template_repo = 'hphp/hack/test/integration/data/simple_repo'

    def test_record_with_no_dirty_files(self):
        self.write_load_config()
        recorder = self.start_hh_record()
        # We don't want hh_client to start a new server instance. Give
        # hh_record a little bit of time to start a server.
        time.sleep(2)
        # This check blocks until server initialization is finished
        self.run_check()
        # Interrupt recorder, which sends all events to stdout
        recorder.send_signal(signal.SIGINT)
        (output, stderr_data) = recorder.communicate(timeout=5)
        recorder.wait()
        sys.stderr.write(output)
        err_msg = "See also hh_record stderr: " + stderr_data + "\n"
        sys.stderr.flush()
        # TODO: Make hh_record output JSON instead.
        self.assertRegex(
            output.strip(),
            '(Loaded_saved_state /tmp/.........../foo with 0 dirtied files)',
            err_msg)
