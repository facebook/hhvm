# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import os
import time
import unittest
from typing import List, Optional

import common_tests
from hh_paths import hh_client


class FreshInitTestDriver(common_tests.CommonTestDriver):
    def write_load_config(self, use_saved_state: bool = False) -> None:
        # Fresh init tests don't care about which files changed, so we can
        # just use the default .hhconfig in the template repo
        pass

    def check_cmd(
        self,
        expected_output: Optional[List[str]],
        stdin: Optional[str] = None,
        options: Optional[List[str]] = None,
        retries: int = 30,
        assert_loaded_saved_state: bool = False,
    ) -> str:
        if options is None:
            options = []
        time.sleep(2)  # wait for Hack to catch up with file system changes

        root = self.repo_dir + os.path.sep
        (output, err, retcode) = self.proc_call(
            [
                hh_client,
                "check",
                "--retries",
                "60",
                "--no-load",
                "--error-format",
                "raw",
                "--config",
                "max_workers=2",
                self.repo_dir,
            ]
            + list(map(lambda x: x.format(root=root), options)),
            stdin=stdin,
        )

        if (retcode == 6 or retcode == 7) and retries > 0:
            # 6 = "No_server_running_should_retry" or "Server_hung_up_should_retry"
            # 7 = "Out_of_time" or "Out_of_retries"
            return self.check_cmd(expected_output, stdin, options, retries - 1)
        if retcode == 7:
            raise unittest.SkipTest("Hack server exit code 7 - out of time/retries")
        self.assertIn(retcode, [0, 2])

        if expected_output is not None:
            self.assertCountEqual(
                map(lambda x: x.format(root=root), expected_output), output.splitlines()
            )
        return err

    def assertEqualString(
        self, first: str, second: str, msg: Optional[str] = None
    ) -> None:
        root = self.repo_dir + os.path.sep
        second = second.format(root=root)
        self.assertEqual(first, second, msg)


class TestFreshInit(common_tests.CommonTests):
    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return common_tests.CommonTestDriver()

    def test_remove_dead_fixmes(self) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php"), "w") as f:
            f.write(
                """<?hh // strict
                function foo(?string $s): void {
                  /* HH_FIXME[4010] We can delete this one */
                  /* HH_FIXME[4089] We need to keep this one */
                  /* HH_FIXME[4099] We can delete this one */
                  if (/* HH_FIXME[4011] We can delete this one */   $s) {
                    print "hello";
                  } else {
                    print "world";
                  }
                  /* HH_FIXME[4099] We can delete this one */
                  /* HH_FIXME[4098] We can delete this one */
                  print "done\n";
                }
            """
            )

        self.test_driver.start_hh_server(
            changed_files=["foo_4.php"], args=["--no-load"]
        )
        self.test_driver.check_cmd(
            expected_output=None, options=["--remove-dead-fixmes"]
        )

        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php")) as f:
            out = f.read()
            self.assertEqual(
                out,
                """<?hh // strict
                function foo(?string $s): void {
                  if ($s) {
                    print "hello";
                  } else {
                    print "world";
                  }
                  print "done\n";
                }
            """,
            )
