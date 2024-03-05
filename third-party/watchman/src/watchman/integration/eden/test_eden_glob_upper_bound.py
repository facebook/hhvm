# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import json
import re
from typing import Any, Dict, List, Optional

from watchman.integration.lib import WatchmanEdenTestCase


def populate(
    repo, threshold: Optional[int] = None, extra_files: Optional[List[str]] = None
) -> None:
    # We ignore ".hg" here just so some of the tests that list files don't have to
    # explicitly filter out the contents of this directory.  However, in most situations
    # the .hg directory normally should not be ignored.
    config: Dict[str, Any] = {"ignore_dirs": [".hg"]}
    if threshold:
        config["eden_file_count_threshold_for_fresh_instance"] = threshold
    config["eden_enable_glob_upper_bounds"] = True
    repo.write_file(".watchmanconfig", json.dumps(config))
    repo.write_file("hello", "hola\n")
    repo.write_file("adir/file", "foo!\n")
    repo.write_file("bdir/test.sh", "#!/bin/bash\necho test\n", mode=0o755)
    repo.write_file("bdir/noexec.sh", "#!/bin/bash\necho test\n")
    repo.symlink("slink", "hello")
    if extra_files:
        for extra_file in extra_files:
            repo.write_file(extra_file, "")
    repo.commit("initial commit.")


class TestEdenGlobUpperBound(WatchmanEdenTestCase.WatchmanEdenTestCase):
    def setUp(self):
        super().setUp()
        self.watchmanCommand("log-level", "debug")

    def test_eden_since_upper_bound(self) -> None:
        root = self.makeEdenMount(populate)
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "allof",
                    ["type", "f"],
                    [
                        "anyof",
                        ["dirname", "adir"],
                        ["match", "bdir/**", "wholename"],
                    ],
                ],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            ["adir/file", "bdir/test.sh", "bdir/noexec.sh"],
        )
        self.assertGlobUpperBound({"adir/**", "bdir/**"})

    def test_eden_trailing_slash(self) -> None:
        root = self.makeEdenMount(populate)
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["allof", ["type", "f"], ["dirname", "adir/"]],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(res["files"], ["adir/file"])
        self.assertGlobUpperBound({"adir/**"})

    def test_eden_since_empty_upper_bound(self) -> None:
        root = self.makeEdenMount(populate)
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "allof",
                    ["type", "f"],
                    ["false"],
                ],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            [],
        )
        self.assertGlobUpperBound([])

    def test_eden_since_upper_bound_case_insensitive(self) -> None:
        root = self.makeEdenMount(
            lambda repo: populate(
                repo, extra_files=["mixedCASE/file1", "MIXEDcase/file2"]
            )
        )
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["idirname", "MixedCase"],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            ["mixedCASE/file1", "MIXEDcase/file2"],
        )
        self.assertGlobUpperBound(
            # We can't bound this query with a glob on a case-sensitive FS.
            (None if self.isCaseSensitiveMount(root) else {"mixedcase/**"})
        )

    def test_eden_since_upper_bound_includedotfiles(self) -> None:
        root = self.makeEdenMount(
            lambda repo: populate(
                repo, extra_files=["dotfiles/.file1", "dotfiles/.file2"]
            )
        )
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "match",
                    "dotfiles/*",
                    "wholename",
                    {"includedotfiles": True},
                ],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            ["dotfiles/.file1", "dotfiles/.file2"],
        )
        self.assertGlobUpperBound({"dotfiles/*"})

    def test_eden_since_upper_bound_name_escaping(self) -> None:
        root = self.makeEdenMount(
            lambda repo: populate(repo, extra_files=["a*b", "[?"])
        )
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "name",
                    ["a*b", "[?"],
                    "wholename",
                ],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            ["a*b", "[?"],
        )
        self.assertGlobUpperBound({r"a\*b", r"\[\?"})

    def test_eden_since_upper_bound_name_normalization(self) -> None:
        root = self.makeEdenMount(lambda repo: populate(repo, extra_files=["a/b"]))
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "name",
                    r"a\b",
                    "wholename",
                ],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            ["a/b"],
        )
        self.assertGlobUpperBound({"a/b"})

    def test_eden_since_upper_bound_name_set_normalization(self) -> None:
        root = self.makeEdenMount(
            lambda repo: populate(repo, extra_files=["a/b", "a/c"])
        )
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "name",
                    [r"a\b", r"a\c"],
                    "wholename",
                ],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            ["a/b", "a/c"],
        )
        self.assertGlobUpperBound({"a/b", "a/c"})

    def test_eden_since_upper_bound_match_noescape(self) -> None:
        root = self.makeEdenMount(
            lambda repo: populate(repo, extra_files=[r"a\b", r"a\c", r"a\?"])
        )
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                # Match a literal 'a\' followed by any character
                "expression": ["match", r"a\?", "wholename", {"noescape": True}],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            [r"a\b", r"a\c", r"a\?"],
        )
        self.assertGlobUpperBound({r"a\\?"})

    def test_eden_since_upper_bound_match_escape(self) -> None:
        root = self.makeEdenMount(
            lambda repo: populate(repo, extra_files=[r"a\b", r"a\c", r"a\?"])
        )
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                # Match a literal 'a\' followed by any character
                "expression": ["match", r"a\\?", "wholename"],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            [r"a\b", r"a\c", r"a\?"],
        )
        self.assertGlobUpperBound({r"a\\?"})

    def test_eden_since_upper_bound_dirname_escaping(self) -> None:
        root = self.makeEdenMount(
            lambda repo: populate(repo, extra_files=["a*b/c", "abbb/d"])
        )
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["dirname", "a*b"],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            ["a*b/c"],
        )
        self.assertGlobUpperBound({r"a\*b/**"})

    def test_eden_since_upper_bound_match_complex_pattern(self) -> None:
        root = self.makeEdenMount(
            lambda repo: populate(
                repo,
                extra_files=[
                    # match
                    "a0c/_/_/_.txt",
                    "abc/_/_.txt",
                    "Abc/_/_.txt",
                    "abc/_/_/_.txt",
                    "acc/_/_/_.txt",
                    # no match
                    "0bc/_/_.txt",
                    "abc/_.txt",
                    "abc/_/_.txz",
                    "abz/_/_.txt",
                ],
            )
        )
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "match",
                    "[[:alpha:]]?[^z]/*/**/*.tx[t]",
                    "wholename",
                ],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            [
                "a0c/_/_/_.txt",
                "abc/_/_.txt",
                "Abc/_/_.txt",
                "abc/_/_/_.txt",
                "acc/_/_/_.txt",
            ],
        )
        self.assertGlobUpperBound({"[[:alpha:]]?[^z]/*/**"})

    def assertGlobUpperBound(self, expected_patterns) -> None:
        if expected_patterns is None:
            pat_summary = re.compile(r".*Did not find a glob upper bound on query\.")
        else:
            pat_summary = re.compile(
                r".*Found "
                + f"{len(set(expected_patterns))}"
                + r" glob pattern\(s\) as upper bound on query\."
            )

        self.assertWaitFor(
            lambda: any(
                pat_summary.match(summary_line)
                for summary_line in self.getServerLogContents()
            ),
            message="logs ready",
            timeout=5,
        )

        pat_one_pattern = re.compile(".*Glob upper bound pattern: (.+)$")

        matches = (
            pat_one_pattern.match(pattern_line)
            for pattern_line in self.getServerLogContents()
        )
        patterns = {match.group(1) for match in filter(None, matches)}

        self.assertSetEqual(
            patterns, set(expected_patterns) if expected_patterns else set()
        )
