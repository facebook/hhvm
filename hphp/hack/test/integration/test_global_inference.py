from __future__ import absolute_import, unicode_literals

from common_tests import CommonTestDriver
from test_case import TestCase


class TestGlobalInference(TestCase[CommonTestDriver]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/global_inference/push_option_out"

    @classmethod
    def get_test_driver(cls) -> CommonTestDriver:
        return CommonTestDriver()

    def test(self) -> None:
        self.test_driver.start_hh_server(
            args=["--config", "infer_missing=global", "--config", "timeout=20"]
        )
        self.test_driver.check_cmd(["No errors!"])
