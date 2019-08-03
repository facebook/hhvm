# pyre-strict
from __future__ import absolute_import, division, print_function, unicode_literals

import copy
import difflib
import pprint
import textwrap
from typing import AbstractSet, Iterable, Mapping, Optional, Sequence, Tuple, Union

from lspcommand import LspCommandProcessor, Transcript, TranscriptEntry
from utils import Json, interpolate_variables


_MessageSpec = Union[
    "_RequestSpec",
    "_NotificationSpec",
    "_WaitForNotificationSpec",
    "_WaitForRequestSpec",
]


_LspIdMap = Mapping[_MessageSpec, Json]


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

    def ignore_notifications(self, *, method: str) -> "LspTestSpec":
        ignored_notification_methods = set(self._ignored_notification_methods)
        ignored_notification_methods.add(method)
        return self._update(ignored_notification_methods=ignored_notification_methods)

    def request(
        self,
        method: str,
        params: Json,
        *,
        result: Json,
        wait: bool = True,
        comment: Optional[str] = None,
        powered_by: Optional[str] = None,
    ) -> "LspTestSpec":
        messages = list(self._messages)
        messages.append(
            _RequestSpec(
                method=method,
                params=params,
                result=result,
                wait=wait,
                comment=comment,
                powered_by=powered_by,
            )
        )
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
        self, method: str, params: Json, *, result: Json, comment: Optional[str] = None
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

    def run(
        self, lsp_command_processor: LspCommandProcessor, variables: Mapping[str, str]
    ) -> Tuple[Transcript, Optional[str]]:
        """Run the test given the LSP command processor.

        Raises an exception with useful debugging information if the test fails."""
        (json_commands, lsp_id_map) = self._get_json_commands(variables=variables)
        transcript = lsp_command_processor.communicate(json_commands=json_commands)
        errors = list(
            self._verify_transcript(transcript=transcript, lsp_id_map=lsp_id_map)
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
    ) -> "LspTestSpec":
        spec = copy.copy(self)
        if messages is not None:
            spec._messages = messages
        if ignored_notification_methods is not None:
            spec._ignored_notification_methods = ignored_notification_methods
        return spec

    def _get_json_commands(
        self, variables: Mapping[str, str]
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

                if message.wait:
                    json_commands.append(
                        {
                            "jsonrpc": "2.0",
                            "method": "$test/waitForResponse",
                            "params": {"id": current_id},
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
                json_commands.append(
                    {
                        "jsonrpc": "2.0",
                        "comment": message.comment,
                        "method": "$test/waitForRequest",
                        "params": {
                            "method": message.method,
                            "params": message.params,
                            "result": message.result,
                        },
                    }
                )
            else:
                raise ValueError(f"unhandled message type {message.__class__.__name__}")
        return (json_commands, lsp_id_map)

    def _verify_transcript(
        self, *, transcript: Transcript, lsp_id_map: "_LspIdMap"
    ) -> Iterable["_ErrorDescription"]:
        handled_entries = set()

        for message in self._messages:
            lsp_id = lsp_id_map[message]
            if isinstance(message, _RequestSpec):
                transcript_id = LspCommandProcessor._client_request_id(lsp_id)
                handled_entries.add(transcript_id)
                assert transcript_id in transcript, (
                    f"Expected message with ID {lsp_id!r} "
                    + f"to have an entry in the transcript "
                    + f"under key {transcript_id!r}, "
                    + f"but it was not found. Transcript: {transcript!r}"
                )
                entry = transcript[transcript_id]
                error_description = self._verify_request(
                    entry=entry, lsp_id=lsp_id, request=message
                )
                if error_description is not None:
                    yield error_description
            elif isinstance(message, _NotificationSpec):
                # Nothing needs to be done here, since we sent the notification
                # and don't expect a response.
                pass
            elif isinstance(message, _WaitForRequestSpec):
                # Nothing needs to be done here -- if we failed to wait for the
                # request, an exception will have been thrown at the
                # `LspCommandProcessor` layer.
                pass
            else:
                raise ValueError(f"unhandled message type {message.__class__.__name__}")

        handled_entries |= set(self._find_ignored_transcript_ids(transcript))
        yield from self._flag_unhandled_notifications(handled_entries, transcript)

    def _verify_request(
        self, *, lsp_id: Json, entry: TranscriptEntry, request: "_RequestSpec"
    ) -> Optional["_ErrorDescription"]:
        actual_result = entry.received.get("result")
        actual_powered_by = entry.received.get("powered_by")
        if request.comment is not None:
            request_description = (
                f"Request with ID {lsp_id!r} (comment: {request.comment!r})"
            )
        else:
            request_description = f"Request with ID {lsp_id!r}"

        if actual_result != request.result:
            error_description = self._pretty_print_diff(
                actual=actual_result, expected=request.result
            )
            description = f"""\
{request_description} got an incorrect result:

{error_description}
    """
            remediation = self._describe_response_for_remediation(
                request=request, actual_response=entry.received
            )
            return _ErrorDescription(description=description, remediation=remediation)
        elif entry.received.get("powered_by") != request.powered_by:
            description = f"""\
{request_description} had an incorrect value for the `powered_by` field
(expected {request.powered_by!r}; got {actual_powered_by!r})
"""
            remediation = self._describe_response_for_remediation(
                request=request, actual_response=entry.received
            )
            return _ErrorDescription(description=description, remediation=remediation)

    def _describe_response_for_remediation(
        self, request: "_RequestSpec", actual_response: Json
    ) -> str:
        method = request.method
        params = request.params
        result = actual_response.get("result")
        powered_by = actual_response.get("powered_by")

        request_snippet = f"""\
    .request("""
        if request.comment is not None:
            request_snippet += f"""
        comment={request.comment!r},"""
        request_snippet += f"""
        method={method!r},
        params={params!r},
        result={result!r},"""
        if not request.wait:
            request_snippet += f"""
        wait=False,"""
        if request.powered_by is not None:
            request_snippet += f"""
        powered_by={powered_by!r},"""
        request_snippet += f"""
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
                and entry.received.get("method") in self._ignored_notification_methods
            ):
                yield transcript_id

    def _flag_unhandled_notifications(
        self, handled_entries: AbstractSet[str], transcript: Transcript
    ) -> Iterable["_ErrorDescription"]:
        for transcript_id, entry in transcript.items():
            if transcript_id in handled_entries:
                continue

            received = entry.received
            if received is None:
                continue

            if entry.sent is not None:
                # We received a request and responded it it.
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

2) To handle this request, add this directive to your test to wait for it and
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
        params={params!r},
    )
"""

            yield _ErrorDescription(description=description, remediation=remediation)

    def _pretty_print_snippet(self, obj: object) -> str:
        return textwrap.indent(pprint.pformat(obj), prefix="  ")

    def _pretty_print_diff(self, actual: object, expected: object) -> str:
        # Similar to the standard library's `unittest` module:
        # https://github.com/python/cpython/blob/35d9c37e271c35b87d64cc7422600e573f3ee244/Lib/unittest/case.py#L1147-L1149  # noqa B950
        return (
            "(+ is expected lines, - is actual lines)\n"
            + "\n".join(
                difflib.ndiff(
                    pprint.pformat(actual).splitlines(),
                    pprint.pformat(expected).splitlines(),
                )
            )
            + "\n"
        )


### Internal. ###


class _RequestSpec:
    __slots__ = ["method", "params", "result", "wait", "comment", "powered_by"]

    def __init__(
        self,
        *,
        method: str,
        params: Json,
        result: Json,
        wait: bool,
        comment: Optional[str],
        powered_by: Optional[str],
    ) -> None:
        self.method = method
        self.params = params
        self.result = result
        self.wait = wait
        self.comment = comment
        self.powered_by = powered_by


class _NotificationSpec:
    __slots__ = ["method", "params", "comment"]

    def __init__(self, *, method: str, params: Json, comment: Optional[str]) -> None:
        self.method = method
        self.params = params
        self.comment = comment


class _WaitForRequestSpec:
    __slots__ = ["method", "params", "result", "comment"]

    def __init__(
        self, *, method: str, params: Json, result: Json, comment: Optional[str]
    ) -> None:
        self.method = method
        self.params = params
        self.result = result
        self.comment = comment


class _WaitForNotificationSpec:
    __slots__ = ["method", "params", "comment"]

    def __init__(self, *, method: str, params: Json, comment: Optional[str]) -> None:
        self.method = method
        self.params = params
        self.comment = comment


class _ErrorDescription:
    def __init__(self, description: str, remediation: str) -> None:
        self.description = description
        self.remediation = remediation

    def __str__(self) -> str:
        return f"""\
Description: {self.description}
Remediation:
{self.remediation}"""
