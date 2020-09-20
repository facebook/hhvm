# pyre-strict
# flake8: noqa: B950

from __future__ import absolute_import, division, print_function, unicode_literals

import copy
import enum
import json
import os
import re
import unittest
import urllib.parse
from typing import Iterable, List, Mapping, Tuple

import common_tests
from hh_paths import hh_server
from lspcommand import LspCommandProcessor, Transcript
from lsptestspec import LspTestSpec, NoResponse, line
from test_case import TestCase
from utils import Json, JsonObject, interpolate_variables


class InsertTextFormat(enum.Enum):
    PlainText = 1
    Snippet = 2


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
symbolindex_search_provider = SqliteIndex
""".format(
                    use_saved_state=use_saved_state_str
                )
            )

    def write_naming_table_saved_state(self) -> str:
        naming_table_saved_state_path = os.path.join(
            self.repo_dir, "naming_table_saved_state.sqlite"
        )
        (stdout, stderr, retcode) = self.proc_call(
            [
                hh_server,
                "--check",
                self.repo_dir,
                "--save-naming",
                naming_table_saved_state_path,
            ]
        )
        assert retcode == 0, (
            f"Failed to save naming table saved state: {retcode}\n"
            + f"STDOUT:\n{stdout}\n"
            + f"STDERR:\n{stderr}\n"
        )
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

    # pyre-fixme[11]: Annotation `Json` is not defined as a type.
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

    # pyre-fixme[11]: Annotation `JsonObject` is not defined as a type.
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
                    if "current_stack" in response["error"]["data"]:
                        del response["error"]["data"]["current_stack"]
                    if "server_finale_stack" in response["error"]["data"]:
                        del response["error"]["data"]["server_finale_stack"]
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
        if retcode == 7:
            self.skipTest(
                "Could not discover builtins directory -- "
                + "got exit code 7 (either Out_of_time or Out_of_retries). "
                + "The test machine is likely under too much load."
            )
        self.assertEqual(retcode, 0)
        constants_path = json.loads(output)[0]["definition_pos"]["filename"]
        return {
            "hhi_path": re.sub("/constants.hhi$", "", constants_path),
            "root_path": self.test_driver.repo_dir,
            "php_file_uri": self.repo_file_uri(test_php),
            "php_file": self.read_repo_file(test_php),
        }

    def test_init_shutdown(self) -> None:
        self.prepare_server_environment()

        self.load_and_run(
            "initialize_shutdown", {"root_path": self.test_driver.repo_dir}
        )

    def test_serverless_ide_completion(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("completion.php"))
        self.test_driver.stop_hh_server()
        spec = (
            self.initialize_spec(LspTestSpec("ide_completion"), use_serverless_ide=True)
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
            .notification(
                comment="Add '$x = $point1['' to test autocomplete for shapes",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 22, "character": 0},
                                "end": {"line": 22, "character": 0},
                            },
                            "text": "$x = $point1['",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after user types a shape",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 22, "character": 14},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "'x'",
                            "kind": 12,
                            "detail": "literal",
                            "inlineDetail": "literal",
                            "sortText": "'x'",
                            "insertText": "'x'",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "'x'",
                                "filename": "${root_path}/completion.php",
                                "line": 22,
                                "char": 19,
                            },
                        },
                        {
                            "label": "'y'",
                            "kind": 12,
                            "detail": "literal",
                            "inlineDetail": "literal",
                            "sortText": "'y'",
                            "insertText": "'y'",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "'y'",
                                "filename": "${root_path}/completion.php",
                                "line": 22,
                                "char": 30,
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add automatically closed apostrophes when typing a shape key, the way visual studio code does it",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 22, "character": 0},
                                "end": {"line": 22, "character": 14},
                            },
                            "text": "$x = $point1['']",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after a shape, with VS Code automatically closed apostrophes",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 22, "character": 14},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "'x",
                            "kind": 12,
                            "detail": "literal",
                            "inlineDetail": "literal",
                            "sortText": "'x",
                            "insertText": "'x",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "'x'",
                                "filename": "${root_path}/completion.php",
                                "line": 22,
                                "char": 19,
                            },
                        },
                        {
                            "label": "'y",
                            "kind": 12,
                            "detail": "literal",
                            "inlineDetail": "literal",
                            "sortText": "'y",
                            "insertText": "'y",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "'y'",
                                "filename": "${root_path}/completion.php",
                                "line": 22,
                                "char": 30,
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 0},
                            },
                            "text": "$x = <",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 6},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertText": "ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:text",
                            "insertText": "ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <a'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 6},
                            },
                            "text": "$x = <a",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <a'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 7},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertText": "ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:text",
                            "insertText": "ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 7},
                            },
                            "text": "$x = <ab:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 9},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertText": "ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:text",
                            "insertText": "ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text '",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 9},
                            },
                            "text": "$x = <ab:cd:text ",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text '",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 17},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "width",
                            "kind": 10,
                            "detail": "?int",
                            "inlineDetail": "?int",
                            "sortText": "width",
                            "insertText": "width",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": "color",
                            "kind": 10,
                            "detail": "?string",
                            "inlineDetail": "?string",
                            "sortText": "color",
                            "insertText": "color",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text w'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 17},
                            },
                            "text": "$x = <ab:cd:text w",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text w'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 18},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "width",
                            "kind": 10,
                            "detail": "?int",
                            "inlineDetail": "?int",
                            "sortText": "width",
                            "insertText": "width",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": "color",
                            "kind": 10,
                            "detail": "?string",
                            "inlineDetail": "?string",
                            "sortText": "color",
                            "insertText": "color",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = new :'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 18},
                            },
                            "text": "$x = new :",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = new :'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 10},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": ":ab:cd:alpha",
                            "insertText": ":ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": ":ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": ":ab:cd:text",
                            "insertText": ":ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = new :a'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 10},
                            },
                            "text": "$x = new :a",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = new :a'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 11},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": ":ab:cd:alpha",
                            "insertText": ":ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": ":ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": ":ab:cd:text",
                            "insertText": ":ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            # Note that this request should match the result in the previous example
            .request(
                line=line(),
                comment="autocomplete resolving after '$x = new :a'",
                method="completionItem/resolve",
                params={
                    "label": ":ab:cd:alpha",
                    "kind": 7,
                    "detail": "class",
                    "inlineDetail": "class",
                    "itemType": ":ab:cd:alpha",
                    "insertText": ":ab:cd:alpha",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "data": {"fullname": ":ab:cd:alpha"},
                },
                result={
                    "label": ":ab:cd:alpha",
                    "kind": 7,
                    "detail": "class",
                    "inlineDetail": "class",
                    "itemType": ":ab:cd:alpha",
                    "documentation": {
                        "kind": "markdown",
                        "value": ":ab:cd:alpha docblock",
                    },
                    "insertText": ":ab:cd:alpha",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "data": {"fullname": ":ab:cd:alpha"},
                },
                powered_by="serverless_ide",
            )
            # Try the same thing again, but this time without "new", instead using "<xhp" style
            .notification(
                comment="Add '$x = <a'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 11},
                            },
                            "text": "$x = <a",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <a'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 7},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertText": "ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:text",
                            "insertText": "ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="autocomplete resolving after '$x = <a'",
                method="completionItem/resolve",
                params={
                    "label": "ab:cd:alpha",
                    "kind": 7,
                    "detail": "class",
                    "inlineDetail": "class",
                    "insertText": "ab:cd:alpha",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "data": {"fullname": ":ab:cd:alpha"},
                },
                result={
                    "label": "ab:cd:alpha",
                    "kind": 7,
                    "detail": "class",
                    "inlineDetail": "class",
                    "documentation": {
                        "kind": "markdown",
                        "value": ":ab:cd:alpha docblock",
                    },
                    "insertText": "ab:cd:alpha",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "data": {"fullname": ":ab:cd:alpha"},
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text/>; $y = $x->'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 7},
                            },
                            "text": "$x = <ab:cd:text/>; $y = $x->",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text/>; $y = $x->'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 29},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":width",
                            "kind": 10,
                            "detail": "?int",
                            "inlineDetail": "?int",
                            "sortText": ":width",
                            "insertText": ":width",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": ":color",
                            "kind": 10,
                            "detail": "?string",
                            "inlineDetail": "?string",
                            "sortText": ":color",
                            "insertText": ":color",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text/>; $y = $x->:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 29},
                            },
                            "text": "$x = <ab:cd:text/>; $y = $x->:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text/>; $y = $x->:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 30},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":width",
                            "kind": 10,
                            "detail": "?int",
                            "inlineDetail": "?int",
                            "sortText": ":width",
                            "insertText": ":width",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": ":color",
                            "kind": 10,
                            "detail": "?string",
                            "inlineDetail": "?string",
                            "sortText": ":color",
                            "insertText": ":color",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'test_fun'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 30},
                            },
                            "text": "test_fun",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'test_fun'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 8},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "test_function",
                            "kind": 3,
                            "detail": "function",
                            "inlineDetail": "function",
                            "sortText": "test_function",
                            "insertText": "test_function",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": "test_function"},
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="autocomplete resolving after 'test_fun'",
                method="completionItem/resolve",
                params={
                    "label": "test_function",
                    "kind": 3,
                    "detail": "function(): void",
                    "inlineDetail": "()",
                    "itemType": "void",
                    "insertText": "test_function",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "line": 8,
                        "char": 10,
                    },
                },
                result={
                    "label": "test_function",
                    "kind": 3,
                    "detail": "function(): void",
                    "inlineDetail": "()",
                    "itemType": "void",
                    "documentation": {
                        "kind": "markdown",
                        "value": "test_function docblock.",
                    },
                    "insertText": "test_function",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "line": 8,
                        "char": 10,
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'switch (Elsa::Alonso) { case Elsa:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 8},
                            },
                            "text": "switch (Elsa::Alonso) { case Elsa:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'switch (Elsa::Alonso) { case Elsa:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 34},
                },
                result={"isIncomplete": False, "items": []},
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'switch (Elsa::Alonso) { case Elsa::'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 34},
                            },
                            "text": "switch (Elsa::Alonso) { case Elsa::",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'switch (Elsa::Alonso) { case Elsa::'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 35},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "class",
                            "kind": 21,
                            "detail": "classname<this>",
                            "inlineDetail": "classname<this>",
                            "sortText": "class",
                            "insertText": "class",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "class",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 13,
                                "char": 6,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "Bard",
                            "kind": 21,
                            "detail": "Elsa",
                            "inlineDetail": "Elsa",
                            "sortText": "Bard",
                            "insertText": "Bard",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "Bard",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 13,
                                "char": 12,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "Alonso",
                            "kind": 21,
                            "detail": "Elsa",
                            "inlineDetail": "Elsa",
                            "sortText": "Alonso",
                            "insertText": "Alonso",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "Alonso",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 13,
                                "char": 12,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "isValid",
                            "kind": 2,
                            "detail": "function(mixed $value): bool",
                            "inlineDetail": "(mixed $value)",
                            "itemType": "bool",
                            "sortText": "isValid",
                            "insertText": "isValid(${1:\\$value})",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "isValid",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 49,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "getValues",
                            "kind": 2,
                            "detail": "function(): darray<string, Elsa>",
                            "inlineDetail": "()",
                            "itemType": "darray<string, Elsa>",
                            "sortText": "getValues",
                            "insertText": "getValues()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "getValues",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 34,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "getNames",
                            "kind": 2,
                            "detail": "function(): darray<Elsa, string>",
                            "inlineDetail": "()",
                            "itemType": "darray<Elsa, string>",
                            "sortText": "getNames",
                            "insertText": "getNames()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "getNames",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 43,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "coerce",
                            "kind": 2,
                            "detail": "function(mixed $value): ?Elsa",
                            "inlineDetail": "(mixed $value)",
                            "itemType": "?Elsa",
                            "sortText": "coerce",
                            "insertText": "coerce(${1:\\$value})",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "coerce",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 56,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "assertAll",
                            "kind": 2,
                            "detail": "function(Traversable<mixed> $values): Container<Elsa>",
                            "inlineDetail": "(Traversable<mixed> $values)",
                            "itemType": "Container<Elsa>",
                            "sortText": "assertAll",
                            "insertText": "assertAll(${1:\\$values})",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "assertAll",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 70,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "assert",
                            "kind": 2,
                            "detail": "function(mixed $value): Elsa",
                            "inlineDetail": "(mixed $value)",
                            "itemType": "Elsa",
                            "sortText": "assert",
                            "insertText": "assert(${1:\\$value})",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "assert",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 63,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'switch (Elsa::Alonso) { case Elsa::Alonso:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 35},
                            },
                            "text": "switch (Elsa::Alonso) { case Elsa::Alonso:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="docblock resolve after 'switch (Elsa::Alonso) { case Elsa::'",
                method="completionItem/resolve",
                params={
                    "label": "isValid",
                    "kind": 2,
                    "detail": "function(mixed $value): bool",
                    "inlineDetail": "(mixed $value)",
                    "itemType": "bool",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "textEdit": {
                        "range": {
                            "start": {"line": 3, "character": 35},
                            "end": {"line": 3, "character": 35},
                        },
                        "newText": "isValid",
                    },
                    "data": {
                        "filename": "${hhi_path}/BuiltinEnum.hhi",
                        "line": 49,
                        "char": 32,
                    },
                },
                result={
                    "label": "isValid",
                    "kind": 2,
                    "detail": "function(mixed $value): bool",
                    "inlineDetail": "(mixed $value)",
                    "itemType": "bool",
                    "documentation": {
                        "kind": "markdown",
                        "value": "Returns whether or not the value is defined as a constant.",
                    },
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "textEdit": {
                        "range": {
                            "start": {"line": 3, "character": 35},
                            "end": {"line": 3, "character": 35},
                        },
                        "newText": "isValid",
                    },
                    "data": {
                        "filename": "${hhi_path}/BuiltinEnum.hhi",
                        "line": 49,
                        "char": 32,
                    },
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="autocomplete after 'switch (Elsa::Alonso) { case Elsa::Alonso:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 42},
                },
                result={"isIncomplete": False, "items": []},
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'TestNS\\'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 42},
                            },
                            "text": "TestNS\\",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'TestNS\\'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 7},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "test_func",
                            "kind": 3,
                            "detail": "function",
                            "inlineDetail": "function",
                            "sortText": "test_func",
                            "insertText": "test_func",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": "TestNS\\test_func"},
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$cc = new CompletionClass(); $cc->interfa'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 7},
                            },
                            "text": "$cc = new CompletionClass(); $cc->interfa",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$cc = new CompletionClass(); $cc->interfa'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 41},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "interfaceDocBlockMethod",
                            "kind": 2,
                            "detail": "function(): void",
                            "inlineDetail": "()",
                            "itemType": "void",
                            "sortText": "interfaceDocBlockMethod",
                            "insertText": "interfaceDocBlockMethod()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "interfaceDocBlockMethod",
                                "filename": "${root_path}/completion.php",
                                "line": 18,
                                "char": 19,
                                "base_class": "\\CompletionClass",
                            },
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="autocomplete resolving after '$cc = new CompletionClass(); $cc->interfa'",
                method="completionItem/resolve",
                params={
                    "label": "interfaceDocBlockMethod",
                    "kind": 2,
                    "detail": "function(): void",
                    "inlineDetail": "()",
                    "itemType": "void",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "textEdit": {
                        "range": {
                            "start": {"line": 3, "character": 34},
                            "end": {"line": 3, "character": 41},
                        },
                        "newText": "interfaceDocBlockMethod",
                    },
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "line": 18,
                        "char": 19,
                    },
                },
                result={
                    "label": "interfaceDocBlockMethod",
                    "kind": 2,
                    "detail": "function(): void",
                    "inlineDetail": "()",
                    "itemType": "void",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "textEdit": {
                        "range": {
                            "start": {"line": 3, "character": 34},
                            "end": {"line": 3, "character": 41},
                        },
                        "newText": "interfaceDocBlockMethod",
                    },
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "line": 18,
                        "char": 19,
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'DeprecatedClass::'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 41},
                            },
                            "text": "DeprecatedClass::",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'DeprecatedClass::'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 17},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "class",
                            "kind": 21,
                            "detail": "classname<this>",
                            "inlineDetail": "classname<this>",
                            "sortText": "class",
                            "insertText": "class",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "class",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 18,
                                "char": 13,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "test_do_not_use",
                            "kind": 2,
                            "detail": "function(): void",
                            "inlineDetail": "()",
                            "itemType": "void",
                            "sortText": "~test_do_not_use",
                            "insertText": "test_do_not_use()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "test_do_not_use",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 22,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "getName",
                            "kind": 2,
                            "detail": "function(): void",
                            "inlineDetail": "()",
                            "itemType": "void",
                            "sortText": "getName",
                            "insertText": "getName()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "getName",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 19,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "getAttributes_DO_NOT_USE",
                            "kind": 2,
                            "detail": "function(): void",
                            "inlineDetail": "()",
                            "itemType": "void",
                            "sortText": "~getAttributes_DO_NOT_USE",
                            "insertText": "getAttributes_DO_NOT_USE()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "getAttributes_DO_NOT_USE",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 21,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "__getLoader",
                            "kind": 2,
                            "detail": "function(): void",
                            "inlineDetail": "()",
                            "itemType": "void",
                            "sortText": "~__getLoader",
                            "insertText": "__getLoader()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "__getLoader",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 20,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'call_lambda(3, $m'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 30, "character": 0},
                                "end": {"line": 30, "character": 0},
                            },
                            "text": "  call_lambda(3, $m",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete results for 'call_lambda(3, $m'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 30, "character": 19},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "$mylambda",
                            "kind": 6,
                            "detail": "local variable",
                            "inlineDetail": "(num $n)",
                            "itemType": "int",
                            "sortText": "$mylambda",
                            "insertText": "$mylambda",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "$mylambda",
                                "filename": "${root_path}/completion.php",
                                "line": 30,
                                "char": 15,
                            },
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="resolve autocompletion for $mylambda'",
                method="completionItem/resolve",
                params={
                    "label": "$mylambda",
                    "kind": 6,
                    "detail": "local variable",
                    "inlineDetail": "(num $n)",
                    "itemType": "int",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "textEdit": {
                        "range": {
                            "start": {"line": 30, "character": 17},
                            "end": {"line": 30, "character": 19},
                        },
                        "newText": "$mylambda",
                    },
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "line": 30,
                        "char": 15,
                    },
                },
                result={
                    "label": "$mylambda",
                    "kind": 6,
                    "detail": "local variable",
                    "inlineDetail": "(num $n)",
                    "itemType": "int",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "textEdit": {
                        "range": {
                            "start": {"line": 30, "character": 17},
                            "end": {"line": 30, "character": 19},
                        },
                        "newText": "$mylambda",
                    },
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "line": 30,
                        "char": 15,
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_completion_legacy(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("completion.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_completion_legacy"), use_serverless_ide=True
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
            .notification(
                comment="Add '$x = <'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 0},
                            },
                            "text": "$x = <",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 6},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertText": "ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:text",
                            "insertText": "ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <a'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 6},
                            },
                            "text": "$x = <a",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <a'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 7},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertText": "ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:text",
                            "insertText": "ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 7},
                            },
                            "text": "$x = <ab:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:'.",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 9},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertText": "ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": "ab:cd:text",
                            "insertText": "ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text '",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 9},
                            },
                            "text": "$x = <ab:cd:text ",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text '",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 17},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "width",
                            "kind": 10,
                            "detail": "?int",
                            "inlineDetail": "?int",
                            "sortText": "width",
                            "insertText": "width",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": "color",
                            "kind": 10,
                            "detail": "?string",
                            "inlineDetail": "?string",
                            "sortText": "color",
                            "insertText": "color",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text w'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 17},
                            },
                            "text": "$x = <ab:cd:text w",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text w'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 18},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "width",
                            "kind": 10,
                            "detail": "?int",
                            "inlineDetail": "?int",
                            "sortText": "width",
                            "insertText": "width",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": "color",
                            "kind": 10,
                            "detail": "?string",
                            "inlineDetail": "?string",
                            "sortText": "color",
                            "insertText": "color",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = new :''",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 18},
                            },
                            "text": "$x = new :",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = new :'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 10},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": ":ab:cd:alpha",
                            "insertText": ":ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": ":ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": ":ab:cd:text",
                            "insertText": ":ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = new :a'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 10},
                            },
                            "text": "$x = new :a",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = new :a'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 11},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": ":ab:cd:alpha",
                            "insertText": ":ab:cd:alpha",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": ":ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "inlineDetail": "class",
                            "sortText": ":ab:cd:text",
                            "insertText": ":ab:cd:text",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            # Note that this request sent should match the result given in the previous example
            .request(
                line=line(),
                comment="autocomplete resolving after '$x = new :a'",
                method="completionItem/resolve",
                params={
                    "label": ":ab:cd:alpha",
                    "kind": 7,
                    "detail": "class",
                    "inlineDetail": "class",
                    "itemType": ":ab:cd:alpha",
                    "insertText": ":ab:cd:alpha",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "data": {"fullname": ":ab:cd:alpha"},
                },
                result={
                    "label": ":ab:cd:alpha",
                    "kind": 7,
                    "detail": "class",
                    "inlineDetail": "class",
                    "itemType": ":ab:cd:alpha",
                    "documentation": {
                        "kind": "markdown",
                        "value": ":ab:cd:alpha docblock",
                    },
                    "insertText": ":ab:cd:alpha",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "data": {"fullname": ":ab:cd:alpha"},
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text/>; $y = $x->'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 11},
                            },
                            "text": "$x = <ab:cd:text/>; $y = $x->",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text/>; $y = $x->'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 29},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":width",
                            "kind": 10,
                            "detail": "?int",
                            "inlineDetail": "?int",
                            "sortText": ":width",
                            "insertText": ":width",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": ":color",
                            "kind": 10,
                            "detail": "?string",
                            "inlineDetail": "?string",
                            "sortText": ":color",
                            "insertText": ":color",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text/>; $y = $x->:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 29},
                            },
                            "text": "$x = <ab:cd:text/>; $y = $x->:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text/>; $y = $x->:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 30},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":width",
                            "kind": 10,
                            "detail": "?int",
                            "inlineDetail": "?int",
                            "sortText": ":width",
                            "insertText": ":width",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": ":color",
                            "kind": 10,
                            "detail": "?string",
                            "inlineDetail": "?string",
                            "sortText": ":color",
                            "insertText": ":color",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'test_fun'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 30},
                            },
                            "text": "test_fun",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'test_fun'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 8},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "test_function",
                            "kind": 3,
                            "detail": "function",
                            "inlineDetail": "function",
                            "sortText": "test_function",
                            "insertText": "test_function",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {"fullname": "test_function"},
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="autocomplete resolving after 'test_fun'",
                method="completionItem/resolve",
                params={
                    "label": "test_function",
                    "kind": 3,
                    "detail": "function(): void",
                    "inlineDetail": "()",
                    "itemType": "void",
                    "insertText": "test_function",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "line": 8,
                        "char": 10,
                    },
                },
                result={
                    "label": "test_function",
                    "kind": 3,
                    "detail": "function(): void",
                    "inlineDetail": "()",
                    "itemType": "void",
                    "documentation": {
                        "kind": "markdown",
                        "value": "test_function docblock.",
                    },
                    "insertText": "test_function",
                    "insertTextFormat": InsertTextFormat.PlainText.value,
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "line": 8,
                        "char": 10,
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'switch (Elsa::Alonso) { case Elsa:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 8},
                            },
                            "text": "switch (Elsa::Alonso) { case Elsa:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'switch (Elsa::Alonso) { case Elsa:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 34},
                },
                result={"isIncomplete": False, "items": []},
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'switch (Elsa::Alonso) { case Elsa::'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 34},
                            },
                            "text": "switch (Elsa::Alonso) { case Elsa::",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'switch (Elsa::Alonso) { case Elsa::'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 35},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "class",
                            "kind": 21,
                            "detail": "classname<this>",
                            "inlineDetail": "classname<this>",
                            "sortText": "class",
                            "insertText": "class",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "class",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 13,
                                "char": 6,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "Bard",
                            "kind": 21,
                            "detail": "Elsa",
                            "inlineDetail": "Elsa",
                            "sortText": "Bard",
                            "insertText": "Bard",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "Bard",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 13,
                                "char": 12,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "Alonso",
                            "kind": 21,
                            "detail": "Elsa",
                            "inlineDetail": "Elsa",
                            "sortText": "Alonso",
                            "insertText": "Alonso",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "Alonso",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 13,
                                "char": 12,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "isValid",
                            "kind": 2,
                            "detail": "function(mixed $value): bool",
                            "inlineDetail": "(mixed $value)",
                            "itemType": "bool",
                            "sortText": "isValid",
                            "insertText": "isValid(${1:\\$value})",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "isValid",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 49,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "getValues",
                            "kind": 2,
                            "detail": "function(): darray<string, Elsa>",
                            "inlineDetail": "()",
                            "itemType": "darray<string, Elsa>",
                            "sortText": "getValues",
                            "insertText": "getValues()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "getValues",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 34,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "getNames",
                            "kind": 2,
                            "detail": "function(): darray<Elsa, string>",
                            "inlineDetail": "()",
                            "itemType": "darray<Elsa, string>",
                            "sortText": "getNames",
                            "insertText": "getNames()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "getNames",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 43,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "coerce",
                            "kind": 2,
                            "detail": "function(mixed $value): ?Elsa",
                            "inlineDetail": "(mixed $value)",
                            "itemType": "?Elsa",
                            "sortText": "coerce",
                            "insertText": "coerce(${1:\\$value})",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "coerce",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 56,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "assertAll",
                            "kind": 2,
                            "detail": "function(Traversable<mixed> $values): Container<Elsa>",
                            "inlineDetail": "(Traversable<mixed> $values)",
                            "itemType": "Container<Elsa>",
                            "sortText": "assertAll",
                            "insertText": "assertAll(${1:\\$values})",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "assertAll",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 70,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "assert",
                            "kind": 2,
                            "detail": "function(mixed $value): Elsa",
                            "inlineDetail": "(mixed $value)",
                            "itemType": "Elsa",
                            "sortText": "assert",
                            "insertText": "assert(${1:\\$value})",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "assert",
                                "filename": "${hhi_path}/BuiltinEnum.hhi",
                                "line": 63,
                                "char": 32,
                                "base_class": "\\Elsa",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'switch (Elsa::Alonso) { case Elsa::Alonso:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 35},
                            },
                            "text": "switch (Elsa::Alonso) { case Elsa::Alonso:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'switch (Elsa::Alonso) { case Elsa::Alonso:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 42},
                },
                result={"isIncomplete": False, "items": []},
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'DeprecatedClass::'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 41},
                            },
                            "text": "DeprecatedClass::",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'DeprecatedClass::'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 17},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "class",
                            "kind": 21,
                            "detail": "classname<this>",
                            "inlineDetail": "classname<this>",
                            "sortText": "class",
                            "insertText": "class",
                            "insertTextFormat": InsertTextFormat.PlainText.value,
                            "data": {
                                "fullname": "class",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 18,
                                "char": 13,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "test_do_not_use",
                            "kind": 2,
                            "detail": "function(): void",
                            "inlineDetail": "()",
                            "itemType": "void",
                            "sortText": "~test_do_not_use",
                            "insertText": "test_do_not_use()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "test_do_not_use",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 22,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "getName",
                            "kind": 2,
                            "detail": "function(): void",
                            "inlineDetail": "()",
                            "itemType": "void",
                            "sortText": "getName",
                            "insertText": "getName()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "getName",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 19,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "getAttributes_DO_NOT_USE",
                            "kind": 2,
                            "detail": "function(): void",
                            "inlineDetail": "()",
                            "itemType": "void",
                            "sortText": "~getAttributes_DO_NOT_USE",
                            "insertText": "getAttributes_DO_NOT_USE()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "getAttributes_DO_NOT_USE",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 21,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "__getLoader",
                            "kind": 2,
                            "detail": "function(): void",
                            "inlineDetail": "()",
                            "itemType": "void",
                            "sortText": "~__getLoader",
                            "insertText": "__getLoader()",
                            "insertTextFormat": InsertTextFormat.Snippet.value,
                            "data": {
                                "fullname": "__getLoader",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 20,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_definition(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("definition.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_definition"), use_serverless_ide=True
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
                line=line(),
                comment="call to `b_definition`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 10},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 6, "character": 9},
                            "end": {"line": 6, "character": 21},
                        },
                        "title": "b_definition",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new BB(1)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 29, "character": 13},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 11, "character": 18},
                            "end": {"line": 11, "character": 29},
                        },
                        "title": "BB::__construct",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new CC(1)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 30, "character": 13},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 14, "character": 6},
                            "end": {"line": 14, "character": 8},
                        },
                        "title": "CC",
                    },
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 11, "character": 18},
                            "end": {"line": 11, "character": 29},
                        },
                        "title": "BB::__construct",
                    },
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new DD(1)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 31, "character": 13},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 17, "character": 6},
                            "end": {"line": 17, "character": 8},
                        },
                        "title": "DD",
                    },
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 11, "character": 18},
                            "end": {"line": 11, "character": 29},
                        },
                        "title": "BB::__construct",
                    },
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new EE(1)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 32, "character": 13},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 21, "character": 18},
                            "end": {"line": 21, "character": 29},
                        },
                        "title": "EE::__construct",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new FF(1)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 33, "character": 13},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 26, "character": 6},
                            "end": {"line": 26, "character": 8},
                        },
                        "title": "FF",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new TakesString(HasString::MyString)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 45, "character": 23},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 40, "character": 6},
                            "end": {"line": 40, "character": 15},
                        },
                        "title": "HasString",
                    }
                ],
                powered_by="serverless_ide",
            )
            .notification(
                comment="make local, unsaved change to the file",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}", "version": 2},
                    "contentChanges": [
                        {
                            "text": "test",
                            "range": {
                                "start": {"line": 3, "character": 9},
                                "end": {"line": 3, "character": 21},
                            },
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="call to `test` instead of `b_definition`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 10},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 28, "character": 9},
                            "end": {"line": 28, "character": 13},
                        },
                        "title": "test",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_overridden_definition(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("override.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_overridden_definition"),
                use_serverless_ide=True,
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
                line=line(),
                comment="find overridden method from trait",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 13, "character": 5},
                },
                result=[
                    {
                        "uri": "file://${root_path}/override.php",
                        "range": {
                            "start": {"line": 7, "character": 18},
                            "end": {"line": 7, "character": 21},
                        },
                        "title": "MyTrait::foo",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="find overridden static method",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 5},
                },
                result=[
                    {
                        "uri": "file://${root_path}/override.php",
                        "range": {
                            "start": {"line": 23, "character": 25},
                            "end": {"line": 23, "character": 28},
                        },
                        "title": "C2::bar",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="find overridden interface method",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 35, "character": 5},
                },
                result=[
                    {
                        "uri": "file://${root_path}/override.php",
                        "range": {
                            "start": {"line": 32, "character": 18},
                            "end": {"line": 32, "character": 22},
                        },
                        "title": "I1::quux",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_document_symbol(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("definition.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_document_symbol"), use_serverless_ide=True
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
                line=line(),
                comment="documentSymbol call",
                method="textDocument/documentSymbol",
                params={"textDocument": {"uri": "${php_file_uri}"}},
                result=[
                    {
                        "name": "testClassMemberInsideConstructorInvocation",
                        "kind": 12,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 44, "character": 0},
                                "end": {"line": 46, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "MyString",
                        "kind": 14,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 41, "character": 8},
                                "end": {"line": 41, "character": 29},
                            },
                        },
                        "containerName": "HasString",
                    },
                    {
                        "name": "HasString",
                        "kind": 5,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 40, "character": 0},
                                "end": {"line": 42, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "__construct",
                        "kind": 6,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 37, "character": 2},
                                "end": {"line": 37, "character": 43},
                            },
                        },
                        "containerName": "TakesString",
                    },
                    {
                        "name": "TakesString",
                        "kind": 5,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 36, "character": 0},
                                "end": {"line": 38, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "FF",
                        "kind": 5,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 26, "character": 0},
                                "end": {"line": 26, "character": 11},
                            },
                        },
                    },
                    {
                        "name": "__construct",
                        "kind": 6,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 21, "character": 2},
                                "end": {"line": 23, "character": 3},
                            },
                        },
                        "containerName": "EE",
                    },
                    {
                        "name": "EE",
                        "kind": 5,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 20, "character": 0},
                                "end": {"line": 24, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "CC",
                        "kind": 5,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 14, "character": 0},
                                "end": {"line": 15, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "__construct",
                        "kind": 6,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 11, "character": 2},
                                "end": {"line": 11, "character": 40},
                            },
                        },
                        "containerName": "BB",
                    },
                    {
                        "name": "BB",
                        "kind": 5,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 10, "character": 0},
                                "end": {"line": 12, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "a_definition",
                        "kind": 12,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 2, "character": 0},
                                "end": {"line": 4, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "b_definition",
                        "kind": 12,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 6, "character": 0},
                                "end": {"line": 8, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "DD",
                        "kind": 5,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 17, "character": 0},
                                "end": {"line": 18, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "test",
                        "kind": 12,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 28, "character": 0},
                                "end": {"line": 34, "character": 1},
                            },
                        },
                    },
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def initialize_spec(
        self,
        spec: LspTestSpec,
        use_serverless_ide: bool,
        supports_status: bool = False,  # does the caller wish to see all status messages?
        supports_init: bool = False,  # do we wish to interact with init, rather than waiting for init ok?
    ) -> LspTestSpec:
        if use_serverless_ide:
            initialization_options = {
                "namingTableSavedStatePath": "${naming_table_saved_state_path}",
                "namingTableSavedStateTestDelay": 0.0,
            }
            if supports_init:
                # A small delay, since otherwise init completes immediately
                # This isn't very racy. All we need is a tiny delay so that
                # other things which are in the queue get processed, rather
                # than continuing synchronously
                initialization_options["namingTableSavedStateTestDelay"] = 0.5
        else:
            initialization_options = {}

        window_capabilities = {}
        if supports_status:
            window_capabilities["status"] = {"dynamicRegistration": False}

        spec = spec.ignore_notifications(method="telemetry/event").request(
            line=line(),
            method="initialize",
            params={
                "initializationOptions": initialization_options,
                "processId": None,
                "rootPath": "${root_path}",
                "capabilities": {
                    "window": window_capabilities,
                    "textDocument": {
                        "completion": {"completionItem": {"snippetSupport": True}}
                    },
                },
            },
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
                    "implementationProvider": True,
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
        if not supports_status:
            spec = spec.ignore_status_diagnostics(True)

        if use_serverless_ide and not supports_init:
            spec = spec.wait_for_notification(
                comment="wait for sIDE to finish init",
                method="telemetry/event",
                params={"type": 4, "message": "[client-ide] Finished init: ok"},
            )

        return spec

    def test_serverless_ide_type_definition(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("type_definition.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_type_definition"), use_serverless_ide=True
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
                line=line(),
                comment="Conditional Type Definition of HH or II",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 32, "character": 2},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 2, "character": 6},
                            "end": {"line": 2, "character": 8},
                        },
                        "title": "\\HH",
                    },
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 12, "character": 6},
                            "end": {"line": 12, "character": 8},
                        },
                        "title": "\\LL",
                    },
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Standard Class Definition",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 40, "character": 2},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 2, "character": 6},
                            "end": {"line": 2, "character": 8},
                        },
                        "title": "\\HH",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Class Type Definition with Casting",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 41, "character": 2},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 2, "character": 6},
                            "end": {"line": 2, "character": 8},
                        },
                        "title": "\\HH",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Primitive Type Definition",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 42, "character": 2},
                },
                result=[],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Function Return Type Definition",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 43, "character": 2},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 12, "character": 6},
                            "end": {"line": 12, "character": 8},
                        },
                        "title": "\\LL",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Function definition with primitive return type",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 44, "character": 2},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 22, "character": 9},
                            "end": {"line": 22, "character": 29},
                        },
                        "title": "(function(): int)",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

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
                line=line(),
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
                line=line(),
                comment="hover over string literal outside call",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 25, "character": 12},  # 9 - 16
                },
                result={"contents": [{"language": "hack", "value": "string"}]},
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over string literal inside call",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 20},  # 16 - 29
                },
                result={"contents": [{"language": "hack", "value": "string"}]},
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over int literal inside call",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 32},  # 31 - 33
                },
                result={"contents": [{"language": "hack", "value": "int"}]},
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over constant reference",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 15, "character": 19},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "THE_ANSWER"},
                        "A comment describing THE_ANSWER",
                        "int THE_ANSWER = 42",
                    ],
                    "range": {
                        "start": {"line": 15, "character": 9},
                        "end": {"line": 15, "character": 19},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
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
                line=line(),
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
                line=line(),
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
                line=line(),
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
                line=line(),
                comment="hover past the end of a file",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 300, "character": 0},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over class with copyright docblock",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 37, "character": 15},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "final class CopyrightClass"},
                        "Testing copyright removal",
                    ],
                    "range": {
                        "start": {"line": 37, "character": 2},
                        "end": {"line": 37, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over class with generated docblock",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 58, "character": 15},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "final class GeneratedClass"},
                        "Testing generated text removal",
                    ],
                    "range": {
                        "start": {"line": 58, "character": 2},
                        "end": {"line": 58, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_file_touched_on_disk(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_file_on_disk_change"),
                use_serverless_ide=True,
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
            .notification(
                method="workspace/didChangeWatchedFiles",
                params={"changes": [{"uri": "${php_file_uri}", "type": 2}]},
            )
            .wait_for_notification(
                comment="wait for sIDE to process file change",
                method="telemetry/event",
                params={
                    "type": 4,
                    "message": "[client-ide] Done processing file changes",
                },
            )
            .request(
                line=line(),
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
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_file_hover_with_errors(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover_with_errors.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_hover_with_errors"), use_serverless_ide=True
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
            .notification(
                method="workspace/didChangeWatchedFiles",
                params={"changes": [{"uri": "${php_file_uri}", "type": 2}]},
            )
            .wait_for_notification(
                comment="wait for sIDE to process file change",
                method="telemetry/event",
                params={
                    "type": 4,
                    "message": "[client-ide] Done processing file changes",
                },
            )
            .request(
                line=line(),
                comment="Totally normal hover",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 14, "character": 37},
                },
                result={
                    "contents": [
                        {
                            "language": "hack",
                            "value": "public static function staticMethod(string $z): void",
                        },
                        'During testing, we\'ll remove the "public" tag from this '
                        "method\n"
                        "to ensure that we can still get IDE services",
                        "Return type: `void`",
                        "Full name: `HoverWithErrorsClass::staticMethod`",
                    ],
                    "range": {
                        "end": {"character": 39, "line": 14},
                        "start": {"character": 27, "line": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Remove the 'public' visibility modifier which triggers AST->AAST errors",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 10, "character": 2},
                                "end": {"line": 10, "character": 8},
                            },
                            "text": "",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="Hover should still work even if visibility modifier has been removed",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 14, "character": 37},
                },
                result={
                    "contents": [
                        {
                            "language": "hack",
                            "value": "public static function staticMethod(string $z): void",
                        },
                        'During testing, we\'ll remove the "public" tag from this '
                        "method\n"
                        "to ensure that we can still get IDE services",
                        "Return type: `void`",
                        "Full name: `HoverWithErrorsClass::staticMethod`",
                    ],
                    "range": {
                        "end": {"character": 39, "line": 14},
                        "start": {"character": 27, "line": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_formatting(self) -> None:
        # This test will fail if hackfmt can't be found
        if not self.test_driver.run_hackfmt_check():
            raise unittest.SkipTest("Hackfmt can't be found. Skipping.")

        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("messy.php"))

        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(LspTestSpec("formatting"), use_serverless_ide=True)
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
                line=line(),
                method="textDocument/formatting",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "options": {"tabSize": 5, "insertSpaces": True},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 0, "character": 0},
                            "end": {"line": 15, "character": 0},
                        },
                        "newText": "<?hh //strict\n\nfunction x(): string {\n"
                        + "     /* @lint-ignore TXT2 3 tabs on purpose */\n"
                        + '     $a = "this";\n\n'
                        + "     /* @lint-ignore TXT2 2 tabs on purpose */\n"
                        + '     $b = "is";\n\n'
                        + "     /* lint-ignore TXT2 1 tab on purpose */\n"
                        + '     $c = "messy"; // 1 tab\n\n'
                        + '     $d = "."; // 4 spaces\n'
                        + '     return "$a"."$b"."$c"."d";\n}\n',
                    }
                ],
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_rangeformatting(self) -> None:
        # This test will fail if hackfmt can't be found
        if not self.test_driver.run_hackfmt_check():
            raise unittest.SkipTest("Hackfmt can't be found. Skipping.")

        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("messy.php"))

        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("range_formatting"), use_serverless_ide=True
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
                line=line(),
                method="textDocument/rangeFormatting",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "range": {
                        "start": {"line": 4, "character": 0},
                        "end": {"line": 5, "character": 0},
                    },
                    "options": {"tabSize": 5, "insertSpaces": True},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 4, "character": 0},
                            "end": {"line": 5, "character": 0},
                        },
                        "newText": '     $a = "this";\n',
                    }
                ],
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_ontypeformatting(self) -> None:
        # This test will fail if hackfmt can't be found
        if not self.test_driver.run_hackfmt_check():
            raise unittest.SkipTest("Hackfmt can't be found. Skipping.")

        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("ontypeformatting.php"))

        spec = (
            self.initialize_spec(
                LspTestSpec("ontypeformatting"), use_serverless_ide=True
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
                line=line(),
                method="textDocument/onTypeFormatting",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 9, "character": 58},
                    "ch": ";",
                    "options": {"tabSize": 2, "insertSpaces": True},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 5, "character": 17},
                            "end": {"line": 9, "character": 58},
                        },
                        "newText": "{\n  test_otf(\n"
                        + "    '1234567890',\n"
                        + "    '1234567890',\n"
                        + "    '1234567890',\n"
                        + "    '1234567890',\n"
                        + "    '1234567890',\n"
                        + "    '1234567890',\n  );",
                    }
                ],
            )
            .request(
                line=line(),
                method="textDocument/onTypeFormatting",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 13, "character": 1},
                    "ch": "}",
                    "options": {"tabSize": 2, "insertSpaces": True},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 13, "character": 0},
                            "end": {"line": 13, "character": 1},
                        },
                        "newText": "{",
                    }
                ],
            )
            .request(
                line=line(),
                method="textDocument/onTypeFormatting",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 15, "character": 16},
                    "ch": "}",
                    "options": {"tabSize": 2, "insertSpaces": True},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 15, "character": 0},
                            "end": {"line": 15, "character": 16},
                        },
                        "newText": "function otf() {}",
                    }
                ],
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_did_change(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("didchange.php")
        spec = (
            self.initialize_spec(LspTestSpec("did_change"), use_serverless_ide=False)
            .wait_for_hh_server_ready()
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
            .notification(
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 7, "character": 11},
                                "end": {"line": 7, "character": 12},
                            },
                            "text": "a",
                        }
                    ],
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 7, "character": 11},
                                "end": {"line": 7, "character": 11},
                            },
                            "severity": 1,
                            "code": 1002,
                            "source": "Hack",
                            "message": "A semicolon ; is expected here.",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                comment="Hack appears to clear out diagnostics before shutting down",
                method="textDocument/publishDiagnostics",
                params={"uri": "${php_file_uri}", "diagnostics": []},
            )
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=True, use_serverless_ide=False)

    def test_go_to_implementation(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("go_to_implementation.php")
        spec = (
            self.initialize_spec(
                LspTestSpec("test_go_to_implementation"), use_serverless_ide=False
            )
            .wait_for_hh_server_ready()
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
                line=line(),
                comment="go to implemenetation: abstract class",
                method="textDocument/implementation",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 1, "character": 17},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 7, "character": 6},
                            "end": {"line": 7, "character": 9},
                        },
                    }
                ],
            )
            .request(
                line=line(),
                comment="go to implemenetation: interface",
                method="textDocument/implementation",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 13, "character": 13},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 17, "character": 6},
                            "end": {"line": 17, "character": 9},
                        },
                    }
                ],
            )
            .request(
                line=line(),
                comment="go to implemenetation: trait",
                method="textDocument/implementation",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 23, "character": 10},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 30, "character": 6},
                            "end": {"line": 30, "character": 16},
                        },
                    }
                ],
            )
            .request(
                line=line(),
                comment="go to implemenetation: method",
                method="textDocument/implementation",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 19, "character": 18},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 8, "character": 18},
                            "end": {"line": 8, "character": 22},
                        },
                    }
                ],
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=True, use_serverless_ide=False)

    def test_signature_help(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("signaturehelp.php")
        spec = (
            self.initialize_spec(
                LspTestSpec("test_signature_help"), use_serverless_ide=False
            )
            .wait_for_hh_server_ready()
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
                line=line(),
                comment="signature help for 0-argument constructor"
                " (left of opening paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 16, "character": 18},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="signature help for 0-argument constructor",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 16, "character": 19},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function __construct(): void",
                            "documentation": "Constructor with doc block",
                            "parameters": [],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
            )
            .request(
                line=line(),
                comment="signature help for 0-argument constructor"
                " (right of closing paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 16, "character": 20},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (left of opening paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 17, "character": 20},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (right of opening paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 17, "character": 21},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function instanceMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (left of first comma)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 17, "character": 22},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function instanceMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 1,
                },
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (right of first comma)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 17, "character": 23},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function instanceMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 1,
                },
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (left of closing paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 17, "character": 24},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function instanceMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 1,
                },
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (right of closing paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 17, "character": 25},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="signature help for 1-argument static method"
                " (left of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 18, "character": 23},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="signature help for 1-argument static method"
                " (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 18, "character": 24},
                },
                result={
                    "signatures": [
                        {
                            "label": "public static function staticMethod"
                            "(string $z): void",
                            "documentation": "Static method with doc block",
                            "parameters": [{"label": "$z"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
            )
            .request(
                line=line(),
                comment="signature help for 2-argument global function"
                " (left of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 19, "character": 17},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="signature help for 2-argument global function"
                " (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 19, "character": 18},
                },
                result={
                    "signatures": [
                        {
                            "label": "function global_function"
                            "(string $s, int $x): void",
                            "documentation": "Global function with doc block",
                            "parameters": [{"label": "$s"}, {"label": "$x"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
            )
            .request(
                line=line(),
                comment="signature help for 1-argument namespace-aliased global"
                " function (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 20, "character": 26},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="signature help for 1-argument namespace-aliased global"
                " function (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 20, "character": 26},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="signature help for 1-argument namespace-aliased global"
                " function (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 20, "character": 27},
                },
                result={
                    "signatures": [
                        {
                            "label": "function Derp\\Lib\\Herp\\aliased_global_func(string $s): void",
                            "documentation": "Namespace-aliased function with doc block",
                            "parameters": [{"label": "$s"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
            )
            .request(
                line=line(),
                comment="signature help for 1-argument namespace-aliased global"
                " function (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 20, "character": 28},
                },
                result={
                    "signatures": [
                        {
                            "label": "function Derp\\Lib\\Herp\\aliased_global_func(string $s): void",
                            "documentation": "Namespace-aliased function with doc block",
                            "parameters": [{"label": "$s"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
            )
            .request(
                line=line(),
                comment="signature help for 2-argument function with params"
                " (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 21, "character": 30},
                },
                result={
                    "signatures": [
                        {
                            "label": "function test_signature_help_params1("
                            "\n  string $param1,\n  string $param2\n): void",
                            "documentation": "comment describing the method"
                            "\n@param $param1 info1"
                            "\n@param param2 info2",
                            "parameters": [
                                {"label": "$param1", "documentation": "info1"},
                                {"label": "$param2", "documentation": "info2"},
                            ],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
            )
            .request(
                line=line(),
                comment="signature help for 2-argument function with params"
                " (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 22, "character": 30},
                },
                result={
                    "signatures": [
                        {
                            "label": "function test_signature_help_params2("
                            "\n  string $param1,\n  string $param2\n): void",
                            "documentation": "comment describing the method"
                            "\n@param $param1 info1",
                            "parameters": [
                                {"label": "$param1", "documentation": "info1"},
                                {"label": "$param2"},
                            ],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
            )
            .request(
                line=line(),
                comment="signature help for 2-argument function with params"
                " (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 23, "character": 30},
                },
                result={
                    "signatures": [
                        {
                            "label": "function test_signature_help_params3("
                            "\n  string $param1,\n  string $param2\n): string",
                            "documentation": "@param $param1 info1"
                            "\n               for param1"
                            "\n@param $param2   info2"
                            "\n@return the string"
                            "\n        'hack'",
                            "parameters": [
                                {
                                    "label": "$param1",
                                    "documentation": "info1 for param1",
                                },
                                {"label": "$param2", "documentation": "info2"},
                            ],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=True, use_serverless_ide=False)

    def test_signature_help_lambda(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("signaturehelp_lambda.php")
        spec = (
            self.initialize_spec(
                LspTestSpec("test_serverless_ide_signature_help_lambda"),
                use_serverless_ide=False,
            )
            .wait_for_hh_server_ready()
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
                line=line(),
                comment="signature help for a normal function call",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 8, "character": 29},
                },
                result={
                    "activeParameter": 0,
                    "activeSignature": 0,
                    "signatures": [
                        {
                            "label": "function test_lambda_sighelp(\n"
                            "  string $str,\n"
                            "  (function(string): int) $f\n"
                            "): int",
                            "parameters": [{"label": "$str"}, {"label": "$f"}],
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="signature help for normal function call within a lambda",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 9, "character": 21},
                },
                result={
                    "activeParameter": 0,
                    "activeSignature": 0,
                    "signatures": [
                        {
                            "label": "function normal_test_func(string $str): void",
                            "parameters": [{"label": "$str"}],
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="signature help for text within a lambda, left side of an open paren",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 10, "character": 15},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="signature help for text within a lambda, right side of an open paren",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 10, "character": 16},
                },
                result=None,
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=True, use_serverless_ide=False)

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
        spec = (
            self.initialize_spec(LspTestSpec("non_blocking"), use_serverless_ide=False)
            .wait_for_hh_server_ready()
            .request(
                line=line(),
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 7, "character": 11},
                },
                result=[
                    {
                        "uri": "file://${root_path}/non_blocking.php",
                        "range": {
                            "start": {"line": 2, "character": 9},
                            "end": {"line": 2, "character": 32},
                        },
                        "title": "non_blocking_definition",
                    }
                ],
                wait_id="definition request",
            )
            .notification(
                comment="remove hh_loop_forever() invocation to break the infinite loop",
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${root_path}/__hh_loop_forever_foo.php",
                        "languageId": "hack",
                        "version": 1,
                        "text": """\
<?hh // strict

function __hh_loop_forever_foo(): int {
  return 4;
}
""",
                    }
                },
            )
            .wait_for_response(wait_id="definition request")
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=True, use_serverless_ide=False)

    def test_serverless_ide_hierarchy_file_change_on_disk(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("incremental_derived.php"))
        changed_php_file_uri = self.repo_file("incremental_base.php")
        variables.update({"changed_php_file_uri": changed_php_file_uri})
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_hierarchy_file_change_on_disk"),
                use_serverless_ide=True,
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
                line=line(),
                comment="hover before change to class hierarchy should be `int`",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 7, "character": 14},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "public function foo(): int"},
                        "Return type: `int`",
                        "Full name: `BaseClassIncremental::foo`",
                    ],
                    "range": {
                        "start": {"line": 7, "character": 12},
                        "end": {"line": 7, "character": 15},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                uri=changed_php_file_uri,
                contents="""\
<?hh // strict
class BaseClassIncremental {
  public function foo(): string { return ''; }
}
""",
                notify=True,
            )
            .request(
                line=line(),
                comment="hover after change to class hierarchy should be `string`",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 7, "character": 14},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "public function foo(): string"},
                        "Return type: `string`",
                        "Full name: `BaseClassIncremental::foo`",
                    ],
                    "range": {
                        "start": {"line": 7, "character": 12},
                        "end": {"line": 7, "character": 15},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_decl_in_unsaved_buffer_changed(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_decl_in_unsaved_buffer_changed"),
                use_serverless_ide=True,
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
                line=line(),
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
            .notification(
                comment="make local, unsaved change to the file",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}", "version": 2},
                    "contentChanges": [
                        {
                            "text": """\
<?hh // strict
// comment
function a_hover(): int {
  return b_hover();
}
# A comment describing b_hover differently.
function b_hover(): string {
  return 42;
}
"""
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="another hover over function invocation, should be string now",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 16},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "string"},
                        "A comment describing b_hover differently.",
                    ],
                    "range": {
                        "start": {"line": 3, "character": 9},
                        "end": {"line": 3, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_decl_two_unsaved_buffers(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("unsaved1.php"))
        variables.update({"unsaved2_file_uri": self.repo_file_uri("unsaved2.php")})
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("test_serverless_ide_decl_two_unsaved_buffers"),
                use_serverless_ide=True,
            )
            .notification(
                comment="open 'unsaved1.php', since we'll be hovering in it",
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
            .notification(
                comment="open 'unsaved2.php' with a bool-returning signature, different from disk",
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${unsaved2_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": """\
<?hh  //strict
function unsaved_bar(): bool { return true; }
""",
                    }
                },
            )
            .request(
                line=line(),
                comment="hover 'unsaved1.php' is with respect to disk contents of 'unsaved2.php'",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 1, "character": 39},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function unsaved_bar(): int"},
                        "Return type: `int`",
                    ],
                    "range": {
                        "start": {"line": 1, "character": 34},
                        "end": {"line": 1, "character": 45},
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="change signature in 'unsaved2.php' to return string",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${unsaved2_file_uri}", "version": 2},
                    "contentChanges": [
                        {
                            "text": """\
<?hh  //strict
function unsaved_bar(): string { return "hello"; }
"""
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="this is a dummy hover in 'unsaved2.php' just to ensure its decl is cached",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${unsaved2_file_uri}"},
                    "position": {"line": 0, "character": 0},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover 'unsaved1.php' is still with respect to disk contents of 'unsaved2.php'",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 1, "character": 39},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function unsaved_bar(): int"},
                        "Return type: `int`",
                    ],
                    "range": {
                        "start": {"line": 1, "character": 34},
                        "end": {"line": 1, "character": 45},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                comment="save signature in 'unsaved2' to return string",
                uri=variables["unsaved2_file_uri"],
                contents="""\
<?hh // strict
function unsaved_bar(): string { return "hello"; }
""",
                notify=True,
            )
            .request(
                line=line(),
                comment="hover 'unsaved1.php' gets new disk contents of 'unsaved2.php'",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 1, "character": 39},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function unsaved_bar(): string"},
                        "Return type: `string`",
                    ],
                    "range": {
                        "start": {"line": 1, "character": 34},
                        "end": {"line": 1, "character": 45},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_hover_without_file_open(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("test_hover_without_file_open"),
                use_serverless_ide=True,
                supports_status=True,
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .ignore_requests(
                comment="Ignore 'initializing...' messages since they're racy",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [{"title": "Restart hh_server"}],
                    "message": "Hack IDE: initializing.\nhh_server: stopped.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="another racy initializing, before hh_server has even responded",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [],
                    "message": "Hack IDE: initializing.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="another racy initialization to ignore, again before hh_server",
                method="window/showStatus",
                params={
                    "type": 3,
                    "actions": [],
                    "message": "Hack IDE: ready.",
                    "shortMessage": "Hack: ready",
                },
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "actions": [{"title": "Restart hh_server"}],
                    "message": "Hack IDE: ready.\nhh_server: stopped.",
                    "shortMessage": "Hack: ready",
                    "type": 3,
                },
                result=NoResponse(),
            )
            .request(
                line=line(),
                comment="hover before file_open will fail",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 20},
                },
                result=None,
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
                line=line(),
                comment="hover after file_open will succeed",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 20},
                },
                result={"contents": [{"language": "hack", "value": "string"}]},
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                method="$test/shutdownServerlessIde",
                params={},
                result=None,
                powered_by="serverless_ide",
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "actions": [
                        {"title": "Restart Hack IDE"},
                        {"title": "Restart hh_server"},
                    ],
                    "message": "Hack IDE has failed. See Output›Hack for details.\nhh_server: stopped.",
                    "shortMessage": "Hack: failed",
                    "type": 1,
                },
                result={"title": "Restart Hack IDE"},
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "actions": [{"title": "Restart hh_server"}],
                    "message": "Hack IDE: ready.\nhh_server: stopped.",
                    "shortMessage": "Hack: ready",
                    "type": 3,
                },
                result=NoResponse(),
            )
            .request(
                line=line(),
                comment="hover after restart will succeed",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 20},
                },
                result={"contents": [{"language": "hack", "value": "string"}]},
                powered_by="serverless_ide",
            )
            .notification(
                method="textDocument/didClose",
                params={"textDocument": {"uri": "${php_file_uri}"}},
            )
            .request(
                line=line(),
                comment="hover after file_close will fail",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 20},
                },
                result=None,
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_hh_server_status_diagnostic(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("unsaved1.php"))
        variables.update(
            {
                "unsaved2_file_uri": self.repo_file_uri("unsaved2.php"),
                "unsaved2_file": self.read_repo_file("unsaved2.php"),
            }
        )
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("test_hh_server_status_diagnostic"), use_serverless_ide=True
            )
            .ignore_status_diagnostics(False)
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
            .wait_for_notification(
                comment="After didOpen(file1), the hh_server_status diagnostic should appear in file1",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 0, "character": 0},
                                "end": {"line": 0, "character": 1},
                            },
                            "severity": 1,
                            "source": "hh_server",
                            "message": "hh_server isn't running, so there may be undetected errors. Try `hh` at the command line... hh_server: stopped.",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        }
                    ],
                    "isStatusFB": True,
                },
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${unsaved2_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${unsaved2_file}",
                    }
                },
            )
            .wait_for_notification(
                comment="After didOpen(file2), the hh_server_status diagnostic should disappear from file1",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [],
                    "isStatusFB": True,
                },
            )
            .wait_for_notification(
                comment="After didOpen(file2), the hh_server_status diagnostic should reappear in file2",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${unsaved2_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 0, "character": 0},
                                "end": {"line": 0, "character": 1},
                            },
                            "severity": 1,
                            "source": "hh_server",
                            "message": "hh_server isn't running, so there may be undetected errors. Try `hh` at the command line... hh_server: stopped.",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        }
                    ],
                    "isStatusFB": True,
                },
            )
            .notification(
                method="textDocument/didClose",
                params={"textDocument": {"uri": "${unsaved2_file_uri}"}},
            )
            .wait_for_notification(
                comment="After didClose(file2), the hh_server_status diagnostic should disappear from file2",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${unsaved2_file_uri}",
                    "diagnostics": [],
                    "isStatusFB": True,
                },
            )
            .wait_for_notification(
                comment="After didClose(file2), the hh_server_status diagnostic should reappear in file1",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 0, "character": 0},
                                "end": {"line": 0, "character": 1},
                            },
                            "severity": 1,
                            "source": "hh_server",
                            "message": "hh_server isn't running, so there may be undetected errors. Try `hh` at the command line... hh_server: stopped.",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        }
                    ],
                    "isStatusFB": True,
                },
            )
            .notification(
                method="textDocument/didClose",
                params={"textDocument": {"uri": "${php_file_uri}"}},
            )
            .wait_for_notification(
                comment="After didClose(file1), the hh_server_status diagnostic should disappear from file1",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [],
                    "isStatusFB": True,
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def _sanitize_gutter_line_numbers(self, s: str) -> str:
        gutter_line_number_re = re.compile(r"^[ ]*[0-9]+ \|", re.MULTILINE)
        return re.sub(gutter_line_number_re, " XXXX |", s)

    def test_lsptestspec_incorrect_request_result(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("test_lsptestspec_incorrect_request_result"),
                use_serverless_ide=True,
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
                line=line(),
                comment="hover over function invocation",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 16},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "int"},
                        "INCORRECT COMMENT HERE",
                    ],
                    "range": {
                        "start": {"line": 3, "character": 9},
                        "end": {"line": 3, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        try:
            self.run_spec(
                spec,
                variables=variables,
                wait_for_server=False,
                use_serverless_ide=True,
            )
            raise AssertionError("Expected an error here")
        except AssertionError as e:
            self.assertEqual(
                self._sanitize_gutter_line_numbers(str(e)),
                """\
Test case test_lsptestspec_incorrect_request_result failed with 1 errors:

Error 1/1:
Description: Request with ID 5 (comment: 'hover over function invocation') \
got an incorrect result:

(+ is expected lines, - is actual lines)
- {'contents': [{'language': 'hack', 'value': 'int'},
+ {'contents': [{'language': 'hack', 'value': 'int'}, 'INCORRECT COMMENT HERE'],
?                                                    +++++++++++++++++++++++++++

-               'A comment describing b_hover.'],
   'range': {'end': {'character': 16, 'line': 3},
             'start': {'character': 9, 'line': 3}}}

Context:
This was the associated request:

hphp/hack/test/integration/test_lsp.py
 XXXX |             .request(
 XXXX |                 line=line(),
 XXXX |                 comment="hover over function invocation",
 XXXX |                 method="textDocument/hover",
 XXXX |                 params={
 XXXX |                     "textDocument": {"uri": "${php_file_uri}"},
 XXXX |                     "position": {"line": 3, "character": 16},
 XXXX |                 },
 XXXX |                 result={
 XXXX |                     "contents": [
 XXXX |                         {"language": "hack", "value": "int"},
 XXXX |                         "INCORRECT COMMENT HERE",
 XXXX |                     ],
 XXXX |                     "range": {
 XXXX |                         "start": {"line": 3, "character": 9},
 XXXX |                         "end": {"line": 3, "character": 16},
 XXXX |                     },
 XXXX |                 },
 XXXX |                 powered_by="serverless_ide",
 XXXX |             )

Remediation:
1) If this was unexpected, then the language server is buggy and should be
fixed.

2) If this was expected, you can update your request with the following code to
make it match:

    .request(
        line=line(),
        comment='hover over function invocation',
        method='textDocument/hover',
        params={'textDocument': {'uri': '${php_file_uri}'}, \
'position': {'line': 3, 'character': 16}},
        result={'contents': [{'language': 'hack', 'value': 'int'}, \
'A comment describing b_hover.'], \
'range': {'start': {'line': 3, 'character': 9}, \
'end': {'line': 3, 'character': 16}}},
        powered_by='serverless_ide',
    )

If you want to examine the raw LSP logs, you can check the `.sent.log` and
`.received.log` files that were generated in the template repo for this test.\
""",
            )

    def test_lsptestspec_unexpected_notification(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("didchange.php")
        spec = (
            self.initialize_spec(LspTestSpec("did_change"), use_serverless_ide=False)
            .wait_for_hh_server_ready()
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
            .notification(
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 7, "character": 11},
                                "end": {"line": 7, "character": 12},
                            },
                            "text": "a",
                        }
                    ],
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 7, "character": 11},
                                "end": {"line": 7, "character": 11},
                            },
                            "severity": 1,
                            "code": 1002,
                            "source": "Hack",
                            "message": "A semicolon ; is expected here.",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        try:
            self.run_spec(
                spec, variables, wait_for_server=True, use_serverless_ide=False
            )
            raise AssertionError("Expected an error here")
        except AssertionError as e:
            self.assertEqual(
                self._sanitize_gutter_line_numbers(str(e)),
                """\
Test case did_change failed with 1 errors:

Error 1/1:
Description: An unexpected notification of type \
'textDocument/publishDiagnostics' was sent by the language server.
Here is the notification payload:

  {'jsonrpc': '2.0',
   'method': 'textDocument/publishDiagnostics',
   'params': {'diagnostics': [],
              'uri': '__PHP_FILE_URI__'}}

Context:
This was the most recent request issued from the language client before it
received the notification:

hphp/hack/test/integration/test_lsp.py
 XXXX |             .request(line=line(), method="shutdown", params={}, result=None)

Remediation:
1) If this was unexpected, then the language server is buggy and should be
fixed.

2) If all notifications of type 'textDocument/publishDiagnostics' should be \
ignored, add this directive
anywhere in your test:

    .ignore_notifications(method='textDocument/publishDiagnostics')

3) If this single instance of the notification was expected, add this directive
to your test to wait for it before proceeding:

    .wait_for_notification(
        method='textDocument/publishDiagnostics',
        params={'uri': '${php_file_uri}', 'diagnostics': []},
    )

If you want to examine the raw LSP logs, you can check the `.sent.log` and
`.received.log` files that were generated in the template repo for this test.\
"""
                # There's an instance of a literal `${php_file_uri}` in there
                # which we don't want to change, so use a different name than
                # that one.
                .replace("__PHP_FILE_URI__", variables["php_file_uri"]),
            )

    def test_serverless_ide_highlight(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("highlight.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_highlight"), use_serverless_ide=True
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
                line=line(),
                comment="document highlight, id 2",
                method="textDocument/documentHighlight",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 10},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 3, "character": 9},
                            "end": {"line": 3, "character": 20},
                        }
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="shutdown, id 3",
                method="shutdown",
                params={},
                result=None,
            )
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_coverage(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("coverage.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_coverage"), use_serverless_ide=True
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
                line=line(),
                comment="Check type coverage",
                method="textDocument/typeCoverage",
                params={"textDocument": {"uri": "${php_file_uri}"}},
                result={
                    "coveredPercent": 0,
                    "uncoveredRanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 12},
                                "end": {"line": 3, "character": 17},
                            }
                        },
                        {
                            "range": {
                                "start": {"line": 3, "character": 8},
                                "end": {"line": 3, "character": 10},
                            }
                        },
                        {
                            "range": {
                                "start": {"line": 3, "character": 2},
                                "end": {"line": 3, "character": 5},
                            }
                        },
                    ],
                    "defaultMessage": "Un-type checked code. Consider adding type annotations.",
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Shutdown",
                method="shutdown",
                params={},
                result=None,
            )
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_status_stopped(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("status_stopped"),
                use_serverless_ide=False,
                supports_status=True,
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "shortMessage": "Hack: stopped",
                    "message": "hh_server: stopped.",
                    "actions": [{"title": "Restart hh_server"}],
                    "type": 1,
                },
                result=NoResponse(),
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=False)

    def test_status_running(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover.php"))

        spec = (
            self.initialize_spec(
                LspTestSpec("status_running"),
                use_serverless_ide=False,
                supports_status=True,
            )
            .ignore_requests(
                comment="Ignore initializing... requests since they're racy",
                method="window/showStatus",
                params={
                    "type": 2,
                    "shortMessage": "Hack: initializing",
                    "message": "hh_server initializing: processing [<test> seconds]",
                    "actions": [],
                },
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={"actions": [], "message": "hh_server: ready.", "type": 3},
                result=NoResponse(),
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=True, use_serverless_ide=False)

    def test_serverless_ide_status_stopped(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover.php"))
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_status_stopped"),
                use_serverless_ide=True,
                supports_status=True,
            )
            .ignore_requests(
                comment="ignore initializing... messages since they're kind of racy",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [{"title": "Restart hh_server"}],
                    "message": "Hack IDE: initializing.\nhh_server: stopped.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="another racy initialization to ignore, before hh_server has even reported its status",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [],
                    "message": "Hack IDE: initializing.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="another racy initialization to ignore, again before hh_server",
                method="window/showStatus",
                params={
                    "type": 3,
                    "actions": [],
                    "message": "Hack IDE: ready.",
                    "shortMessage": "Hack: ready",
                },
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "message": "Hack IDE: ready.\nhh_server: stopped.",
                    "shortMessage": "Hack: ready",
                    "actions": [{"title": "Restart hh_server"}],
                    "type": 3,
                },
                result=NoResponse(),
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_status_restart(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover.php"))

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_status_restart"),
                use_serverless_ide=True,
                supports_status=True,
            )
            .ignore_requests(
                comment="Ignore initializing messages since they're racy",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [],
                    "message": "Hack IDE: initializing.\nhh_server initializing: processing [<test> seconds]",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="Another form of initializing to ignore",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [],
                    "message": "Hack IDE: initializing.\nhh_server: ready.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="Another form of initializing to ignore before we've even heard the first peep from hh_server",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [],
                    "message": "Hack IDE: initializing.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="another racy initialization to ignore, again before hh_server",
                method="window/showStatus",
                params={
                    "type": 3,
                    "actions": [],
                    "message": "Hack IDE: ready.",
                    "shortMessage": "Hack: ready",
                },
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "actions": [],
                    "message": "Hack IDE: ready.\nhh_server: ready.",
                    "shortMessage": "Hack: ready",
                    "type": 3,
                },
                result=NoResponse(),
            )
            .request(
                line=line(),
                method="$test/shutdownServerlessIde",
                params={},
                result=None,
                powered_by="serverless_ide",
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "actions": [{"title": "Restart Hack IDE"}],
                    "message": "Hack IDE has failed. See Output›Hack for details.\nhh_server: ready.",
                    "shortMessage": "Hack: failed",
                    "type": 1,
                },
                result={"title": "Restart Hack IDE"},
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "actions": [],
                    "message": "Hack IDE: ready.\nhh_server: ready.",
                    "shortMessage": "Hack: ready",
                    "type": 3,
                },
                result=NoResponse(),
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=True, use_serverless_ide=True)

    def test_serverless_ide_failed_to_load_saved_state(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("hover.php"))
        assert "naming_table_saved_state_path" in variables
        variables["naming_table_saved_state_path"] = "/tmp/nonexistent"

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_status_failed_to_load_saved_state"),
                use_serverless_ide=True,
                supports_status=True,
                supports_init=True,
            )
            .ignore_requests(
                comment="Ignore initializing since they're kind of racy",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [],
                    "message": "Hack IDE: initializing.\nhh_server initializing: processing [<test> seconds]",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="Ignore another form of initializing",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [],
                    "message": "Hack IDE: initializing.\nhh_server: ready.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="Ignore another form of initializing, from before we've even heard the first peep out of hh_server",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [],
                    "message": "Hack IDE: initializing.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="Ignore another form of initializing, again before hh_server",
                method="window/showStatus",
                params={
                    "type": 1,
                    "actions": [{"title": "Restart Hack IDE"}],
                    "message": "Hack IDE has failed. See Output›Hack for details.",
                    "shortMessage": "Hack: failed",
                },
            )
            .wait_for_notification(
                method="window/logMessage",
                params={
                    "type": 1,
                    "message": "Hack IDE has failed.\nThis is unexpected.\nPlease file a bug within your IDE.\nMore details: http://dummy/HH_TEST_MODE",
                },
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "actions": [{"title": "Restart Hack IDE"}],
                    "message": "Hack IDE has failed. See Output›Hack for details.\nhh_server: ready.",
                    "shortMessage": "Hack: failed",
                    "type": 1,
                },
                result=NoResponse(),
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=True, use_serverless_ide=True)

    def test_workspace_symbol(self) -> None:
        self.prepare_server_environment()
        variables = self.setup_php_file("didchange.php")
        spec = (
            self.initialize_spec(
                LspTestSpec("test_workspace_symbol"), use_serverless_ide=False
            )
            .wait_for_hh_server_ready()
            .request(
                line=line(),
                comment="Look up symbols",
                method="workspace/symbol",
                params={"query": "TestNS\\test"},
                result=[
                    {
                        "name": "TestNS\\test_func",
                        "kind": 12,
                        "location": {
                            "uri": "file://${root_path}/completion_extras_namespace.php",
                            "range": {
                                "start": {"line": 4, "character": 9},
                                "end": {"line": 4, "character": 25},
                            },
                        },
                    }
                ],
            )
            .request(
                line=line(),
                comment="Look up symbols starting with 'test_f' within multiple namespaces",
                method="workspace/symbol",
                params={"query": "test_f"},
                result=[
                    {
                        "name": "test_function",
                        "kind": 12,
                        "location": {
                            "uri": "file://${root_path}/completion.php",
                            "range": {
                                "start": {"line": 7, "character": 9},
                                "end": {"line": 7, "character": 22},
                            },
                        },
                    },
                    {
                        "name": "TestNS\\test_func",
                        "kind": 12,
                        "location": {
                            "uri": "file://${root_path}/completion_extras_namespace.php",
                            "range": {
                                "start": {"line": 4, "character": 9},
                                "end": {"line": 4, "character": 25},
                            },
                        },
                    },
                ],
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=True, use_serverless_ide=False)

    def test_serverless_ide_during_hh_server_restart(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("didchange.php"))
        spec = (
            self.initialize_spec(
                LspTestSpec("test_serverless_ide_during_hh_server_restart"),
                use_serverless_ide=True,
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
            .notification(
                comment="Send a 'didChange' notification before HH Server is functional.",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 7, "character": 9},
                                "end": {"line": 7, "character": 11},
                            },
                            "text": "'foo'",
                        }
                    ],
                },
            )
            .start_hh_server("Start HH Server; should detect the bad edit")
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "code": 4110,
                            "message": "Invalid return type",
                            "range": {
                                "end": {"character": 14, "line": 7},
                                "start": {"character": 9, "line": 7},
                            },
                            "relatedInformation": [
                                {
                                    "location": {
                                        "range": {
                                            "end": {"character": 27, "line": 6},
                                            "start": {"character": 24, "line": 6},
                                        },
                                        "uri": "${php_file_uri}",
                                    },
                                    "message": "Expected int",
                                },
                                {
                                    "location": {
                                        "range": {
                                            "end": {"character": 14, "line": 7},
                                            "start": {"character": 9, "line": 7},
                                        },
                                        "uri": "${php_file_uri}",
                                    },
                                    "message": "But got string",
                                },
                            ],
                            "relatedLocations": [
                                {
                                    "location": {
                                        "range": {
                                            "end": {"character": 27, "line": 6},
                                            "start": {"character": 24, "line": 6},
                                        },
                                        "uri": "${php_file_uri}",
                                    },
                                    "message": "Expected int",
                                },
                                {
                                    "location": {
                                        "range": {
                                            "end": {"character": 14, "line": 7},
                                            "start": {"character": 9, "line": 7},
                                        },
                                        "uri": "${php_file_uri}",
                                    },
                                    "message": "But got string",
                                },
                            ],
                            "severity": 1,
                            "source": "Hack",
                        }
                    ],
                },
            )
            .stop_hh_server("Shutdown HH Server")
            .start_hh_server("Restart HH Server")
            .wait_for_notification(
                comment="On startup it thinks everything is okay ...",
                method="textDocument/publishDiagnostics",
                params={"uri": "${php_file_uri}", "diagnostics": []},
            )
            .wait_for_notification(
                comment="But then hh_server sends a hello message and it gets the edited files, which leads it to see the problem.",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "code": 4110,
                            "message": "Invalid return type",
                            "range": {
                                "end": {"character": 14, "line": 7},
                                "start": {"character": 9, "line": 7},
                            },
                            "relatedInformation": [
                                {
                                    "location": {
                                        "range": {
                                            "end": {"character": 27, "line": 6},
                                            "start": {"character": 24, "line": 6},
                                        },
                                        "uri": "${php_file_uri}",
                                    },
                                    "message": "Expected int",
                                },
                                {
                                    "location": {
                                        "range": {
                                            "end": {"character": 14, "line": 7},
                                            "start": {"character": 9, "line": 7},
                                        },
                                        "uri": "${php_file_uri}",
                                    },
                                    "message": "But got string",
                                },
                            ],
                            "relatedLocations": [
                                {
                                    "location": {
                                        "range": {
                                            "end": {"character": 27, "line": 6},
                                            "start": {"character": 24, "line": 6},
                                        },
                                        "uri": "${php_file_uri}",
                                    },
                                    "message": "Expected int",
                                },
                                {
                                    "location": {
                                        "range": {
                                            "end": {"character": 14, "line": 7},
                                            "start": {"character": 9, "line": 7},
                                        },
                                        "uri": "${php_file_uri}",
                                    },
                                    "message": "But got string",
                                },
                            ],
                            "severity": 1,
                            "source": "Hack",
                        }
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=True, use_serverless_ide=True)

    def test_serverless_ide_naming_error1(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables.update(self.setup_php_file("didchange.php"))
        variables.update(
            {
                "main_file": self.repo_file("main.php"),
                "main_file_contents": """\
<?hh
function main(): int {
    return aaa();
}
""",
                "file_a": self.repo_file("a.php"),
                "file_b": self.repo_file("b.php"),
            }
        )
        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_naming_error1"), use_serverless_ide=True
            )
            .write_to_disk(
                uri="${main_file}", contents="${main_file_contents}", notify=True
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${main_file}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${main_file_contents}",
                    }
                },
            )
            .request(
                line=line(),
                comment="Ensure that hover over `aaa` works even when the name is not yet defined",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${main_file}"},
                    "position": {"line": 2, "character": 13},
                },
                result={
                    "contents": [{"language": "hack", "value": "_"}],
                    "range": {
                        "start": {"line": 2, "character": 11},
                        "end": {"line": 2, "character": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                comment="create file A",
                uri="${file_a}",
                contents="""\
<?hh
function aaa(): int {
    return 1;
}
""",
                notify=True,
            )
            .request(
                line=line(),
                comment="Ensure that hover over `aaa` works when there are no naming errors",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${main_file}"},
                    "position": {"line": 2, "character": 13},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function aaa(): int"},
                        "Return type: `int`",
                    ],
                    "range": {
                        "start": {"line": 2, "character": 11},
                        "end": {"line": 2, "character": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                comment="create file B",
                uri="${file_b}",
                contents="""\
<?hh
function aaa(): string {
    return "foo";
}
""",
                notify=True,
            )
            .request(
                line=line(),
                comment="Ensure that hover over `aaa` works even when there is a duplicate name",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${main_file}"},
                    "position": {"line": 2, "character": 13},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function aaa(): int"},
                        "Return type: `int`",
                    ],
                    "range": {
                        "start": {"line": 2, "character": 11},
                        "end": {"line": 2, "character": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                comment="delete file A", uri="${file_a}", contents=None, notify=True
            )
            .request(
                line=line(),
                comment="Now that we've fixed the error, hover should work.",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${main_file}"},
                    "position": {"line": 2, "character": 13},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function aaa(): string"},
                        "Return type: `string`",
                    ],
                    "range": {
                        "start": {"line": 2, "character": 11},
                        "end": {"line": 2, "character": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_naming_error2(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        self.test_driver.stop_hh_server()
        variables.update(self.setup_php_file("naming_error_caller.php"))
        variables.update(
            {
                "contents": self.read_repo_file("naming_error_declaration.php"),
                "original": self.repo_file("naming_error_declaration.php"),
                "copy": self.repo_file("naming_error_copy.php"),
            }
        )
        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_naming_error2"), use_serverless_ide=True
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
            .write_to_disk(
                comment="create copy",
                uri="${copy}",
                contents="${contents}",
                notify=True,
            )
            .write_to_disk(
                comment="delete copy", uri="${copy}", contents=None, notify=True
            )
            .request(
                line=line(),
                comment="hover should work fine after making copy then deleting copy.",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 15},
                },
                result={
                    "contents": [
                        {
                            "language": "hack",
                            "value": "function naming_error_declaration(): void",
                        },
                        "Return type: `void`",
                    ],
                    "range": {
                        "start": {"line": 3, "character": 2},
                        "end": {"line": 3, "character": 26},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_naming_error3(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        self.test_driver.stop_hh_server()
        variables.update(self.setup_php_file("naming_error_caller.php"))
        variables.update(
            {
                "contents": self.read_repo_file("naming_error_declaration.php"),
                "original": self.repo_file("naming_error_declaration.php"),
                "copy": self.repo_file("naming_error_copy.php"),
            }
        )
        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_naming_error3"), use_serverless_ide=True
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
            .write_to_disk(
                comment="create copy",
                uri="${copy}",
                contents="${contents}",
                notify=True,
            )
            .write_to_disk(
                comment="delete original", uri="${original}", contents=None, notify=True
            )
            .request(
                line=line(),
                comment="hover should work fine after making copy then deleting original.",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 15},
                },
                result={
                    "contents": [
                        {
                            "language": "hack",
                            "value": "function naming_error_declaration(): void",
                        },
                        "Return type: `void`",
                    ],
                    "range": {
                        "start": {"line": 3, "character": 2},
                        "end": {"line": 3, "character": 26},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_requests_before_init(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables["root_path"] = self.test_driver.repo_dir
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("test_serverless_ide_requests_before_init"),
                use_serverless_ide=True,
                supports_status=True,
                supports_init=True,
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .ignore_requests(
                comment="Ignore 'initializing...' messages since they're racy",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [{"title": "Restart hh_server"}],
                    "message": "Hack IDE: initializing.\nhh_server: stopped.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="another racy initialization, before we've yet heard from hh_server",
                method="window/showStatus",
                params={
                    "type": 2,
                    "actions": [],
                    "message": "Hack IDE: initializing.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                comment="another racy initialization, if HackIDE is done before hh_server has yet sent status",
                method="window/showStatus",
                params={
                    "type": 3,
                    "actions": [],
                    "message": "Hack IDE: ready.",
                    "shortMessage": "Hack: ready",
                },
            )
            .write_to_disk(
                notify=True,
                wait=False,
                uri="file://${root_path}/beforeInit1.php",
                contents="<?hh // strict\nfunction beforeInit1(): int {\n  return 42;\n}\n",
            )
            .notification(
                comment="open a file before init has finished",
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "file://${root_path}/beforeInit2.php",
                        "languageId": "hack",
                        "version": 1,
                        "text": "<?hh // strict\nfunction beforeInit2(): void {\n  $foo = beforeInit1();\n}\n",
                    }
                },
            )
            .request(
                line=line(),
                comment="hover before init will fail",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "file://${root_path}/beforeInit2.php"},
                    "position": {"line": 2, "character": 4},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="documentSymbol before init will succeed",
                method="textDocument/documentSymbol",
                params={"textDocument": {"uri": "file://${root_path}/beforeInit2.php"}},
                result=[
                    {
                        "name": "beforeInit2",
                        "kind": 12,
                        "location": {
                            "uri": "file://${root_path}/beforeInit2.php",
                            "range": {
                                "start": {"line": 1, "character": 0},
                                "end": {"line": 3, "character": 1},
                            },
                        },
                    }
                ],
                powered_by="serverless_ide",
            )
            .wait_for_notification(
                comment="wait for sIDE to init",
                method="telemetry/event",
                params={"type": 4, "message": "[client-ide] Finished init: ok"},
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "actions": [{"title": "Restart hh_server"}],
                    "message": "Hack IDE: ready.\nhh_server: stopped.",
                    "shortMessage": "Hack: ready",
                    "type": 3,
                },
                result=NoResponse(),
            )
            .request(
                line=line(),
                comment="hover after init will succeed",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "file://${root_path}/beforeInit2.php"},
                    "position": {"line": 2, "character": 4},
                },
                result={
                    "contents": [{"language": "hack", "value": "int"}],
                    "range": {
                        "start": {"line": 2, "character": 2},
                        "end": {"line": 2, "character": 6},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)

    def test_serverless_ide_workspace_symbol(self) -> None:
        variables = dict(self.prepare_serverless_ide_environment())
        variables["root_path"] = self.test_driver.repo_dir
        self.test_driver.stop_hh_server()

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_workspace_symbol"), use_serverless_ide=True
            )
            .request(
                line=line(),
                comment="workspace symbol call, global, powered by sqlite (generated during serverless-ide-init)",
                method="workspace/symbol",
                params={"query": "TakesString"},
                result=[
                    {
                        "name": "TakesString",
                        "kind": 5,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 36, "character": 6},
                                "end": {"line": 36, "character": 17},
                            },
                        },
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="workspace symbol call, member (derived from naming-table)",
                method="workspace/symbol",
                params={"query": "TakesString::"},
                result=[
                    {
                        "name": "__construct",
                        "kind": 6,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 37, "character": 18},
                                "end": {"line": 37, "character": 29},
                            },
                        },
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, wait_for_server=False, use_serverless_ide=True)
