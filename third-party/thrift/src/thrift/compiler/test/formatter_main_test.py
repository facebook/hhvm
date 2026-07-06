#!/usr/bin/env fbpython
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

import json
import subprocess
import tempfile
import unittest
from pathlib import Path

import pkg_resources


_THRIFT_FMT = pkg_resources.resource_filename(__name__, "thrift-fmt")


class FormatterMainTest(unittest.TestCase):
    def test_lint_json_replaces_invalid_utf8(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            thrift_file = Path(tmp) / "invalid.thrift"
            thrift_file.write_bytes(b"\x80\x81")

            result = subprocess.run(
                [_THRIFT_FMT, "--severity=error", str(thrift_file)],
                check=True,
                stderr=subprocess.PIPE,
                stdout=subprocess.PIPE,
            )

        message = json.loads(result.stdout.decode("utf-8"))
        self.assertEqual(message["path"], str(thrift_file))
        self.assertEqual(message["line"], 1)
        self.assertIsNone(message["char"])
        self.assertEqual(message["code"], "THRIFTFORMAT")
        self.assertEqual(message["severity"], "error")
        self.assertEqual(message["name"], "parser")
        self.assertEqual(message["description"], "unexpected token in input: \ufffd")


if __name__ == "__main__":
    unittest.main()
