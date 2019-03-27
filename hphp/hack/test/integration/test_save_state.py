from __future__ import absolute_import, division, print_function, unicode_literals

import json
import os
import shlex
import stat
import time
import unittest
from typing import IO, Optional, Union

import common_tests
import hierarchy_tests
from hh_paths import hh_client
from saved_state_test_driver import (
    SavedStateClassicTestDriver,
    SavedStateTestDriver,
    SaveStateResult,
)


def write_echo_json(f: IO, obj: object) -> None:
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
"""
            )


class LazyInitCommonTests(
    common_tests.CommonTests, LazyInitTestDriver, unittest.TestCase
):
    pass


class LazyInitHeirarchyTests(
    hierarchy_tests.HierarchyTests, LazyInitTestDriver, unittest.TestCase
):
    pass


class SavedStateCommonTests(
    common_tests.CommonTests, SavedStateTestDriver, unittest.TestCase
):
    pass


class SavedStateBarebonesTestsClassic(
    common_tests.BarebonesTests, SavedStateClassicTestDriver, unittest.TestCase
):
    pass


class SavedStateHierarchyTests(
    hierarchy_tests.HierarchyTests, SavedStateTestDriver, unittest.TestCase
):
    pass


class SavedStateTests(SavedStateTestDriver, unittest.TestCase):
    template_repo: Optional[str] = "hphp/hack/test/integration/data/simple_repo"
    bin_dir: Optional[str]

    def test_hhconfig_change(self) -> None:
        """
        Start hh_server, then change .hhconfig and check that the server
        restarts itself
        """
        self.start_hh_server()
        self.check_cmd(["No errors!"])
        with open(os.path.join(self.repo_dir, ".hhconfig"), "w") as f:
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
        self.check_cmd(None, assert_loaded_saved_state=False)

        # this should start a new server
        self.check_cmd(["No errors!"])
        # check how the old one exited
        log_file = (
            self.proc_call([hh_client, "--logname", self.repo_dir])[0].strip() + ".old"
        )
        with open(log_file) as f:
            logs = f.read()
            self.assertIn(".hhconfig changed in an incompatible way", logs)

    def test_watchman_timeout(self) -> None:
        with open(os.path.join(self.repo_dir, "hh.conf"), "a") as f:
            f.write(
                r"""
watchman_init_timeout = 1
"""
            )

        assert self.bin_dir is not None
        with open(os.path.join(self.bin_dir, "watchman"), "w") as f:
            f.write(r"""sleep 2""")
            os.fchmod(f.fileno(), stat.S_IRWXU)

        self.run_check()
        # Stop the server, ensuring that its logs get flushed
        self.proc_call([hh_client, "stop", self.repo_dir])
        self.assertIn("Watchman_sig.Types.Timeout", self.get_server_logs())

    def test_incrementally_generated_saved_state(self) -> None:
        old_saved_state: SaveStateResult = self.dump_saved_state()
        new_file = os.path.join(self.repo_dir, "class_3b.php")
        self.add_file_that_depends_on_class_a(new_file)
        self.check_cmd(["No errors!"], assert_loaded_saved_state=False)
        new_saved_state: SaveStateResult = (
            self.dump_saved_state(assert_edges_added=True)
        )
        assert (
            new_saved_state.returned_values.edges_added is not None
            and new_saved_state.returned_values.edges_added > 0
        )

        self.change_return_type_on_base_class(
            os.path.join(self.repo_dir, "class_1.php")
        )
        self.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
                "{root}class_3b.php:5:8,15: Invalid return type (Typing[4110])",
                "  {root}class_3b.php:4:26,28: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
            ],
            assert_loaded_saved_state=False,
        )
        self.proc_call([hh_client, "stop", self.repo_dir])
        # Start server with the original saved state. Will be missing the
        # second error because of the missing edge.
        self.start_hh_server(
            changed_files=["class_1.php"], saved_state_path=old_saved_state.path
        )
        self.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
            ]
        )
        self.proc_call([hh_client, "stop", self.repo_dir])
        # Start another server with the new saved state. Will have both errors.
        self.start_hh_server(
            changed_files=["class_1.php"], saved_state_path=new_saved_state.path
        )
        self.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
                "{root}class_3b.php:5:8,15: Invalid return type (Typing[4110])",
                "  {root}class_3b.php:4:26,28: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
            ]
        )

    def test_incrementally_generated_saved_state_after_loaded_saved_state(self) -> None:
        # Same as the above test, except we begin the test by starting up
        # a Hack Server that loads a saved state.
        self.start_hh_server()
        # Hack server is now started with a saved state
        self.check_cmd(["No errors!"], assert_loaded_saved_state=True)
        old_saved_state = self.dump_saved_state()
        new_file = os.path.join(self.repo_dir, "class_3b.php")
        self.add_file_that_depends_on_class_a(new_file)
        self.check_cmd(["No errors!"], assert_loaded_saved_state=True)
        new_saved_state = self.dump_saved_state(assert_edges_added=True)
        assert (
            new_saved_state.returned_values.edges_added is not None
            and new_saved_state.returned_values.edges_added > 0
        )

        self.change_return_type_on_base_class(
            os.path.join(self.repo_dir, "class_1.php")
        )
        self.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
                "{root}class_3b.php:5:8,15: Invalid return type (Typing[4110])",
                "  {root}class_3b.php:4:26,28: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
            ],
            assert_loaded_saved_state=True,
        )
        self.proc_call([hh_client, "stop", self.repo_dir])
        # Start server with the original saved state. Will be missing the
        # second error because of the missing edge.
        self.start_hh_server(
            changed_files=["class_1.php"], saved_state_path=old_saved_state.path
        )
        self.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
            ]
        )
        self.proc_call([hh_client, "stop", self.repo_dir])
        # Start another server with the new saved state. Will have both errors.
        self.start_hh_server(
            changed_files=["class_1.php"], saved_state_path=new_saved_state.path
        )
        self.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
                "{root}class_3b.php:5:8,15: Invalid return type (Typing[4110])",
                "  {root}class_3b.php:4:26,28: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
            ]
        )

    def test_incrementally_generated_saved_state_with_errors(self) -> None:
        # Introduce an error in "master"
        self.change_return_type_on_base_class(
            os.path.join(self.repo_dir, "class_1.php")
        )

        saved_state_with_1_error: SaveStateResult = self.dump_saved_state(
            ignore_errors=True
        )

        self.proc_call([hh_client, "stop", self.repo_dir])

        # Start server with the saved state, assume there are no local changes.
        self.start_hh_server(
            changed_files=None, saved_state_path=saved_state_with_1_error.path
        )

        # We still expect that the error from the saved state shows up.
        self.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
            ]
        )

        self.proc_call([hh_client, "stop", self.repo_dir])

        new_file = os.path.join(self.repo_dir, "class_3b.php")
        self.add_file_that_depends_on_class_a(new_file)

        # Start server with the saved state, the only change is in the new file.
        self.start_hh_server(
            changed_files=["class_3b.php"],
            saved_state_path=saved_state_with_1_error.path,
        )

        # Now we expect 2 errors - one from the saved state and one
        # from the change.
        self.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
                "{root}class_3b.php:5:8,15: Invalid return type (Typing[4110])",
                "  {root}class_3b.php:4:26,28: This is an int",
                "  {root}class_1.php:5:33,38: It is incompatible with a string",
            ],
            assert_loaded_saved_state=False,
        )

        saved_state_with_2_errors = self.dump_saved_state(ignore_errors=True)

        self.proc_call([hh_client, "stop", self.repo_dir])

        # Let's fix the error
        self.change_return_type_on_base_class(
            filename=os.path.join(self.repo_dir, "class_1.php"), type="int", value="11"
        )

        # Start another server with the new saved state. Will have both errors.
        self.start_hh_server(
            changed_files=["class_1.php"],
            saved_state_path=saved_state_with_2_errors.path,
        )

        self.check_cmd(["No errors!"], assert_loaded_saved_state=True)

    def test_replace_state_after_saving(self) -> None:
        # Save state
        result = self.dump_saved_state(assert_edges_added=True)
        assert (
            result.returned_values.edges_added is not None
            and result.returned_values.edges_added > 0
        )

        # Save state again - confirm the same number of edges is dumped
        result2 = self.dump_saved_state(assert_edges_added=True)
        self.assertEqual(
            result.returned_values.edges_added, result2.returned_values.edges_added
        )

        # Save state with the 'replace' arg
        replace_result1 = self.dump_saved_state(
            assert_edges_added=True, replace_state_after_saving=True
        )

        self.assertEqual(
            result.returned_values.edges_added,
            replace_result1.returned_values.edges_added,
        )

        # Save state with the new arg - confirm there are 0 new edges
        replace_result2 = self.dump_saved_state(
            assert_edges_added=True, replace_state_after_saving=True
        )
        self.assertEqual(replace_result2.returned_values.edges_added, 0)

        # Make a change
        # Save state - confirm there are only the # of new edges
        #   corresponding to the one change
        new_file = os.path.join(self.repo_dir, "class_3b.php")
        self.add_file_that_depends_on_class_a(new_file)
        self.check_cmd(["No errors!"], assert_loaded_saved_state=False)
        replace_incremental = self.dump_saved_state(
            assert_edges_added=True, replace_state_after_saving=True
        )

        assert (
            replace_incremental.returned_values.edges_added is not None
            and replace_incremental.returned_values.edges_added
            < result.returned_values.edges_added
        )
        assert replace_incremental.returned_values.edges_added > 0
        self.check_cmd(["No errors!"], assert_loaded_saved_state=False)

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
