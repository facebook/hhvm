#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

from __future__ import annotations

import io
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from typing import cast, TextIO
from unittest.mock import patch

from hphp.hack.test.milner import verify_runtime as runtime, verify_well_typed as static


def outcome(
    stdout: bytes = b"", stderr: bytes = b"", returncode: int = 0
) -> subprocess.CompletedProcess[bytes]:
    return subprocess.CompletedProcess(
        ["checker", "program.php"], returncode, stdout, stderr
    )


class DiagnosticTests(unittest.TestCase):
    def test_success_marker_cannot_hide_an_expected_error(self) -> None:
        for stdout, stderr in (
            (b"No errors\n", b"error: Typing[4475] overlap\n"),
            (b"error: Typing[4475] overlap\n", b"No errors\n"),
        ):
            self.assertFalse(
                static.matches_expected_output(
                    r"Typing\[4475\]", outcome(stdout, stderr, returncode=2)
                )
            )

    def test_success_requires_successful_exit(self) -> None:
        self.assertTrue(
            static.matches_expected_output("No errors", outcome(b"No errors\n"))
        )
        self.assertFalse(
            static.matches_expected_output(
                "No errors", outcome(b"No errors\n", returncode=2)
            )
        )
        self.assertFalse(static.matches_expected_output("No errors", outcome()))

    def test_matching_error_does_not_hide_another_diagnostic(self) -> None:
        expected = r"Typing\[4475\]"
        overlap = b"error: Typing[4475] Invalid case type declaration\n"
        parser = (
            b"error: Parsing[1002] Type aliases to type constants are not supported\n"
        )
        self.assertTrue(
            static.matches_expected_output(
                expected, outcome(stderr=overlap, returncode=2)
            )
        )
        self.assertFalse(
            static.matches_expected_output(expected, outcome(overlap, parser, 2))
        )
        self.assertFalse(
            static.matches_expected_output(expected, outcome(parser, overlap, 2))
        )

    def test_explicit_alternatives_apply_to_every_diagnostic(self) -> None:
        expected = r"(No errors|Typing\[4475\]|Parsing\[1002\])"
        text = b"error: Typing[4475] overlap\nerror: Parsing[1002] parser\n"
        self.assertTrue(
            static.matches_expected_output(expected, outcome(stderr=text, returncode=2))
        )
        self.assertFalse(
            static.matches_expected_output(
                expected,
                outcome(stderr=text + b"error: Naming[2049] unbound\n", returncode=2),
            )
        )

    def test_legacy_diagnostic_format_is_checked(self) -> None:
        text = b"ERROR: File x.php, line 1:\noverlap (Typing[4475])\nERROR: File x.php, line 2:\nunknown (Naming[2049])\n"
        self.assertFalse(
            static.matches_expected_output(
                r"Typing\[4475\]", outcome(stderr=text, returncode=2)
            )
        )

    def test_signal_and_inconsistent_exit_are_rejected(self) -> None:
        text = b"error: Typing[4475] overlap\n"
        for code in (-11, 1, 137, 0):
            self.assertFalse(
                static.matches_expected_output(
                    r"Typing\[4475\]", outcome(stderr=text, returncode=code)
                )
            )

    def test_incidental_success_text_is_not_success(self) -> None:
        text = b'error: Typing[4110] Invalid argument\n  5 | echo "No errors";\n'
        self.assertFalse(
            static.matches_expected_output(
                "No errors", outcome(stderr=text, returncode=2)
            )
        )

    def test_source_excerpt_does_not_add_diagnostic_codes(self) -> None:
        text = (
            b'error: Typing[4475] overlap\n  5 | echo "Typing[123]"; // (Naming[456])\n'
        )
        self.assertTrue(
            static.matches_expected_output(
                r"Typing\[4475\]", outcome(stderr=text, returncode=2)
            )
        )

    def test_unknown_diagnostic_header_is_rejected(self) -> None:
        for header in (b"error: unknown error", b"ERROR: File x.php, line 1:"):
            self.assertFalse(
                static.matches_expected_output(
                    "No errors", outcome(b"No errors\n", header)
                )
            )

    def test_legacy_expected_error_is_accepted(self) -> None:
        text = b"ERROR: File x.php, line 1:\noverlap (Typing[4475])\n"
        self.assertTrue(
            static.matches_expected_output(
                r"Typing\[4475\]", outcome(stderr=text, returncode=2)
            )
        )

    def test_unexpected_warning_is_not_ignored(self) -> None:
        text = b"warning: Warn[12036] unresolved type\n"
        self.assertFalse(
            static.matches_expected_output("No errors", outcome(b"No errors\n", text))
        )

    def test_real_baseline_mixed_error_is_rejected(self) -> None:
        text = (
            b"error: Parsing[1002] Type aliases to type constants are not supported [1]\n"
            b"[1] 3 | case type Overlap = CTC_3::TC_0 | CTC_3::TC_0;\n"
            b"error: Typing[4475] Invalid case type declaration [1]\n"
            b"2 errors found\n"
        )
        self.assertFalse(
            static.matches_expected_output(
                r"Typing\[4475\]", outcome(stderr=text, returncode=2)
            )
        )

    def test_generation_failure_retains_stderr_and_commands(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            failed = subprocess.CompletedProcess(
                ["milner"], 7, None, b"generator failed"
            )
            with patch.object(static.subprocess, "run", return_value=failed):
                result = static.milner_and_type_check(
                    "milner", "checker", folder, "input.template", "No errors", 3, False
                )
            assert result is not None
            self.assertEqual(result.stderr, b"generator failed")
            self.assertEqual(result.returncode, 7)
            self.assertEqual(result.commands[0][-2:], ["--seed", "3"])
            static.record_failure(result)
            saved = json.loads(Path(result.path + ".failure.json").read_text())
            self.assertEqual(saved["stderr"], "generator failed")

    def test_timeout_is_retained_as_failure(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            expired = subprocess.TimeoutExpired(
                ["milner"], 120, stderr=b"partial stderr"
            )
            with patch.object(static.subprocess, "run", side_effect=expired):
                result = static.milner_and_type_check(
                    "milner", "checker", folder, "input.template", "No errors", 3, False
                )
            assert result is not None
            self.assertIsNone(result.returncode)
            self.assertIn(b"partial stderr", result.stderr)
            self.assertIn(b"timed out", result.stderr)

    def test_checker_timeout_preserves_attempted_command(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            expired = subprocess.TimeoutExpired(
                ["checker"], 7, stderr=b"partial stderr"
            )
            with patch.object(
                static.subprocess, "run", side_effect=[outcome(), expired]
            ) as run:
                result = static.milner_and_type_check(
                    "milner",
                    "checker",
                    folder,
                    "input.template",
                    "No errors",
                    3,
                    False,
                    timeout=7,
                )
            assert result is not None
            self.assertEqual(len(result.commands), 2)
            self.assertEqual(result.commands[-1][0], "checker")
            self.assertIn(b"partial stderr", result.stderr)
            self.assertIsNone(result.returncode)
            self.assertEqual(run.call_args.kwargs["timeout"], 7)

    def test_generator_launch_error_is_retained(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            with patch.object(
                static.subprocess,
                "run",
                side_effect=FileNotFoundError("missing milner"),
            ):
                result = static.milner_and_type_check(
                    "milner", "checker", folder, "input.template", "No errors", 3, False
                )
            assert result is not None
            self.assertIn(b"missing milner", result.stderr)
            self.assertIsNone(result.returncode)
            self.assertTrue(Path(result.path).exists())


class GenerationTests(unittest.TestCase):
    def test_workers_cannot_choose_colliding_output_paths(self) -> None:
        def run(
            command: list[str], **kwargs: object
        ) -> subprocess.CompletedProcess[bytes]:
            cast(TextIO, kwargs["stdout"]).write(command[-1])
            return subprocess.CompletedProcess(command, 0, None, b"")

        with tempfile.TemporaryDirectory() as folder:
            with patch.object(runtime.random, "randint", return_value=42):
                with patch.object(runtime.subprocess, "run", side_effect=run):
                    exit_code, programs = runtime.generate_programs(
                        "milner", folder, "input.template", 32, []
                    )
            self.assertEqual(exit_code, 0)
            self.assertEqual(len({program.seed for program in programs}), 32)
            self.assertEqual(len({program.program_file for program in programs}), 32)
            for program in programs:
                self.assertEqual(
                    Path(program.program_file).read_text(), str(program.seed)
                )

    def test_duplicate_explicit_seed_preserves_the_original(self) -> None:
        with tempfile.TemporaryDirectory() as folder:
            program_file = Path(folder) / "input.template.42.out"
            program_file.write_text("original evidence\n")
            with patch.object(runtime.subprocess, "run") as run:
                result = runtime.generate_program(
                    "milner", folder, "input.template", [], seed=42
                )
            assert isinstance(result, runtime.Failure)
            self.assertIsNone(result.process)
            self.assertIn("File exists", result.error or "")
            self.assertEqual(program_file.read_text(), "original evidence\n")
            run.assert_not_called()

    def test_invalid_sample_sizes_fail_before_generation(self) -> None:
        for size in (-1, 2**31 + 1):
            arguments = [
                "verify_runtime",
                "--template",
                "input.template",
                "--milner-exe",
                "milner",
                "--hhvm-exe",
                "hhvm",
                "--sample-size",
                str(size),
            ]
            with self.subTest(size=size):
                with patch.object(sys, "argv", arguments):
                    with patch.object(sys, "stderr", io.StringIO()):
                        with patch.object(runtime, "verify_runtime") as verify:
                            with self.assertRaises(SystemExit) as error:
                                runtime.main()
                self.assertEqual(error.exception.code, 2)
                verify.assert_not_called()


class RuntimeTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.folder = Path(self.temp.name)
        self.source = self.folder / "bundle.php"

    def tearDown(self) -> None:
        self.temp.cleanup()

    def bundle(self) -> bytes:
        text = b"<?hh\n<<__EntryPoint>>\nfunction main(): void {}\n"
        self.source.write_bytes(text)
        return text

    def test_runtime_failure_sidecar_retains_original_source(self) -> None:
        self.bundle()
        process = subprocess.CompletedProcess(
            ["hhvm", "main.php"], 255, b"output", b"failure"
        )
        failure = runtime.Failure(
            1, process, ["generate", "hhvm main.php"], str(self.source)
        )
        runtime.record_failure(str(self.folder), failure)
        saved = json.loads(Path(str(self.source) + ".failure.json").read_text())
        self.assertEqual(saved["program"], str(self.source))
        self.assertEqual(saved["stderr"], "failure")
        self.assertTrue(self.source.exists())

    def test_hhbbc_failure_does_not_run_the_program(self) -> None:
        self.bundle()
        program = runtime.MilnerSuccess(1, str(self.source), ["generate"])
        failed = subprocess.CompletedProcess(["hhvm", "--hphp"], -11, b"", b"crash")
        with patch.object(runtime.subprocess, "run", return_value=failed) as run:
            result = runtime.run_hhvm_program(
                "hhvm", str(self.folder), ["hhvm", "--hphp"], ["hhvm"], program, "HHBBC"
            )
        assert result is not None
        self.assertEqual(result.process, failed)
        self.assertEqual(run.call_count, 1)

    def test_compile_and_runtime_timeouts_are_retained(self) -> None:
        self.bundle()
        program = runtime.MilnerSuccess(1, str(self.source), ["generate"])
        expired = subprocess.TimeoutExpired(
            ["hhvm"], 7, output=b"partial output", stderr=b"partial error"
        )
        for preceding in ([], [outcome()]):
            with self.subTest(stage="compile" if not preceding else "runtime"):
                with patch.object(
                    runtime.subprocess, "run", side_effect=preceding + [expired]
                ) as run:
                    result = runtime.run_hhvm_program(
                        "hhvm",
                        str(self.folder),
                        ["hhvm", "--hphp"],
                        ["hhvm"],
                        program,
                        "HHBBC",
                        timeout=7,
                    )
                assert result is not None
                self.assertIsNone(result.process)
                self.assertIn("timed out", result.error or "")
                self.assertEqual(len(result.cmds), len(preceding) + 2)
                self.assertEqual(run.call_count, len(preceding) + 1)
                self.assertEqual(run.call_args.kwargs["timeout"], 7)
                runtime.record_failure(str(self.folder), result)
                saved = json.loads(Path(str(self.source) + ".failure.json").read_text())
                self.assertIsNone(saved["returncode"])
                self.assertEqual(saved["stdout"], "partial output")
                self.assertEqual(saved["stderr"], "partial error")

    def test_runtime_launch_error_is_a_failure(self) -> None:
        self.bundle()
        program = runtime.MilnerSuccess(1, str(self.source), ["generate"])
        with patch.object(
            runtime.subprocess, "run", side_effect=FileNotFoundError("missing hhvm")
        ):
            result = runtime.run_hhvm_program(
                "hhvm", str(self.folder), [], [], program, "Sandbox"
            )
        assert result is not None
        self.assertIsNone(result.process)
        self.assertIn("missing hhvm", result.error or "")
        self.assertEqual(len(result.cmds), 2)

    def test_generation_timeout_retains_program_and_error(self) -> None:
        expired = subprocess.TimeoutExpired(["milner"], 7, stderr=b"generation error")
        with patch.object(runtime.subprocess, "run", side_effect=expired) as run:
            result = runtime.generate_program(
                "milner", str(self.folder), "input.template", [], timeout=7
            )
        assert isinstance(result, runtime.Failure)
        self.assertIsNone(result.process)
        self.assertIn("timed out", result.error or "")
        self.assertEqual(result.stderr, b"generation error")
        self.assertTrue(Path(result.program_file).exists())
        self.assertEqual(run.call_args.kwargs["timeout"], 7)


class VirtualFileTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.folder = Path(self.temp.name)
        self.source = self.folder / "bundle.php"

    def tearDown(self) -> None:
        self.temp.cleanup()

    def bundle(self) -> bytes:
        text = (
            b"//// modules.php\n<?hh\nnew module milner {}\n"
            b"//// library.php\n<?hh\nmodule milner;\ninternal function id(int $x): int { return $x; }\n"
            b"//// main.php\n<?hh\nmodule milner;\n<<__EntryPoint>>\nfunction main(): void {}\n"
            b"// Auxiliary definitions\nclass Generated {}\n"
        )
        self.source.write_bytes(text)
        return text

    def test_plain_file_keeps_its_path(self) -> None:
        self.source.write_bytes(b"<?hh\n<<__EntryPoint>>\nfunction main():void{}\n")
        self.assertEqual(
            runtime.prepare_runtime_files(str(self.source)),
            (str(self.source), [str(self.source)]),
        )

    def test_split_preserves_original_and_main_auxiliaries(self) -> None:
        original = self.bundle()
        main, files = runtime.prepare_runtime_files(str(self.source))
        self.assertEqual(self.source.read_bytes(), original)
        self.assertEqual(Path(main).name, "main.php")
        self.assertEqual(
            [Path(path).name for path in files],
            ["modules.php", "library.php", "main.php"],
        )
        self.assertTrue(
            Path(main)
            .read_bytes()
            .endswith(b"// Auxiliary definitions\nclass Generated {}\n")
        )
        self.assertIn(b"module milner;", Path(main).read_bytes())

    def test_escape_and_absolute_paths_are_rejected(self) -> None:
        for path in (
            "../outside.php",
            "/absolute.php",
            "inside/../../outside.php",
            " ",
        ):
            self.source.write_text(
                f"//// {path}\n<?hh\n<<__EntryPoint>>\nfunction main():void{{}}\n"
            )
            with self.assertRaises(ValueError):
                runtime.prepare_runtime_files(str(self.source))

    def test_duplicate_paths_are_rejected(self) -> None:
        self.source.write_text(
            "//// main.php\n<?hh\n//// ./main.php\n<?hh\n<<__EntryPoint>>\nfunction main():void{}\n"
        )
        with self.assertRaises(ValueError):
            runtime.prepare_runtime_files(str(self.source))

    def test_preamble_is_not_silently_discarded(self) -> None:
        self.source.write_text(
            "<?hh\n//// main.php\n<?hh\n<<__EntryPoint>>\nfunction main():void{}\n"
        )
        with self.assertRaises(ValueError):
            runtime.prepare_runtime_files(str(self.source))

    def test_ambiguous_entrypoint_is_rejected(self) -> None:
        self.source.write_text(
            "//// a.php\n<?hh\n<<__EntryPoint>>\nfunction a():void{}\n//// b.php\n<?hh\n<<__EntryPoint>>\nfunction b():void{}\n"
        )
        with self.assertRaises(ValueError):
            runtime.prepare_runtime_files(str(self.source))

    def test_hhbbc_compiles_all_files_and_runs_main(self) -> None:
        self.bundle()
        commands: list[list[str]] = []

        def run(
            command: list[str], **kwargs: object
        ) -> subprocess.CompletedProcess[bytes]:
            commands.append(command)
            return subprocess.CompletedProcess(command, 0, b"", b"")

        program = runtime.MilnerSuccess(1, str(self.source), ["generate"])
        with patch.object(runtime.subprocess, "run", side_effect=run):
            result = runtime.run_hhvm_program(
                "hhvm", str(self.folder), ["hhvm", "--hphp"], ["hhvm"], program, "HHBBC"
            )
        self.assertIsNone(result)
        self.assertEqual(
            [Path(path).name for path in commands[0][-3:]],
            ["modules.php", "library.php", "main.php"],
        )
        self.assertEqual(Path(commands[1][-1]).name, "main.php")
        self.assertNotIn(str(self.source), commands[0])

    def test_sandbox_runs_physical_main(self) -> None:
        self.bundle()
        commands: list[list[str]] = []

        def run(
            command: list[str], **kwargs: object
        ) -> subprocess.CompletedProcess[bytes]:
            commands.append(command)
            return subprocess.CompletedProcess(command, 0, b"", b"")

        program = runtime.MilnerSuccess(1, str(self.source), ["generate"])
        with patch.object(runtime.subprocess, "run", side_effect=run):
            self.assertIsNone(
                runtime.run_hhvm_program(
                    "hhvm", str(self.folder), [], [], program, "Sandbox"
                )
            )
        self.assertEqual(Path(commands[0][-1]).name, "main.php")
        self.assertNotIn(str(self.source), commands[0])

    def test_invalid_bundle_is_a_retained_failure(self) -> None:
        self.source.write_bytes(b"//// ../main.php\n<?hh\n<<__EntryPoint>>\n")
        original = self.source.read_bytes()
        program = runtime.MilnerSuccess(1, str(self.source), ["generate"])
        with patch.object(runtime.subprocess, "run") as run:
            result = runtime.run_hhvm_program(
                "hhvm", str(self.folder), [], [], program, "Sandbox"
            )
        assert result is not None
        self.assertIsNone(result.process)
        self.assertIn("Invalid virtual file", result.error or "")
        run.assert_not_called()
        runtime.record_failure(str(self.folder), result)
        saved = json.loads(Path(str(self.source) + ".failure.json").read_text())
        self.assertIsNone(saved["returncode"])
        self.assertEqual(saved["commands"], ["generate"])
        self.assertEqual(self.source.read_bytes(), original)


if __name__ == "__main__":
    unittest.main()
