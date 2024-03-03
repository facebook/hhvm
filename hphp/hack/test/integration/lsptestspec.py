# pyre-strict
from __future__ import absolute_import, division, print_function, unicode_literals

import copy
import difflib
import inspect
import itertools
import os.path
import pprint
import textwrap
from dataclasses import dataclass
from typing import (
    AbstractSet,
    Iterable,
    List,
    Mapping,
    Optional,
    Sequence,
    Tuple,
    Union,
)

import libcst
from libcst.metadata import CodeRange, MetadataWrapper, PositionProvider
from lspcommand import LspCommandProcessor, Transcript, TranscriptEntry
from utils import (
    fixup_hhi_json,
    interpolate_variables,
    Json,
    uninterpolate_variables,
    VariableMap,
)


_MessageSpec = Union[
    "_RequestSpec",
    "_DebugRequestSpec",
    "_NotificationSpec",
    "_WaitForNotificationSpec",
    "_WaitForRequestSpec",
    "_WaitForResponseSpec",
    "_WaitForHhServerReadySpec",
]


# pyre-fixme[5]: Global expression must be annotated.
_LspIdMap = Mapping[_MessageSpec, Json]

_Traceback = Sequence[inspect.FrameInfo]


@dataclass
class _CallSiteInfo:
    line_num: int
    traceback: _Traceback


class NoResponse:
    """Indicates that no response should be sent (different from `None` since
    `None` is a valid JSON value)."""

    pass


def line() -> int:
    """Get the line number that this function was called at.

    Previously, we used to do this automatically whenever we called
    `.request`. However, a recent upgrade of Python breaks that functionality
    for chained function calls in some cases, and it instead reports the line
    number of the first function call in the chain. We use `line()` to ensure
    that we don't have a chained function call and can get the line number
    accurately.
    """
    cf = inspect.currentframe()
    assert (
        cf is not None
    ), "Must be able to get current call frame to produce error messages for test"
    # pyre-fixme[16]: `Optional` has no attribute `f_lineno`.
    return cf.f_back.f_lineno


class LspTestSpec:
    """Represents an LSP test to be run, in a declarative fashion.

    Since `LspTestSpec`s are just values, they can be composed using regular
    functions. For example, you can make an `initialize_spec` function that
    returns an `LspTestSpec` with the `initialize` request already sent and
    checked."""

    def __init__(self, name: str) -> None:
        self.name = name
        self._messages: Sequence["_MessageSpec"] = []
        self._ignored_notification_methods: AbstractSet[str] = set()
        # pyre-fixme[11]: Annotation `Json` is not defined as a type.
        self._ignored_requests: Sequence[Tuple[str, Optional[Json]]] = []

    def ignore_notifications(self, *, method: str) -> "LspTestSpec":
        """For example .ignore_notifications(method="textDocument/publishDiagnostics") --
        normally an unexpected notification from the LSP server would result in test failure,
        but this directive means that unexpected notifications with this exact method name do not.
        """
        ignored_notification_methods = set(self._ignored_notification_methods)
        ignored_notification_methods.add(method)
        return self._update(ignored_notification_methods=ignored_notification_methods)

    def ignore_requests(
        self, *, method: str, params: Optional[Json], comment: Optional[str] = None
    ) -> "LspTestSpec":
        """For example .ignore_requests(comment="for_tester", method="window/showStatus", params={"type":2}) --
        normally an unexpected request from the LSP server would result in test failure,
        but this directive means that unexpected requests with this exact method name and params do not.
        If you pass params=None then unexpected requests with this exact method name and *any* params do not.
        Typically used for window/showStatus messages!"""
        ignored_requests = list(self._ignored_requests)
        ignored_requests.append((method, params))
        return self._update(ignored_requests=ignored_requests)

    def request(
        self,
        line: int,
        method: str,
        params: Json,
        *,
        result: Json,
        wait_id: Optional[str] = None,
        comment: Optional[str] = None,
        powered_by: Optional[str] = None,
    ) -> "LspTestSpec":
        """Add a request to this test's messages."""
        traceback = inspect.stack()
        assert traceback is not None, "Failed to get traceback info"

        messages = list(self._messages)
        if wait_id is not None and any(
            isinstance(message, _RequestSpec) and message.wait_id == wait_id
            for message in messages
        ):
            raise ValueError(f"Duplicate wait ID: {wait_id}")

        messages.append(
            _RequestSpec(
                method=method,
                params=params,
                result=result,
                wait_id=wait_id,
                comment=comment,
                powered_by=powered_by,
                call_site_info=_CallSiteInfo(line_num=line, traceback=traceback),
            )
        )
        return self._update(messages=messages)

    def debug(self) -> "LspTestSpec":
        """Issue a `telemetry/rage` request for debugging.

        The language server has to support the `telemetry/rage` request. Once
        the response is received, its debugging output is rendered in the test
        output. This can be useful when trying to debug the internal state of
        the language server.

        The test will not pass while there's a `debug()` statement in its spec,
        so it must be removed before committing the code.
        """
        messages = list(self._messages)
        messages.append(_DebugRequestSpec())
        return self._update(messages=messages)

    def notification(
        self, method: str, params: Json, *, comment: Optional[str] = None
    ) -> "LspTestSpec":
        messages = list(self._messages)
        messages.append(
            _NotificationSpec(method=method, params=params, comment=comment)
        )
        return self._update(messages=messages)

    def wait_for_server_request(
        self,
        method: str,
        params: Json,
        *,
        result: Union[Json, NoResponse],
        comment: Optional[str] = None,
    ) -> "LspTestSpec":
        messages = list(self._messages)
        messages.append(
            _WaitForRequestSpec(
                method=method, params=params, result=result, comment=comment
            )
        )
        return self._update(messages=messages)

    def wait_for_notification(
        self, method: str, params: Json, *, comment: Optional[str] = None
    ) -> "LspTestSpec":
        messages = list(self._messages)
        messages.append(
            _WaitForNotificationSpec(method=method, params=params, comment=comment)
        )
        return self._update(messages=messages)

    def wait_for_response(self, wait_id: str) -> "LspTestSpec":
        messages = list(self._messages)
        messages.append(_WaitForResponseSpec(wait_id=wait_id))
        return self._update(messages=messages)

    def wait_for_hh_server_ready(self) -> "LspTestSpec":
        messages = list(self._messages)
        messages.append(_WaitForHhServerReadySpec())
        return self._update(messages=messages)

    def start_hh_server(self, comment: str) -> "LspTestSpec":
        return self.request(
            line=line(),
            comment=comment,
            method="$test/startHhServer",
            params=None,
            result=None,
            powered_by="serverless_ide",
        )

    def stop_hh_server(self, comment: str) -> "LspTestSpec":
        return self.request(
            line=line(),
            comment=comment,
            method="$test/stopHhServer",
            params=None,
            result=None,
            powered_by="serverless_ide",
        )

    def write_to_disk(
        self,
        *,
        comment: Optional[str] = None,
        uri: str,
        contents: Optional[str],
        notify: bool,
    ) -> "LspTestSpec":
        """Write a file to disk in the middle of the LSP test.

        If `contents` is `None`, delete the file from disk.

        If `notify` is `True`, also send a `workspace/didChangeWatchedFiles`
        notification to the language server corresponding to the file you just
        changed.
        """
        messages = list(self._messages)
        messages.append(
            _NotificationSpec(
                method="$test/writeToDisk",
                params={"uri": uri, "contents": contents},
                comment=comment,
            )
        )
        if notify:
            messages.append(
                _NotificationSpec(
                    method="workspace/didChangeWatchedFiles",
                    params={"changes": [{"uri": uri, "type": 2}]},
                    comment=comment,
                )
            )
        return self._update(messages=messages)

    def run(
        self, lsp_command_processor: LspCommandProcessor, variables: VariableMap
    ) -> Tuple[Transcript, Optional[str]]:
        """Run the test given the LSP command processor.

        Raises an exception with useful debugging information if the test fails."""
        (json_commands, lsp_id_map) = self._get_json_commands(variables=variables)
        transcript = lsp_command_processor.communicate(json_commands=json_commands)
        errors = list(
            self._verify_transcript(
                variables=variables, transcript=transcript, lsp_id_map=lsp_id_map
            )
        )
        if errors:
            num_errors = len(errors)
            error_details = (
                f"Test case {self.name} failed with {num_errors} errors:\n\n"
            )
            for i, error in enumerate(errors, 1):
                error_details += f"Error {i}/{num_errors}:\n"
                error_details += str(error) + "\n"
            error_details += """\
If you want to examine the raw LSP logs, you can check the `.sent.log` and
`.received.log` files that were generated in the template repo for this test."""
        else:
            error_details = None
        return (transcript, error_details)

    ### Internal. ###

    def _update(
        self,
        messages: Optional[Sequence["_MessageSpec"]] = None,
        ignored_notification_methods: Optional[AbstractSet[str]] = None,
        ignored_requests: Optional[Sequence[Tuple[str, Json]]] = None,
    ) -> "LspTestSpec":
        spec = copy.copy(self)
        if messages is not None:
            spec._messages = messages
        if ignored_notification_methods is not None:
            spec._ignored_notification_methods = ignored_notification_methods
        if ignored_requests is not None:
            spec._ignored_requests = ignored_requests
        return spec

    def _get_json_commands(
        self,
        variables: VariableMap,
        # pyre-fixme[11]: Annotation `_LspIdMap` is not defined as a type.
    ) -> Tuple[Sequence[Json], "_LspIdMap"]:
        """Transforms this test spec into something the LSP command processor
        can interpret."""
        json_commands = []
        lsp_id_map = {}
        current_id = 0
        for message in self._messages:
            current_id += 1
            lsp_id_map[message] = current_id

            if isinstance(message, _RequestSpec):
                json_commands.append(
                    {
                        "jsonrpc": "2.0",
                        "comment": message.comment,
                        "id": current_id,
                        "method": message.method,
                        "params": interpolate_variables(
                            message.params, variables=variables
                        ),
                    }
                )

                if message.wait_id is None:
                    # Assume that if no wait ID was explicitly passed, we want
                    # to wait on the response before sending the next message.
                    json_commands.append(
                        {
                            "jsonrpc": "2.0",
                            "method": "$test/waitForResponse",
                            "params": {"id": current_id},
                        }
                    )
            elif isinstance(message, _DebugRequestSpec):
                json_commands.append(
                    {
                        "jsonrpc": "2.0",
                        "id": current_id,
                        "method": "telemetry/rage",
                        "params": {},
                    }
                )
            elif isinstance(message, _NotificationSpec):
                json_commands.append(
                    {
                        "jsonrpc": "2.0",
                        "comment": message.comment,
                        "method": message.method,
                        "params": interpolate_variables(
                            message.params, variables=variables
                        ),
                    }
                )
            elif isinstance(message, _WaitForRequestSpec):
                params = {
                    "method": message.method,
                    "params": interpolate_variables(
                        message.params, variables=variables
                    ),
                }
                if not isinstance(message.result, NoResponse):
                    params["result"] = message.result
                json_commands.append(
                    {
                        "jsonrpc": "2.0",
                        "comment": message.comment,
                        "method": "$test/waitForRequest",
                        "params": params,
                    }
                )
            elif isinstance(message, _WaitForNotificationSpec):
                json_commands.append(
                    {
                        "jsonrpc": "2.0",
                        "comment": message.comment,
                        "method": "$test/waitForNotification",
                        "params": {
                            "method": message.method,
                            "params": interpolate_variables(
                                message.params, variables=variables
                            ),
                        },
                    }
                )
            elif isinstance(message, _WaitForResponseSpec):
                lsp_ids = [
                    lsp_id
                    for previous_message, lsp_id in lsp_id_map.items()
                    if isinstance(previous_message, _RequestSpec)
                    and previous_message.wait_id == message.wait_id
                ]
                assert len(lsp_ids) == 1, (
                    f"Should have had exactly one previous message with wait ID {message.wait_id!r}, "  # noqa: B950
                    + "but got {len(lsp_ids)}"
                )
                [lsp_id] = lsp_ids

                json_commands.append(
                    {
                        "jsonrpc": "2.0",
                        "method": "$test/waitForResponse",
                        "params": {"id": lsp_id},
                    }
                )
            elif isinstance(message, _WaitForHhServerReadySpec):
                json_commands.append(
                    {
                        "jsonrpc": "2.0",
                        "method": "$test/waitForHhServerReady",
                        "params": {},
                    }
                )
            else:
                raise ValueError(f"unhandled message type {message.__class__.__name__}")
        return (json_commands, lsp_id_map)

    def _verify_transcript(
        self, *, variables: VariableMap, transcript: Transcript, lsp_id_map: "_LspIdMap"
    ) -> Iterable["_ErrorDescription"]:
        handled_entries = set()

        for message in self._messages:
            lsp_id = lsp_id_map[message]
            if isinstance(message, _RequestSpec):
                transcript_id = LspCommandProcessor._client_request_id(lsp_id)
                handled_entries.add(transcript_id)
                assert transcript_id in transcript, (
                    f"Expected message with ID {lsp_id!r} to have an entry in the transcript "
                    + f"under key {transcript_id!r}, "
                    + f"but it was not found. Transcript: {transcript!r}"
                )
                entry = transcript[transcript_id]
                error_description = self._verify_request(
                    variables=variables, entry=entry, lsp_id=lsp_id, request=message
                )
                if error_description is not None:
                    yield error_description
            elif isinstance(message, _DebugRequestSpec):
                transcript_id = LspCommandProcessor._client_request_id(lsp_id)
                handled_entries.add(transcript_id)
                assert transcript_id in transcript, (
                    f"Expected message with ID {lsp_id!r} to have an entry in the transcript "
                    + f"under key {transcript_id!r}, "
                    + f"but it was not found. Transcript: {transcript!r}"
                )
                entry = transcript[transcript_id]
                error_description = self._render_telemetry_rage(
                    debug_request=message, result=entry.received["result"]
                )
                yield error_description
            elif isinstance(message, _NotificationSpec):
                # Nothing needs to be done here, since we sent the notification
                # and don't expect a response.
                pass
            elif isinstance(
                message,
                (
                    _WaitForRequestSpec,
                    _WaitForNotificationSpec,
                    _WaitForResponseSpec,
                    _WaitForHhServerReadySpec,
                ),
            ):
                # Nothing needs to be done here -- if we failed to wait for the
                # message, an exception will have been thrown at the
                # `LspCommandProcessor` layer.
                pass
            else:
                raise ValueError(f"unhandled message type {message.__class__.__name__}")

        handled_entries |= set(self._find_ignored_transcript_ids(transcript))
        yield from self._flag_unhandled_messages(
            handled_entries, variables, transcript, lsp_id_map
        )

    def _verify_request(
        self,
        *,
        variables: VariableMap,
        lsp_id: Json,
        entry: TranscriptEntry,
        request: "_RequestSpec",
    ) -> Optional["_ErrorDescription"]:
        actual_result = entry.received.get("result")
        actual_powered_by = entry.received.get("powered_by")
        if request.comment is not None:
            request_description = (
                f"Request with ID {lsp_id!r} (comment: {request.comment!r})"
            )
        else:
            request_description = f"Request with ID {lsp_id!r}"

        # Because of the way hack allocates a different HHI folder for each running
        # process, let's replace the standard HHI foldername
        actual_result = fixup_hhi_json(actual_result)
        expected_result = interpolate_variables(
            payload=request.result, variables=variables
        )
        expected_result = fixup_hhi_json(expected_result)

        if actual_result != expected_result:
            error_description = self._pretty_print_diff(
                actual=actual_result, expected=expected_result
            )
            description = f"""\
{request_description} got an incorrect result:

{error_description}"""
            request_context = self._get_context_for_call_site_info(
                request.call_site_info
            )
            context = f"""\
This was the associated request:

{request_context}"""
            remediation = self._describe_response_for_remediation(
                variables=variables, request=request, actual_response=entry.received
            )
            return _ErrorDescription(
                description=description, context=context, remediation=remediation
            )
        elif entry.received.get("powered_by") != request.powered_by:
            description = f"""\
{request_description} had an incorrect value for the `powered_by` field
(expected {request.powered_by!r}; got {actual_powered_by!r})
"""
            request_context = self._get_context_for_call_site_info(
                request.call_site_info
            )
            context = f"""\
This was the associated request:

{request_context}"""
            remediation = self._describe_response_for_remediation(
                variables=variables, request=request, actual_response=entry.received
            )
            return _ErrorDescription(
                description=description, context=context, remediation=remediation
            )

    def _get_context_for_call_site_info(self, call_site_info: _CallSiteInfo) -> str:
        # Find the first caller frame that isn't in this source file. The
        # assumption is that the first such frame is in the test code.
        caller_frame = next(
            frame for frame in call_site_info.traceback if frame.filename != __file__
        )
        source_filename = caller_frame.filename
        with open(source_filename) as f:
            source_text = f.read()

        (
            start_line_num_0idx_incl,
            end_line_num_0idx_incl,
        ) = self._find_line_range_for_function_call(
            file_contents=source_text, line_num_1idx=call_site_info.line_num
        )
        return self._pretty_print_file_context(
            file_path=source_filename,
            file_contents=source_text,
            start_line_num_0idx_incl=start_line_num_0idx_incl,
            end_line_num_0idx_incl=end_line_num_0idx_incl,
        )

    def _find_line_range_for_function_call(
        self, file_contents: str, line_num_1idx: int
    ) -> Tuple[int, int]:
        tree = libcst.parse_module(file_contents)
        function_call_finder = _FunctionCallFinder()
        MetadataWrapper(tree).visit(function_call_finder)
        function_calls_containing_line = [
            (node, node_range)
            for node, node_range in function_call_finder.function_calls
            if node_range.start.line <= line_num_1idx <= node_range.end.line
        ]
        node_range = max(
            function_calls_containing_line,
            key=lambda node_with_range: node_with_range[1].end.line
            - node_with_range[1].start.line,
        )[1]
        start_line_num_0idx_incl = node_range.start.line - 1
        end_line_num_0idx_incl = node_range.end.line - 1
        return (start_line_num_0idx_incl, end_line_num_0idx_incl)

    def _pretty_print_file_context(
        self,
        file_path: str,
        file_contents: str,
        start_line_num_0idx_incl: int,
        end_line_num_0idx_incl: int,
    ) -> str:
        source_lines = file_contents.splitlines(keepends=True)
        context_lines = source_lines[
            start_line_num_0idx_incl : end_line_num_0idx_incl + 1
        ]
        context_lines = [
            # Include the line number in a gutter for display.
            f"{line_num:>5} | {line_contents}"
            for line_num, line_contents in enumerate(
                context_lines, start=start_line_num_0idx_incl + 1
            )
        ]
        file_context = "".join(context_lines)

        # The full path is likely not useful, since it includes any temporary
        # directories that Buck introduced.
        prefix = os.path.commonprefix([file_path, __file__])
        display_filename = file_path[len(prefix) :]
        return display_filename + "\n" + file_context

    def _describe_response_for_remediation(
        self, variables: VariableMap, request: "_RequestSpec", actual_response: Json
    ) -> str:
        method = request.method
        params = request.params
        result = uninterpolate_variables(
            payload=actual_response.get("result"), variables=variables
        )
        powered_by = actual_response.get("powered_by")

        request_snippet = """\
    .request(
        line=line(),"""
        if request.comment is not None:
            request_snippet += f"""
        comment={request.comment!r},"""
        request_snippet += f"""
        method={method!r},
        params={params!r},
        result={result!r},"""
        if request.wait_id is not None:
            request_snippet += f"""
        wait_id={request.wait_id!r},"""
        if powered_by is not None:
            request_snippet += f"""
        powered_by={powered_by!r},"""
        request_snippet += """
    )"""

        remediation = f"""\
1) If this was unexpected, then the language server is buggy and should be
fixed.

2) If this was expected, you can update your request with the following code to
make it match:

{request_snippet}
"""
        return remediation

    def _find_ignored_transcript_ids(self, transcript: Transcript) -> Iterable[str]:
        for transcript_id, entry in transcript.items():
            if (
                entry.received is not None
                and "id" not in entry.received
                and entry.received.get("method") in self._ignored_notification_methods
            ):
                yield transcript_id

            if (
                entry.received is not None
                and "id" in entry.received
                and "method" in entry.received
                and "params" in entry.received
                and (
                    (entry.received["method"], entry.received["params"])
                    in self._ignored_requests
                    or (entry.received["method"], None) in self._ignored_requests
                )
            ):
                yield transcript_id

    def _flag_unhandled_messages(
        self,
        handled_entries: AbstractSet[str],
        variables: VariableMap,
        transcript: Transcript,
        lsp_id_map: _LspIdMap,
    ) -> Iterable["_ErrorDescription"]:
        for transcript_id, entry in transcript.items():
            if transcript_id in handled_entries:
                continue

            received = entry.received
            if received is None:
                continue

            if entry.sent is not None:
                # We received a request and responded to it.
                continue

            method = received["method"]
            params = received["params"]
            payload = self._pretty_print_snippet(received)
            if "id" in received:
                description = f"""\
An unexpected request of type {method!r} was sent by the language server.
Here is the request payload:

{payload}
"""
                at_nocommit = "@" + "nocommit"
                remediation = f"""\
1) If this was unexpected, then the language server is buggy and should be
fixed.

2) If all requests of type {method!r} with theses params should be ignored,
add this directive anywhere in your test:

    .{self.ignore_requests.__name__}(method={method!r}, params={params!r})

3) To handle this request, add this directive to your test to wait for it and
respond to it before proceeding:

    .{self.wait_for_server_request.__name__}(
        method={method!r},
        params={params!r},
        result={{
            "{at_nocommit}": "fill in request data here",
        }},
    )
"""
            else:
                if any(
                    isinstance(message, _WaitForNotificationSpec)
                    and message.method == method
                    and interpolate_variables(
                        payload=message.params, variables=variables
                    )
                    == params
                    for message in self._messages
                ):
                    # This was a notification we we explicitly waiting for, so skip
                    # it.
                    continue

                uninterpolated_params = uninterpolate_variables(
                    payload=params, variables=variables
                )
                description = f"""\
An unexpected notification of type {method!r} was sent by the language server.
Here is the notification payload:

{payload}
"""
                remediation = f"""\
1) If this was unexpected, then the language server is buggy and should be
fixed.

2) If all notifications of type {method!r} should be ignored, add this directive
anywhere in your test:

    .{self.ignore_notifications.__name__}(method={method!r})

3) If this single instance of the notification was expected, add this directive
to your test to wait for it before proceeding:

    .{self.wait_for_notification.__name__}(
        method={method!r},
        params={uninterpolated_params!r},
    )
"""

            previous_request = self._find_previous_request(
                transcript, lsp_id_map, current_id=transcript_id
            )
            if previous_request is not None:
                request_context = self._get_context_for_call_site_info(
                    previous_request.call_site_info
                )
            else:
                request_context = "<no previous request was found>"
            context = f"""\
This was the most recent request issued from the language client before it
received the notification:

{request_context}"""

            yield _ErrorDescription(
                description=description, context=context, remediation=remediation
            )

    def _find_previous_request(
        self, transcript: Transcript, lsp_id_map: _LspIdMap, current_id: str
    ) -> Optional["_RequestSpec"]:
        previous_transcript_entries = itertools.takewhile(
            lambda kv: kv[0] != current_id, transcript.items()
        )
        previous_request_entries = [
            entry.sent
            for _id, entry in previous_transcript_entries
            if entry.sent is not None and LspCommandProcessor._is_request(entry.sent)
        ]
        if previous_request_entries:
            previous_request_lsp_id = previous_request_entries[-1]["id"]
        else:
            return None

        [corresponding_request] = [
            request
            for request, lsp_id in lsp_id_map.items()
            if lsp_id == previous_request_lsp_id
        ]
        assert isinstance(
            corresponding_request, _RequestSpec
        ), "We should have identified a client-to-server request at this point"
        return corresponding_request

    def _render_telemetry_rage(
        self, debug_request: "_DebugRequestSpec", result: Json
    ) -> "_ErrorDescription":
        sections = []
        for row in result:
            title = row["title"]
            if title is None:
                title = "<none>"
            data = row.get("data")
            sections.append(
                f"""\
### Section {title} ###
{data}
"""
            )
        sections = textwrap.indent("".join(sections), prefix="  ")
        description = f"""\
Here are the results of issuing a `telemetry/rage` request to the language
server:

{sections}"""
        context = """\
<none>
"""
        remediation = """\
Remove this `debug` request once you're done debugging.
"""
        return _ErrorDescription(
            description=description, context=context, remediation=remediation
        )

    def _pretty_print_snippet(self, obj: object) -> str:
        return textwrap.indent(pprint.pformat(obj), prefix="  ")

    def _pretty_print_diff(self, actual: object, expected: object) -> str:
        # Similar to the standard library's `unittest` module:
        # https://github.com/python/cpython/blob/35d9c37e271c35b87d64cc7422600e573f3ee244/Lib/unittest/case.py#L1147-L1149  # noqa B950
        return (
            "(- is expected lines, + is actual lines)\n"
            + "\n".join(
                difflib.ndiff(
                    pprint.pformat(expected).splitlines(),
                    pprint.pformat(actual).splitlines(),
                )
            )
            + "\n"
        )


### Internal. ###


class _FunctionCallFinder(libcst.CSTVisitor):
    """Find function calls and their locations in the given syntax tree.

    Chained function calls include the entire chain as the callee. For example,
    the chain `x().y().z()` might include `x().y().z` as the callee and `()` as
    the function call itself. But in the case of function call chains, we really
    want just the range covered by the parentheses.

    However, that's not directly available in `libcst`, so we approximate this
    by finding the location of `z` and assume that's where the function call
    starts.
    """

    METADATA_DEPENDENCIES = (PositionProvider,)

    def __init__(self) -> None:
        self.function_calls: List[Tuple[libcst.Call, CodeRange]] = []

    def visit_Call(self, node: libcst.Call) -> None:
        node_range = self.get_metadata(PositionProvider, node)

        start_node = node.func
        while isinstance(start_node, libcst.Attribute):
            start_node = start_node.attr
        start_node_range = self.get_metadata(PositionProvider, start_node)
        start_position = start_node_range.start
        end_position = node_range.end
        node_range = CodeRange(start=start_position, end=end_position)

        self.function_calls.append((node, node_range))


class _RequestSpec:
    __slots__ = [
        "method",
        "params",
        "result",
        "wait_id",
        "comment",
        "powered_by",
        "call_site_info",
    ]

    def __init__(
        self,
        *,
        method: str,
        params: Json,
        result: Json,
        wait_id: Optional[str],
        comment: Optional[str],
        powered_by: Optional[str],
        call_site_info: _CallSiteInfo,
    ) -> None:
        # pyre-fixme[4]: Attribute must be annotated.
        self.method = method
        # pyre-fixme[4]: Attribute must be annotated.
        self.params = params
        # pyre-fixme[4]: Attribute must be annotated.
        self.result = result
        # pyre-fixme[4]: Attribute must be annotated.
        self.wait_id = wait_id
        # pyre-fixme[4]: Attribute must be annotated.
        self.comment = comment
        # pyre-fixme[4]: Attribute must be annotated.
        self.powered_by = powered_by
        # pyre-fixme[4]: Attribute must be annotated.
        self.call_site_info = call_site_info


class _DebugRequestSpec:
    pass


class _NotificationSpec:
    __slots__ = ["method", "params", "comment"]

    def __init__(self, *, method: str, params: Json, comment: Optional[str]) -> None:
        # pyre-fixme[4]: Attribute must be annotated.
        self.method = method
        # pyre-fixme[4]: Attribute must be annotated.
        self.params = params
        # pyre-fixme[4]: Attribute must be annotated.
        self.comment = comment


class _WaitForRequestSpec:
    __slots__ = ["method", "params", "result", "comment"]

    def __init__(
        self,
        *,
        method: str,
        params: Json,
        result: Union[Json, NoResponse],
        comment: Optional[str],
    ) -> None:
        # pyre-fixme[4]: Attribute must be annotated.
        self.method = method
        # pyre-fixme[4]: Attribute must be annotated.
        self.params = params
        # pyre-fixme[4]: Attribute must be annotated.
        self.result = result
        # pyre-fixme[4]: Attribute must be annotated.
        self.comment = comment


class _WaitForNotificationSpec:
    __slots__ = ["method", "params", "comment"]

    def __init__(self, *, method: str, params: Json, comment: Optional[str]) -> None:
        # pyre-fixme[4]: Attribute must be annotated.
        self.method = method
        # pyre-fixme[4]: Attribute must be annotated.
        self.params = params
        # pyre-fixme[4]: Attribute must be annotated.
        self.comment = comment


class _WaitForResponseSpec:
    __slots__ = ["wait_id"]

    def __init__(self, *, wait_id: str) -> None:
        # pyre-fixme[4]: Attribute must be annotated.
        self.wait_id = wait_id


class _WaitForHhServerReadySpec:
    pass


class _ErrorDescription:
    def __init__(self, description: str, context: str, remediation: str) -> None:
        self.description = description
        self.context = context
        self.remediation = remediation

    def __str__(self) -> str:
        result = f"""\
Description: {self.description}
"""
        if self.context is not None:
            result += f"""\
Context:
{self.context}
"""
        result += f"""\
Remediation:
{self.remediation}"""
        return result
