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
# // Temp test command: buck2 test //xplat/thrift/compiler/codemod:add_operational_annotations_test
# pyre-unsafe

import os
import shutil
import tempfile
import textwrap
import unittest

import pkg_resources
from xplat.thrift.compiler.codemod.test_utils import read_file, run_binary, write_file


class AddOperationalAnnotationsTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def trim(self, s):
        # Strip whitespace from each line and remove trailing newlines.
        return "\n".join([line.strip() for line in s.splitlines()]).rstrip("\n")

    def write_and_test(self, file, content, expected_results):
        write_file(file, textwrap.dedent(content))

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, file)
        actual_results = read_file(file)

        self.assertEqual(
            self.trim(actual_results),
            self.trim(expected_results),
        )

    def test_no_operational_annotation(self):
        # When no operational univerese annotation is present, the codemod should not modify the file.
        self.write_and_test(
            "no_annotation.thrift",
            """\
                struct Foo {
                  1: string name;
                }
                """,
            """\
                struct Foo {
                  1: string name;
                }
                """,
        )

    def test_adding_operational_annotation(self):
        # Test case to add include statements and PPF/OPE category annotations.
        self.write_and_test(
            "single_annotation.thrift",
            """\
                @universe.Universe{id = universe.UniverseIdentifier.OPERATIONAL}
                struct Foo {
                  1: string name;
                }
                """,
            """\
                include "dsi/logger/logger_config_fbcode/zone_policy.thrift"
                include "configerator/structs/capella/types/annotation_types/operational_data/operational_data_annotation.thrift"

                @universe.Universe{id = universe.UniverseIdentifier.OPERATIONAL}
                @zone_policy.PurposePolicy{
                  name = purpose_policy_names.TPurposePolicyName.DEFAULT_PURPOSES_OPERATIONAL,
                  cipp_enforcement_mode = data_access_policy_metadata.CIPPEnforcementMode.NONE,
                }
                @operational_data_annotation.Logger{
                  category = operational_data_annotation.Category.RESTRICTED_DEFAULT,
                }
                struct Foo {
                  1: string name;
                }
                """,
        )

    def test_already_has_operational_annotation(self):
        # Test case to not add OPE annotations if they already exist.
        self.write_and_test(
            "single_annotation.thrift",
            """\
                include "dsi/logger/logger_config_fbcode/zone_policy.thrift"
                include "configerator/structs/capella/types/annotation_types/operational_data/operational_data_annotation.thrift"

                @universe.Universe{id = universe.UniverseIdentifier.OPERATIONAL}
                @zone_policy.PurposePolicy{
                  name = purpose_policy_names.TPurposePolicyName.DEFAULT_PURPOSES_OPERATIONAL,
                  cipp_enforcement_mode = data_access_policy_metadata.CIPPEnforcementMode.NONE,
                }
                @operational_data_annotation.Logger{
                  category = operational_data_annotation.Category.RESTRICTED_DEFAULT,
                }
                struct Foo {
                  1: string name;
                }
                """,
            """\
                include "dsi/logger/logger_config_fbcode/zone_policy.thrift"
                include "configerator/structs/capella/types/annotation_types/operational_data/operational_data_annotation.thrift"

                @universe.Universe{id = universe.UniverseIdentifier.OPERATIONAL}
                @zone_policy.PurposePolicy{
                  name = purpose_policy_names.TPurposePolicyName.DEFAULT_PURPOSES_OPERATIONAL,
                  cipp_enforcement_mode = data_access_policy_metadata.CIPPEnforcementMode.NONE,
                }
                @operational_data_annotation.Logger{
                  category = operational_data_annotation.Category.RESTRICTED_DEFAULT,
                }
                struct Foo {
                  1: string name;
                }
                """,
        )
