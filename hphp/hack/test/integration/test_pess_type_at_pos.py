# pyre-unsafe
import common_tests
from test_case import TestCase


class ImplicitPessTests(TestCase[common_tests.CommonTestDriver]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/implicit_pess_repo"

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return common_tests.CommonTestDriver()

    def test_type_at_pos_enum(self) -> None:
        """
        Test hh_client --type-at-pos
        """
        self.test_driver.start_hh_server()

        self.test_driver.check_cmd_and_json_cmd(
            ["(string & ~MyEnum)"],
            [
                '{{"type":"(string & ~MyEnum)",'
                + '"pos":{{"filename":"","line":0,"char_start":0,"char_end":0}},'
                + '"full_type":{{"src_pos":{{"filename":"{root}foo_enum.php","line":8,"char_start":24,"char_end":29}},"kind":"enum","name":"\\\\MyEnum",'
                + '"as":{{"src_pos":{{"filename":"{root}foo_enum.php","line":3,"char_start":24,"char_end":29}},"kind":"primitive","name":"string"}}}}}}'
            ],
            options=["--type-at-pos", "{root}foo_enum.php:15:3"],
        )
