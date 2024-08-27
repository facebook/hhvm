# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
# pyre-strict
# flake8: noqa: B950

from __future__ import absolute_import, division, print_function, unicode_literals

import copy
import json
import os
import sys
import unittest
import urllib.parse
from typing import Dict, Iterable, List, Mapping, Optional, Tuple

import hphp.hack.test.integration.common_tests as common_tests
from hphp.hack.test.integration.hh_paths import hh_server
from hphp.hack.test.integration.lspcommand import LspCommandProcessor, Transcript
from hphp.hack.test.integration.lsptestspec import line, LspTestSpec
from hphp.hack.test.integration.test_case import TestCase
from hphp.hack.test.integration.utils import interpolate_variables, Json, JsonObject


class LspTestDriver(common_tests.CommonTestDriver):
    def write_load_config(
        self,
        use_saved_state: bool = False,
    ) -> None:
        # Will use the .hhconfig already in the repo directory
        # As for hh.conf, we'll write it explicitly each test.
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
lazy_init2 = {use_saved_state}
ide_symbolindex_search_provider = LocalIndex
allow_unstable_features = true
""".format(
                    use_saved_state=str(use_saved_state).lower(),
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


class LspTestBase(TestCase[LspTestDriver]):
    """
    Do not add any tests to this class, it is only for
    providing set-up and helpers for lsp-level tests
    """

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
                    del response["error"]["data"]
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
                "textDocument/publishDiagnostics",
                "client/registerCapability",
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
        lsp_args: List[str],
    ) -> None:
        with LspCommandProcessor.create(
            self.test_driver.test_env, lsp_args, self.test_driver.repo_dir
        ) as lsp:
            observed_transcript = lsp.communicate(test)

        self.write_observed(test_name, observed_transcript)

        expected_items = self.prepare_responses(expected)
        observed_items = self.prepare_responses(
            list(self.get_important_received_items(observed_transcript))
        )

        # If the server's busy, maybe the machine's just under too much
        # pressure to give results in a timely fashion. Doing a retry would
        # only defer the question of what to do in that case, so instead
        # we'll just skip.
        self.throw_skip_if_transcript_includes_server_busy(observed_transcript)

        # validation checks that the number of items matches and that
        # the responses are exactly identical to what we expect.
        # Python equality on lists requires identical lengths,
        # identical order, and does == on each element...
        if expected_items != observed_items:
            msg = "Observed this:\n" + json.dumps(
                observed_transcript, indent=2, separators=(",", ": "), sort_keys=True
            )
            while (
                expected_items
                and observed_items
                and expected_items[0] == observed_items[0]
            ):
                expected_items.pop(0)
                observed_items.pop(0)
            msg += (
                f"\n\nIt first went wrong here...\n"
                f"Expected:\n{expected_items[0] if expected_items else '[none]'}\n\n"
                f"Observed:\n{observed_items[0] if observed_items else '[none]'}\n"
            )
            raise AssertionError(msg)

    def throw_skip_if_transcript_includes_server_busy(
        self, transcript: Transcript
    ) -> None:
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

    def write_hhconf_and_naming_table(self) -> Dict[str, str]:
        self.maxDiff = None
        self.test_driver.write_load_config(use_saved_state=False)
        naming_table_saved_state_path = (
            self.test_driver.write_naming_table_saved_state()
        )
        return dict(
            {
                "naming_table_saved_state_path": naming_table_saved_state_path,
                "root_path": self.test_driver.repo_dir,
            }
        )

    def load_and_run(
        self,
        test_name: str,
        variables: Mapping[str, str],
        lsp_args: Optional[List[str]] = None,
    ) -> None:
        test, expected = self.load_test_data(test_name, variables)
        if lsp_args is None:
            lsp_args = []
        self.run_lsp_test(
            test_name=test_name,
            test=test,
            expected=expected,
            lsp_args=lsp_args,
        )

    def run_spec(
        self,
        spec: LspTestSpec,
        variables: Mapping[str, str],
        fall_back_to_full_index: bool = True,
        lsp_extra_args: List[str] = [],
    ) -> None:
        lsp_args = [
            "--config",
            f"ide_fall_back_to_full_index={str(fall_back_to_full_index).lower()}",
        ] + lsp_extra_args
        with LspCommandProcessor.create(
            self.test_driver.test_env, lsp_args, self.test_driver.repo_dir
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

        # If the server's busy, maybe the machine's just under too much
        # pressure to give results in a timely fashion. Doing a retry would
        # only defer the question of what to do in that case, so instead
        # we'll just skip.
        self.throw_skip_if_transcript_includes_server_busy(observed_transcript)

        if error_details is not None:
            logs = self.test_driver.get_all_logs(self.test_driver.repo_dir)
            print("CLIENT_LOG:\n%s\n\n" % logs.client_log, file=sys.stderr)
            print("IDE_LOG:\n%s\n\n" % logs.ide_log, file=sys.stderr)
            print("LSP_LOG:\n%s\n\n" % logs.lsp_log, file=sys.stderr)
            raise AssertionError(error_details)

    def setup_php_file(self, test_php: str) -> Mapping[str, str]:
        return {
            "php_file_uri": self.repo_file_uri(test_php),
            "php_file": self.read_repo_file(test_php),
        }

    def initialize_spec(
        self,
        spec: LspTestSpec,
        has_status_capability: bool = False,  # do we tell the server that we have the "status" capability, i.e. want to receive window/showStatus?
        wait_for_init_done: bool = True,  # do we wish to wait for init to be done before the test starts?
    ) -> LspTestSpec:
        initialization_options = {
            "namingTableSavedStatePath": "${naming_table_saved_state_path}",
            "namingTableSavedStateTestDelay": 0.0,
        }
        if not wait_for_init_done:
            # A small delay, since otherwise init completes immediately
            # This isn't very racy. All we need is a tiny delay so that
            # other things which are in the queue get processed, rather
            # than continuing synchronously
            initialization_options["namingTableSavedStateTestDelay"] = 0.5

        window_capabilities = {}
        if has_status_capability:
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
                        "willSaveWaitUntil": True,
                        "save": {"includeText": False},
                    },
                    "hoverProvider": True,
                    "completionProvider": {
                        "resolveProvider": True,
                        "triggerCharacters": [
                            "$",
                            ">",
                            "\\",
                            ":",
                            "<",
                            "[",
                            "'",
                            '"',
                            "{",
                            "#",
                        ],
                    },
                    "signatureHelpProvider": {"triggerCharacters": ["(", ","]},
                    "definitionProvider": True,
                    "typeDefinitionProvider": True,
                    "referencesProvider": True,
                    "documentHighlightProvider": True,
                    "documentSymbolProvider": True,
                    "workspaceSymbolProvider": True,
                    "codeActionProvider": {"resolveProvider": True},
                    "documentFormattingProvider": True,
                    "documentRangeFormattingProvider": True,
                    "documentOnTypeFormattingProvider": {
                        "firstTriggerCharacter": ";",
                        "moreTriggerCharacter": ["}"],
                    },
                    "renameProvider": True,
                    "implementationProvider": True,
                    "rageProvider": True,
                    "experimental": {"snippetTextEdit": True, "autoCloseJsx": True},
                }
            },
        )
        spec = spec.wait_for_server_request(
            method="client/registerCapability",
            params={
                "registrations": [
                    {
                        "id": "did-change-watched-files",
                        "method": "workspace/didChangeWatchedFiles",
                        "registerOptions": {
                            "watchers": [
                                {
                                    "globPattern": "**/*.{php,phpt,hack,hackpartial,hck,hh,hhi,xhp}",
                                    "kind": 7,
                                }
                            ]
                        },
                    }
                ]
            },
            result=None,
        )

        if wait_for_init_done:
            spec = spec.wait_for_notification(
                comment="wait for clientIdeDaemon to finish init",
                method="telemetry/event",
                params={"type": 4, "message": "[client-ide] Finished init: ok"},
            )

        return spec
