from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import json
import os
import re
import select
import shutil
import signal
import subprocess
import sys
import tempfile
import time

from hh_paths import hh_server, hh_client

class CommonTestDriver(object):

    # This needs to be overridden in child classes. The files in this
    # directory will be used to set up the initial environment for each
    # test.
    template_repo: str = None

    @classmethod
    def setUpClass(cls):
        cls.maxDiff = 2000
        cls.base_tmp_dir = tempfile.mkdtemp()
        # we don't create repo_dir using mkdtemp() because we want to create
        # it with copytree(). copytree() will fail if the directory already
        # exists.
        cls.repo_dir = os.path.join(cls.base_tmp_dir, 'repo')
        # Where the hhi files, socket, etc get extracted
        cls.hh_tmp_dir = tempfile.mkdtemp()
        cls.bin_dir = tempfile.mkdtemp()
        hh_server_dir = os.path.dirname(hh_server)
        cls.test_env = dict(os.environ, **{
            'HH_TEST_MODE': '1',
            'HH_TMPDIR': cls.hh_tmp_dir,
            'PATH': '%s:%s:/bin:/usr/bin:/usr/local/bin' %
                (hh_server_dir, cls.bin_dir),
            'HH_HOME': os.path.dirname(hh_client),
            'OCAMLRUNPARAM': 'b',
            'HH_LOCALCONF_PATH': cls.repo_dir,
            })

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.base_tmp_dir)
        shutil.rmtree(cls.bin_dir)
        shutil.rmtree(cls.hh_tmp_dir)

    def write_load_config(self, *changed_files):
        """
        Writes out a script that will print the list of changed files,
        and adds the path to that script to .hhconfig
        """
        raise NotImplementedError()

    def wait_until_server_ready(self):
        """
        We don't want to accidentally connect to an old hh_server, so we wait 2
        seconds for the monitor to start up the new server first.
        """
        time.sleep(2)
        self.run_check()

    def start_hh_server(self, changed_files=None, saved_state_path=None):
        """ Start an hh_server. changed_files is ignored here (as it
        has no meaning) and is only exposed in this API for the derived
        classes.
        """
        if changed_files is None:
            changed_files = []
        cmd = [hh_server, "--daemon", "--max-procs", "2", self.repo_dir]
        self.proc_call(cmd)
        self.wait_until_server_ready()

    def get_server_logs(self):
        time.sleep(2)  # wait for logs to be written
        log_file = self.proc_call([
            hh_client, '--logname', self.repo_dir])[0].strip()
        with open(log_file) as f:
            return f.read()

    def get_monitor_logs(self):
        time.sleep(2)  # wait for logs to be written
        log_file = self.proc_call([
            hh_client, '--monitor-logname', self.repo_dir])[0].strip()
        with open(log_file) as f:
            return f.read()

    def setUp(self):
        shutil.copytree(self.template_repo, self.repo_dir)

    def tearDownWithRetries(self, retries=3):
        (_, _, exit_code) = self.proc_call([
            hh_client,
            'stop',
            self.repo_dir
        ])
        if exit_code == 0:
            shutil.rmtree(self.repo_dir)
            return
        elif retries > 0 and exit_code != 0:
            self.tearDownWithRetries(retries=retries - 1)
        else:
            self.assertEqual(exit_code, 0, msg="Stopping hh_server failed")

    def tearDown(self):
        self.tearDownWithRetries()

    @classmethod
    def proc_create(cls, args, env):
        return subprocess.Popen(
                args,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=dict(cls.test_env, **env),
                universal_newlines=True)

    @classmethod
    def proc_call(cls, args, env=None, stdin=None):
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
    def wait_pid_with_timeout(cls, pid, timeout):
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

    def run_check(self, stdin=None, options=None):
        options = [] if options is None else options
        root = self.repo_dir + os.path.sep
        return self.proc_call(
            [
                hh_client,
                'check',
                '--retries',
                '20',
                self.repo_dir
            ] + list(map(lambda x: x.format(root=root), options)),
            stdin=stdin)

    # Runs `hh_client check` asserting the stdout is equal the expected.
    # Returns stderr.
    # Note: assert_laoded_mini_state is ignored here and only used
    # in some derived classes.
    def check_cmd(
            self,
            expected_output,
            stdin=None,
            options=None,
            assert_loaded_mini_state=False
    ):
        (output, err, retcode) = self.run_check(stdin, options)
        root = self.repo_dir + os.path.sep
        if retcode != 0:
            print("check returned non-zero code: " + str(retcode), file=sys.stderr)
        if expected_output is not None:
            self.assertCountEqual(
                map(lambda x: x.format(root=root), expected_output),
                output.splitlines())
        return err

    def check_cmd_and_json_cmd(
        self,
        expected_output,
        expected_json,
        stdin=None,
        options=None
    ):
        # we run the --json version first because --json --refactor doesn't
        # change any files, but plain --refactor does (i.e. the latter isn't
        # idempotent)
        self.check_cmd(expected_json, stdin, options + ['--json'])
        self.check_cmd(expected_output, stdin, options)

    def subscribe_debug(self):
        proc = self.proc_create([
            hh_client,
            'debug',
            self.repo_dir
            ], env={})
        return DebugSubscription(proc)

    def start_hh_loop_forever_assert_timeout(self):
        # create a file with 10 dependencies. Only "big" jobs, that use
        # workers can be interrupted at the moment.
        with open(os.path.join(self.repo_dir, '__hh_loop_forever_foo.php'), 'w') as f:
            f.write("""<?hh //strict
            function __hh_loop_forever_foo(): int {
              return 4;
            }""")

        for i in range(1, 10):
            with open(
                os.path.join(self.repo_dir, "__hh_loop_forever_bar%d.php" % i),
                "w"
            ) as f:
                f.write("""<?hh //strict
                function __hh_loop_forever_bar%d(): int {
                  return __hh_loop_forever_foo();
                }""" % i)

        self.check_cmd(['No errors!'])

        # trigger rechecking of all 11 files, and make one of them loop
        # until cancelled
        with open(os.path.join(self.repo_dir, '__hh_loop_forever_foo.php'), 'w') as f:
            f.write("""<?hh //strict
            function __hh_loop_forever_foo(): string {
              hh_loop_forever();
            }""")

        # this should timeout due to infinite loop
        try:
            # empty output means no results due to timeout
            self.check_cmd([], options=['--retries', '1'])
        except AssertionError:
            # one of the test drivers doesn't like timeouts
            pass

    def stop_hh_loop_forever(self):
        # subsequent change should interrupt the "loop forever" part
        with open(os.path.join(self.repo_dir, '__hh_loop_forever_foo.php'), 'w') as f:
            f.write("""<?hh //strict
            function __hh_loop_forever_foo(): int {
              return 4;
            }""")

        self.check_cmd(['No errors!'])


class DebugSubscription(object):
    """
    Wraps `hh_client debug`.
    """

    def __init__(self, proc):
        self.proc = proc
        hello = self.read_msg()
        assert(hello['data'] == 'hello')

    def read_msg(self):
        line = self.proc.stdout.readline()
        return json.loads(line)

    def get_incremental_logs(self):
        msgs = {}
        while True:
            msg = self.read_msg()
            if msg['type'] == 'info' and msg['data'] == 'incremental_done':
                break
            msgs[msg['name']] = msg
        return msgs


# The most basic of tests.
# Exercises server responsiveness, and updating errors after changing files
class BarebonesTests(object):

    template_repo = 'hphp/hack/test/integration/data/simple_repo'

    # hh should should work with 0 retries.
    def test_responsiveness(self):
        self.start_hh_server()
        self.check_cmd(['No errors!'])
        self.check_cmd(['No errors!'], options=['--retries', '0'])

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

        self.start_hh_server(changed_files=['foo_4.php'])

        self.check_cmd([
            '{root}foo_4.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_4.php:3:27,29: This is an int',
            '  {root}foo_4.php:4:24,26: It is incompatible with a string',
        ])

    def test_new_naming_error(self):
        """
        Add a new file which contains a naming collisions with an old file
        """
        with open(os.path.join(self.repo_dir, 'foo_4.php'), 'w') as f:
            f.write("""
            <?hh
            class FOO {}
            function H () {}
            """)

        self.start_hh_server(changed_files=['foo_4.php'])

        self.check_cmd([
            '{root}foo_4.php:3:19,21: Could not find FOO (Naming[2006])',
            '  {root}foo_3.php:7:15,17: Did you mean Foo?',
            '{root}foo_4.php:3:19,21: Name already bound: FOO (Naming[2012])',
            '  {root}foo_3.php:7:15,17: Previous definition Foo differs only in capitalization ',
            '{root}foo_4.php:4:22,22: Could not find H (Naming[2006])',
            '  {root}foo_3.php:3:18,18: Did you mean h?',
            '{root}foo_4.php:4:22,22: Name already bound: H (Naming[2012])',
            '  {root}foo_3.php:3:18,18: Previous definition h differs only in capitalization ',
        ])

    # We put this test in Barebones tests so that dependencies on class B
    # show an error (i.e. class_3.php) with both the save state driver
    # and the classic save state driver
    def test_modify_extends_deps(self):
        """
        Introduce a change to a base class that causes an error
        in a use case on one of its subclasses.
        """
        with open(os.path.join(self.repo_dir, 'class_1.php'), 'w') as f:
            f.write("""
                <?hh // strict
                class B {
                  public static function foo () : bool {
                      return true;
                  }
                }
            """)
        self.start_hh_server(changed_files=['class_1.php'])
        self.check_cmd([
            '{root}class_3.php:5:12,19: Invalid return type (Typing[4110])',
            '  {root}class_3.php:4:28,30: This is an int',
            '  {root}class_1.php:4:51,54: It is incompatible with a bool',
        ])


# Common tests, includes the Barebones Tests above
class CommonTests(BarebonesTests):

    def test_json_errors(self):
        """
        If you ask for errors in JSON format, you will get them on standard
        output. Changing this will break the tools that depend on it (like
        editor plugins), and this test is here to remind you about it.
        """
        self.start_hh_server()

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

        self.start_hh_server(changed_files=['foo_2.php'])

        self.check_cmd([
            '{root}foo_2.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_2.php:3:27,29: This is an int',
            '  {root}foo_2.php:4:24,26: It is incompatible with a string',
        ])

    def test_deleted_file(self):
        """
        Delete a file that still has dangling references before restoring from
        a saved state.
        """
        os.remove(os.path.join(self.repo_dir, 'foo_2.php'))

        self.start_hh_server(changed_files=['foo_2.php'])

        self.check_cmd([
            '{root}foo_1.php:4:20,20: Unbound name: g (a global function) (Naming[2049])',
            '{root}foo_1.php:4:20,20: Unbound name: g (a global constant) (Naming[2049])',
            ])

    def test_file_delete_after_load(self):
        """
        Delete a file that still has dangling references after restoring from
        a saved state.
        """
        self.start_hh_server()
        self.check_cmd(['No errors!'])
        debug_sub = self.subscribe_debug()

        os.remove(os.path.join(self.repo_dir, 'foo_2.php'))
        msgs = debug_sub.get_incremental_logs()
        self.assertEqual(msgs['to_redecl_phase1']['files'], ['foo_2.php'])
        self.assertEqual(msgs['to_redecl_phase2']['files'], ['foo_2.php'])
        self.assertEqual(set(msgs['to_recheck']['files']),
                set(['foo_1.php', 'foo_2.php']))
        self.check_cmd([
            '{root}foo_1.php:4:20,20: Unbound name: g (a global function) (Naming[2049])',
            '{root}foo_1.php:4:20,20: Unbound name: g (a global constant) (Naming[2049])',
            ])

    def test_duplicated_file(self):
        self.start_hh_server(changed_files=['foo_2.php'])
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

        self.start_hh_server(
            changed_files=['foo_1.php', 'foo_2.php', 'bar_2.php'],
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

    def test_find_refs(self):
        """
        Test hh_client --find-refs, --find-class-refs
        """
        self.start_hh_server()

        self.check_cmd_and_json_cmd([
            'File "{root}foo_3.php", line 11, characters 13-13: h',
            '1 total results'
            ], [
            '[{{"name":"h","filename":"{root}foo_3.php","line":11,"char_start":13,"char_end":13}}]'
            ], options=['--find-refs', 'h'])

        self.check_cmd_and_json_cmd([
            'File "{root}foo_3.php", line 10, characters 17-19: Foo::__construct',
            '1 total results'
            ], [
            '[{{"name":"Foo::__construct","filename":"{root}foo_3.php","line":10,"char_start":17,"char_end":19}}]'
            ], options=['--find-refs', 'Foo::__construct'])

        self.check_cmd_and_json_cmd([
            'File "{root}foo_3.php", line 10, characters 17-19: Foo',
            '1 total results'
            ], [
            '[{{"name":"Foo","filename":"{root}foo_3.php","line":10,'
            '"char_start":17,"char_end":19}}]'
            ], options=['--find-class-refs', 'Foo'])

    def test_ide_find_refs(self):
        self.start_hh_server()

        self.check_cmd_and_json_cmd(
            [
                'Foo',
                'File "{root}foo_3.php", line 10, characters 17-19:',
                '1 total results'
            ], [
                '[{{"name":"Foo","filename":"{root}foo_3.php",'
                '"line":10,"char_start":17,"char_end":19}}]'
            ],
            options=['--ide-find-refs', '1:20'],
            stdin='<?hh function test(Foo $foo) { new Foo(); }')

    def test_ide_highlight_refs(self):
        self.start_hh_server()

        self.check_cmd_and_json_cmd(
            [
                'line 1, characters 20-22',
                'line 1, characters 36-38',
                '2 total results',
            ], [
                '[{{"line":1,"char_start":20,"char_end":22}},'
                '{{"line":1,"char_start":36,"char_end":38}}]'
            ],
            options=['--ide-highlight-refs', '1:20'],
            stdin='<?hh function test(Foo $foo) { new Foo(); }')

    def test_search(self):
        """
        Test hh_client --search
        """

        self.start_hh_server()

        self.check_cmd_and_json_cmd([
            'File "{root}foo_3.php", line 9, characters 18-40: some_long_function_name, function'
            ], [
            '[{{"name":"some_long_function_name","filename":"{root}foo_3.php","desc":"function","line":9,"char_start":18,"char_end":40,"scope":""}}]'
            ], options=['--search', 'some_lo'])

    def test_search_case_insensitive1(self):
        """
        Test that global search is not case sensitive
        """
        self.maxDiff = None
        self.start_hh_server()

        self.check_cmd([
            'File "{root}foo_4.php", line 4, characters 10-24: '
            'aaaaaaaaaaa_fun, function',
            'File "{root}foo_4.php", line 3, characters 7-23: '
            'Aaaaaaaaaaa_class, class',
        ], options=['--search', 'Aaaaaaaaaaa'])

    def test_search_case_insensitive2(self):
        """
        Test that global search is not case sensitive
        """
        self.start_hh_server()

        self.check_cmd([
            'File "{root}foo_4.php", line 4, characters 10-24: '
            'aaaaaaaaaaa_fun, function',
            'File "{root}foo_4.php", line 3, characters 7-23: '
            'Aaaaaaaaaaa_class, class',
        ], options=['--search', 'aaaaaaaaaaa'])

    def test_auto_complete(self):
        """
        Test hh_client --auto-complete
        """

        self.start_hh_server()

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

    def test_list_files(self):
        """
        Test hh_client --list-files
        """
        os.remove(os.path.join(self.repo_dir, 'foo_2.php'))
        self.start_hh_server(changed_files=['foo_2.php'])
        self.check_cmd_and_json_cmd([
            '{root}foo_1.php',
            ], [
            '{root}foo_1.php',  # see comment for identify-function
            ], options=['--list-files'])

    def test_misc_ide_tools(self):
        """
        Test hh_client --type-at-pos
        """
        self.start_hh_server()

        self.check_cmd_and_json_cmd([
            'string'
            ], [
            '{{"type":"string",' +
            '"pos":{{"filename":"","line":0,"char_start":0,"char_end":0}},' +
            '"full_type":{{"kind":"primitive","name":"string"}}}}'
            ], options=['--type-at-pos', '{root}foo_3.php:11:14'])

    def test_ide_get_definition(self):
        """
        Test hh_client --ide-get-definition
        """
        self.start_hh_server()

        self.check_cmd_and_json_cmd([
            'name: \\bar, kind: function, span: line 1, characters 42-44,'
            ' is_declaration: false',
            'definition:',
            ' bar',
            '   kind: function',
            '   id: function::bar',
            '   position: File "", line 1, characters 15-17:',
            '   span: File "", line 1, character 6 - line 1, character 22:',
            '   modifiers: ',
            '   params:',
            '',
            ''
            ], [
            '[{{"name":"\\\\bar","result_type":"function",'
            '"pos":{{"filename":"","line":1,"char_start":42,"char_end":44}},'
            '"definition_pos":{{"filename":"","line":1,"char_start":15,'
            '"char_end":17}},"definition_span":{{"filename":"","line_start":1,'
            '"char_start":6,"line_end":1,"char_end":22}},'
            '"definition_id":"function::bar"}}]'
            ],
            options=['--ide-get-definition', '1:43'],
            stdin='<?hh function bar() {} function test() { bar() }')

    def test_ide_outline(self):
        """
        Test hh_client --ide-outline
        """
        self.start_hh_server()

        """
        This call is here to ensure that server is running. Outline command
        doesn't require it to be, but integration test suite assumes it and
        checks it's state after each test.
        """
        self.check_cmd(['No errors!'])

        self.check_cmd_and_json_cmd([
            'bar',
            '  kind: function',
            '  id: function::bar',
            '  position: File "", line 1, characters 15-17:',
            '  span: File "", line 1, character 6 - line 1, character 22:',
            '  modifiers: ',
            '  params:',
            '',
            ], [
            '[{{"kind":"function","name":"bar","id":"function::bar",'
            '"position":{{"filename":"",'
            '"line":1,"char_start":15,"char_end":17}},"span":'
            '{{"filename":"","line_start":1,"char_start":6,"line_end":1,'
            '"char_end":22}},"modifiers":[],"params":[]}}]',
            ],
            options=['--ide-outline'],
            stdin='<?hh function bar() {}')

    def test_ide_get_definition_multi_file(self):
        """
        Test hh_client --ide-get-definition when definition we look for is
        in file different from input file
        """
        self.start_hh_server()

        self.check_cmd_and_json_cmd([
            'name: \\ClassToBeIdentified::methodToBeIdentified, kind: method,'
            ' span: line 1, characters 45-64, is_declaration: false',
            'definition:',
            ' methodToBeIdentified',
            '   kind: method',
            '   id: method::ClassToBeIdentified::methodToBeIdentified',
            '   position: File "{root}foo_5.php", line 4, characters 26-45:',
            '   span: File "{root}foo_5.php", line 4, character 3 - line 4,'
            ' character 50:',
            '   modifiers: public static ',
            '   params:',
            '',
            '',
            ], [
            '[{{"name":"\\\\ClassToBeIdentified::methodToBeIdentified",'
            '"result_type":"method","pos":{{"filename":"","line":1,'
            '"char_start":45,"char_end":64}},"definition_pos":'
            '{{"filename":"{root}foo_5.php","line":4,"char_start":26,'
            '"char_end":45}},"definition_span":{{"filename":"{root}foo_5.php",'
            '"line_start":4,"char_start":3,"line_end":4,"char_end":50}},'
            '"definition_id":'
            '"method::ClassToBeIdentified::methodToBeIdentified"}}]'
            ],
            options=['--ide-get-definition', '1:50'],
            stdin='<?hh function test() { '
                  'ClassToBeIdentified::methodToBeIdentified () }')

    def test_format(self):
        """
        Test --format
        """
        self.start_hh_server()
        self.check_cmd_and_json_cmd([
            'function test1(int $x) {{',
            '  $x = $x * x + 3;',
            '  return f($x);',
            '}}'
            ], [
            '{{"error_message":"","result":"function test1(int $x) {{\\n''  $x'
            ' = $x * x + 3;\\n  return f($x);\\n}}\\n","internal_error":false}}'
            ],
            options=['--format', '7', '63'],
            stdin='''<?hh

function test1(int $x) { $x = $x*x + 3; return f($x); }
function test2(int $x) { $x = $x*x + 3; return f($x); }
''')

    def test_abnormal_typechecker_exit_message(self):
        """
        Tests that the monitor outputs a useful message when its typechecker
        exits abnormally.
        """

        self.start_hh_server()
        monitor_logs = self.get_monitor_logs()
        m = re.search(
            'Just started typechecker server with pid: ([0-9]+)',
            monitor_logs
        )
        self.assertIsNotNone(m)
        pid = m.group(1)
        self.assertIsNotNone(pid)
        os.kill(int(pid), signal.SIGTERM)
        # For some reason, waitpid in the monitor after the kill signal
        # sent above doesn't preserve ordering - maybe because they're
        # in separate processes? Give it some time.
        time.sleep(1)
        client_error = self.check_cmd(['No errors!'], assert_loaded_mini_state=False)
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
        self.start_hh_server(changed_files=['foo_4.php', 'foo_5.php'])
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

    def test_refactor_methods(self):
        with open(os.path.join(self.repo_dir, 'foo_4.php'), 'w') as f:
            f.write("""
            <?hh
            class Bar extends Foo {
                public function f() {}
                public function g() {}
            }

            class Baz extends Bar {
                public function g() {
                    $this->f();
                }
            }
            """)
        self.start_hh_server(changed_files=['foo_4.php'])

        self.check_cmd_and_json_cmd(['Rewrote 1 files.'],
                ['[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":86,"char_end":87,"line":4,"col_start":33,'
                '"col_end":33,"patch_type":"replace","replacement":"wat"}},'
                '{{"char_start":248,"char_end":249,"line":10,"col_start":28,'
                '"col_end":28,"patch_type":"replace","replacement":"wat"}}]}}]'],
                options=['--refactor', 'Method', 'Bar::f', 'Bar::wat'])
        self.check_cmd_and_json_cmd(['Rewrote 1 files.'],
                ['[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":127,"char_end":128,"line":5,"col_start":33,'
                '"col_end":33,"patch_type":"replace",'
                '"replacement":"overrideMe"}},{{"char_start":217,'
                '"char_end":218,"line":9,"col_start":33,"col_end":33,'
                '"patch_type":"replace","replacement":"overrideMe"}}]}}]'],
                options=['--refactor', 'Method', 'Bar::g', 'Bar::overrideMe'])
        self.check_cmd_and_json_cmd(['Rewrote 2 files.'],
                ['[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":48,"char_end":51,"line":3,"col_start":31,'
                '"col_end":33,"patch_type":"replace","replacement":"Qux"}}]}},'
                '{{"filename":"{root}foo_3.php","patches":[{{'
                '"char_start":94,"char_end":97,"line":7,"col_start":15,'
                '"col_end":17,"patch_type":"replace","replacement":"Qux"}},'
                '{{"char_start":163,"char_end":166,"line":10,"col_start":17,'
                '"col_end":19,"patch_type":"replace","replacement":"Qux"}}]'
                '}}]'],
                options=['--refactor', 'Class', 'Foo', 'Qux'])

        with open(os.path.join(self.repo_dir, 'foo_4.php')) as f:
            out = f.read()
            self.assertEqual(out, """
            <?hh
            class Bar extends Qux {
                public function wat() {}
                public function overrideMe() {}
            }

            class Baz extends Bar {
                public function overrideMe() {
                    $this->wat();
                }
            }
            """)

        with open(os.path.join(self.repo_dir, 'foo_3.php')) as f:
            out = f.read()
            self.assertEqual(out, """
        <?hh
        function h(): string {
            return "a";
        }

        class Qux {}

        function some_long_function_name() {
            new Qux();
            h();
        }
""")

    def test_refactor_functions(self):
        with open(os.path.join(self.repo_dir, 'foo_4.php'), 'w') as f:
            f.write("""
            <?hh
            function wow() {
                wat();
                return f();
            }

            function wat() {}
            """)
        self.start_hh_server(changed_files=['foo_4.php'])

        self.check_cmd_and_json_cmd(['Rewrote 1 files.'],
                ['[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":134,"char_end":137,"line":8,"col_start":22,'
                '"col_end":24,"patch_type":"replace","replacement":"woah"}},'
                '{{"char_start":63,"char_end":66,"line":4,"col_start":17,'
                '"col_end":19,"patch_type":"replace","replacement":"woah"}}]'
                '}}]'],
                options=['--refactor', 'Function', 'wat', 'woah'])
        self.check_cmd_and_json_cmd(['Rewrote 2 files.'],
                ['[{{"filename":"{root}foo_4.php","patches":[{{'
                '"char_start":94,"char_end":95,"line":5,"col_start":24,'
                '"col_end":24,"patch_type":"replace","replacement":"fff"}}]}},'
                '{{"filename":"{root}foo_1.php","patches":[{{'
                '"char_start":31,"char_end":32,"line":3,"col_start":18,'
                '"col_end":18,"patch_type":"replace","replacement":"fff"}}]'
                '}}]'],
                options=['--refactor', 'Function', 'f', 'fff'])

        with open(os.path.join(self.repo_dir, 'foo_4.php')) as f:
            out = f.read()
            self.assertEqual(out, """
            <?hh
            function wow() {
                woah();
                return fff();
            }

            function woah() {}
            """)

        with open(os.path.join(self.repo_dir, 'foo_1.php')) as f:
            out = f.read()
            self.assertEqual(out, """
        <?hh
        function fff() {
            return g() + 1;
        }
""")

    def test_refactor_typedefs(self):
        with open(os.path.join(self.repo_dir, 'foo_4.php'), 'w') as f:
            f.write("""
            <?hh
            newtype NewType = int;
            type Type = int;

            class MyClass {
                public function myFunc(Type $x): NewType {
                    return $x;
                }
            }
            """)
        self.start_hh_server(changed_files=['foo_4.php'])

        self.check_cmd_and_json_cmd(['Rewrote 1 files.'],
        ['[{{"filename":"{root}foo_4.php","patches":[{{'
        '"char_start":38,"char_end":45,"line":3,"col_start":21,'
        '"col_end":27,"patch_type":"replace","replacement":"NewTypeX"}},'
        '{{"char_start":160,"char_end":167,"line":7,"col_start":50,'
        '"col_end":56,"patch_type":"replace","replacement":"NewTypeX"}}]'
        '}}]'],
        options=['--refactor', 'Class', 'NewType', 'NewTypeX'])

        self.check_cmd_and_json_cmd(['Rewrote 1 files.'],
        ['[{{"filename":"{root}foo_4.php","patches":[{{'
        '"char_start":71,"char_end":75,"line":4,"col_start":18,'
        '"col_end":21,"patch_type":"replace","replacement":"TypeX"}},'
        '{{"char_start":151,"char_end":155,"line":7,"col_start":40,'
        '"col_end":43,"patch_type":"replace","replacement":"TypeX"}}]'
        '}}]'],
        options=['--refactor', 'Class', 'Type', 'TypeX'])

        with open(os.path.join(self.repo_dir, 'foo_4.php')) as f:
            out = f.read()
            self.assertEqual(out, """
            <?hh
            newtype NewTypeX = int;
            type TypeX = int;

            class MyClass {
                public function myFunc(TypeX $x): NewTypeX {
                    return $x;
                }
            }
            """)

    def test_auto_namespace_alias_addition(self):
        """
        Add namespace alias and check if it is still good
        """

        self.start_hh_server()
        self.check_cmd(['No errors!'])

        with open(os.path.join(self.repo_dir, 'auto_ns_2.php'), 'w') as f:
            f.write("""
            <?hh
            function haha() {
                Herp\\f();
                return 1;
            }
            """)

        self.check_cmd(['No errors!'])

    def test_interrupt(self):
        # filesystem interruptions are only triggered by Watchman
        with open(os.path.join(self.repo_dir, '.watchmanconfig'), 'w') as f:
            f.write("\n")
        with open(os.path.join(self.repo_dir, 'hh.conf'), 'a') as f:
            f.write("use_watchman = true\n" +
                    "interrupt_on_watchman = true\n" +
                    "interrupt_on_client = true\n" +
                    "watchman_subscribe_v2 = true\n"
                    )

        self.start_hh_server()
        self.start_hh_loop_forever_assert_timeout()
        self.check_cmd(
            ["string"],
            options=['--type-at-pos', '{root}foo_3.php:11:14']
        )
        self.stop_hh_loop_forever()

    def test_status_single(self):
        """
        Test hh_client check --single
        """
        self.start_hh_server()

        with open(os.path.join(self.repo_dir, 'typing_error.php'), 'w') as f:
            f.write("<?hh //strict\n function aaaa(): int { return h(); }")

        self.check_cmd([
            '{root}typing_error.php:2:32,34: Invalid return type (Typing[4110])',
            '  {root}typing_error.php:2:19,21: This is an int',
            '  {root}foo_3.php:3:23,28: It is incompatible with a string',
        ],
            options=['--single', '{root}typing_error.php'],
            stdin=''
        )

        self.check_cmd([
            ':2:32,34: Invalid return type (Typing[4110])',
            '  :2:19,21: This is an int',
            '  {root}foo_3.php:3:23,28: It is incompatible with a string',
        ],
            options=['--single', '-'],
            stdin='<?hh //strict\n function aaaa(): int { return h(); }'
        )
