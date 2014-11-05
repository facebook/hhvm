from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import glob
import os
import shutil
import subprocess
import tempfile
import time
import unittest

from utils import touch, write_files, proc_call, relativize_error_paths

def load_server(hh_server, repo_dir, saved_state_path, changed_files=[]):
    """
    Loads hh_server from a saved state archive.

    hh_server: path to hh_server binary
    repo_dir: Repository to run hh_server on
    saved_state_path: Path to file containing saved server state
    changed_files: list of strings
    """
    subprocess.check_call([
        hh_server,
        '-d',
        repo_dir,
        '--load', " ".join([saved_state_path] + changed_files),
    ])
    time.sleep(1)

class TestSaveRestore(unittest.TestCase):
    @classmethod
    def init_class(cls, hh_server, hh_client):
        """
        Should be called before tests are run. This is an awkward hack to allow
        us to pass command-line options to unittests, but Python doesn't
        provide any nicer way.
        """
        cls.hh_server = hh_server
        cls.hh_client = hh_client

    @classmethod
    def setUpClass(cls):
        cls.maxDiff = 2000
        cls.repo_dir = tempfile.mkdtemp()
        cls.saved_state_dir = tempfile.mkdtemp()

        touch(os.path.join(cls.repo_dir, '.hhconfig'))

        cls.files = {}

        cls.files['foo_1.php'] = """
        <?hh
        function f() {
            return g() + 1;
        }
        """

        cls.files['foo_2.php'] = """
        <?hh
        function g(): string {
            return "a";
        }
        """

        cls.files['foo_3.php'] = """
        <?hh
        function h(): string {
            return 1;
        }
        """

        cls.initial_errors = [
            'foo_1.php:4:20,22: Typing error (Typing[4110])',
            '  foo_1.php:4:20,22: This is a num (int/float) because this is used in an arithmetic operation',
            '  foo_2.php:3:23,28: It is incompatible with a string',
            'foo_3.php:4:20,20: Invalid return type (Typing[4110])',
            '  foo_3.php:3:23,28: This is a string',
            '  foo_3.php:4:20,20: It is incompatible with an int',
        ]

        write_files(cls.files, cls.repo_dir)

        subprocess.call([
            cls.hh_server,
            '--check', cls.repo_dir,
            '--save', os.path.join(cls.saved_state_dir, 'foo'),
        ])

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.repo_dir)
        shutil.rmtree(cls.saved_state_dir)

    def setUp(self):
        write_files(self.files, self.repo_dir)

    def tearDown(self):
        subprocess.call([
            self.hh_client,
            'stop',
            self.repo_dir
        ])
        for p in glob.glob(os.path.join(self.repo_dir, '*')):
            os.remove(p)

    def check_errors(self, expected_errors):
        output = proc_call([
            self.hh_client,
            'check',
            '--autostart-server',
            'false',
            self.repo_dir
        ])
        self.assertCountEqual(
            expected_errors,
            relativize_error_paths(output.splitlines()))

    def test_mtime_update(self):
        """
        Update mtimes of files and check that errors remain unchanged.
        """
        load_server(
            self.hh_server,
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'))

        self.check_errors(self.initial_errors)
        touch(os.path.join(self.repo_dir, 'foo_1.php'))
        self.check_errors(self.initial_errors)
        touch(os.path.join(self.repo_dir, 'foo_2.php'))
        self.check_errors(self.initial_errors)

    def test_new_error(self):
        """
        Replace an error in an existing file with a new one.
        """
        with open(os.path.join(self.repo_dir, 'foo_2.php'), 'w') as f:
            f.write("""
            <?hh
            function g(): int {
                return 'a';
            }
            """)

        load_server(
            self.hh_server,
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            [os.path.join(self.repo_dir, 'foo_2.php')]
        )

        self.check_errors([
            'foo_2.php:4:24,26: Invalid return type (Typing[4110])',
            '  foo_2.php:3:27,29: This is an int',
            '  foo_2.php:4:24,26: It is incompatible with a string',
            'foo_3.php:4:20,20: Invalid return type (Typing[4110])',
            '  foo_3.php:3:23,28: This is a string',
            '  foo_3.php:4:20,20: It is incompatible with an int',
        ])

    def test_new_error_after_load(self):
        """
        Replace an error in an existing file with a new one after restoring
        from saved state.
        """

        load_server(
            self.hh_server,
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            [os.path.join(self.repo_dir, 'foo_2.php')]
        )

        with open(os.path.join(self.repo_dir, 'foo_2.php'), 'w') as f:
            f.write("""
            <?hh
            function g(): int {
                return 'a';
            }
            """)

        self.check_errors([
            'foo_2.php:4:24,26: Invalid return type (Typing[4110])',
            '  foo_2.php:3:27,29: This is an int',
            '  foo_2.php:4:24,26: It is incompatible with a string',
            'foo_3.php:4:20,20: Invalid return type (Typing[4110])',
            '  foo_3.php:3:23,28: This is a string',
            '  foo_3.php:4:20,20: It is incompatible with an int',
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

        load_server(
            self.hh_server,
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            [os.path.join(self.repo_dir, 'foo_4.php')]
        )

        self.check_errors(self.initial_errors + [
            'foo_4.php:4:24,26: Invalid return type (Typing[4110])',
            '  foo_4.php:3:27,29: This is an int',
            '  foo_4.php:4:24,26: It is incompatible with a string',
        ])

    def test_deleted_file(self):
        """
        Delete a file containing an error.
        """
        os.remove(os.path.join(self.repo_dir, 'foo_3.php'))

        load_server(
            self.hh_server,
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            [os.path.join(self.repo_dir, 'foo_3.php')]
        )

        self.check_errors([
            'foo_1.php:4:20,22: Typing error (Typing[4110])',
            '  foo_1.php:4:20,22: This is a num (int/float) because this is used in an arithmetic operation',
            '  foo_2.php:3:23,28: It is incompatible with a string',
        ])

    def test_deleted_file_after_load(self):
        """
        Delete a file containing an error after restoring from a saved state.
        """
        load_server(
            self.hh_server,
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            [os.path.join(self.repo_dir, 'foo_3.php')]
        )

        os.remove(os.path.join(self.repo_dir, 'foo_3.php'))

        self.check_errors([
            'foo_1.php:4:20,22: Typing error (Typing[4110])',
            '  foo_1.php:4:20,22: This is a num (int/float) because this is used in an arithmetic operation',
            '  foo_2.php:3:23,28: It is incompatible with a string',
        ])

    def test_moved_files(self):
        """
        Move a file containing errors + a file referenced from an error
        originating in another file.
        """
        os.rename(
            os.path.join(self.repo_dir, 'foo_2.php'),
            os.path.join(self.repo_dir, 'bar_2.php'),
        )
        os.rename(
            os.path.join(self.repo_dir, 'foo_3.php'),
            os.path.join(self.repo_dir, 'bar_3.php'),
        )

        load_server(
            self.hh_server,
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            [os.path.join(self.repo_dir, 'foo_2.php'),
             os.path.join(self.repo_dir, 'bar_2.php'),
             os.path.join(self.repo_dir, 'foo_3.php'),
             os.path.join(self.repo_dir, 'bar_3.php')]
        )

        try:
            self.check_errors([
                'foo_1.php:4:20,22: Typing error (Typing[4110])',
                '  foo_1.php:4:20,22: This is a num (int/float) because this is used in an arithmetic operation',
                '  bar_2.php:3:23,28: It is incompatible with a string',
                'bar_3.php:4:20,20: Invalid return type (Typing[4110])',
                '  bar_3.php:3:23,28: This is a string',
                '  bar_3.php:4:20,20: It is incompatible with an int',
            ])
        except AssertionError:
            # FIXME: figure out why moving a file sometimes duplicates
            # errors. this occurs regardless of whether we are restoring from
            # a saved state or are running from a fresh initialized state,
            # and seems to be due to a race condition
            self.check_errors([
                'foo_1.php:4:20,22: Typing error (Typing[4110])',
                '  foo_1.php:4:20,22: This is a num (int/float) because this is used in an arithmetic operation',
                '  bar_2.php:3:23,28: It is incompatible with a string',
                'bar_3.php:4:20,20: Invalid return type (Typing[4110])',
                '  bar_3.php:3:23,28: This is a string',
                '  bar_3.php:4:20,20: It is incompatible with an int',
                'bar_3.php:4:20,20: Invalid return type (Typing[4110])',
                '  bar_3.php:3:23,28: This is a string',
                '  bar_3.php:4:20,20: It is incompatible with an int',
            ])
