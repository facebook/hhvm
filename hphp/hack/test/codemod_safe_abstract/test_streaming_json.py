#!/usr/bin/env python3

# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

from __future__ import annotations

import json
import os
import resource
import subprocess
import tempfile
import unittest
from pathlib import Path


class StreamingJsonTest(unittest.TestCase):
    def setUp(self) -> None:
        self.codemod = os.environ["CODEMOD_SAFE_ABSTRACT"]
        self.temp_dir = tempfile.TemporaryDirectory()
        self.root = Path(self.temp_dir.name)

    def tearDown(self) -> None:
        self.temp_dir.cleanup()

    def run_codemod_file(self, errors_file: Path) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [
                self.codemod,
                "--errors",
                str(errors_file),
                "--root",
                str(self.root),
            ],
            capture_output=True,
            check=False,
            text=True,
        )

    def run_codemod(self, document: object) -> subprocess.CompletedProcess[str]:
        errors_file = self.root / "errors.json"
        with errors_file.open("w", encoding="utf-8") as output:
            json.dump(document, output)
        return self.run_codemod_file(errors_file)

    @staticmethod
    def message(path: Path, code: int, description: str) -> dict[str, object]:
        return {
            "path": str(path),
            "line": 4,
            "start": 3,
            "end": 10,
            "code": code,
            "descr": description,
        }

    def test_streams_many_unrelated_diagnostics(self) -> None:
        errors_file = self.root / "errors.json"
        diagnostic = {
            "message": [
                self.message(self.root / "unused.php", 9999, "unrelated diagnostic")
            ]
        }
        with errors_file.open("w", encoding="utf-8") as output:
            output.write('{"metadata":{"before":true},"errors":[')
            for index in range(1_000_000):
                if index:
                    output.write(",")
                json.dump(diagnostic, output, separators=(",", ":"))
            output.write('],"passed":false}')

        result = self.run_codemod_file(errors_file)
        peak_rss_kib = resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("input_diagnostics=1000000", result.stdout)
        self.assertIn("codemoddable_diagnostics=0", result.stdout)
        self.assertIn("rewrites=0", result.stdout)
        self.assertLess(peak_rss_kib, 384 * 1024)

    def assert_parse_error(self, result: subprocess.CompletedProcess[str]) -> None:
        self.assertEqual(result.returncode, 2, result.stderr)
        self.assertTrue(
            result.stderr.startswith(
                f"invalid hh JSON in {self.root / 'errors.json'}:"
            ),
            result.stderr,
        )
        self.assertNotIn("Fatal error", result.stderr)
        self.assertNotIn("SAFE_ABSTRACT_SUMMARY", result.stdout)
        self.assertNotIn("Rewriting", result.stdout)

    def test_rejects_invalid_envelopes_and_nested_json(self) -> None:
        for document in (
            "",
            "[]",
            "{}",
            '{"errors":null}',
            '{"errors":[],"errors":[]}',
            '{"errors":[]} {}',
            '{"errors":[],"metadata":{"nested":[1,]}}',
            '{"errors":[{"message":[}',
        ):
            with self.subTest(document=document):
                errors_file = self.root / "errors.json"
                errors_file.write_text(document, encoding="utf-8")
                self.assert_parse_error(self.run_codemod_file(errors_file))

    def test_rejects_invalid_diagnostics_before_editing(self) -> None:
        target = self.root / "Target.php"
        original = (
            "<?hh\n\nclass Target {\n"
            "  public static function make(): this {\n"
            "    return new static();\n"
            "  }\n}\n"
        )
        target.write_text(original, encoding="utf-8")
        message = self.message(target, 12026, "Cannot instantiate via `static`.")
        valid = {"message": [message]}
        invalid_diagnostics = [None, {}, {"message": None}, {"message": []}]
        for field in message:
            missing = dict(message)
            del missing[field]
            invalid_diagnostics.append({"message": [missing]})
            invalid_diagnostics.append({"message": [{**message, field: None}]})
        for path in ("", "relative.php", f"{self.root}-sibling/Target.php"):
            invalid_diagnostics.append({"message": [{**message, "path": path}]})
        for code in (12026.5, 1e100, -1e100):
            invalid_diagnostics.append({"message": [{**message, "code": code}]})
        invalid_diagnostics.append(
            {"message": [{**message, "code": 4526}, {"descr": None}]}
        )
        for invalid in invalid_diagnostics:
            with self.subTest(diagnostic=invalid):
                result = self.run_codemod({"errors": [valid, invalid]})
                self.assert_parse_error(result)
                self.assertIn("unexpected diagnostic 2", result.stderr)
                self.assertEqual(target.read_text(encoding="utf-8"), original)

    def test_wide_arrays_and_objects(self) -> None:
        # Exercise both materialized and skipped containers, plus the top-level
        # member loop. Their width must not grow the parser's call stack.
        wide_array = [None] * 100_000
        wide_object = dict.fromkeys(map(str, range(100_000)))
        diagnostic = {
            "message": [self.message(self.root / "unused.php", 9999, "unrelated")],
            "array": wide_array,
            "object": wide_object,
            "numbers": [1.5, 1e100, -1e100],
        }
        result = self.run_codemod(
            {
                **wide_object,
                "errors": [diagnostic],
                "metadata": {"array": wide_array, "object": wide_object},
            }
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("input_diagnostics=1", result.stdout)
        self.assertIn("rewrites=0", result.stdout)

    def test_classifies_call_needs_concrete_receivers_exactly(self) -> None:
        caller = self.root / "Caller.php"
        caller.write_text(
            "<?hh\n\nclass Caller {\n"
            "  <<Some__NeedsConcreteMarker>>\n"
            "  public static function call(): void {\n"
            "    self::make();\n"
            "  }\n}\n",
            encoding="utf-8",
        )
        indirect = [
            {
                "message": [
                    {
                        **self.message(
                            caller,
                            12024,
                            "Dangerous call to `make` (a `<<__NeedsConcrete>>` "
                            f"method) via `{receiver}`.",
                        ),
                        "line": 6,
                    }
                ]
            }
            for receiver in ("self", "parent", "static")
        ]
        direct = {
            "message": [
                self.message(
                    self.root / "missing.php",
                    12024,
                    "Dangerous call to `Trivia::make` (a `<<__NeedsConcrete>>` "
                    "method). It is expecting a concrete receiver but `Trivia` is "
                    "not concrete.",
                )
            ]
        }

        result = self.run_codemod({"errors": [*indirect, direct], "passed": True})

        contents = caller.read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("matching_diagnostics=4", result.stdout)
        self.assertIn("codemoddable_diagnostics=3", result.stdout)
        self.assertIn("unique_targets=1", result.stdout)
        self.assertIn("rewrites=1", result.stdout)
        self.assertIn("<<__NeedsConcrete, Some__NeedsConcreteMarker>>", contents)
        self.assertEqual(contents.count("__NeedsConcrete"), 2)

    def test_deduplicates_and_rewrites_override_target(self) -> None:
        base = self.root / "Base.php"
        child = self.root / "Child.php"
        # Override violations are typing errors (4526).
        code = 4526
        base.write_text(
            "<?hh\n\nclass Base {\n"
            "  public static function make(): this {\n"
            "    return new static();\n"
            "  }\n}\n",
            encoding="utf-8",
        )
        child.write_text("<?hh\n", encoding="utf-8")
        primary = self.message(child, code, "Override is missing an attribute")
        target = self.message(base, code, "Previously defined here")

        diagnostic = {"message": [primary, target]}
        result = self.run_codemod({"errors": [diagnostic, diagnostic], "passed": True})

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("<<__NeedsConcrete>>", base.read_text(encoding="utf-8"))
        self.assertIn("codemoddable_diagnostics=2", result.stdout)
        self.assertIn("unique_targets=1", result.stdout)
        self.assertIn("rewrites=1", result.stdout)

    def test_does_not_duplicate_existing_attribute(self) -> None:
        target = self.root / "Target.php"
        for attributes in (
            "<<__NeedsConcrete>>",
            "<<SomeOtherAttribute, __NeedsConcrete>>",
        ):
            with self.subTest(attributes=attributes):
                original = (
                    "<?hh\n\nclass Target {\n"
                    f"  {attributes}\n"
                    "  public static function make(): this {\n"
                    "    return new static();\n"
                    "  }\n}\n"
                )
                target.write_text(original, encoding="utf-8")
                message = {
                    **self.message(target, 12026, "Cannot instantiate via `static`."),
                    "line": 6,
                }

                result = self.run_codemod({"errors": [{"message": [message]}]})

                self.assertEqual(target.read_text(encoding="utf-8"), original)
                self.assertEqual(result.returncode, 2, result.stderr)
                self.assertIn("rewrites=0", result.stdout)
                self.assertNotIn("SAFE_ABSTRACT_ADD", result.stdout)

    def test_rejects_ambiguous_override_target(self) -> None:
        primary = self.message(
            self.root / "Child.php", 4526, "Override is missing an attribute"
        )
        target = self.message(self.root / "Base.php", 4526, "Previously defined here")

        result = self.run_codemod(
            {"errors": [{"message": [primary, target, target]}], "passed": True}
        )

        self.assert_parse_error(result)
        self.assertIn("expected exactly one override target", result.stderr)

    def test_rejects_missing_override_target(self) -> None:
        primary = self.message(
            self.root / "Child.php", 4526, "Override is missing an attribute"
        )

        result = self.run_codemod({"errors": [{"message": [primary]}], "passed": True})

        self.assert_parse_error(result)
        self.assertIn("expected exactly one override target", result.stderr)

    def test_rejects_truncated_document(self) -> None:
        errors_file = self.root / "errors.json"
        target = self.root / "Target.php"
        original = (
            "<?hh\n\nclass Target {\n"
            "  public static function make(): this {\n"
            "    return new static();\n"
            "  }\n}\n"
        )
        target.write_text(original, encoding="utf-8")
        diagnostic = {
            "message": [self.message(target, 12026, "Cannot instantiate via `static`.")]
        }
        errors_file.write_text('{"errors":[' + json.dumps(diagnostic), encoding="utf-8")

        result = self.run_codemod_file(errors_file)

        self.assert_parse_error(result)
        self.assertIn("invalid hh JSON", result.stderr)
        self.assertEqual(target.read_text(encoding="utf-8"), original)
        self.assertNotIn("SAFE_ABSTRACT_RUN_SUCCESS", result.stdout)

    def test_ignores_direct_class_call(self) -> None:
        direct_call = self.message(
            self.root / "missing.php",
            12024,
            "This call requires a concrete class",
        )

        result = self.run_codemod(
            {"errors": [{"message": [direct_call]}], "passed": True}
        )

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("matching_diagnostics=1", result.stdout)
        self.assertIn("codemoddable_diagnostics=0", result.stdout)


if __name__ == "__main__":
    unittest.main()
