# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

import pywatchman
from watchman.integration.lib import WatchmanInstance, WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestWatchRestrictions(WatchmanTestCase.WatchmanTestCase):
    def test_rootRestrict(self) -> None:
        config = {"root_restrict_files": [".git", ".foo"]}
        expect = [
            ("directory", ".git", True),
            ("file", ".foo", True),
            ("directory", ".foo", True),
            (None, None, False),
            ("directory", ".svn", False),
            ("file", "baz", False),
        ]
        self.runWatchTests(config=config, expect=expect)

    def runWatchTests(self, config, expect) -> None:
        with WatchmanInstance.Instance(config=config) as inst:
            inst.start()
            client = self.getClient(inst)

            for filetype, name, expect_pass in expect:
                for watch_type in ["watch", "watch-project"]:
                    # encode the test criteria in the dirname so that we can
                    # figure out which test scenario failed more easily
                    d = self.mkdtemp(
                        suffix="-%s-%s-%s-%s"
                        % (filetype, name, expect_pass, watch_type)
                    )
                    if filetype == "directory":
                        os.mkdir(os.path.join(d, name))
                    elif filetype == "file":
                        self.touchRelative(d, name)

                    assert_functions = {
                        (True, "watch"): self.assertWatchSucceeds,
                        (True, "watch-project"): self.assertWatchProjectSucceeds,
                        (False, "watch"): self.assertWatchIsRestricted,
                        (False, "watch-project"): self.assertWatchProjectIsRestricted,
                    }
                    assert_function = assert_functions[(expect_pass, watch_type)]
                    assert_function(inst, client, d)

    def assertWatchSucceeds(self, inst, client, path) -> None:
        client.query("watch", path)

    def assertWatchProjectSucceeds(self, inst, client, path) -> None:
        client.query("watch-project", path)

    def assertWatchIsRestricted(self, inst, client, path) -> None:
        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            client.query("watch", path)
        message = str(ctx.exception)
        self.assertIn("unable to resolve root {0}".format(path), message)
        self.assertIn(
            (
                "Your watchman administrator has configured watchman to "
                + "prevent watching path `{0}`"
            ).format(path),
            message,
        )
        self.assertIn(
            "None of the files listed in global config root_files are present "
            + "and enforce_root_files is set to true.",
            message,
        )
        self.assertIn(
            "root_files is defined by the `{0}` config file".format(inst.cfg_file),
            message,
        )
        self.assertIn(
            "config file and includes `.watchmanconfig`, `.git`, and `.foo`.", message
        )
        self.assertIn(
            "One or more of these files must be present in order to allow a "
            + "watch.  Try pulling and checking out a newer version of the "
            + "project?",
            message,
        )

    def assertWatchProjectIsRestricted(self, inst, client, path) -> None:
        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            client.query("watch-project", path)
        message = str(ctx.exception)
        self.assertIn(
            (
                "None of the files listed in global config root_files are "
                + "present in path `{0}` or any of its parent directories."
            ).format(path),
            message,
        )
        self.assertIn(
            "root_files is defined by the `{0}` config file".format(inst.cfg_file),
            message,
        )
        self.assertIn(
            "config file and includes `.watchmanconfig`, `.git`, and `.foo`.", message
        )
        self.assertIn(
            "One or more of these files must be present in order to allow a "
            + "watch. Try pulling and checking out a newer version of the "
            + "project?",
            message,
        )

    def test_invalidRoot(self) -> None:
        d = self.mkdtemp()
        invalid = os.path.join(d, "invalid")
        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("watch", invalid)
        msg = str(ctx.exception)
        if "No such file or directory" in msg:
            # unix
            return
        if "The system cannot find the file specified" in msg:
            # windows
            return
        self.assertTrue(False, msg)
