# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import json

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestCommand(WatchmanTestCase.WatchmanTestCase):
    def test_unknown_commands_print_json_error(self) -> None:
        stdout, stderr = self.watchmanInstance().commandViaCLI(
            ["--pretty", "unknown-command"]
        )
        self.assertEqual(b"", stderr)
        self.assertNotEqual(b"", stdout)
        error = json.loads(stdout)
        self.assertRegex(
            error["error"],
            ".*watchman::CommandValidationError: failed to validate command: unknown command unknown-command",
        )
