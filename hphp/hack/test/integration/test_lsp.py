from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import common_tests
import copy
import json
import os
import unittest
import urllib.parse
from lspcommand import LspCommandProcessor


class LspTestDriver(common_tests.CommonTestDriver):

    def write_load_config(self, *changed_files):
        # use default .hhconfig in the template repo
        pass

    def assertEqualString(self, first, second, msg=None):
        pass


class TestLsp(LspTestDriver, unittest.TestCase):

    template_repo = 'hphp/hack/test/integration/data/lsp_exchanges/'

    def repo_file(self, file):
        return os.path.join(self.repo_dir, file)

    def read_repo_file(self, file):
        with open(self.repo_file(file), "r") as f:
            return f.read()

    def repo_file_uri(self, file):
        return urllib.parse.urljoin('file://', self.repo_file(file))

    def parse_test_data(self, file, variables):
        text = self.read_repo_file(file)
        data = json.loads(text)
        for variable, value in variables.items():
            data = self.replace_variable(data, variable, value)
        return data

    def replace_variable(self, json, variable, text):
        if isinstance(json, dict):
            return {k:
                    self.replace_variable(v, variable, text) for k, v in json.items()}
        elif isinstance(json, list):
            return [self.replace_variable(i, variable, text) for i in json]
        elif isinstance(json, str):
            return json.replace('${' + variable + '}', text)
        else:
            return json

    def load_test_data(self, test_name, variables):
        test = self.parse_test_data(test_name + '.json', variables)
        expected = self.parse_test_data(test_name + '.expected', variables)
        return (test, expected)

    def write_observed(self, test_name, observed_transcript):
        file = os.path.join(self.template_repo, test_name + '.observed.log')
        text = json.dumps(list(
            self.get_important_received_items(observed_transcript)), indent=2)
        with open(file, "w") as f:
            f.write(text)

    # sorts a list of responses using the 'id' parameter so they can be
    # compared in sequence even if they came back from the server out of sequence.
    # this can happen based on how json rpc is specified to work.
    # if 'id' isn't present the response is a notification.  we sort notifications
    # by their entire text.
    def order_response(self, response):
        if 'id' in response:
            return str(response['id'])
        else:
            return json.dumps(response, indent=2)

    def sort_responses(self, responses):
        return sorted(responses, key=lambda response: self.order_response(response))

    # removes stack traces from error responses since these can be noisy
    # as code changes and they contain execution environment specific details
    # by ignoring these when comparing responses we might miss some minor issues
    # but will still catch the core error being thrown or not.
    def sanitize_exceptions(self, responses):
        sanitized = copy.deepcopy(responses)
        for response in sanitized:
            if "error" in response:
                if "data" in response["error"]:
                    if "stack" in response["error"]["data"]:
                        del response["error"]["data"]["stack"]
        return sanitized

    # dumps an LSP response into a standard json format that can be used for
    # doing precise text comparison in a way that is human readable in the case
    # of there being an error.
    def serialize_responses(self, responses):
        return [json.dumps(response, indent=2) for response in responses]

    # generates received responses from an LSP communication transcript
    # ignoring the non-deterministic ones "progress" and "actionRequired"
    def get_important_received_items(self, transcript):
        for entry in transcript.values():
            received = entry.get("received") or None
            if received is None:
                continue
            method = received.get("method") or ""
            if method == "window/progress" or method == "window/actionRequired":
                continue
            yield received

    # gets a set of loaded responses ready for validation by sorting them
    # by id and serializing them for precise text comparison
    def prepare_responses(self, responses):
        return self.serialize_responses(
            self.sanitize_exceptions(self.sort_responses(responses))
        )

    def run_lsp_test(self, test_name, test, expected):
        self.run_check()  # wait until hh_server is ready before starting lsp
        with LspCommandProcessor.create(self.test_env) as lsp:
            observed_transcript = lsp.communicate(test)

        self.write_observed(test_name, observed_transcript)

        expected_items = self.prepare_responses(expected)
        observed_items = self.prepare_responses(
            list(self.get_important_received_items(observed_transcript))
        )

        # If the server's busy, maybe the machine's just under too much pressure
        # to give results in a timely fashion. Doing a retry would only defer
        # the question of what to do in that case, so instead we'll just skip.
        if "'message': 'Server busy'" in str(observed_transcript):
            raise unittest.SkipTest('Hack server busy')
            return

        # validation checks that the number of items matches and that
        # the responses are exactly identical to what we expect
        self.assertEqual(len(expected_items), len(observed_items),
                         'Wrong count. Observed this:\n' +
                         json.dumps(observed_transcript, indent=2,
                                    separators=(',', ': ')))
        for i in range(len(expected_items)):
            self.assertEqual(observed_items[i], expected_items[i])

    def prepare_environment(self):
        self.write_load_config()
        (output, err, _) = self.run_check()
        if 'Error: Ran out of retries' in err:
            raise unittest.SkipTest('Hack server could not be launched')
        self.assertEqual(output.strip(), 'No errors!')

    def load_and_run(self, test_name, variables):
        test, expected = self.load_test_data(test_name, variables)
        self.run_lsp_test(test_name=test_name,
                          test=test,
                          expected=expected)

    def setup_php_file(self, test_php):
        return {
            'root_path': self.repo_dir,
            'php_file_uri': self.repo_file_uri(test_php),
            'php_file': self.read_repo_file(test_php),
        }

    def test_init_shutdown(self):
        self.prepare_environment()

        self.load_and_run('initialize_shutdown',
                          {'root_path': self.repo_dir})

    def test_definition(self):
        self.prepare_environment()
        variables = self.setup_php_file('definition.php')
        self.load_and_run('definition', variables)

    def test_hover(self):
        self.prepare_environment()
        variables = self.setup_php_file('hover.php')
        self.load_and_run('hover', variables)

    def test_highlight(self):
        self.prepare_environment()
        variables = self.setup_php_file('highlight.php')
        self.load_and_run('highlight', variables)

    def test_formatting(self):
        self.prepare_environment()
        variables = self.setup_php_file('messy.php')
        self.load_and_run('formatting', variables)

    def test_did_change(self):
        self.prepare_environment()
        variables = self.setup_php_file('didchange.php')
        self.load_and_run('didchange', variables)

    def test_non_existing_method(self):
        self.prepare_environment()
        variables = self.setup_php_file('nomethod.php')
        self.load_and_run('nomethod', variables)

    def test_bad_call(self):
        self.prepare_environment()
        variables = self.setup_php_file('bad_call.php')
        self.load_and_run('bad_call', variables)
