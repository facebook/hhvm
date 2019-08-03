# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import copy
import json
import os
import re
import unittest
import urllib.parse
from typing import Iterable, List, Mapping, Tuple

import common_tests
from hh_paths import hh_server
from lspcommand import LspCommandProcessor, Transcript
from lsptestspec import LspTestSpec
from test_case import TestCase
from utils import Json, JsonObject, interpolate_variables


class LspTestDriver(common_tests.CommonTestDriver):
    def write_load_config(self, use_saved_state: bool = False) -> None:
        # Will use the .hhconfig already in the repo directory
        # As for hh.conf, we'll write it explicitly each test.
        # Note that hh.conf uses lower-case...
        use_saved_state_str = "true" if use_saved_state else "false"
        with open(os.path.join(self.repo_dir, "hh.conf"), "w") as f:
            f.write(
                """
use_watchman = true
watchman_subscribe_v2 = true
interrupt_on_watchman = true
interrupt_on_client = true
max_workers = 2
load_state_natively_v4 = {use_saved_state}
use_mini_state = {use_saved_state}
require_mini_state = {use_saved_state}
lazy_decl = {use_saved_state}
lazy_parse = {use_saved_state}
lazy_init2 = {use_saved_state}
""".format(
                    use_saved_state=use_saved_state_str
                )
            )

    def write_naming_table_saved_state(self) -> str:
        naming_table_saved_state_path = os.path.join(
            self.repo_dir, "naming_table_saved_state.sqlite"
        )
        (_, _, retcode) = self.proc_call(
            [
                hh_server,
                "--check",
                self.repo_dir,
                "--save-naming",
                naming_table_saved_state_path,
            ]
        )
        assert retcode == 0, "Failed to save naming table saved state"
        return naming_table_saved_state_path


class TestLsp(TestCase[LspTestDriver]):
    @classmethod
    def get_test_driver(cls) -> LspTestDriver:
        return LspTestDriver()

    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/lsp_exchanges/"

    def repo_file(self, file: str) -> str:
        return os.path.join(self.test_driver.repo_dir, file)

    def read_repo_file(self, file: str) -> str:
        with open(self.repo_file(file), "r") as f:
            return f.read()

    def repo_file_uri(self, file: str) -> str:
        return urllib.parse.urljoin("file://", self.repo_file(file))

    def parse_test_data(self, file: str, variables: Mapping[str, str]) -> Json:
        text = self.read_repo_file(file)
        data: Json = json.loads(text)
        data = interpolate_variables(data, variables)
        return data

    def load_test_data(
        self, test_name: str, variables: Mapping[str, str]
    ) -> Tuple[Json, Json]:
        test = self.parse_test_data(test_name + ".json", variables)
        expected = self.parse_test_data(test_name + ".expected", variables)
        return (test, expected)

    def write_observed(self, test_name: str, observed_transcript: Json) -> None:
        file = os.path.join(self.test_driver.template_repo, test_name + ".observed.log")
        text = json.dumps(
            list(self.get_important_received_items(observed_transcript)), indent=2
        )
        with open(file, "w") as f:
            f.write(text)

    def order_response(self, response: JsonObject) -> str:
        if "id" in response:
            return str(response["id"])
        else:
            return json.dumps(response, indent=2)

    # sorts a list of responses using the 'id' parameter so they can be
    # compared in sequence even if they came back from the server out of sequence.
    # this can happen based on how json rpc is specified to work.
    # if 'id' isn't present the response is a notification.  we sort notifications
    # by their entire text.
    def sort_responses(self, responses: Iterable[JsonObject]) -> List[JsonObject]:
        return sorted(responses, key=lambda response: self.order_response(response))

    # removes stack traces from error responses since these can be noisy
    # as code changes and they contain execution environment specific details
    # by ignoring these when comparing responses we might miss some minor issues
    # but will still catch the core error being thrown or not.
    def sanitize_exceptions(
        self, responses: Iterable[JsonObject]
    ) -> Iterable[JsonObject]:
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
    def serialize_responses(self, responses: Iterable[Json]) -> List[str]:
        return [json.dumps(response, indent=2) for response in responses]

    # generates received responses from an LSP communication transcript
    # ignoring the non-deterministic ones "progress" and "actionRequired"
    def get_important_received_items(self, transcript: Transcript) -> Iterable[Json]:
        for entry in transcript.values():
            received = entry.received or None
            if received is None:
                continue
            method = received.get("method") or ""
            if method in [
                "window/progress",
                "window/actionRequired",
                "window/showStatus",
                "telemetry/event",
            ]:
                continue
            yield received

    # gets a set of loaded responses ready for validation by sorting them
    # by id and serializing them for precise text comparison
    def prepare_responses(self, responses: Iterable[JsonObject]) -> List[str]:
        return self.serialize_responses(
            self.sanitize_exceptions(self.sort_responses(responses))
        )

    def run_lsp_test(
        self,
        test_name: str,
        test: Json,
        expected: Json,
        wait_for_server: bool,
        use_serverless_ide: bool,
    ) -> None:
        if wait_for_server:
            assert not use_serverless_ide, (
                "Warning: both `wait_for_server` and `use_serverless_ide` "
                + "were set to `True` for testing in "
                + self.run_lsp_test.__name__
                + ". "
                + "While this is a possible test case, it hasn't been written yet, "
                + "so it's more likely that this is a mistake "
                + "and you're accidentally relying on hh_server to fulfill "
                + "serverless IDE requests."
                + "(If you're writing that test, "
                + "then it's time to remove this assertion.)"
            )

            # wait until hh_server is ready before starting lsp
            self.test_driver.run_check()
        elif use_serverless_ide:
            self.test_driver.stop_hh_server()

        with LspCommandProcessor.create(
            self.test_driver.test_env, use_serverless_ide=use_serverless_ide
        ) as lsp:
            observed_transcript = lsp.communicate(test)

        self.write_observed(test_name, observed_transcript)

        expected_items = self.prepare_responses(expected)
        observed_items = self.prepare_responses(
            list(self.get_important_received_items(observed_transcript))
        )

        if not use_serverless_ide:
            # If the server's busy, maybe the machine's just under too much
            # pressure to give results in a timely fashion. Doing a retry would
            # only defer the question of what to do in that case, so instead
            # we'll just skip.
            self.throw_on_skip(observed_transcript)

        # validation checks that the number of items matches and that
        # the responses are exactly identical to what we expect
        self.assertEqual(
            len(expected_items),
            len(observed_items),
            "Wrong count. Observed this:\n"
            + json.dumps(observed_transcript, indent=2, separators=(",", ": ")),
        )
        for i in range(len(expected_items)):
            self.assertEqual(expected_items[i], observed_items[i])

    def throw_on_skip(self, transcript: Transcript) -> None:
        failure_messages = ["Server busy", "timed out"]
        for entry in transcript.values():
            received = entry.received
            if received is None:
                continue
            if received.get("error"):
                message = received["error"]["message"]
                for failure_message in failure_messages:
                    if failure_message in message:
                        raise unittest.SkipTest(message)

    def prepare_server_environment(self) -> None:
        self.maxDiff = None
        self.test_driver.write_load_config()
        self.test_driver.start_hh_server()
        (output, err, _) = self.test_driver.run_check()
        if "Error: Ran out of retries" in err:
            raise unittest.SkipTest("Hack server could not be launched")
        self.assertEqual(output.strip(), "No errors!")

    def prepare_serverless_ide_environment(self) -> Mapping[str, str]:
        self.maxDiff = None
        self.test_driver.write_load_config(use_saved_state=False)
        naming_table_saved_state_path = (
            self.test_driver.write_naming_table_saved_state()
        )
        return {"naming_table_saved_state_path": naming_table_saved_state_path}

    def load_and_run(
        self,
        test_name: str,
        variables: Mapping[str, str],
        wait_for_server: bool = True,
        use_serverless_ide: bool = False,
    ) -> None:
        test, expected = self.load_test_data(test_name, variables)
        self.run_lsp_test(
            test_name=test_name,
            test=test,
            expected=expected,
            wait_for_server=wait_for_server,
            use_serverless_ide=use_serverless_ide,
        )

    def run_spec(
        self,
        spec: LspTestSpec,
        variables: Mapping[str, str],
        wait_for_server: bool,
        use_serverless_ide: bool,
    ) -> None:
        if wait_for_server:
            assert not use_serverless_ide, (
                "Warning: both `wait_for_server` and `use_serverless_ide` "
                + "were set to `True` for testing in "
                + self.run_lsp_test.__name__
                + ". "
                + "While this is a possible test case, it hasn't been written yet, "
                + "so it's more likely that this is a mistake "
                + "and you're accidentally relying on hh_server to fulfill "
                + "serverless IDE requests."
                + "(If you're writing that test, "
                + "then it's time to remove this assertion.)"
            )

            # wait until hh_server is ready before starting lsp
            self.test_driver.run_check()
        elif use_serverless_ide:
            self.test_driver.stop_hh_server()

        with LspCommandProcessor.create(
            self.test_driver.test_env, use_serverless_ide=use_serverless_ide
        ) as lsp_command_processor:
            (observed_transcript, error_details) = spec.run(
                lsp_command_processor=lsp_command_processor, variables=variables
            )
        file = os.path.join(self.test_driver.template_repo, spec.name + ".sent.log")
        text = json.dumps(
            [
                sent
                for sent, _received in observed_transcript.values()
                if sent is not None
            ],
            indent=2,
        )
        with open(file, "w") as f:
            f.write(text)

        file = os.path.join(self.test_driver.template_repo, spec.name + ".received.log")
        text = json.dumps(
            [
                received
                for _sent, received in observed_transcript.values()
                if received is not None
            ],
            indent=2,
        )
        with open(file, "w") as f:
            f.write(text)

        if not use_serverless_ide:
            # If the server's busy, maybe the machine's just under too much
            # pressure to give results in a timely fashion. Doing a retry would
            # only defer the question of what to do in that case, so instead
            # we'll just skip.
            self.throw_on_skip(observed_transcript)

        if error_details is not None:
            raise AssertionError(error_details)

    def setup_php_file(self, test_php: str) -> Mapping[str, str]:
        # We want the path to the builtins directory. This is best we can do.
        (output, err, retcode) = self.test_driver.run_check(
            options=["--identify-function", "2:21", "--json"],
            stdin="<?hh // partial\nfunction f():void {PHP_EOL;}\n",
        )
        self.assertEqual(retcode, 0)
        constants_path = json.loads(output)[0]["definition_pos"]["filename"]
        return {
            "hhi_path": re.sub("/constants.hhi$", "", constants_path),
            "root_path": self.test_driver.repo_dir,
            "php_file_uri": self.repo_file_uri(test_php),
            "php_file": self.read_repo_file(test_php),
            # Sometimes Windows happens.
            "path_sep": os.sep,
        }

    def test_init_shutdown(self) -> None:
        self.prepare_server_environment()

        self.load_and_run(
            "initialize_shutdown", {"root_path": self.test_driver.repo_dir}
        )

    def test_completion(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("completion.php")
        self.load_and_run("completion", variables)

    def test_completion_legacy(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("completion.php")
        self.load_and_run("completion_legacy", variables)

    def test_definition(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("definition.php")
        self.load_and_run("definition", variables)

    def test_serverless_ide_definition(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("definition.php"))
        self.test_driver.stop_hh_server()
        self.load_and_run(
            "serverless_ide_definition",
            variables,
            wait_for_server=False,
            use_serverless_ide=True,
        )

    def test_type_definition(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("type_definition.php")
        self.load_and_run("type_definition", variables)

    def test_hover(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("hover.php")
        self.load_and_run("hover", variables)

    def initialize_spec(
        self, spec: LspTestSpec, use_serverless_ide: bool
    ) -> LspTestSpec:
        spec = spec.ignore_notifications(method="telemetry/event").request(
            method="initialize",
            params={
                "initializationOptions": {
                    "namingTableSavedStatePath": "${naming_table_saved_state_path}"
                },
                "processId": None,
                "rootPath": "${root_path}",
                "capabilities": {},
            },
            wait=True,
            result={
                "capabilities": {
                    "textDocumentSync": {
                        "openClose": True,
                        "change": 2,
                        "willSave": False,
                        "willSaveWaitUntil": False,
                        "save": {"includeText": False},
                    },
                    "hoverProvider": True,
                    "completionProvider": {
                        "resolveProvider": True,
                        "triggerCharacters": ["$", ">", "\\", ":", "<", "[", "'", '"'],
                    },
                    "signatureHelpProvider": {"triggerCharacters": ["(", ","]},
                    "definitionProvider": True,
                    "typeDefinitionProvider": True,
                    "referencesProvider": True,
                    "documentHighlightProvider": True,
                    "documentSymbolProvider": True,
                    "workspaceSymbolProvider": True,
                    "codeActionProvider": False,
                    "documentFormattingProvider": True,
                    "documentRangeFormattingProvider": True,
                    "documentOnTypeFormattingProvider": {
                        "firstTriggerCharacter": ";",
                        "moreTriggerCharacter": ["}"],
                    },
                    "renameProvider": True,
                    "typeCoverageProvider": True,
                    "rageProvider": True,
                }
            },
        )
        if use_serverless_ide:
            spec = spec.wait_for_server_request(
                method="client/registerCapability",
                params={
                    "registrations": [
                        {
                            "id": "did-change-watched-files",
                            "method": "workspace/didChangeWatchedFiles",
                            "registerOptions": {
                                "watchers": [{"globPattern": "**", "kind": 7}]
                            },
                        }
                    ]
                },
                result=None,
            )
        return spec

    def test_serverless_ide_hover(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_hover"), use_serverless_ide=True
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                comment="hover over function invocation",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 16},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "int"},
                        "A comment describing b_hover.",
                    ],
                    "range": {
                        "start": {"line": 3, "character": 9},
                        "end": {"line": 3, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(
                comment="hover over whitespace",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 1},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                comment="hover over a keyword",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 2, "character": 1},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                comment="hover over a comment",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 1, "character": 4},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                comment="hover past the end of a line",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 100},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                comment="hover past the end of a file",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 300, "character": 0},
                },
                result=None,
                powered_by="serverless_ide",
                wait=True,
            )
            .notification(
                comment="make local, unsaved change to the file",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}", "version": 2},
                    "contentChanges": [
                        {
                            "text": """\
<?hh  //strict
// comment
function a_hover(): int {
  return b_hover();
}
# Another comment describing b_hover.
function b_hover(): int {
  return 42;
}
"""
                        }
                    ],
                },
            )
            .request(
                comment="another hover over function invocation",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 16},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "int"},
                        "Another comment describing b_hover.",
                    ],
                    "range": {
                        "start": {"line": 3, "character": 9},
                        "end": {"line": 3, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(method="shutdown", params={}, result=None)
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_coverage(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("coverage.php")
        self.load_and_run("coverage", variables)

    def test_highlight(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("highlight.php")
        self.load_and_run("highlight", variables)

    def test_formatting(self) -> None:

        # This test will fail if hackfmt can't be found
        if not self.test_driver.run_hackfmt_check():
            raise unittest.SkipTest("Hackfmt can't be found. Skipping.")

        self.prepare_server_environment()
        variables = self.setup_php_file("messy.php")
        self.load_and_run("formatting", variables)

    def test_ontypeformatting(self) -> None:

        # This test will fail if hackfmt can't be found
        if not self.test_driver.run_hackfmt_check():
            raise unittest.SkipTest("Hackfmt can't be found. Skipping.")

        self.prepare_server_environment()
        variables = self.setup_php_file("ontypeformatting.php")
        self.load_and_run("ontypeformatting", variables)

    def test_did_change(self) -> None:
        # Disabling this test because it has a race condition:
        # see T27194253 for transcript
        # self.prepare_server_environment()
        # variables = self.setup_php_file('didchange.php')
        # self.load_and_run('didchange', variables)
        return

    def test_signature_help(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("signaturehelp.php")
        self.load_and_run("signaturehelp", variables)

    def test_rename(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("rename.php")
        self.load_and_run("rename", variables)

    def test_references(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("references.php")
        self.load_and_run("references", variables)

    def test_non_existing_method(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("nomethod.php")
        self.load_and_run("nomethod", variables)

    def test_bad_call(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("bad_call.php")
        self.load_and_run("bad_call", variables)

    def test_non_blocking(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("non_blocking.php")
        self.test_driver.start_hh_loop_forever_assert_timeout()
        self.load_and_run("non_blocking", variables, wait_for_server=False)
