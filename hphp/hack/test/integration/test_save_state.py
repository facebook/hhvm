# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import json
import os
import shlex
import shutil
import sqlite3
import stat
import time
import unittest
from typing import Optional, TextIO

import common_tests
import hierarchy_tests
from hh_paths import hh_client
from saved_state_test_driver import (
    SavedStateClassicTestDriver,
    SavedStateTestDriver,
    SaveStateResult,
)
from test_case import TestCase


def write_echo_json(f: TextIO, obj: object) -> None:
    f.write("echo %s\n" % shlex.quote(json.dumps(obj)))


class LazyInitTestDriver(SavedStateTestDriver):
    def write_local_conf(self) -> None:
        with open(os.path.join(self.repo_dir, "hh.conf"), "w") as f:
            f.write(
                r"""
# some comment
use_mini_state = true
use_watchman = true
watchman_subscribe_v2 = true
lazy_decl = true
lazy_parse = true
lazy_init2 = true
incremental_init = true
enable_fuzzy_search = false
max_workers = 2
"""
            )


class LazyInitCommonTests(common_tests.CommonTests):
    @classmethod
    def get_test_driver(cls) -> LazyInitTestDriver:
        return LazyInitTestDriver()


class LazyInitHeirarchyTests(hierarchy_tests.HierarchyTests):
    @classmethod
    def get_test_driver(cls) -> LazyInitTestDriver:
        return LazyInitTestDriver()


class SavedStateCommonTests(common_tests.CommonTests):
    @classmethod
    def get_test_driver(cls) -> SavedStateTestDriver:
        return SavedStateTestDriver()


class SavedStateBarebonesTestsClassic(common_tests.BarebonesTests):
    @classmethod
    def get_test_driver(cls) -> SavedStateClassicTestDriver:
        return SavedStateClassicTestDriver()


class SavedStateHierarchyTests(hierarchy_tests.HierarchyTests):
    @classmethod
    def get_test_driver(cls) -> SavedStateTestDriver:
        return SavedStateTestDriver()


class SavedStateTests(TestCase[SavedStateTestDriver]):
    @classmethod
    def get_test_driver(cls) -> SavedStateTestDriver:
        return SavedStateTestDriver()

    def test_hhconfig_change(self) -> None:
        """
        Start hh_server, then change .hhconfig and check that the server
        restarts itself
        """
        self.test_driver.start_hh_server()
        self.test_driver.check_cmd(["No errors!"])
        with open(os.path.join(self.test_driver.repo_dir, ".hhconfig"), "w") as f:
            f.write(
                r"""
# some comment
assume_php = true
"""
            )

        # Server may take some time to kill itself.
        time.sleep(2)

        # The sleep(2) above also almost-always ensures another race condition
        # goes the way we want: The informant-directed restart doesn't happen
        # *during* processing of a new client connection. The ambiguity of that
        # situation (whether or not the newly-connected client did read the
        # new hhconfig file contents or not) means that the Monitor can't safely
        # start a new server instance until the *next* client connects. Just in
        # case the race doesn't go the way we want, add another "check_cmd"
        # call here to force the Monitor into the state we want.
        self.test_driver.check_cmd(None, assert_loaded_saved_state=False)

        # this should start a new server
        self.test_driver.check_cmd(["No errors!"])
        # check how the old one exited
        log_file = (
            self.test_driver.proc_call(
                [hh_client, "--logname", self.test_driver.repo_dir]
            )[0].strip()
            + ".old"
        )
        with open(log_file) as f:
            logs = f.read()
            self.assertIn(".hhconfig changed in an incompatible way", logs)

    def test_watchman_timeout(self) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "hh.conf"), "a") as f:
            f.write(
                r"""
watchman_init_timeout = 1
"""
            )

        with open(os.path.join(self.test_driver.bin_dir, "watchman"), "w") as f:
            f.write(r"""sleep 2""")
            os.fchmod(f.fileno(), stat.S_IRWXU)

        self.test_driver.run_check()
        # Stop the server, ensuring that its logs get flushed
        self.test_driver.proc_call([hh_client, "stop", self.test_driver.repo_dir])
        self.assertIn("Watchman_sig.Types.Timeout", self.test_driver.get_server_logs())

    def test_save_partial_state(self) -> None:
        self.test_driver.start_hh_server()

        result1 = self.test_driver.save_partial(
            files_to_check=["class_1.php"], assert_edges_added=True, filename="partial1"
        )

        self.assertTrue(
            result1.returned_values.get_edges_added() == 0,
            "class_1 has no dependencies",
        )

        result2 = self.test_driver.save_partial(
            files_to_check=["class_2.php"], assert_edges_added=True, filename="partial2"
        )
        assert result2.returned_values.get_edges_added() > 0

        result3 = self.test_driver.save_partial(
            files_to_check=["class_3.php"], assert_edges_added=True, filename="partial3"
        )
        assert result3.returned_values.get_edges_added() > 0

        result4 = self.test_driver.save_partial(
            files_to_check=["class_1.php", "class_2.php", "class_3.php"],
            assert_edges_added=True,
            filename="partial4",
        )
        assert (
            result4.returned_values.get_edges_added()
            == result3.returned_values.get_edges_added()
        )

        result5 = self.test_driver.save_partial(
            files_to_check=[
                {"from_prefix_incl": "class_1.php", "to_prefix_excl": "class_3.php"}
            ],
            assert_edges_added=True,
            filename="partial5",
        )
        assert (
            result5.returned_values.get_edges_added()
            == result2.returned_values.get_edges_added()
        )

    def test_incrementally_generated_saved_state(self) -> None:
        old_saved_state: SaveStateResult = self.test_driver.dump_saved_state()
        new_file = os.path.join(self.test_driver.repo_dir, "class_3b.php")
        self.add_file_that_depends_on_class_a(new_file)
        self.test_driver.check_cmd(["No errors!"], assert_loaded_saved_state=False)
        new_saved_state: SaveStateResult = (
            self.test_driver.dump_saved_state(assert_edges_added=True)
        )
        assert new_saved_state.returned_values.get_edges_added() > 0

        self.change_return_type_on_base_class(
            os.path.join(self.test_driver.repo_dir, "class_1.php")
        )
        self.test_driver.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
                "{root}class_3b.php:5:8,15: Invalid return type (Typing[4110])",
                "  {root}class_3b.php:4:26,28: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
            ],
            assert_loaded_saved_state=False,
        )
        self.test_driver.proc_call([hh_client, "stop", self.test_driver.repo_dir])
        # Start server with the original saved state. Will be missing the
        # second error because of the missing edge.
        self.test_driver.start_hh_server(
            changed_files=["class_1.php"], saved_state_path=old_saved_state.path
        )
        self.test_driver.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
            ]
        )
        self.test_driver.proc_call([hh_client, "stop", self.test_driver.repo_dir])
        # Start another server with the new saved state. Will have both errors.
        self.test_driver.start_hh_server(
            changed_files=["class_1.php"], saved_state_path=new_saved_state.path
        )
        self.test_driver.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
                "{root}class_3b.php:5:8,15: Invalid return type (Typing[4110])",
                "  {root}class_3b.php:4:26,28: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
            ]
        )

    def test_incrementally_generated_saved_state_after_loaded_saved_state(self) -> None:
        # Same as the above test, except we begin the test by starting up
        # a Hack Server that loads a saved state.
        self.test_driver.start_hh_server()
        # Hack server is now started with a saved state
        self.test_driver.check_cmd(["No errors!"], assert_loaded_saved_state=True)
        old_saved_state = self.test_driver.dump_saved_state()

        new_file = os.path.join(self.test_driver.repo_dir, "class_3b.php")
        self.add_file_that_depends_on_class_a(new_file)
        self.test_driver.check_cmd(["No errors!"], assert_loaded_saved_state=True)
        new_saved_state = self.test_driver.dump_saved_state(assert_edges_added=True)

        assert new_saved_state.returned_values.get_edges_added() > 0

        self.change_return_type_on_base_class(
            os.path.join(self.test_driver.repo_dir, "class_1.php")
        )
        self.test_driver.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
                "{root}class_3b.php:5:8,15: Invalid return type (Typing[4110])",
                "  {root}class_3b.php:4:26,28: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
            ],
            assert_loaded_saved_state=True,
        )
        self.test_driver.proc_call([hh_client, "stop", self.test_driver.repo_dir])
        # Start server with the original saved state. Will be missing the
        # second error because of the missing edge.
        self.test_driver.start_hh_server(
            changed_files=["class_1.php"], saved_state_path=old_saved_state.path
        )
        self.test_driver.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
            ]
        )
        self.test_driver.proc_call([hh_client, "stop", self.test_driver.repo_dir])
        # Start another server with the new saved state. Will have both errors.
        self.test_driver.start_hh_server(
            changed_files=["class_1.php"], saved_state_path=new_saved_state.path
        )
        self.test_driver.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
                "{root}class_3b.php:5:8,15: Invalid return type (Typing[4110])",
                "  {root}class_3b.php:4:26,28: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
            ]
        )

    def test_incrementally_generated_saved_state_with_errors(self) -> None:
        # Introduce an error in "master"
        self.change_return_type_on_base_class(
            os.path.join(self.test_driver.repo_dir, "class_1.php")
        )

        saved_state_with_1_error: SaveStateResult = self.test_driver.dump_saved_state(
            ignore_errors=True
        )

        self.test_driver.proc_call([hh_client, "stop", self.test_driver.repo_dir])

        # Start server with the saved state, assume there are no local changes.
        self.test_driver.start_hh_server(
            changed_files=None, saved_state_path=saved_state_with_1_error.path
        )

        # We still expect that the error from the saved state shows up.
        self.test_driver.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
            ]
        )

        self.test_driver.proc_call([hh_client, "stop", self.test_driver.repo_dir])

        new_file = os.path.join(self.test_driver.repo_dir, "class_3b.php")
        self.add_file_that_depends_on_class_a(new_file)

        # Start server with the saved state, the only change is in the new file.
        self.test_driver.start_hh_server(
            changed_files=["class_3b.php"],
            saved_state_path=saved_state_with_1_error.path,
        )

        # Now we expect 2 errors - one from the saved state and one
        # from the change.
        self.test_driver.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
                "{root}class_3b.php:5:8,15: Invalid return type (Typing[4110])",
                "  {root}class_3b.php:4:26,28: Expected int",
                "  {root}class_1.php:5:33,38: But got string",
            ],
            assert_loaded_saved_state=False,
        )

        saved_state_with_2_errors = self.test_driver.dump_saved_state(
            ignore_errors=True
        )

        self.test_driver.proc_call([hh_client, "stop", self.test_driver.repo_dir])

        # Let's fix the error
        self.change_return_type_on_base_class(
            filename=os.path.join(self.test_driver.repo_dir, "class_1.php"),
            type="int",
            value="11",
        )

        # Start another server with the new saved state. Will have both errors.
        self.test_driver.start_hh_server(
            changed_files=["class_1.php"],
            saved_state_path=saved_state_with_2_errors.path,
        )

        self.test_driver.check_cmd(["No errors!"], assert_loaded_saved_state=True)

    def test_replace_state_after_saving(self) -> None:
        # Save state
        result = self.test_driver.dump_saved_state(assert_edges_added=True)
        assert result.returned_values.get_edges_added() > 0

        # Save state again - confirm the same number of edges is dumped
        result2 = self.test_driver.dump_saved_state(assert_edges_added=True)
        self.assertEqual(
            result.returned_values.get_edges_added(),
            result2.returned_values.get_edges_added(),
        )

        # Save state with the 'replace' arg
        replace_result1 = self.test_driver.dump_saved_state(
            assert_edges_added=True, replace_state_after_saving=True
        )

        self.assertEqual(
            result.returned_values.get_edges_added(),
            replace_result1.returned_values.get_edges_added(),
        )

        # Save state with the new arg - confirm there are 0 new edges
        replace_result2 = self.test_driver.dump_saved_state(
            assert_edges_added=True, replace_state_after_saving=True
        )
        self.assertEqual(replace_result2.returned_values.get_edges_added(), 0)

        # Make a change
        # Save state - confirm there are only the # of new edges
        #   corresponding to the one change
        new_file = os.path.join(self.test_driver.repo_dir, "class_3b.php")
        self.add_file_that_depends_on_class_a(new_file)
        self.test_driver.check_cmd(["No errors!"], assert_loaded_saved_state=False)
        replace_incremental = self.test_driver.dump_saved_state(
            assert_edges_added=True, replace_state_after_saving=True
        )

        assert (
            replace_incremental.returned_values.get_edges_added()
            < result.returned_values.get_edges_added()
        )
        assert replace_incremental.returned_values.get_edges_added() > 0
        self.test_driver.check_cmd(["No errors!"], assert_loaded_saved_state=False)

    def add_file_that_depends_on_class_a(self, filename: str) -> None:
        with open(filename, "w") as f:
            f.write(
                """<?hh // strict

class UsesAToo {
public function test() : int {
return A::foo();
}

}
            """
            )

    def change_return_type_on_base_class(
        self, filename: str, type: str = "string", value: str = '"Hello"'
    ) -> None:
        # Change the return type
        with open(filename, "w") as f:
            f.write(
                """<?hh // strict

class B {

public static function foo () : %s {
  return %s;
}
}
            """
                % (type, value)
            )


class ReverseNamingTableFallbackTestDriver(SavedStateTestDriver):
    enable_naming_table_fallback = True

    def write_local_conf(self) -> None:
        with open(os.path.join(self.repo_dir, "hh.conf"), "w") as f:
            f.write(
                r"""
# some comment
use_mini_state = true
use_watchman = true
watchman_subscribe_v2 = true
lazy_decl = true
lazy_parse = true
lazy_init2 = true
enable_naming_table_fallback = true
"""
            )


class ReverseNamingTableSavedStateCommonTests(common_tests.CommonTests):
    @classmethod
    def get_test_driver(cls) -> ReverseNamingTableFallbackTestDriver:
        return ReverseNamingTableFallbackTestDriver()


class ReverseNamingTableSavedStateHierarchyTests(hierarchy_tests.HierarchyTests):
    @classmethod
    def get_test_driver(cls) -> ReverseNamingTableFallbackTestDriver:
        return ReverseNamingTableFallbackTestDriver()


class ReverseNamingTableSavedStateTests(SavedStateTests):
    @classmethod
    def get_test_driver(cls) -> ReverseNamingTableFallbackTestDriver:
        return ReverseNamingTableFallbackTestDriver()

    def test_file_moved(self) -> None:
        new_file = os.path.join(self.test_driver.repo_dir, "class_3b.php")
        self.add_file_that_depends_on_class_a(new_file)
        self.test_driver.check_cmd(["No errors!"], assert_loaded_saved_state=False)
        naming_table_path = self.test_driver.dump_naming_saved_state(
            self.test_driver.repo_dir,
            saved_state_path=os.path.join(self.test_driver.repo_dir, "new"),
        )

        self.test_driver.proc_call([hh_client, "stop", self.test_driver.repo_dir])
        new_file2 = os.path.join(self.test_driver.repo_dir, "class_3c.php")
        shutil.move(new_file, new_file2)

        self.test_driver.start_hh_server(
            changed_files=[],
            changed_naming_files=["class_3c.php"],
            naming_saved_state_path=naming_table_path,
        )
        self.test_driver.check_cmd(["No errors!"], assert_loaded_saved_state=True)
