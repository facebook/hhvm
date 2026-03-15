# pyre-strict
from __future__ import absolute_import, unicode_literals

import os
import textwrap

from hphp.hack.test.integration.common_tests import CommonTestDriver
from hphp.hack.test.integration.hh_paths import hh_client
from hphp.hack.test.integration.test_case import TestCase


class OutlineForAgentsDriver(CommonTestDriver):
    def write_load_config(self, use_saved_state: bool = False) -> None:
        pass

    def outline_for_agents(self, file_name: str) -> str:
        path = os.path.join(self.repo_dir, file_name)
        (stdout, stderr, retcode) = self.proc_call(
            [hh_client, "--outline-for-agents", path, self.repo_dir]
        )
        assert retcode == 0, f"hh_client --outline-for-agents failed: {stderr}"
        return stdout


class TestOutlineForAgents(TestCase["OutlineForAgentsDriver"]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/outline_for_agents_repo"

    @classmethod
    def get_test_driver(cls) -> OutlineForAgentsDriver:
        return OutlineForAgentsDriver()

    def test_outline_for_agents(self) -> None:
        output = self.test_driver.outline_for_agents("outline_test.php")
        expected = textwrap.dedent(
            """\
            function main | lines 3-6
            class Foo | lines 8-19
              const MY_CONST | line 9
              $name | line 10
              function greet | lines 12-14
              function helper | lines 16-18
            enum Color | lines 21-24
              RED
              BLUE
            type MyAlias | line 26
            interface IFoo | lines 28-30
              function doSomething | line 29
            trait MyTrait | lines 32-37
              function traitMethod | lines 34-36
            """
        )
        self.assertEqual(output, expected)
