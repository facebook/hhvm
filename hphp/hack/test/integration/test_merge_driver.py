# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import json
import os
import shutil
import subprocess
from typing import List, Optional

from common_tests import CommonTestDriver
from test_case import TestCase


class MergeDriverTests(TestCase[CommonTestDriver]):
    @classmethod
    def get_test_driver(cls) -> CommonTestDriver:
        return CommonTestDriver()

    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/repo_with_merge_driver"

    def write_local_conf(self, watchman_subscribe: bool = True) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "hh.conf"), "w") as f:
            watchman_subscribe_str = "watchman_subscribe_v2 = "
            if watchman_subscribe:
                watchman_subscribe_str += "true"
            else:
                watchman_subscribe_str += "false"
            f.write(
                r"""
# some comment
use_mini_state = true
use_watchman = true
lazy_decl = true
lazy_parse = true
lazy_init2 = true
enable_fuzzy_search = false
use_dummy_informant = false
"""
            )
            f.write(watchman_subscribe_str)
            f.write("\n")

    # Write our custom merge driver.
    # In order for the hh_client launched by mercurial to
    # find the correct hh_server (correct in the sense that
    # it's the same hh_server as the one launched within this
    # python test), we need to dump out test_env which includes
    # the HH_TMPDIR.
    def write_merge_driver(self) -> None:
        test_env_json = json.dumps(self.test_driver.test_env)
        with open(
            os.path.join(
                self.test_driver.repo_dir, "scripts", "mergedriver_test_env.json"
            ),
            "w",
        ) as f:
            f.write(test_env_json)

    def setUp(self) -> None:
        shutil.copytree(self.test_driver.template_repo, self.test_driver.repo_dir)
        self.write_merge_driver()

    def tearDown(self) -> None:
        pass

    def write_hgrc(self) -> None:
        with open(os.path.join(self.test_driver.repo_dir, ".hg", "hgrc"), "w") as f:
            f.write(
                r"""
[extensions]
mergedriver =

[experimental]
mergedriver = python:scripts/mergedriver.py
"""
            )

    def check_call(self, cmd: List[str], timeout: Optional[float] = None) -> None:
        if timeout is None:
            subprocess.check_call(
                cmd, cwd=self.test_driver.repo_dir, env=self.test_driver.test_env
            )
        else:
            subprocess.check_call(
                cmd,
                cwd=self.test_driver.repo_dir,
                env=self.test_driver.test_env,
                timeout=timeout,
            )

    def init_hg_repo(self) -> None:
        cmd = ["hg", "init"]
        self.check_call(cmd)
        cmd = ["hg", "add"]
        self.check_call(cmd)
        cmd = ["hg", "commit", "-m", "starting"]
        self.check_call(cmd)
        self.write_hgrc()

    def hg_commit(self, msg: str) -> None:
        cmd = ["hg", "commit", "-m", msg]
        self.check_call(cmd)

    foo_1_start = """\
<?hh // partial
function f() {
    return 1;
}
"""

    foo_1_first_append = r"""
function returns_int() {
    return 5;
}
"""

    foo_1_second_append = r"""
function returns_string() {
    return "hello";
}
"""

    def write_foo_1_and_commit(self, content: str, commit_msg: str) -> None:
        foo_1_path = os.path.join(self.test_driver.repo_dir, "foo_1.php")
        with open(foo_1_path, "w") as f:
            f.write(content)
        self.hg_commit(commit_msg)

    # Merge driver runs Hack build, hack server is already running
    def test_mergedriver_finishes_quickly_hack_already_running(self) -> None:
        self.write_local_conf()
        self.init_hg_repo()
        self.write_foo_1_and_commit(self.foo_1_start, "starting")
        self.write_foo_1_and_commit(
            self.foo_1_start + self.foo_1_first_append, "first append"
        )
        self.write_foo_1_and_commit(
            self.foo_1_start + self.foo_1_first_append + self.foo_1_second_append,
            "second append",
        )
        # Start a Hack server before triggering the mergedriver
        self.test_driver.check_cmd(["No errors!"])
        # Backing out the first append will trigger 3-way merge logic, firing
        # the merge driver, which calls Hack build
        cmd = ["hg", "backout", ".^", "--no-commit"]
        self.check_call(cmd, timeout=20)

    # Hack is launched by the merge driver. Test setup is same as before, except
    # we don't start a Hack server.
    def test_mergedriver_finishes_quickly_hack_not_running(self) -> None:
        self.write_local_conf(watchman_subscribe=True)
        self.init_hg_repo()
        self.write_foo_1_and_commit(self.foo_1_start, "starting")
        self.write_foo_1_and_commit(
            self.foo_1_start + self.foo_1_first_append, "first append"
        )
        self.write_foo_1_and_commit(
            self.foo_1_start + self.foo_1_first_append + self.foo_1_second_append,
            "second append",
        )
        # Backing out the first append will trigger 3-way merge logic, firing
        # the merge driver, which calls Hack build
        cmd = ["hg", "backout", ".^", "--no-commit"]
        # Settling inside the server is allotted up to 3 minutes.
        # Give it another 30 seconds for startup time.
        self.check_call(cmd, timeout=210)
