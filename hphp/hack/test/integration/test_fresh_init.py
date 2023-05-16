# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import os
import time
import unittest
from typing import List, Optional

import common_tests
from hh_paths import hh_client


class FreshInitTestDriver(common_tests.CommonTestDriver):
    def write_load_config(
        self, use_serverless_ide: bool = False, use_saved_state: bool = False
    ) -> None:
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
                "120",
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
                  } else if ($s /* HH_FIXME[4011] We can delete this one */) {
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
                  } else if ($s ) {
                    print "world";
                  }
                  print "done\n";
                }
            """,
            )

    def test_remove_dead_fixmes_single_alive(self) -> None:
        with open(
            os.path.join(self.test_driver.repo_dir, "foo_fixme_single_alive.php"), "w"
        ) as f:
            f.write(
                """<?hh
                function takes_int(int $_): void {}

                function foo(): void {
                  /* HH_FIXME[4110] not dead. */
                  takes_int("not an int");
                }
            """
            )

        self.test_driver.start_hh_server(
            changed_files=["foo_fixme_single_alive.php"], args=["--no-load"]
        )
        self.test_driver.check_cmd(
            expected_output=None, options=["--remove-dead-fixmes"]
        )

        with open(
            os.path.join(self.test_driver.repo_dir, "foo_fixme_single_alive.php")
        ) as f:
            out = f.read()
            self.assertEqual(
                out,
                """<?hh
                function takes_int(int $_): void {}

                function foo(): void {
                  /* HH_FIXME[4110] not dead. */
                  takes_int("not an int");
                }
            """,
            )

    def test_remove_dead_unsafe_casts(self) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "foo_5.php"), "w") as f:
            f.write(
                r"""<?hh
                function takes_string(string $i): void {}
                function id<T>(T $t): T { return $t; }

                function foo(?string $s): ?string {
                  takes_string(\HH\FIXME\UNSAFE_CAST<?string, string>($s)); // Not redundant
                  \HH\FIXME\UNSAFE_CAST<mixed, ?string>($s); // Redundant
                  if (\HH\FIXME\UNSAFE_CAST<mixed, ?string>(id($s)) === 'test') { // Redundant
                    print "hello";
                    return \HH\FIXME\UNSAFE_CAST<?string, string>($s); // Not redundant
                  } else {
                    return \HH\FIXME\UNSAFE_CAST<mixed, ?string>($s); // Redundant
                  }
                }
            """
            )

        self.test_driver.start_hh_server(
            changed_files=["foo_5.php"],
            args=["--no-load", "--config", "populate_dead_unsafe_cast_heap=true"],
        )
        self.test_driver.check_cmd(
            expected_output=None, options=["--remove-dead-unsafe-casts"]
        )

        with open(os.path.join(self.test_driver.repo_dir, "foo_5.php")) as f:
            out = f.read()
            self.assertEqual(
                out,
                r"""<?hh
                function takes_string(string $i): void {}
                function id<T>(T $t): T { return $t; }

                function foo(?string $s): ?string {
                  takes_string(\HH\FIXME\UNSAFE_CAST<?string, string>($s)); // Not redundant
                  $s; // Redundant
                  if (id($s) === 'test') { // Redundant
                    print "hello";
                    return \HH\FIXME\UNSAFE_CAST<?string, string>($s); // Not redundant
                  } else {
                    return $s; // Redundant
                  }
                }
            """,
            )
