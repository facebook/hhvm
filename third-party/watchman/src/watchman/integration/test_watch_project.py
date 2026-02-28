# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

import pywatchman
from watchman.integration.lib import WatchmanInstance, WatchmanTestCase
from watchman.integration.lib.path_utils import norm_absolute_path, norm_relative_path


def make_empty_watchmanconfig(dir) -> None:
    with open(os.path.join(dir, ".watchmanconfig"), "w") as f:
        f.write("{}")


@WatchmanTestCase.expand_matrix
class TestWatchProject(WatchmanTestCase.WatchmanTestCase):
    def runProjectTests(
        self, config, expect, touch_watchmanconfig: bool = False
    ) -> None:
        with WatchmanInstance.Instance(config=config) as inst:
            inst.start()
            client = self.getClient(inst)

            for touch, expect_watch, expect_rel, expect_pass in expect:
                # encode the test criteria in the dirname so that we can
                # figure out which test scenario failed more easily

                suffix = "-%s-%s-%s-%s" % (touch, expect_watch, expect_rel, expect_pass)
                suffix = suffix.replace("/", "Z")
                d = self.mkdtemp(suffix=suffix)

                dir_to_watch = os.path.join(d, "a", "b", "c")
                os.makedirs(dir_to_watch, 0o777)
                dir_to_watch = norm_absolute_path(dir_to_watch)
                self.touchRelative(d, touch)
                if touch_watchmanconfig:
                    make_empty_watchmanconfig(d)

                if expect_watch:
                    expect_watch = os.path.join(d, expect_watch)
                else:
                    expect_watch = d

                if expect_pass:
                    res = client.query("watch-project", dir_to_watch)

                    self.assertEqual(
                        norm_absolute_path(os.path.join(d, expect_watch)),
                        norm_absolute_path(res["watch"]),
                    )
                    if not expect_rel:
                        self.assertEqual(None, res.get("relative_path"))
                    else:
                        self.assertEqual(
                            norm_relative_path(expect_rel),
                            norm_relative_path(res.get("relative_path")),
                        )
                else:
                    with self.assertRaises(pywatchman.WatchmanError) as ctx:
                        client.query("watch-project", dir_to_watch)
                    self.assertIn(
                        (
                            "None of the files listed in global config "
                            + "root_files are present in path `"
                            + dir_to_watch
                            + "` or any of its parent directories.  "
                            + "root_files is defined by the"
                        ),
                        str(ctx.exception),
                    )

    def test_watchProject(self) -> None:
        expect = [
            ("a/b/c/.git", "a/b/c", None, True),
            ("a/b/.hg", "a/b", "c", True),
            ("a/.foo", "a", "b/c", True),
            (".bar", None, "a/b/c", True),
            ("a/.bar", "a", "b/c", True),
            (".svn", "a/b/c", None, True),
            ("a/baz", "a/b/c", None, True),
        ]
        self.runProjectTests({"root_files": [".git", ".hg", ".foo", ".bar"]}, expect)

    def test_watchProjectWatchmanConfig(self) -> None:
        expect = [
            ("a/b/c/.git", None, "a/b/c", True),
            ("a/b/.hg", None, "a/b/c", True),
            ("a/.foo", None, "a/b/c", True),
            (".bar", None, "a/b/c", True),
            ("a/.bar", None, "a/b/c", True),
            (".svn", None, "a/b/c", True),
            ("a/baz", None, "a/b/c", True),
        ]
        self.runProjectTests(
            {"root_files": [".git", ".hg", ".foo", ".bar"]},
            expect,
            touch_watchmanconfig=True,
        )

    def test_watchProjectEnforcing(self) -> None:
        config = {
            "root_files": [".git", ".hg", ".foo", ".bar"],
            "enforce_root_files": True,
        }
        expect = [
            ("a/b/c/.git", "a/b/c", None, True),
            ("a/b/.hg", "a/b", "c", True),
            ("a/.foo", "a", "b/c", True),
            (".bar", None, "a/b/c", True),
            ("a/.bar", "a", "b/c", True),
            (".svn", None, None, False),
            ("a/baz", None, None, False),
        ]
        self.runProjectTests(config=config, expect=expect)

    def test_reUseNestedWatch(self) -> None:
        d = self.mkdtemp()
        abc = os.path.join(d, "a", "b", "c")
        os.makedirs(abc, 0o777)
        make_empty_watchmanconfig(abc)

        res = self.watchmanCommand("watch-project", d)
        self.assertEqual(d, norm_absolute_path(res["watch"]))

        res = self.watchmanCommand("watch-project", abc)
        self.assertEqual(d, norm_absolute_path(res["watch"]))
        self.assertEqual("a/b/c", norm_relative_path(res["relative_path"]))
