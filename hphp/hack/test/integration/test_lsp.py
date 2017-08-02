from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import common_tests
import json
import os
import unittest
from lspcommand import LspCommandProcessor


class LspTestDriver(common_tests.CommonTestDriver):

    def write_load_config(self, *changed_files):
        # use default .hhconfig in the template repo
        pass

    def assertEqualString(self, first, second, msg=None):
        pass


class TestLsp(LspTestDriver, unittest.TestCase):

    template_repo = common_tests.CommonTests.template_repo
    test_data_root = 'hphp/hack/test/integration/data/lsp_exchanges/'

    def read_file(self, file):
        with open(file, "r") as f:
            return f.read()

    def parse_test_data(self, file, variables):
        test = self.read_file(os.path.join(self.test_data_root, file))
        for variable, value in variables.items():
            test = test.replace('${' + variable + '}', value)
        return test

    def load_test_data(self, test_name, variables):
        test = self.parse_test_data(test_name + '.json', variables)
        expected = self.parse_test_data(test_name + '.expected', variables)
        return (test, expected)

    def generate_expected(self, test_name, observed_transcript):
        file = os.path.join(self.test_data_root, test_name + '.expected')
        text = self.prepare_responses(self.get_received_items(observed_transcript))
        with open(file, "w") as f:
            f.write(text)

    # sorts a list of responses using the 'id' parameter so they can be
    # compared in sequence even if they came back from the server out of sequence.
    # this can happen based on how json rpc is specified to work.
    def sort_responses(self, responses):
        return sorted(responses, key=lambda response: response['id'])

    # dumps an LSP response into a standard json format that can be used for
    # doing precise text comparison in a way that is human readable in the case
    # of there being an error.
    def serialize_responses(self, responses):
        return [json.dumps(response, indent=2) for response in responses]

    # extracts a list of received responses from an LSP communication transcript
    def get_received_items(self, transcript):
        return [entry['received'] for entry in transcript.values() if entry['received']]

    # gets a set of loaded responses ready for validation by sorting them
    # by id and serializing them for precise text comparison
    def prepare_responses(self, responses):
        return self.serialize_responses(self.sort_responses(responses))

    def run_lsp_test(self, test_name, test, expected, generate):
        commands = LspCommandProcessor.parse_commands(test)
        with LspCommandProcessor.create(self.test_env) as lsp:
            observed_transcript = lsp.communicate(commands)

        if not generate:
            expected_items = self.prepare_responses(json.loads(expected))
            observed_items = self.prepare_responses(
                self.get_received_items(observed_transcript)
            )

            # validation checks that the number of items matches and that
            # the responses are exactly identical to what we expect
            self.assertEqual(len(expected_items), len(observed_items))
            for i in range(len(expected_items)):
                self.assertEqual(observed_items[i], expected_items[i])
        else:
            self.generate_expected(test_name, observed_transcript)

    def test_init_shutdown(self):
        self.write_load_config()
        self.check_cmd(['No errors!'])

        variables = {
            'root_path': LspCommandProcessor.path_expand(self.repo_dir)
        }

        test_name = 'initialize_shutdown'
        test, expected = self.load_test_data(test_name, variables)
        self.run_lsp_test(test_name=test_name,
                          test=test,
                          expected=expected,
                          generate=False)
