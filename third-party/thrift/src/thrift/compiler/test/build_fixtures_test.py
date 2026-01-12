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

import asyncio
import io
import os
import os.path
import tempfile
import unittest
from unittest import mock

from xplat.thrift.compiler.test import build_fixtures


class TestBuildFixtures(unittest.TestCase):
    def setUp(self) -> None:
        self.maxDiff = None

    def test_build_fixtures_help(self) -> None:
        # GIVEN

        # WHEN
        with mock.patch("sys.stdout", new_callable=io.StringIO) as mock_stdout:
            with self.assertRaises(SystemExit):
                asyncio.run(build_fixtures.main(["build_fixtures.par", "--help"]))

            # The parser sees build_fixtures_test.par instead of build_fixtures.par
            # as the script name. Replace it here.
            output = mock_stdout.getvalue()

            # THEN
            self.assertTrue("usage:" in output)

    def test_build_fixtures_incorrect_repo_root_directory(self) -> None:
        # GIVEN
        with tempfile.TemporaryDirectory() as temp_dir:
            not_a_directory = os.path.join(temp_dir, "not_a_directory.txt")
            with open(not_a_directory, "w") as f:
                f.write("not a directory")

            # THEN
            with self.assertRaisesRegex(
                RuntimeError, r"Expected .* to be a directory.*"
            ):
                asyncio.run(
                    build_fixtures.main(
                        [
                            "build_fixtures.par",
                            "--fixture-root",
                            not_a_directory,
                        ]
                    )
                )

    def test_build_fixtures_incorrect_fixtures_directory(self) -> None:
        # GIVEN
        with tempfile.TemporaryDirectory() as temp_dir:
            os.chdir(temp_dir)

            # THEN
            with self.assertRaisesRegex(
                RuntimeError,
                r"Expected .*/thrift/compiler/test/fixtures to be a directory.*",
            ):
                asyncio.run(
                    build_fixtures.main(["build_fixtures.par", "--fixture-root", "."])
                )


if __name__ == "__main__":
    unittest.main()
