# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import json
import os
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time
from typing import ClassVar, List, Mapping, Optional, Tuple

from hh_paths import hackfmt, hh_client, hh_merge_deps, hh_server
from test_case import TestCase, TestDriver
from utils import Json, JsonObject


class DebugSubscription(object):
    """
    Wraps `hh_client debug`.
    """

    # pyre-fixme[24]: Generic type `subprocess.Popen` expects 1 type parameter.
    def __init__(self, proc: subprocess.Popen) -> None:
        self.proc = proc
        hello = self.read_msg()
        assert hello["data"] == "hello"

    # pyre-fixme[11]: Annotation `Json` is not defined as a type.
    def read_msg(self) -> Json:
        # pyre-fixme[16]: `Optional` has no attribute `readline`.
        line = self.proc.stdout.readline()
        return json.loads(line)

    # pyre-fixme[11]: Annotation `JsonObject` is not defined as a type.
    def get_incremental_logs(self) -> JsonObject:
        msgs = {}
        while True:
            msg = self.read_msg()
            if msg["type"] == "info" and msg["data"] == "incremental_done":
                break
            msgs[msg["name"]] = msg
        return msgs


class CommonTestDriver(TestDriver):

    # This needs to be overridden in child classes. The files in this
    # directory will be used to set up the initial environment for each
    # test.
    template_repo: ClassVar[str]
    repo_dir: ClassVar[str]
    test_env: ClassVar[Mapping[str, str]]
    base_tmp_dir: ClassVar[str]
    hh_tmp_dir: ClassVar[str]
    bin_dir: ClassVar[str]

    @classmethod
    def setUpClass(cls, template_repo: str) -> None:
        cls.template_repo = template_repo
        cls.maxDiff = 2000
        cls.base_tmp_dir = tempfile.mkdtemp()
        # we don't create repo_dir using mkdtemp() because we want to create
        # it with copytree(). copytree() will fail if the directory already
        # exists.
        cls.repo_dir = os.path.join(cls.base_tmp_dir, "repo")
        # Where the hhi files, socket, etc get extracted
        cls.hh_tmp_dir = tempfile.mkdtemp()
        cls.bin_dir = tempfile.mkdtemp()
        hh_server_dir = os.path.dirname(hh_server)

        hh_merge_deps_dir = os.path.dirname(hh_merge_deps)
        print("hh_server_dir " + hh_server_dir)
        print("hh_merge_deps_dir " + hh_merge_deps_dir)

        cls.test_env = dict(
            os.environ,
            **{
                "HH_TEST_MODE": "1",
                "HH_TMPDIR": cls.hh_tmp_dir,
                "HACKFMT_TEST_PATH": hackfmt,
                "PATH": (
                    "%s:%s:%s:/bin:/usr/bin:/usr/local/bin"
                    % (hh_server_dir, hh_merge_deps_dir, cls.bin_dir)
                ),
                "HH_HOME": os.path.dirname(hh_client),
                "OCAMLRUNPARAM": "b",
                "HH_LOCALCONF_PATH": cls.repo_dir,
            },
        )

    @classmethod
    def tearDownClass(cls) -> None:
        shutil.rmtree(cls.base_tmp_dir)
        shutil.rmtree(cls.bin_dir)
        shutil.rmtree(cls.hh_tmp_dir)

    def write_load_config(
        self, use_serverless_ide: bool = False, use_saved_state: bool = False
    ) -> None:
        """
        Writes out a script that will print the list of changed files,
        and adds the path to that script to .hhconfig
        """
        raise NotImplementedError()

    def wait_until_server_ready(self) -> None:
        """
        We don't want to accidentally connect to an old hh_server, so we wait 2
        seconds for the monitor to start up the new server first.
        """
        time.sleep(2)
        self.run_check()

    def start_hh_server(
        self,
        changed_files: Optional[List[str]] = None,
        saved_state_path: Optional[str] = None,
        args: Optional[List[str]] = None,
    ) -> None:
        """Start an hh_server. changed_files is ignored here (as it
        has no meaning) and is only exposed in this API for the derived
        classes.
        """
        if changed_files is None:
            changed_files = []
        if args is None:
            args = []
        cmd = [hh_server, "--daemon", "--max-procs", "2", self.repo_dir] + args
        self.proc_call(cmd)
        self.wait_until_server_ready()

    def stop_hh_server(self, retries: int = 3) -> None:
        (_, _, exit_code) = self.proc_call([hh_client, "stop", self.repo_dir])
        if exit_code == 0:
            return
        elif retries > 0 and exit_code != 0:
            self.stop_hh_server(retries=retries - 1)
        else:
            self.assertEqual(exit_code, 0, msg="Stopping hh_server failed")

    def get_server_logs(self) -> str:
        time.sleep(2)  # wait for logs to be written
        log_file = self.proc_call([hh_client, "--logname", self.repo_dir])[0].strip()
        with open(log_file) as f:
            return f.read()

    def get_monitor_logs(self) -> str:
        time.sleep(2)  # wait for logs to be written
        log_file = self.proc_call([hh_client, "--monitor-logname", self.repo_dir])[
            0
        ].strip()
        with open(log_file) as f:
            return f.read()

    def setUp(self) -> None:
        shutil.copytree(self.template_repo, self.repo_dir)

    def tearDownWithRetries(self, retries: int = 3) -> None:
        self.stop_hh_server(retries=retries)
        shutil.rmtree(self.repo_dir)

    def tearDown(self) -> None:
        self.tearDownWithRetries()

    @classmethod
    # pyre-fixme[24]: Generic type `subprocess.Popen` expects 1 type parameter.
    def proc_create(cls, args: List[str], env: Mapping[str, str]) -> subprocess.Popen:
        return subprocess.Popen(
            args,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=dict(cls.test_env, **env),
            universal_newlines=True,
        )

    @classmethod
    def proc_call(
        cls,
        args: List[str],
        env: Optional[Mapping[str, str]] = None,
        stdin: Optional[str] = None,
    ) -> Tuple[str, str, int]:
        """
        Invoke a subprocess, return stdout, send stderr to our stderr (for
        debugging)
        """
        env = {} if env is None else env
        print(" ".join(args), file=sys.stderr)
        proc = cls.proc_create(args, env)
        (stdout_data, stderr_data) = proc.communicate(stdin)
        sys.stderr.write(stderr_data)
        sys.stderr.flush()
        retcode = proc.wait()
        return (stdout_data, stderr_data, retcode)

    @classmethod
    def wait_pid_with_timeout(cls, pid: int, timeout: int) -> None:
        """
        Like os.waitpid but with a timeout in seconds.
        """
        waited_time = 0
        while True:
            pid_expected, _ = os.waitpid(pid, os.WNOHANG)
            if pid_expected == pid:
                break
            elif waited_time >= timeout:
                raise subprocess.TimeoutExpired
            else:
                time.sleep(1)
                waited_time += 1

    def run_check(
        self, stdin: Optional[str] = None, options: Optional[List[str]] = None
    ) -> Tuple[str, str, int]:
        options = [] if options is None else options
        root = self.repo_dir + os.path.sep
        return self.proc_call(
            [
                hh_client,
                "check",
                "--retries",
                "20",
                "--error-format",
                "raw",
                self.repo_dir,
            ]
            + list(map(lambda x: x.format(root=root), options)),
            stdin=stdin,
        )

    # Check to see if you can run hackfmt
    def run_hackfmt_check(self) -> bool:
        try:
            (stdout_data, stderr_data, retcode) = self.proc_call([hackfmt, "-help"])
            return retcode == 0
        # If the file isn't found you will get this
        except FileNotFoundError:
            return False

    def run_hackfmt(
        self,
        stdin: Optional[str] = None,
        options: Optional[List[str]] = None,
        expected_output: Optional[str] = None,
    ) -> bool:
        options = [] if options is None else options
        (output, err, retcode) = self.proc_call([hackfmt] + options, stdin=stdin)
        if retcode != 0:
            print("check returned non-zero code: " + str(retcode), file=sys.stderr)
        if expected_output is not None:
            self.assertEqual(expected_output, output)
        return True

    # Runs `hh_client check` asserting the stdout is equal the expected.
    # Returns stderr.
    # Note: assert_laoded_mini_state is ignored here and only used
    # in some derived classes.
    def check_cmd(
        self,
        expected_output: Optional[List[str]],
        stdin: Optional[str] = None,
        options: Optional[List[str]] = None,
        assert_loaded_saved_state: bool = False,
    ) -> str:
        (output, err, retcode) = self.run_check(stdin, options)
        root = self.repo_dir + os.path.sep
        if retcode != 0:
            print("check returned non-zero code: " + str(retcode), file=sys.stderr)
        if expected_output is not None:
            # pyre-fixme[8]: Attribute has type `int`; used as `None`.
            self.maxDiff = None
            self.assertCountEqual(
                map(lambda x: x.format(root=root), expected_output), output.splitlines()
            )
        return err

    def check_cmd_and_json_cmd(
        self,
        expected_output: List[str],
        expected_json: List[str],
        stdin: Optional[str] = None,
        options: Optional[List[str]] = None,
    ) -> None:
        # we run the --json version first because --json --refactor doesn't
        # change any files, but plain --refactor does (i.e. the latter isn't
        # idempotent)
        if options is None:
            options = []
        self.check_cmd(expected_json, stdin, options + ["--json"])
        self.check_cmd(expected_output, stdin, options)

    def subscribe_debug(self) -> DebugSubscription:
        proc = self.proc_create([hh_client, "debug", self.repo_dir], env={})
        return DebugSubscription(proc)

    def start_hh_loop_forever_assert_timeout(self) -> None:
        # create a file with 10 dependencies. Only "big" jobs, that use
        # workers can be interrupted at the moment.
        with open(os.path.join(self.repo_dir, "__hh_loop_forever_foo.php"), "w") as f:
            f.write(
                """<?hh //strict
            function __hh_loop_forever_foo(): int {
              return 4;
            }"""
            )

        for i in range(1, 10):
            with open(
                os.path.join(self.repo_dir, "__hh_loop_forever_bar%d.php" % i), "w"
            ) as f:
                f.write(
                    """<?hh //strict
                function __hh_loop_forever_bar%d(): int {
                  return __hh_loop_forever_foo();
                }"""
                    % i
                )

        self.check_cmd(["No errors!"])

        # trigger rechecking of all 11 file, and make one of them loop
        # until cancelled
        with open(os.path.join(self.repo_dir, "__hh_loop_forever_foo.php"), "w") as f:
            f.write(
                """<?hh //strict
            function __hh_loop_forever_foo(): string {
              hh_loop_forever();
            }"""
            )

        # this should timeout due to infinite loop
        try:
            # empty output means no results due to timeout
            self.check_cmd([], options=["--retries", "1"])
        except AssertionError:
            # one of the test drivers doesn't like timeouts
            pass

    def stop_hh_loop_forever(self) -> None:
        # subsequent change should interrupt the "loop forever" part
        with open(os.path.join(self.repo_dir, "__hh_loop_forever_foo.php"), "w") as f:
            f.write(
                """<?hh //strict
            function __hh_loop_forever_foo(): int {
              return 4;
            }"""
            )

        self.check_cmd(["No errors!"])


# The most basic of tests.
# Exercises server responsiveness, and updating errors after changing files
class BarebonesTests(TestCase[CommonTestDriver]):

    # hh should should work with 0 retries.
    def test_responsiveness(self) -> None:
        self.test_driver.start_hh_server()
        self.test_driver.check_cmd(["No errors!"])
        self.test_driver.check_cmd(["No errors!"], options=["--retries", "0"])

    def test_new_file(self) -> None:
        """
        Add a new file that contains an error.
        """
        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php"), "w") as f:
            f.write(
                """<?hh

            function k(): int {
                return 'a';
            }
            """
            )

        self.test_driver.start_hh_server(changed_files=["foo_4.php"])

        self.test_driver.check_cmd(
            [
                "{root}foo_4.php:4:24,26: Invalid return type (Typing[4110])",
                "  {root}foo_4.php:3:27,29: Expected `int`",
                "  {root}foo_4.php:4:24,26: But got `string`",
            ]
        )

    def test_new_naming_error(self) -> None:
        """
        Add a new file which contains a naming collisions with an old file
        """
        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php"), "w") as f:
            f.write(
                """<?hh //partial

            class FOO {}
            function H () {}
            """
            )

        self.test_driver.start_hh_server(changed_files=["foo_4.php"])

        self.test_driver.check_cmd(
            [
                "{root}foo_4.php:3:19,21: Name already bound: `FOO` (Naming[2012])",
                "  {root}foo_3.php:7:15,17: Previous definition `F~~oo~~` differs only by case ",
                "{root}foo_4.php:4:22,22: Name already bound: `H` (Naming[2012])",
                "  {root}foo_3.php:3:18,18: Previous definition `~~h~~` differs only by case ",
            ]
        )

    # We put this test in Barebones tests so that dependencies on class B
    # show an error (i.e. class_3.php) with both the save state driver
    # and the classic save state driver
    def test_modify_extends_deps(self) -> None:
        """
        Introduce a change to a base class that causes an error
        in a use case on one of its subclasses.
        """
        with open(os.path.join(self.test_driver.repo_dir, "class_1.php"), "w") as f:
            f.write(
                """<?hh // strict

                class B {
                  public static function foo () : bool {
                      return true;
                  }
                }
            """
            )
        self.test_driver.start_hh_server(changed_files=["class_1.php"])
        self.test_driver.check_cmd(
            [
                "{root}class_3.php:5:12,19: Invalid return type (Typing[4110])",
                "  {root}class_3.php:4:28,30: Expected `int`",
                "  {root}class_1.php:4:51,54: But got `bool`",
            ]
        )


# Common tests, includes the Barebones Tests above
class CommonTests(BarebonesTests):
    def test_json_errors(self) -> None:
        """
        If you ask for errors in JSON format, you will get them on standard
        output. Changing this will break the tools that depend on it (like
        editor plugins), and this test is here to remind you about it.
        """
        self.test_driver.start_hh_server()

        stderr = self.test_driver.check_cmd([], options=["--json"])
        last_line = stderr.splitlines()[-1]
        output = json.loads(last_line)

        self.assertEqual(output["errors"], [])
        self.assertEqual(output["passed"], True)
        self.assertIn("version", output)

    def test_modify_file(self) -> None:
        """
        Add an error to a file that previously had none.
        """
        with open(os.path.join(self.test_driver.repo_dir, "foo_2.php"), "w") as f:
            f.write(
                """<?hh

            function g(): int {
                return 'a';
            }
            """
            )

        self.test_driver.start_hh_server(changed_files=["foo_2.php"])

        self.test_driver.check_cmd(
            [
                "{root}foo_2.php:4:24,26: Invalid return type (Typing[4110])",
                "  {root}foo_2.php:3:27,29: Expected `int`",
                "  {root}foo_2.php:4:24,26: But got `string`",
            ]
        )

    def test_deleted_file(self) -> None:
        """
        Delete a file that still has dangling references before restoring from
        a saved state.
        """
        os.remove(os.path.join(self.test_driver.repo_dir, "foo_2.php"))

        self.test_driver.start_hh_server(changed_files=["foo_2.php"])

        self.test_driver.check_cmd(
            [
                "{root}foo_1.php:4:20,20: Unbound name: `g` (a global function) (Naming[2049])"
            ]
        )

    def test_file_delete_after_load(self) -> None:
        """
        Delete a file that still has dangling references after restoring from
        a saved state.
        """
        self.test_driver.start_hh_server()
        self.test_driver.check_cmd(["No errors!"])
        debug_sub = self.test_driver.subscribe_debug()

        os.remove(os.path.join(self.test_driver.repo_dir, "foo_2.php"))
        msgs = debug_sub.get_incremental_logs()
        self.assertEqual(msgs["to_redecl_phase1"]["files"], ["foo_2.php"])
        self.assertEqual(msgs["to_redecl_phase2"]["files"], ["foo_2.php"])
        self.assertEqual(
            set(msgs["to_recheck"]["files"]), set(["foo_1.php", "foo_2.php"])
        )
        self.test_driver.check_cmd(
            [
                "{root}foo_1.php:4:20,20: Unbound name: `g` (a global function) (Naming[2049])"
            ]
        )

    def test_duplicated_file(self) -> None:
        self.test_driver.start_hh_server(changed_files=["foo_2.php"])
        self.test_driver.check_cmd(["No errors!"])

        shutil.copyfile(
            os.path.join(self.test_driver.repo_dir, "foo_2.php"),
            os.path.join(self.test_driver.repo_dir, "foo_2_dup.php"),
        )

        self.test_driver.check_cmd(
            [
                "{root}foo_2_dup.php:3:18,18: Name already bound: `g` (Naming[2012])",
                "  {root}foo_2.php:3:18,18: Previous definition is here",
            ]
        )

        os.remove(os.path.join(self.test_driver.repo_dir, "foo_2.php"))
        self.test_driver.check_cmd(["No errors!"])

    def test_moved_file(self) -> None:
        """
        Move a file, then create an error that references a definition in it.
        Check that the new file name is displayed in the error.
        """

        self.test_driver.start_hh_server(
            changed_files=["foo_1.php", "foo_2.php", "bar_2.php"]
        )

        os.rename(
            os.path.join(self.test_driver.repo_dir, "foo_2.php"),
            os.path.join(self.test_driver.repo_dir, "bar_2.php"),
        )

        with open(os.path.join(self.test_driver.repo_dir, "foo_1.php"), "w") as f:
            f.write(
                """<?hh

            function f(): string {
                return g();
            }
            """
            )

        self.test_driver.check_cmd(
            [
                "{root}foo_1.php:4:24,26: Invalid return type (Typing[4110])",
                "  {root}foo_1.php:3:27,32: Expected `string`",
                "  {root}bar_2.php:3:23,25: But got `int`",
            ]
        )

    def test_find_refs(self) -> None:
        """
        Test hh_client --find-refs, --find-class-refs
        """
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd_and_json_cmd(
            ['File "{root}foo_3.php", line 11, characters 13-13: h', "1 total results"],
            [
                '[{{"name":"h","filename":"{root}foo_3.php","line":11,"char_start":13,"char_end":13}}]'
            ],
            options=["--find-refs", "h"],
        )

        self.test_driver.check_cmd_and_json_cmd(
            [
                'File "{root}foo_3.php", line 10, characters 17-19: Foo::__construct',
                "1 total results",
            ],
            [
                '[{{"name":"Foo::__construct","filename":"{root}foo_3.php","line":10,"char_start":17,"char_end":19}}]'
            ],
            options=["--find-refs", "Foo::__construct"],
        )

        self.test_driver.check_cmd_and_json_cmd(
            [
                'File "{root}foo_3.php", line 10, characters 17-19: Foo',
                "1 total results",
            ],
            [
                '[{{"name":"Foo","filename":"{root}foo_3.php","line":10,'
                '"char_start":17,"char_end":19}}]'
            ],
            options=["--find-class-refs", "Foo"],
        )

    def test_identify_symbol(self) -> None:
        """
        Test hh_client --identify
        """
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd_and_json_cmd(
            [],
            [
                '[{{"full_name":"B","pos":{{"filename":"{root}class_1.php","line":3,"char_start":7,"char_end":7}},"kind":"class"}}]'
            ],
            options=["--identify", "B"],
        )

        self.test_driver.check_cmd_and_json_cmd(
            [],
            [
                '[{{"full_name":"B::foo","pos":{{"filename":"{root}class_1.php","line":5,"char_start":26,"char_end":28}},"kind":"method"}}]'
            ],
            options=["--identify", "B::foo"],
        )

        self.test_driver.check_cmd_and_json_cmd(
            [],
            [
                '[{{"full_name":"CONST_SOME_COOL_VALUE","pos":{{"filename":"{root}const_1.php","line":3,"char_start":11,"char_end":31}},"kind":"const"}}]'
            ],
            options=["--identify", "CONST_SOME_COOL_VALUE"],
        )

        self.test_driver.check_cmd_and_json_cmd(
            [],
            [
                '[{{"full_name":"FbidMapField","pos":{{"filename":"{root}enum_1.php","line":3,"char_start":6,"char_end":17}},"kind":"enum"}}]'
            ],
            options=["--identify", "FbidMapField"],
        )

        self.test_driver.check_cmd_and_json_cmd(
            [],
            [
                '[{{"full_name":"FbidMapField::FBID","pos":{{"filename":"{root}enum_1.php","line":4,"char_start":3,"char_end":6}},"kind":"const"}}]'
            ],
            options=["--identify", "FbidMapField::FBID"],
        )

        self.test_driver.check_cmd_and_json_cmd(
            [],
            [
                '[{{"full_name":"f","pos":{{"filename":"{root}foo_1.php","line":3,"char_start":18,"char_end":18}},"kind":"function"}}]'
            ],
            options=["--identify", "f"],
        )

    def test_ide_find_refs(self) -> None:
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd_and_json_cmd(
            [
                "Foo",
                'File "{root}foo_3.php", line 10, characters 17-19:',
                "1 total results",
            ],
            [
                '[{{"name":"Foo","filename":"{root}foo_3.php",'
                '"line":10,"char_start":17,"char_end":19}}]'
            ],
            options=["--ide-find-refs", "1:20"],
            stdin="<?hh function test(Foo $foo) { new Foo(); }",
        )

    def test_ide_highlight_refs(self) -> None:
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd_and_json_cmd(
            ["line 1, characters 20-22", "line 1, characters 36-38", "2 total results"],
            [
                '[{{"line":1,"char_start":20,"char_end":22}},'
                '{{"line":1,"char_start":36,"char_end":38}}]'
            ],
            options=["--ide-highlight-refs", "1:20"],
            stdin="<?hh function test(Foo $foo) { new Foo(); }",
        )

    def test_search(self) -> None:
        """
        Test hh_client --search
        """

        self.test_driver.start_hh_server()

        self.test_driver.check_cmd_and_json_cmd(
            [
                'File "{root}foo_3.php", line 9, characters 18-40: some_long_function_name, function'
            ],
            [
                '[{{"name":"some_long_function_name","filename":"{root}foo_3.php","desc":"function","line":9,"char_start":18,"char_end":40,"scope":""}}]'
            ],
            options=["--search", "some_lo"],
        )

    def test_search_case_insensitive1(self) -> None:
        """
        Test that global search is not case sensitive
        """
        self.maxDiff = None
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd(
            [
                'File "{root}foo_4.php", line 4, characters 10-24: '
                "aaaaaaaaaaa_fun, function",
                'File "{root}foo_4.php", line 3, characters 7-23: '
                "Aaaaaaaaaaa_class, class",
            ],
            options=["--search", "Aaaaaaaaaaa"],
        )

    def test_search_case_insensitive2(self) -> None:
        """
        Test that global search is not case sensitive
        """
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd(
            [
                'File "{root}foo_4.php", line 4, characters 10-24: '
                "aaaaaaaaaaa_fun, function",
                'File "{root}foo_4.php", line 3, characters 7-23: '
                "Aaaaaaaaaaa_class, class",
            ],
            options=["--search", "aaaaaaaaaaa"],
        )

    def test_auto_complete(self) -> None:
        """
        Test hh_client --auto-complete
        """

        self.test_driver.start_hh_server()

        self.test_driver.check_cmd_and_json_cmd(
            ["some_long_function_name (function(): _)"],
            [
                # test the --json output because the non-json one doesn't contain
                # the filename, and we are especially interested in testing file
                # paths
                # the doubled curly braces are because this string gets passed
                # through format()
                '[{{"name":"some_long_function_name",'
                '"type":"(function(): _)",'
                '"pos":{{"filename":"{root}foo_3.php",'
                '"line":9,"char_start":18,"char_end":40}},'
                '"func_details":{{"min_arity":0,"return_type":"_","params":[]}},'
                '"expected_ty":false}}]'
            ],
            options=["--auto-complete"],
            stdin="<?hh function f() { some_AUTO332\n",
        )

    def test_list_files(self) -> None:
        """
        Test hh_client --list-files
        """
        os.remove(os.path.join(self.test_driver.repo_dir, "foo_2.php"))
        self.test_driver.start_hh_server(changed_files=["foo_2.php"])
        self.test_driver.check_cmd_and_json_cmd(
            ["{root}foo_1.php"],
            ["{root}foo_1.php"],  # see comment for identify-function
            options=["--list-files"],
        )

    def test_type_at_pos(self) -> None:
        """
        Test hh_client --type-at-pos
        """
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd_and_json_cmd(
            ["string"],
            [
                '{{"type":"string",'
                + '"pos":{{"filename":"","line":0,"char_start":0,"char_end":0}},'
                + '"full_type":{{"src_pos":{{"filename":"{root}foo_3.php","line":3,"char_start":23,"char_end":28}},"kind":"primitive","name":"string"}}}}'
            ],
            options=["--type-at-pos", "{root}foo_3.php:11:14"],
        )

    def test_type_at_pos_batch(self) -> None:
        """
        Test hh_client --type-at-pos-batch
        """
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd(
            [
                '{{"position":'
                + '{{"file":"{root}foo_3.php",'
                + '"line":11,'
                + '"character":14}}'
                + ',"type":{{'
                + '"src_pos":{{"filename":"{root}foo_3.php","line":3,"char_start":23,"char_end":28}},'
                + '"kind":"primitive",'
                + '"name":"string"}}}}'
            ],
            options=["--type-at-pos-batch", "{root}foo_3.php:11:14"],
        )

    def test_ide_get_definition(self) -> None:
        """
        Test hh_client --ide-get-definition
        """
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd_and_json_cmd(
            [
                "name: \\bar, kind: function, span: line 1, characters 42-44,"
                " is_declaration: false",
                "definition:",
                " bar",
                "   kind: function",
                "   id: function::bar",
                '   position: File "", line 1, characters 15-17:',
                '   span: File "", line 1, character 6 - line 1, character 22:',
                "   modifiers: ",
                "   params:",
                "",
                "",
            ],
            [
                '[{{"name":"\\\\bar","result_type":"function",'
                '"pos":{{"filename":"","line":1,"char_start":42,"char_end":44}},'
                '"definition_pos":{{"filename":"","line":1,"char_start":15,'
                '"char_end":17}},"definition_span":{{"filename":"","line_start":1,'
                '"char_start":6,"line_end":1,"char_end":22}},'
                '"definition_id":"function::bar"}}]'
            ],
            options=["--ide-get-definition", "1:43"],
            stdin="<?hh function bar() {} function test() { bar() }",
        )

    def test_ide_outline(self) -> None:
        """
        Test hh_client --ide-outline
        """
        self.test_driver.start_hh_server()

        """
        This call is here to ensure that server is running. Outline command
        doesn't require it to be, but integration test suite assumes it and
        checks it's state after each test.
        """
        self.test_driver.check_cmd(["No errors!"])

        self.test_driver.check_cmd_and_json_cmd(
            [
                "bar",
                "  kind: function",
                "  id: function::bar",
                '  position: File "", line 1, characters 15-17:',
                '  span: File "", line 1, character 6 - line 1, character 22:',
                "  modifiers: ",
                "  params:",
                "",
            ],
            [
                '[{{"kind":"function","name":"bar","id":"function::bar",'
                '"position":{{"filename":"",'
                '"line":1,"char_start":15,"char_end":17}},"span":'
                '{{"filename":"","line_start":1,"char_start":6,"line_end":1,'
                '"char_end":22}},"modifiers":[],"params":[]}}]'
            ],
            options=["--ide-outline"],
            stdin="<?hh function bar() {}",
        )

    def test_ide_get_definition_multi_file(self) -> None:
        """
        Test hh_client --ide-get-definition when definition we look for is
        in file different from input file
        """
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd_and_json_cmd(
            [
                "name: \\ClassToBeIdentified::methodToBeIdentified, kind: method,"
                " span: line 1, characters 45-64, is_declaration: false",
                "definition:",
                " methodToBeIdentified",
                "   kind: method",
                "   id: method::ClassToBeIdentified::methodToBeIdentified",
                '   position: File "{root}foo_5.php", line 4, characters 26-45:',
                '   span: File "{root}foo_5.php", line 4, character 3 - line 4,'
                " character 50:",
                "   modifiers: public static ",
                "   params:",
                "",
                "",
            ],
            [
                '[{{"name":"\\\\ClassToBeIdentified::methodToBeIdentified",'
                '"result_type":"method","pos":{{"filename":"","line":1,'
                '"char_start":45,"char_end":64}},"definition_pos":'
                '{{"filename":"{root}foo_5.php","line":4,"char_start":26,'
                '"char_end":45}},"definition_span":{{"filename":"{root}foo_5.php",'
                '"line_start":4,"char_start":3,"line_end":4,"char_end":50}},'
                '"definition_id":'
                '"method::ClassToBeIdentified::methodToBeIdentified"}}]'
            ],
            options=["--ide-get-definition", "1:50"],
            stdin="<?hh function test() { "
            "ClassToBeIdentified::methodToBeIdentified () }",
        )

    def test_abnormal_typechecker_exit_message(self) -> None:
        """
        Tests that the monitor outputs a useful message when its typechecker
        exits abnormally.
        """

        self.test_driver.start_hh_server()
        monitor_logs = self.test_driver.get_monitor_logs()
        m = re.search(
            "Just started typechecker server with pid: ([0-9]+)", monitor_logs
        )
        self.assertIsNotNone(m)
        assert m is not None, "for mypy"
        pid = m.group(1)
        self.assertIsNotNone(pid)
        os.kill(int(pid), signal.SIGTERM)
        # For some reason, waitpid in the monitor after the kill signal
        # sent above doesn't preserve ordering - maybe because they're
        # in separate processes? Give it some time.
        time.sleep(1)
        client_error = self.test_driver.check_cmd(
            expected_output=None, assert_loaded_saved_state=False
        )
        self.assertIn("Last server killed by signal", client_error)

    def test_duplicate_parent(self) -> None:
        """
        This checks that we handle duplicate parent classes, i.e. when Bar
        extends Foo and there are two declarations of Foo. We want to make sure
        that when the duplicate gets removed, we recover correctly by
        redeclaring Bar with the remaining parent class.
        """
        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php"), "w") as f:
            f.write(
                """<?hh //partial

            class Foo { // also declared in foo_3.php in setUpClass
                public static $x;
            }
            """
            )
        with open(os.path.join(self.test_driver.repo_dir, "foo_5.php"), "w") as f:
            f.write(
                """<?hh //partial

            class Bar extends Foo {}

            function main(Bar $a) {
                return $a::$y;
            }
            """
            )
        self.test_driver.start_hh_server(changed_files=["foo_4.php", "foo_5.php"])
        self.test_driver.check_cmd(
            [
                "{root}foo_4.php:3:19,21: Name already bound: `Foo` (Naming[2012])",
                "  {root}foo_3.php:7:15,17: Previous definition is here",
                "{root}foo_5.php:6:28,29: No class variable `$y` in `Bar` (Typing[4090])",
                "  {root}foo_5.php:3:19,21: Declaration of `Bar` is here",
            ]
        )

        os.remove(os.path.join(self.test_driver.repo_dir, "foo_4.php"))
        self.test_driver.check_cmd(
            [
                "{root}foo_5.php:6:28,29: No class variable `$y` in `Bar` (Typing[4090])",
                "  {root}foo_5.php:3:19,21: Declaration of `Bar` is here",
            ]
        )

        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php"), "w") as f:
            f.write(
                """<?hh //partial

            class Foo {
                public static $y;
            }
            """
            )
        os.remove(os.path.join(self.test_driver.repo_dir, "foo_3.php"))
        self.test_driver.check_cmd(["No errors!"])

    def test_refactor_methods(self) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php"), "w") as f:
            f.write(
                """<?hh //partial

            class Bar extends Foo {
                public function f() {}
                public function g() {}
            }

            class Baz extends Bar {
                public function g() {
                    $this->f();
                }
            }
            """
            )
        self.test_driver.start_hh_server(changed_files=["foo_4.php"])

        self.test_driver.check_cmd_and_json_cmd(
            ["Rewrote 1 file."],
            [
                '[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":84,"char_end":85,"line":4,"col_start":33,'
                '"col_end":33,"patch_type":"replace","replacement":"wat"}},'
                '{{"char_start":246,"char_end":247,"line":10,"col_start":28,'
                '"col_end":28,"patch_type":"replace","replacement":"wat"}}]}}]'
            ],
            options=["--refactor", "Method", "Bar::f", "Bar::wat"],
        )
        self.test_driver.check_cmd_and_json_cmd(
            ["Rewrote 1 file."],
            [
                '[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":125,"char_end":126,"line":5,"col_start":33,'
                '"col_end":33,"patch_type":"replace",'
                '"replacement":"overrideMe"}},{{"char_start":215,'
                '"char_end":216,"line":9,"col_start":33,"col_end":33,'
                '"patch_type":"replace","replacement":"overrideMe"}}]}}]'
            ],
            options=["--refactor", "Method", "Bar::g", "Bar::overrideMe"],
        )

        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php")) as f:
            out = f.read()
            self.assertEqual(
                out,
                """<?hh //partial

            class Bar extends Foo {
                public function wat() {}
                public function overrideMe() {}
            }

            class Baz extends Bar {
                public function overrideMe() {
                    $this->wat();
                }
            }
            """,
            )

    def test_refactor_classes(self) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php"), "w") as f:
            f.write(
                """<?hh //partial

            class Bar extends Foo {
                const int FOO = 42;

                private static int $val = 0;

                public function f() {}
                public function g() {}
                public static function h() {}
                public static function i() {
                    self::h();
                    self::$val = 1;
                    static::$val = 2;
                    $x = self::FOO;
                }
            }

            class Baz extends Bar {
                public function g() {
                    $this->f();
                    parent::g();
                }
            }
            """
            )
        self.test_driver.start_hh_server(changed_files=["foo_4.php"])

        self.test_driver.check_cmd_and_json_cmd(
            ["Rewrote 2 files."],
            [
                '[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":46,"char_end":49,"line":3,"col_start":31,'
                '"col_end":33,"patch_type":"replace","replacement":"Qux"}}]}},'
                '{{"filename":"{root}foo_3.php","patches":[{{'
                '"char_start":96,"char_end":99,"line":7,"col_start":15,'
                '"col_end":17,"patch_type":"replace","replacement":"Qux"}},'
                '{{"char_start":165,"char_end":168,"line":10,"col_start":17,'
                '"col_end":19,"patch_type":"replace","replacement":"Qux"}}]'
                "}}]"
            ],
            options=["--refactor", "Class", "Foo", "Qux"],
        )
        self.test_driver.check_cmd_and_json_cmd(
            ["Rewrote 1 file."],
            [
                '[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":34,"char_end":37,"line":3,"col_start":19,'
                '"col_end":21,"patch_type":"replace","replacement":"Quux"}},'
                '{{"char_start":508,"char_end":511,"line":19,"col_start":31,'
                '"col_end":33,"patch_type":"replace","replacement":"Quux"}}]}}]'
            ],
            options=["--refactor", "Class", "Bar", "Quux"],
        )

        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php")) as f:
            out = f.read()
            self.assertEqual(
                out,
                """<?hh //partial

            class Quux extends Qux {
                const int FOO = 42;

                private static int $val = 0;

                public function f() {}
                public function g() {}
                public static function h() {}
                public static function i() {
                    self::h();
                    self::$val = 1;
                    static::$val = 2;
                    $x = self::FOO;
                }
            }

            class Baz extends Quux {
                public function g() {
                    $this->f();
                    parent::g();
                }
            }
            """,
            )

        with open(os.path.join(self.test_driver.repo_dir, "foo_3.php")) as f:
            out = f.read()
            self.assertEqual(
                out,
                """<?hh //partial

        function h(): string {
            return "a";
        }

        class Qux {}

        function some_long_function_name() {
            new Qux();
            h();
        }
""",
            )

    def test_refactor_functions(self) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php"), "w") as f:
            f.write(
                """<?hh //partial

            function wow() {
                wat();
                return f();
            }

            function wat() {}
            """
            )
        self.test_driver.start_hh_server(changed_files=["foo_4.php"])

        self.test_driver.check_cmd_and_json_cmd(
            ["Rewrote 1 file."],
            [
                '[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":132,"char_end":135,"line":8,"col_start":22,'
                '"col_end":24,"patch_type":"replace","replacement":"woah"}},'
                '{{"char_start":61,"char_end":64,"line":4,"col_start":17,'
                '"col_end":19,"patch_type":"replace","replacement":"woah"}}]'
                "}}]"
            ],
            options=["--refactor", "Function", "wat", "woah"],
        )
        self.test_driver.check_cmd_and_json_cmd(
            ["Rewrote 2 files."],
            [
                '[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":92,"char_end":93,"line":5,"col_start":24,'
                '"col_end":24,"patch_type":"replace","replacement":"fff"}}]}},'
                '{{"filename":"{root}foo_1.php","patches":[{{'
                '"char_start":33,"char_end":34,"line":3,"col_start":18,'
                '"col_end":18,"patch_type":"replace","replacement":"fff"}}]'
                "}}]"
            ],
            options=["--refactor", "Function", "f", "fff"],
        )

        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php")) as f:
            out = f.read()
            self.assertEqual(
                out,
                """<?hh //partial

            function wow() {
                woah();
                return fff();
            }

            function woah() {}
            """,
            )

        with open(os.path.join(self.test_driver.repo_dir, "foo_1.php")) as f:
            out = f.read()
            self.assertEqual(
                out,
                """<?hh //partial

        function fff() {
            return g() + 1;
        }
""",
            )

    def test_refactor_typedefs(self) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php"), "w") as f:
            f.write(
                """<?hh //partial

            newtype NewType = int;
            type Type = int;

            class MyClass {
                public function myFunc(Type $x): NewType {
                    return $x;
                }
            }
            """
            )
        self.test_driver.start_hh_server(changed_files=["foo_4.php"])

        self.test_driver.check_cmd_and_json_cmd(
            ["Rewrote 1 file."],
            [
                '[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":36,"char_end":43,"line":3,"col_start":21,'
                '"col_end":27,"patch_type":"replace","replacement":"NewTypeX"}},'
                '{{"char_start":158,"char_end":165,"line":7,"col_start":50,'
                '"col_end":56,"patch_type":"replace","replacement":"NewTypeX"}}]'
                "}}]"
            ],
            options=["--refactor", "Class", "NewType", "NewTypeX"],
        )

        self.test_driver.check_cmd_and_json_cmd(
            ["Rewrote 1 file."],
            [
                '[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":69,"char_end":73,"line":4,"col_start":18,'
                '"col_end":21,"patch_type":"replace","replacement":"TypeX"}},'
                '{{"char_start":149,"char_end":153,"line":7,"col_start":40,'
                '"col_end":43,"patch_type":"replace","replacement":"TypeX"}}]'
                "}}]"
            ],
            options=["--refactor", "Class", "Type", "TypeX"],
        )

        with open(os.path.join(self.test_driver.repo_dir, "foo_4.php")) as f:
            out = f.read()
            self.assertEqual(
                out,
                """<?hh //partial

            newtype NewTypeX = int;
            type TypeX = int;

            class MyClass {
                public function myFunc(TypeX $x): NewTypeX {
                    return $x;
                }
            }
            """,
            )

    def test_auto_namespace_alias_addition(self) -> None:
        """
        Add namespace alias and check if it is still good
        """

        self.test_driver.start_hh_server()
        self.test_driver.check_cmd(["No errors!"])

        with open(os.path.join(self.test_driver.repo_dir, "auto_ns_2.php"), "w") as f:
            f.write(
                """<?hh //partial

            function haha() {
                Herp\\f();
                return 1;
            }
            """
            )

        self.test_driver.check_cmd(["No errors!"])

    def test_interrupt(self) -> None:
        # filesystem interruptions are only triggered by Watchman
        with open(os.path.join(self.test_driver.repo_dir, ".watchmanconfig"), "w") as f:
            f.write("{}")
        with open(os.path.join(self.test_driver.repo_dir, "hh.conf"), "a") as f:
            f.write(
                "use_watchman = true\n"
                + "interrupt_on_watchman = true\n"
                + "interrupt_on_client = true\n"
                + "watchman_subscribe_v2 = true\n"
            )

        self.test_driver.start_hh_server()
        self.test_driver.start_hh_loop_forever_assert_timeout()
        self.test_driver.check_cmd(
            ["string"], options=["--type-at-pos", "{root}foo_3.php:11:14"]
        )
        self.test_driver.stop_hh_loop_forever()

    def test_status_single(self) -> None:
        """
        Test hh_client check --single
        """
        self.test_driver.start_hh_server()

        with open(
            os.path.join(self.test_driver.repo_dir, "typing_error.php"), "w"
        ) as f:
            f.write("<?hh //strict\n function aaaa(): int { return h(); }")

        self.test_driver.check_cmd(
            [
                "{root}typing_error.php:2:32,34: Invalid return type (Typing[4110])",
                "  {root}typing_error.php:2:19,21: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
            ],
            options=["--single", "{root}typing_error.php"],
            stdin="",
        )

        self.test_driver.check_cmd(
            [
                ":2:32,34: Invalid return type (Typing[4110])",
                "  :2:19,21: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
            ],
            options=["--single", "-"],
            stdin="<?hh //strict\n function aaaa(): int { return h(); }",
        )

    def test_lint_xcontroller(self) -> None:
        self.test_driver.start_hh_server()

        with open(os.path.join(self.test_driver.repo_dir, "in_list.txt"), "w") as f:
            f.write(os.path.join(self.test_driver.repo_dir, "xcontroller.php"))

        with open(os.path.join(self.test_driver.repo_dir, "xcontroller.php"), "w") as f:
            f.write(
                "<?hh\n class MyXController extends XControllerBase { "
                "public function getPath() { return f(); }  }"
            )

        self.test_driver.check_cmd(
            [
                'File "{root}xcontroller.php", line 2, characters 8-20:',
                "When linting MyXController: The body of isDelegateOnly should "
                "only contain `return true;` or `return false;` (Lint[5615])",
                'File "{root}xcontroller.php", line 2, characters 8-20:',
                "When linting MyXController: getPath method of MyXController must "
                "be present and return a static literal for build purposes (Lint[5615])",
            ],
            options=["--lint-xcontroller", "{root}in_list.txt"],
        )

    def test_incremental_typecheck_same_file(self) -> None:
        self.maxDiff = None
        self.test_driver.start_hh_server()

        # Important: typecheck the file after creation but before adding contents
        # to test forward naming table updating.
        open(
            os.path.join(
                self.test_driver.repo_dir, "test_incremental_typecheck_same_file.php"
            ),
            "w",
        ).close()
        self.test_driver.check_cmd(["No errors!"])

        with open(
            os.path.join(
                self.test_driver.repo_dir, "test_incremental_typecheck_same_file.php"
            ),
            "w",
        ) as f:
            f.write(
                """<?hh // strict

                // test_incremental_typecheck_same_file
                class TestIncrementalTypecheckSameFile {}
            """
            )
        self.test_driver.check_cmd(["No errors!"])

        # Notice how the only change is the removed doc block.
        with open(
            os.path.join(
                self.test_driver.repo_dir, "test_incremental_typecheck_same_file.php"
            ),
            "w",
        ) as f:
            f.write(
                """<?hh // strict

                class TestIncrementalTypecheckSameFile {}
            """
            )
        self.test_driver.check_cmd(["No errors!"])
