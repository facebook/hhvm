from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import glob
import json
import os
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time

from hh_paths import hh_server, hh_client
from utils import write_files

class CommonSaveStateTests(object):

    @classmethod
    def setUpClass(cls):
        cls.maxDiff = 2000
        # we create the state in a different dir from the one we run our tests
        # on, to verify that the saved state does not depend on any absolute
        # paths
        init_dir = tempfile.mkdtemp()
        cls.repo_dir = tempfile.mkdtemp()
        cls.config_path = os.path.join(cls.repo_dir, '.hhconfig')
        cls.tmp_dir = tempfile.mkdtemp()
        cls.hh_tmp_dir = tempfile.mkdtemp()
        cls.saved_state_name = 'foo'
        hh_server_dir = os.path.dirname(hh_server)
        cls.test_env = dict(os.environ, **{
            'HH_TEST_MODE': '1',
            'HH_TMPDIR': cls.hh_tmp_dir,
            'PATH': '%s:%s:/bin:/usr/bin:/usr/local/bin' %
                (hh_server_dir, cls.tmp_dir),
            'OCAMLRUNPARAM': 'b',
            'HH_LOCALCONF_PATH': cls.repo_dir,
            })

        with open(os.path.join(init_dir, '.hhconfig'), 'w') as f:
            f.write(r"""
# some comment
assume_php = false""")

        cls.files = {}

        cls.files['foo_1.php'] = """
        <?hh
        function f() {
            return g() + 1;
        }
        """

        cls.files['foo_2.php'] = """
        <?hh
        function g(): int {
            return 0;
        }
        """

        cls.files['foo_3.php'] = """
        <?hh
        function h(): string {
            return "a";
        }

        class Foo {}

        function some_long_function_name() {
            new Foo();
            h();
        }
        """

        write_files(cls.files, init_dir)

        cls.save_command(init_dir)

        shutil.rmtree(init_dir)

    @classmethod
    def save_command(cls):
        raise NotImplementedError()

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp_dir)
        shutil.rmtree(cls.hh_tmp_dir)

    @classmethod
    def saved_state_path(cls):
        return os.path.join(cls.tmp_dir, cls.saved_state_name)

    def write_load_config(self, *changed_files):
        raise NotImplementedError()

    def start_hh_server(self):
        cmd = [hh_server, self.repo_dir]
        print(" ".join(cmd), file=sys.stderr)
        return subprocess.Popen(
                cmd,
                stderr=subprocess.PIPE,
                env=self.test_env)

    def get_server_logs(self):
        time.sleep(2)  # wait for logs to be written
        log_file = self.proc_call([
            hh_client, '--logname', self.repo_dir])[0].strip()
        with open(log_file) as f:
            return f.read()

    def setUp(self):
        if os.path.isdir(self.repo_dir) is False:
            os.mkdir(self.repo_dir)
        write_files(self.files, self.repo_dir)

    def tearDown(self):
        (_, _, exit_code) = self.proc_call([
            hh_client,
            'stop',
            self.repo_dir
        ])
        self.assertEqual(exit_code, 0, msg="Stopping hh_server failed")

        shutil.rmtree(self.repo_dir)

    @classmethod
    def proc_call(cls, args, env=None, stdin=None):
        """
        Invoke a subprocess, return stdout, send stderr to our stderr (for
        debugging)
        """
        env = {} if env is None else env
        print(" ".join(args), file=sys.stderr)
        proc = subprocess.Popen(
                args,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=dict(cls.test_env, **env),
                universal_newlines=True)
        (stdout_data, stderr_data) = proc.communicate(stdin)
        sys.stderr.write(stderr_data)
        sys.stderr.flush()
        retcode = proc.wait()
        return (stdout_data, stderr_data, retcode)

    # Runs `hh_client check` asserting the stdout is equal the expected.
    # Returns stderr.
    def check_cmd(self, expected_output, stdin=None, options=None):
        raise NotImplementedError()

    def check_cmd_and_json_cmd(
        self,
        expected_output,
        expected_json,
        stdin=None,
        options=None
    ):
        self.check_cmd(expected_output, stdin, options)
        options.append("--json")
        self.check_cmd(expected_json, stdin, options)

    # hh should should work with 0 retries.
    def test_responsiveness(self):
        self.write_load_config()
        self.check_cmd(['No errors!'])
        self.check_cmd(['No errors!'], options=['--retries', '0'])

    def test_json_errors(self):
        """
        If you ask for errors in JSON format, you will get them on standard
        output. Changing this will break the tools that depend on it (like
        editor plugins), and this test is here to remind you about it.
        """
        self.write_load_config()

        stderr = self.check_cmd([], options=["--json"])
        last_line = stderr.splitlines()[-1]
        output = json.loads(last_line)

        self.assertEqual(output["errors"], [])
        self.assertEqual(output["passed"], True)
        self.assertIn("version", output)

    def test_modify_file(self):
        """
        Add an error to a file that previously had none.
        """
        with open(os.path.join(self.repo_dir, 'foo_2.php'), 'w') as f:
            f.write("""
            <?hh
            function g(): int {
                return 'a';
            }
            """)

        self.write_load_config('foo_2.php')

        self.check_cmd([
            '{root}foo_2.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_2.php:3:27,29: This is an int',
            '  {root}foo_2.php:4:24,26: It is incompatible with a string',
        ])

    def test_new_file(self):
        """
        Add a new file that contains an error.
        """
        with open(os.path.join(self.repo_dir, 'foo_4.php'), 'w') as f:
            f.write("""
            <?hh
            function k(): int {
                return 'a';
            }
            """)

        self.write_load_config('foo_4.php')

        self.check_cmd([
            '{root}foo_4.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_4.php:3:27,29: This is an int',
            '  {root}foo_4.php:4:24,26: It is incompatible with a string',
        ])

    def test_deleted_file(self):
        """
        Delete a file that still has dangling references after restoring from
        a saved state.
        """
        os.remove(os.path.join(self.repo_dir, 'foo_2.php'))

        self.write_load_config('foo_2.php')

        self.check_cmd([
            '{root}foo_1.php:4:20,20: Unbound name: g (a global function) (Naming[2049])',
            '{root}foo_1.php:4:20,20: Unbound name: g (a global constant) (Naming[2049])',
            ])

    def test_duplicated_file(self):
        self.write_load_config('foo_2.php')
        self.check_cmd(['No errors!'])

        shutil.copyfile(
                os.path.join(self.repo_dir, 'foo_2.php'),
                os.path.join(self.repo_dir, 'foo_2_dup.php'))

        self.check_cmd([
            '{root}foo_2_dup.php:3:18,18: Name already bound: g (Naming[2012])',
            '  {root}foo_2.php:3:18,18: Previous definition is here'])

        os.remove(os.path.join(self.repo_dir, 'foo_2.php'))
        self.check_cmd(['No errors!'])

    def test_moved_file(self):
        """
        Move a file, then create an error that references a definition in it.
        Check that the new file name is displayed in the error.
        """

        self.write_load_config(
            'foo_1.php', 'foo_2.php', 'bar_2.php',
        )

        os.rename(
            os.path.join(self.repo_dir, 'foo_2.php'),
            os.path.join(self.repo_dir, 'bar_2.php'),
        )

        with open(os.path.join(self.repo_dir, 'foo_1.php'), 'w') as f:
            f.write("""
            <?hh
            function f(): string {
                return g();
            }
            """)

        self.check_cmd([
            '{root}foo_1.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_1.php:3:27,32: This is a string',
            '  {root}bar_2.php:3:23,25: It is incompatible with an int',

            ])

    def test_ide_tools(self):
        """
        Test hh_client --search, --find-refs, --find-class-refs, --type-at-pos,
        and --list-files

        We *could* break this up into multiple tests, but starting the server
        takes time and this test is slow enough already
        """

        self.write_load_config()

        self.check_cmd_and_json_cmd([
            'File "{root}foo_3.php", line 9, characters 18-40: some_long_function_name, function'
            ], [
            '[{{"name":"some_long_function_name","filename":"{root}foo_3.php","desc":"function","line":9,"char_start":18,"char_end":40,"scope":""}}]'
            ], options=['--search', 'some_lo'])

        self.check_cmd_and_json_cmd([
            'File "{root}foo_3.php", line 11, characters 13-13: h',
            '1 total results'
            ], [
            '[{{"name":"h","filename":"{root}foo_3.php","line":11,"char_start":13,"char_end":13}}]'
            ], options=['--find-refs', 'h'])

        self.check_cmd_and_json_cmd([
            'File "{root}foo_3.php", line 10, characters 13-21: Foo::__construct',
            '1 total results'
            ], [
            '[{{"name":"Foo::__construct","filename":"{root}foo_3.php","line":10,"char_start":13,"char_end":21}}]'
            ], options=['--find-refs', 'Foo::__construct'])

        self.check_cmd_and_json_cmd([
            'File "{root}foo_3.php", line 10, characters 17-19: Foo::__construct',
            '1 total results'
            ], [
            '[{{"name":"Foo::__construct","filename":"{root}foo_3.php","line":10,"char_start":17,"char_end":19}}]'
            ], options=['--find-class-refs', 'Foo'])

        self.check_cmd_and_json_cmd([
            'string'
            ], [
            '{{"type":"string","pos":{{"filename":"{root}foo_3.php","line":3,"char_start":23,"char_end":28}}}}'
            ], options=['--type-at-pos', '{root}foo_3.php:11:13'])

        self.check_cmd_and_json_cmd([
            'some_long_function_name (function(): _)'
            ], [
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
            options=['--auto-complete'],
            stdin='<?hh function f() { some_AUTO332\n')

        self.check_cmd_and_json_cmd([
            'Foo::bar'
            ], [
            # looks like identify-function doesn't support JSON -
            # but still be careful changing this, since tools
            # may just call everything with --json flag and it would
            # be a breaking change
            'Foo::bar'
            ],
            options=['--identify-function', '1:51'],
            stdin='<?hh class Foo { private function bar() { $this->bar() }}')

        os.remove(os.path.join(self.repo_dir, 'foo_2.php'))
        self.check_cmd_and_json_cmd([
            '{root}foo_1.php',
            ], [
            '{root}foo_1.php',  # see comment for identify-function
            ], options=['--list-files'])

    def test_abnormal_typechecker_exit_message(self):
        """
        Tests that the monitor outputs a useful message when its typechecker
        exits abnormally.
        """

        self.write_load_config()
        # Start a fresh server and monitor.
        launch_logs = self.check_cmd(['No errors!'])
        self.assertIn('Server launched with the following command', launch_logs)
        self.assertIn('Logs will go to', launch_logs)
        log_file_pattern = re.compile('Logs will go to (.*)')
        monitor_log_match = log_file_pattern.search(launch_logs)
        self.assertIsNotNone(monitor_log_match)
        monitor_log_path = monitor_log_match.group(1)
        self.assertIsNotNone(monitor_log_path)
        with open(monitor_log_path) as f:
            monitor_logs = f.read()
            m = re.search(
                    'Just started typechecker server with pid: ([0-9]+)',
                    monitor_logs)
            self.assertIsNotNone(m)
            pid = m.group(1)
            self.assertIsNotNone(pid)
            os.kill(int(pid), signal.SIGTERM)
            # For some reason, waitpid in the monitor after the kill signal
            # sent above doesn't preserve ordering - maybe because they're
            # in separate processes? Give it some time.
            time.sleep(1)
            client_error = self.check_cmd(['No errors!'])
            self.assertIn('Last server killed by signal', client_error)

    def test_duplicate_parent(self):
        """
        This checks that we handle duplicate parent classes, i.e. when Bar
        extends Foo and there are two declarations of Foo. We want to make sure
        that when the duplicate gets removed, we recover correctly by
        redeclaring Bar with the remaining parent class.
        """
        with open(os.path.join(self.repo_dir, 'foo_4.php'), 'w') as f:
            f.write("""
            <?hh
            class Foo { // also declared in foo_3.php in setUpClass
                public static $x;
            }
            """)
        with open(os.path.join(self.repo_dir, 'foo_5.php'), 'w') as f:
            f.write("""
            <?hh
            class Bar extends Foo {}

            function main(Bar $a) {
                return $a::$y;
            }
            """)
        self.write_load_config('foo_4.php', 'foo_5.php')
        self.check_cmd([
            '{root}foo_4.php:3:19,21: Name already bound: Foo (Naming[2012])',
            '  {root}foo_3.php:7:15,17: Previous definition is here',
            '{root}foo_5.php:6:28,29: Could not find class variable $y in type Bar (Typing[4090])',
            '  {root}foo_5.php:3:19,21: Declaration of Bar is here',
            ])

        os.remove(os.path.join(self.repo_dir, 'foo_4.php'))
        self.check_cmd([
            '{root}foo_5.php:6:28,29: Could not find class variable $y in type Bar (Typing[4090])',
            '  {root}foo_5.php:3:19,21: Declaration of Bar is here',
            ])

        with open(os.path.join(self.repo_dir, 'foo_4.php'), 'w') as f:
            f.write("""
            <?hh
            class Foo {
                public static $y;
            }
            """)
        os.remove(os.path.join(self.repo_dir, 'foo_3.php'))
        self.check_cmd(['No errors!'])
