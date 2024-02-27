# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import json
import os
import shlex
import shutil
import stat
import time
from typing import TextIO

import common_tests
import hierarchy_tests
from hh_paths import hh_client
from saved_state_test_driver import SavedStateTestDriver, SaveStateResult
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
fetch_remote_old_decls = false
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


class SavedStateHierarchyTests(hierarchy_tests.HierarchyTests):
    @classmethod
    def get_test_driver(cls) -> SavedStateTestDriver:
        return SavedStateTestDriver()


class SavedStateTests(TestCase[SavedStateTestDriver]):
    @classmethod
    def get_test_driver(cls) -> SavedStateTestDriver:
        return SavedStateTestDriver()

    def test_watchman_timeout(self) -> None:
        with open(os.path.join(self.test_driver.repo_dir, "hh.conf"), "a") as f:
            f.write(
                r"""
watchman_init_timeout = 1
"""
            )

        with open(os.path.join(self.test_driver.bin_dir, "watchman"), "w") as f:
            f.write(
                r"""#!/bin/sh
sleep 2"""
            )
            os.fchmod(f.fileno(), stat.S_IRWXU)

        self.test_driver.run_check()
        # Stop the server, ensuring that its logs get flushed
        self.test_driver.proc_call([hh_client, "stop", self.test_driver.repo_dir])
        logs = self.test_driver.get_all_logs(self.test_driver.repo_dir)
        self.assertIn("Watchman_sig.Types.Timeout", logs.current_server_log)


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
fetch_remote_old_decls = false
"""
            )


class ReverseNamingTableSavedStateHierarchyTests(hierarchy_tests.HierarchyTests):
    @classmethod
    def get_test_driver(cls) -> ReverseNamingTableFallbackTestDriver:
        return ReverseNamingTableFallbackTestDriver()
