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

from __future__ import absolute_import, division, print_function, unicode_literals

import json
import os
import shutil
import subprocess
import tempfile
import unittest


class TestJSONGenerate(unittest.TestCase):
    unsupportedThriftFiles = ["DebugProtoTest"]

    thriftFiles = [
        "ThriftTest",
        "OptionalRequiredTest",
        "ManyTypedefs",
        "EnumTest",
        "DocTest",
        "AnnotationTest",
    ]

    namespaces = {
        "ThriftTest": "thrift.test",
        "OptionalRequiredTest": "thrift.test.optional",
        "DocTest": "thrift.test.doc",
    }

    def setUp(self):
        self.addCleanup(self.cleanup)
        self.temp_dir = tempfile.mkdtemp()

    def cleanup(self):
        if os.path.exists(self.temp_dir):
            shutil.rmtree(self.temp_dir)

    def getGenPath(self, thriftFile):
        output_path = os.path.join(self.temp_dir, "gen-json/")
        output_path += self.namespaces.get(thriftFile, thriftFile).replace(".", "/")
        output_path += ".json"
        return output_path

    def testGen(self):
        thrift_compiler = self.getThriftCompiler()
        for thriftFile in self.thriftFiles + self.unsupportedThriftFiles:
            path = "thrift/test/" + thriftFile + ".thrift"
            self.assertTrue(os.path.exists(path))
            proc = subprocess.Popen(
                [thrift_compiler, "-I", ".", "-gen", "json", "-o", self.temp_dir, path],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
            )
            output = proc.communicate()[0]
            proc.wait()
            self.assertTrue(os.path.exists(self.getGenPath(thriftFile)), output)

        for JSONFile in self.thriftFiles:
            with open(self.getGenPath(JSONFile)) as jsonData:
                json.load(jsonData)

        for JSONFile in self.unsupportedThriftFiles:
            path = os.path.join(self.temp_dir, "gen-json", JSONFile + ".json")
            jsonData = open(path)
            self.assertRaises(TypeError, json.loads, jsonData)

    def getThriftCompiler(self):
        # Use the $THRIFT_COMPILER environment variable if it is set
        # This will normally be set when run via buck.
        path = os.environ.get("THRIFT_COMPILER")
        if path is not None:
            return path

        # Otherwise try to find the compiler location automatically.
        # This makes testing easier when the test program is run manually.
        for build_dir in ("buck-out/gen", "_bin"):
            path = os.path.join(build_dir, "thrift/compiler/thrift")
            if os.access(path, os.X_OK):
                return path

        raise Exception("unable to find the thrift compiler")


if __name__ == "__main__":
    unittest.main()
