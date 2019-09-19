# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import contextlib
import pprint
import subprocess
import urllib
import uuid
from typing import (
    BinaryIO,
    Callable,
    Iterator,
    Mapping,
    NamedTuple,
    Optional,
    Sequence,
    Tuple,
    Type,
    TypeVar,
)

from hh_paths import hh_client
from jsonrpc_stream import JsonRpcStreamReader, JsonRpcStreamWriter
from utils import Json


class TranscriptEntry(NamedTuple):
    sent: Optional[Json]
    received: Optional[Json]

    def __repr__(self) -> str:
        return (
            "TranscriptEntry"
            + pprint.pformat({"sent": self.sent, "received": self.received})
            + "\n"
        )


Transcript = Mapping[str, TranscriptEntry]
U = TypeVar("U", bound="LspCommandProcessor")


class LspCommandProcessor:
    def __init__(
        self,
        proc: subprocess.Popen,
        reader: JsonRpcStreamReader,
        writer: JsonRpcStreamWriter,
    ) -> None:
        self.proc = proc
        self.reader = reader
        self.writer = writer

    @classmethod
    @contextlib.contextmanager
    def create(
        cls: Type[U], env: Mapping[str, str], use_serverless_ide: bool
    ) -> Iterator[U]:
        args = ["--enhanced-hover"]
        if use_serverless_ide:
            args.append("--serverless-ide")
        proc = subprocess.Popen(
            [hh_client, "lsp"] + args,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=env,
        )
        try:
            stdout: BinaryIO = proc.stdout  # pyre-ignore
            stdin: BinaryIO = proc.stdin  # pyre-ignore
            reader = JsonRpcStreamReader(stdout)
            writer = JsonRpcStreamWriter(stdin)
            yield cls(proc, reader, writer)
        finally:
            proc.stdin.close()
            proc.stdout.close()
            proc.stderr.close()

    # request_timeout is the number of seconds to wait for responses
    # that we expect the server to send back.  this timeout can
    # be longer because it typically won't be hit.
    #
    # notify_timeout is the number of seconds to wait for responses
    # from the server that aren't caused by a request.  these could
    # be errors or server notifications.
    def communicate(
        self,
        json_commands: Sequence[Json],
        request_timeout: float = 30,
        notify_timeout: float = 1,
    ) -> Transcript:
        transcript = self._send_commands({}, json_commands)
        transcript = self._wait_for_shutdown(transcript)

        # because it's possible the server sent us notifications
        # along with responses we need to try to keep reading
        # from the stream to get anything that might be left.
        dummy_id = LspCommandProcessor.dummy_request_id()
        transcript = self._read_extra_responses(transcript, notify_timeout)
        return {k: v for k, v in transcript.items() if k != dummy_id}

    def _send_commands(
        self, transcript: Transcript, commands: Sequence[Json]
    ) -> Transcript:
        for command in commands:
            if command["method"] == "$test/waitForRequest":
                transcript = self._wait_for_request(transcript, command)
            elif command["method"] == "$test/waitForResponse":
                transcript = self._wait_for_response(
                    transcript, command["params"]["id"]
                )
            elif command["method"] == "$test/waitForNotification":
                transcript = self._wait_for_notification(transcript, command)
            elif command["method"] == "$test/waitForHhServerReady":
                # Hack: HackLSP server only connects to hh_server asynchronously.
                # We want to delay until after it's connected before testing more.
                transcript = self._wait_for_initialized(transcript)
            elif command["method"] == "$test/writeToDisk":
                self._write_to_disk(command)
            else:
                self.writer.write(command)
                transcript = self._scribe(transcript, sent=command, received=None)

        return transcript

    def _wait_for_initialized(self, transcript: Transcript) -> Transcript:
        dummy_command = {
            "jsonrpc": "2.0",
            "method": "workspace/symbol",
            "id": -1,
            "params": {"query": "my test query"},
        }
        id = self._client_request_id(dummy_command["id"])

        def has_error_message(entry: TranscriptEntry, message: str) -> bool:
            if (
                entry.received is None
                or entry.received.get("error") is None
                or entry.received["error"].get("message") is None
            ):
                return False
            else:
                return message in entry.received["error"]["message"]

        while True:
            transcript = dict(transcript)
            if id in transcript:
                del transcript[id]
            transcript = self._send_commands(transcript, [dummy_command])
            transcript = self._wait_for_response(
                transcript, request_id=dummy_command["id"]
            )

            if not has_error_message(
                transcript[id], "Server busy"
            ) and not has_error_message(transcript[id], "hh_server initializing"):
                return transcript

    def _wait_for_shutdown(self, transcript: Transcript) -> Transcript:
        shutdown_commands = [
            v.sent
            for v in transcript.values()
            if v.sent is not None and v.sent.get("method") == "shutdown"
        ]
        num_shutdown_commands = len(shutdown_commands)
        assert num_shutdown_commands == 1, (
            "Expected this test case to have sent "
            + "exactly 1 shutdown command to wait for, "
            + f"but instead got {num_shutdown_commands}. "
            + "If you are writing an LSP test, "
            + "make sure that you have exactly one `shutdown` command at the end, "
            + "as we wait for that `shutdown` command to return "
            + "to know when the test is done running. "
            + f"Here are the shutdown commands I saw: {shutdown_commands}"
        )
        [shutdown_command] = shutdown_commands
        return self._wait_for_response(transcript, shutdown_command["id"])

    def _wait_for_response(
        self, transcript: Transcript, request_id: Json
    ) -> Transcript:
        transcript_id = self._client_request_id(request_id)
        request = transcript[transcript_id].sent

        while transcript[transcript_id].received is None:
            message = self._try_read_logged(timeout_seconds=5.0)
            assert message is not None, (
                f"Timed out while waiting for the response to request {transcript_id}. "
                + f"Here is the request: {request}. "
                + f"Here is the transcript so far: {transcript}"
            )
            transcript = self._scribe(transcript, sent=None, received=message)
        return transcript

    def _wait_for_message_from_server(
        self, transcript: Transcript, method: str, params: Json
    ) -> Tuple[Transcript, Json]:
        def is_target_message(entry: TranscriptEntry) -> bool:
            return (
                entry.received is not None
                and entry.received.get("method") == method
                and entry.received.get("params") == params
            )

        while not any(is_target_message(entry) for entry in transcript.values()):
            message = self._try_read_logged(timeout_seconds=5)
            params_pretty = pprint.pformat(params)
            assert (
                message is not None
            ), f"""\
Timed out while waiting for a {method!r} message to be sent from the server.
The message was expected to have params:

{params_pretty}

Transcript of all the messages we saw:

{transcript}"""
            transcript = self._scribe(transcript, sent=None, received=message)

        [message] = [
            entry.received for entry in transcript.values() if is_target_message(entry)
        ]
        return (transcript, message)

    def _wait_for_request(self, transcript: Transcript, command: Json) -> Transcript:
        method = command["params"]["method"]
        params = command["params"]["params"]
        result = command["params"]["result"]
        (transcript, message) = self._wait_for_message_from_server(
            transcript, method=method, params=params
        )

        response = {"jsonrpc": 2.0, "id": message["id"], "result": result}
        self.writer.write(response)
        transcript = self._scribe(transcript, sent=response, received=None)
        return transcript

    def _wait_for_notification(
        self, transcript: Transcript, command: Json
    ) -> Transcript:
        method = command["params"]["method"]
        params = command["params"]["params"]
        (transcript, _message) = self._wait_for_message_from_server(
            transcript, method=method, params=params
        )
        return transcript

    def _write_to_disk(self, command: Json) -> None:
        params = command["params"]
        path = urllib.parse.urlparse(params["uri"]).path
        contents = params["contents"]
        with open(path, "w") as f:
            f.write(contents)

    def _read_request_responses(
        self, transcript: Transcript, commands: Sequence[Json], timeout_seconds: float
    ) -> Transcript:
        for _ in self._requests_in(commands):
            response = self._try_read_logged(timeout_seconds)
            if response:
                transcript = self._scribe(transcript, sent=None, received=response)
        return transcript

    def _read_extra_responses(
        self, transcript: Transcript, timeout_seconds: float
    ) -> Transcript:
        while True:
            response = self._try_read_logged(timeout_seconds)
            if not response:
                break
            transcript = self._scribe(transcript, sent=None, received=response)
        return transcript

    def _scribe(
        self, transcript: Transcript, sent: Optional[Json], received: Optional[Json]
    ) -> Transcript:
        transcript = dict(transcript)
        id = self._transcript_id(sent, received)
        existing_entry = transcript.get(id)
        if existing_entry:
            if not sent:
                sent = existing_entry.sent
            if not received:
                received = existing_entry.received
        assert sent is not None or received is not None

        transcript[id] = TranscriptEntry(sent=sent, received=received)
        return transcript

    def _transcript_id(self, sent: Optional[Json], received: Optional[Json]) -> str:
        assert sent is not None or received is not None

        def make_id(
            json: Json, is_client_request: bool, idgen: Callable[[], str]
        ) -> str:
            if LspCommandProcessor._has_id(json):
                if is_client_request:
                    return LspCommandProcessor._client_request_id(json["id"])
                else:
                    return LspCommandProcessor._server_request_id(json["id"])
            else:
                return idgen()

        if sent:
            is_client_request = LspCommandProcessor._is_request(sent)
            return make_id(
                sent, is_client_request, LspCommandProcessor._client_notify_id
            )
        elif received:
            is_client_request = not LspCommandProcessor._is_request(received)
            return make_id(
                received, is_client_request, LspCommandProcessor._server_notify_id
            )
        else:
            raise Exception("This should have failed up above in the assert")

    def _requests_in(self, commands: Sequence[Json]) -> Sequence[Json]:
        return [c for c in commands if LspCommandProcessor._has_id(c)]

    def _try_read_logged(self, timeout_seconds: float) -> Optional[Json]:
        response = self.reader.read(timeout_seconds)
        return response

    @staticmethod
    def _has_id(json: Json) -> bool:
        return "id" in json

    @staticmethod
    def _is_request(json: Json) -> bool:
        return "id" in json and "method" in json

    @staticmethod
    def _client_notify_id() -> str:
        return LspCommandProcessor._notify_id("NOTIFY_CLIENT_TO_SERVER_")

    @staticmethod
    def _server_notify_id() -> str:
        return LspCommandProcessor._notify_id("NOTIFY_SERVER_TO_CLIENT_")

    @staticmethod
    def _notify_id(prefix: str) -> str:
        return prefix + str(uuid.uuid4())

    @staticmethod
    def _client_request_id(id: Json) -> str:
        return "REQUEST_CLIENT_TO_SERVER_" + str(id)

    @staticmethod
    def _server_request_id(id: Json) -> str:
        return "REQUEST_SERVER_TO_CLIENT_" + str(id)

    @staticmethod
    def dummy_request_id() -> str:
        return LspCommandProcessor._client_request_id(-1)
