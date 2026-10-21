import os
import unittest
from pathlib import Path

from hphp.hack.test.integration.common_tests import CommonTestDriver
from hphp.hack.test.integration.eden_test_driver import EdenTestDriver
from hphp.hack.test.integration.test_case import TestCase


PREFETCH_CONFIG = (
    "use_watchman = false\n"
    "eden_prefetch_enabled = true\n"
    "eden_prefetch_fanout_threshold = 1\n"
)


class PrefetchTestDriver(CommonTestDriver):
    def setUp(self) -> None:
        super().setUp()
        Path(self.repo_dir, "hh.conf").write_text(PREFETCH_CONFIG)


def assert_prefetch_recheck(test: unittest.TestCase, driver: CommonTestDriver) -> None:
    """Check that a real recheck logs prefetch and leaves type-checking healthy."""
    driver.start_hh_server()
    driver.check_cmd(["No errors!"])
    file = Path(driver.repo_dir, "foo_1.php")
    file.write_text(
        file.read_text() + "\nfunction prefetch_recheck_test(): int { return 1; }\n"
    )
    driver.check_cmd(["No errors!"])

    server_log = driver.get_all_logs(driver.repo_dir).current_server_log
    test.assertRegex(
        server_log,
        r"EdenFS prefetch: files=[1-9][0-9]* "
        r"(?:duration=[0-9.]+s )?outcome=(completed|stopped|command_failed) "
        r"cleanup_failed=false",
    )


class PrefetchNonEdenTests(TestCase[PrefetchTestDriver]):
    @classmethod
    def get_test_driver(cls) -> PrefetchTestDriver:
        return PrefetchTestDriver()

    def test_prefetch_in_non_eden_sapling_repo(self) -> None:
        """Prefetch in an ordinary Sapling repo must not break a real recheck."""
        driver = self.test_driver
        _, stderr, exit_code = driver.proc_call(["sl", "init", driver.repo_dir])
        self.assertEqual(exit_code, 0, stderr)
        commit_id, stderr, exit_code = driver.proc_call(
            ["sl", "--cwd", driver.repo_dir, "whereami"]
        )
        self.assertEqual(exit_code, 0, stderr)
        self.assertRegex(commit_id.strip(), r"^[0-9a-f]{40}$")
        self.assertFalse(os.path.lexists(Path(driver.repo_dir, ".eden")))
        assert_prefetch_recheck(self, driver)

    def test_prefetch_outside_sapling_repo(self) -> None:
        """Prefetch errors outside any repository must not break a real recheck."""
        driver = self.test_driver
        _, _, exit_code = driver.proc_call(["sl", "--cwd", driver.repo_dir, "whereami"])
        self.assertNotEqual(exit_code, 0)
        assert_prefetch_recheck(self, driver)


class PrefetchEdenTestDriver(EdenTestDriver, PrefetchTestDriver):
    @classmethod
    def prepare_initial_commit(cls, output_folder: str) -> list[str]:
        Path(output_folder, "hh.conf").write_text(PREFETCH_CONFIG)
        return super().prepare_initial_commit(output_folder) + ["hh.conf"]

    def tearDown(self) -> None:
        self.stop_hh_server()
        super().tearDown()


class PrefetchEdenTests(TestCase[PrefetchEdenTestDriver]):
    @classmethod
    def get_test_driver(cls) -> PrefetchEdenTestDriver:
        return PrefetchEdenTestDriver()

    def test_prefetch_in_eden_mount(self) -> None:
        """Prefetch on an actual Eden mount must not break a real recheck."""
        driver = self.test_driver
        self.assertTrue(os.path.lexists(Path(driver.repo_dir, ".eden")))
        assert_prefetch_recheck(self, driver)
