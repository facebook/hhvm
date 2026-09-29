# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

from __future__ import annotations

from pathlib import Path

from hphp.hack.test.integration.eden_test_driver import EdenTestDriver
from hphp.hack.test.integration.hh_paths import hh_client
from hphp.hack.test.integration.lsp_test_base import LspTestBase, LspTestDriver
from hphp.hack.test.integration.lspcommand import LspCommandProcessor
from hphp.hack.test.integration.lsptestspec import line, LspTestSpec
from hphp.hack.test.integration.utils import Json


class EdenfsIdeWatcherTestDriver(EdenTestDriver, LspTestDriver):
    pass


class EdenfsIdeWatcherTests(LspTestBase):
    @classmethod
    def get_test_driver(cls) -> EdenfsIdeWatcherTestDriver:
        """Combine Eden checkout setup with the LSP test harness."""
        return EdenfsIdeWatcherTestDriver()

    def test_commit_transition_updates_open_buffer_hover(self) -> None:
        """Verify that enabling the watcher keeps IDE results current across commits.

        Commit-induced disk changes must be picked up without VSCode file-change
        notifications. Hover in an open buffer must use up-to-date declarations
        from disk while still respecting the buffer's unsaved contents.
        """
        driver = self.test_driver
        assert isinstance(driver, EdenfsIdeWatcherTestDriver)
        # The existing fixture defines foo() in one file and calls it through a
        # subclass in another. Prepare commits where foo() returns int and string,
        # leaving the checkout on the int-returning commit.
        string_return_revision = self._prepare_return_type_change()

        # Describe the editor session: open the file containing the call with
        # unsaved edits, then check that hover initially reports int.
        spec = self._open_unsaved_derived_buffer()
        spec = self._expect_foo_return_type(spec, "int")
        spec = spec.switch_commit(
            repo_dir=driver.eden_mount_point,
            commit=string_return_revision,
            comment="Change the base-class declaration by switching commits",
        )

        # We send no VSCode file-change notification. Wait for the watcher to
        # detect the checkout and for hover to report string at the same call.
        spec = self._expect_foo_return_type(spec, "string", retry_timeout=30)
        ide_log = self._run_spec_and_read_ide_log(spec, watcher_enabled=True)
        self.assertIn("[ide-eden-watcher] Started commit-transition watcher", ide_log)

    def _prepare_return_type_change(self) -> str:
        """Create two revisions of the existing LSP inheritance fixture.

        incremental_base.php defines BaseClassIncremental.foo(). The other file,
        incremental_derived.php, defines a subclass and calls its inherited foo()
        method. Only the base file changes: foo() returns int in the first commit
        and string in the second.

        Leave the checkout on the int-returning commit and return the
        string-returning revision.
        """
        driver = self.test_driver
        assert isinstance(driver, EdenfsIdeWatcherTestDriver)
        int_return_revision = driver.commitAllChanges()
        base_file = Path(self.repo_file("incremental_base.php"))
        base_file.write_text(
            "<?hh\nclass BaseClassIncremental {\n"
            "  public function foo(): string { return ''; }\n}\n"
        )
        string_return_revision = driver.commitAllChanges(allow_empty=False)
        driver.gotoRev(int_return_revision)
        return string_return_revision

    def _open_unsaved_derived_buffer(self) -> LspTestSpec:
        """Build an LSP spec opening incremental_derived.php with unsaved edits."""
        # Add two blank lines to the editor's copy of the file without saving.
        # The foo() call moves from line 7 on disk to line 9 in the editor
        # (both zero-based), so hover must use the editor's text to find the call.
        derived_file = Path(self.repo_file("incremental_derived.php"))
        unsaved_contents = derived_file.read_text().replace("<?hh", "<?hh\n\n", 1)
        return (
            self.initialize_spec(LspTestSpec(self.id()))
            # These tests check hover freshness, not automatic error publication.
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": derived_file.as_uri(),
                        "languageId": "hack",
                        "version": 1,
                        "text": unsaved_contents,
                    }
                },
            )
        )

    def _expect_foo_return_type(
        self, spec: LspTestSpec, return_type: str, *, retry_timeout: float | None = None
    ) -> LspTestSpec:
        """Append a hover assertion for the base method's type in the unsaved buffer."""
        # LSP positions are zero-based; the unsaved buffer's foo() call is on
        # line 9. Checking its range also guards against using the disk contents.
        expected_hover: Json = {
            "contents": [
                "Defined in `BaseClassIncremental`",
                "---",
                {"language": "hack", "value": f"public function foo(): {return_type}"},
            ],
            "range": {
                "start": {"line": 9, "character": 12},
                "end": {"line": 9, "character": 15},
            },
        }
        return spec.request(
            line=line(),
            method="textDocument/hover",
            params={
                "textDocument": {"uri": self.repo_file_uri("incremental_derived.php")},
                "position": {"line": 9, "character": 14},
            },
            result=expected_hover,
            retry_timeout=retry_timeout,
            powered_by="serverless_ide",
        )

    def _run_spec_and_read_ide_log(
        self, spec: LspTestSpec, *, watcher_enabled: bool
    ) -> str:
        """Run the spec with the chosen watcher setting and return the IDE log.

        Start from a naming-table snapshot of the current checkout and verify
        both the protocol expectations and a clean LSP process exit.
        """
        # Build the naming snapshot before the spec runs its checkout action,
        # so the daemon starts with the int-returning declaration.
        variables = self.write_hhconf_and_naming_table()
        spec = spec.request(
            line=line(), method="shutdown", params={}, result=None
        ).notification(method="exit", params={})

        with LspCommandProcessor.create(
            env=self.test_driver.test_env,
            lsp_args=[
                "--config",
                "ide_fall_back_to_full_index=true",
                "--config",
                f"ide_file_watcher_enabled={str(watcher_enabled).lower()}",
            ],
            repo_dir=self.test_driver.repo_dir,
        ) as processor:
            try:
                _, error_details = spec.run(processor, variables)
                self.assertIsNone(error_details, error_details)
                self.assertEqual(processor.proc.wait(timeout=30), 0)
            finally:
                if processor.proc.poll() is None:
                    processor.proc.kill()
                    processor.proc.wait(timeout=30)

        log_path, _, retcode = self.test_driver.proc_call(
            [hh_client, "--ide-logname", self.test_driver.repo_dir]
        )
        self.assertEqual(retcode, 0)
        return Path(log_path.strip()).read_text()
