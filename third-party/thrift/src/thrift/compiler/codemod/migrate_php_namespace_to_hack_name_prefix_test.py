# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# pyre-unsafe

import difflib
import filecmp
import os
import shutil
import subprocess
import tempfile
import textwrap
import unittest
from pathlib import Path

import pkg_resources
from xplat.thrift.compiler.codemod.test_utils import read_file, run_binary, write_file


class MigratePhpNamespaceToHackNamePrefixTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None
        self.binary = pkg_resources.resource_filename(__name__, "codemod")
        self.thrift = pkg_resources.resource_filename(__name__, "thrift_bin")
        self.hack_annotation = pkg_resources.resource_filename(
            __name__, "thrift/annotation/hack.thrift"
        )
        os.makedirs("thrift/annotation", exist_ok=True)
        shutil.copyfile(self.hack_annotation, "thrift/annotation/hack.thrift")

    def test_rewrites_named_package_for_mangled_services(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace php foo.php.ns

                package "meta.com/foo"

                const i32 ANSWER = 42;

                struct S {
                  1: string name;
                }

                service MyService {
                  S ping(1: S request);
                }
                """
            ),
        )

        run_binary(self.binary, "foo.thrift", "--mangledsvcs")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                include "thrift/annotation/hack.thrift"

                @hack.NamePrefix{prefix = "foo_php_ns_", apply_to_services = true}
                @hack.LegacyOmitPrefixInNameString
                @hack.ConstantsClass{name = "foo_php_ns_CONSTANTS"}
                package "meta.com/foo"

                namespace hack ""

                const i32 ANSWER = 42;

                struct S {
                  1: string name;
                }

                service MyService {
                  S ping(1: S request);
                }
                """
            ),
        )

    def test_adds_empty_package_for_non_mangled_services(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace php Foo.Bar

                struct S {
                  1: i32 x;
                }

                service MyService {
                  S ping(1: S request);
                }
                """
            ),
        )

        run_binary(self.binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                include "thrift/annotation/hack.thrift"

                @hack.NamePrefix{prefix = "Foo_Bar_"}
                @hack.LegacyAlwaysIncludeNamePrefixInProcessor
                @hack.LegacyOmitPrefixInNameString
                package;

                struct S {
                  1: i32 x;
                }

                service MyService {
                  S ping(1: S request);
                }
                """
            ),
        )

    def test_omits_skip_services_when_no_service(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace php Foo.Bar

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

        run_binary(self.binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                include "thrift/annotation/hack.thrift"

                @hack.NamePrefix{prefix = "Foo_Bar_"}
                @hack.LegacyOmitPrefixInNameString
                package;

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

    def test_keeps_generated_php_identical_for_mangled_services(self):
        original = textwrap.dedent(
            """\
            namespace php foo.php.ns

            package "meta.com/foo"

            const i32 ANSWER = 42;

            struct S {
              1: string name;
            }

            service MyService {
              S ping(1: S request);
            }
            """
        )
        self._assert_generated_php_is_identical(
            original,
            before_gen="hack:mangledsvcs=1",
            after_gen="hack",
            codemod_args=["--mangledsvcs"],
        )

    def test_keeps_generated_php_identical_for_non_mangled_services(self):
        original = textwrap.dedent(
            """\
            namespace php Foo.Bar

            struct S {
              1: string name;
            }

            service MyService {
              S ping(1: S request);
            }
            """
        )
        self._assert_generated_php_is_identical(
            original,
            before_gen="hack",
            after_gen="hack",
            codemod_args=[],
        )

    def test_remove_only_strips_php_namespace_without_annotations(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace php Foo.Bar

                struct S {
                  1: i32 x;
                }

                service MyService {
                  S ping(1: S request);
                }
                """
            ),
        )

        run_binary(self.binary, "foo.thrift", "--remove-only")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                struct S {
                  1: i32 x;
                }

                service MyService {
                  S ping(1: S request);
                }
                """
            ),
        )

    def test_remove_only_is_a_no_op_when_php_namespace_absent(self):
        original = textwrap.dedent(
            """\
            struct S {
              1: i32 x;
            }
            """
        )
        write_file("foo.thrift", original)

        run_binary(self.binary, "foo.thrift", "--remove-only")

        self.assertEqual(read_file("foo.thrift"), original)

    def test_remove_only_strips_php_namespace_even_when_hack_present(self):
        # Unlike full migration, --remove-only doesn't care about existing
        # hack namespace or @hack.NamePrefix; it only strips `namespace php`.
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace php Foo.Bar
                namespace hack ""

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

        run_binary(self.binary, "foo.thrift", "--remove-only")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                namespace hack ""

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

    def test_remove_only_and_mangledsvcs_are_mutually_exclusive(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace php Foo.Bar

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

        with self.assertRaises(subprocess.CalledProcessError):
            run_binary(self.binary, "foo.thrift", "--remove-only", "--mangledsvcs")

    def _assert_generated_php_is_identical(
        self,
        thrift_source,
        *,
        before_gen,
        after_gen,
        codemod_args,
    ):
        write_file("foo.thrift", thrift_source)

        before_dir = Path(self.tmp) / "before"
        self._run_thrift(before_gen, before_dir)
        run_binary(self.binary, "foo.thrift", *codemod_args)
        after_dir = Path(self.tmp) / "after"
        self._run_thrift(after_gen, after_dir)

        self._assert_directory_tree_equal(
            before_dir / "gen-hack",
            after_dir / "gen-hack",
        )

    def _run_thrift(self, gen, output_dir):
        output_dir.mkdir(parents=True, exist_ok=True)
        subprocess.check_call(
            [
                self.thrift,
                "-I",
                self.tmp,
                "-o",
                output_dir,
                "--gen",
                gen,
                "foo.thrift",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    def _assert_directory_tree_equal(self, expected_dir, actual_dir):
        comparison = filecmp.dircmp(expected_dir, actual_dir)
        self.assertEqual(comparison.left_only, [])
        self.assertEqual(comparison.right_only, [])
        self.assertEqual(comparison.funny_files, [])

        for filename in comparison.common_files:
            expected_contents = read_file(os.path.join(expected_dir, filename))
            actual_contents = read_file(os.path.join(actual_dir, filename))
            if expected_contents == actual_contents:
                continue

            diff = "\n".join(
                difflib.unified_diff(
                    expected_contents.splitlines(),
                    actual_contents.splitlines(),
                    fromfile=str(Path(expected_dir) / filename),
                    tofile=str(Path(actual_dir) / filename),
                    lineterm="",
                )
            )
            self.fail(diff)

        for subdir in comparison.common_dirs:
            self._assert_directory_tree_equal(
                Path(expected_dir) / subdir,
                Path(actual_dir) / subdir,
            )
