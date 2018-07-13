from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from typing import (
    Any,
    Callable,
    Iterator,
    Mapping,
    NamedTuple,
    Optional,
    Sequence,
    Type,
    TypeVar,
)
import contextlib
import subprocess
import uuid
from hh_paths import hh_client
from jsonrpc_stream import JsonRpcStreamReader, JsonRpcStreamWriter


Json = Mapping[str, Any]


class TranscriptEntry(NamedTuple):
    sent: Json
    received: Optional[Json]


Transcript = Mapping[str, TranscriptEntry]


class LspCommandProcessor:
    U = TypeVar("U", bound="LspCommandProcessor")

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
    def create(cls: Type[U], env: Mapping[str, str]) -> Iterator[U]:
        proc = subprocess.Popen(
            [hh_client, "lsp", "--enhanced-hover"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=env,
        )
        try:
            reader = JsonRpcStreamReader(proc.stdout)
            writer = JsonRpcStreamWriter(proc.stdin)
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

        # we are expecting at least one response per request sent so
        # we read these giving the server more time to respond with them.
        transcript = self._read_request_responses(
            transcript, json_commands, request_timeout
        )

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
            self.writer.write(command)
            transcript = self._scribe(transcript, sent=command, received=None)
            # Hack: HackLSP server only connects to hh_server asynchronously.
            # We want to delay until after it's connected before testing more.
            if command["method"] == "initialize":
                transcript = self._wait_for_initialized(transcript)

        return transcript

    def _wait_for_initialized(self, transcript: Transcript) -> Transcript:
        dummy_command = {
            "jsonrpc": "2.0",
            "method": "workspace/symbol",
            "id": -1,
            "params": {"query": "my test query"},
        }
        id = self._request_id(dummy_command)

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
            transcript = self._send_commands(transcript, [dummy_command])
            transcript = self._read_request_responses(
                transcript, [dummy_command], timeout_seconds=5
            )

            if (
                not any(
                    has_error_message(entry, "Server busy")
                    for entry in transcript.values()
                )
                and not any(
                    has_error_message(entry, "hh_server initializing")
                    for entry in transcript.values()
                )
                and id in transcript
                and transcript[id].received is not None
            ):
                return transcript

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
        else:
            assert sent is not None

        transcript[id] = TranscriptEntry(sent=sent, received=received)
        return transcript

    def _transcript_id(self, sent: Optional[Json], received: Optional[Json]) -> str:
        assert sent is not None or received is not None

        def make_id(json: Json, idgen: Callable[[], str]) -> str:
            if LspCommandProcessor._has_id(json):
                return LspCommandProcessor._request_id(json)
            else:
                return idgen()

        if sent:
            return make_id(sent, LspCommandProcessor._client_notify_id)
        elif received:
            return make_id(received, LspCommandProcessor._server_notify_id)
        else:
            raise Exception("This should have failed up above in the assert")

    def _requests_in(self, commands: Sequence[Json]) -> Sequence[Json]:
        return [c for c in commands if LspCommandProcessor._has_id(c)]

    def _try_read_logged(self, timeout_seconds: float) -> Optional[Json]:
        response = self.reader.try_read(timeout_seconds)
        return response

    @staticmethod
    def _has_id(json: Json) -> bool:
        return "id" in json

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
    def _request_id(json_command: Json) -> str:
        return LspCommandProcessor.request_id(json_command["id"])

    @staticmethod
    def request_id(id: str) -> str:
        return "REQUEST_" + str(id)

    @staticmethod
    def dummy_request_id() -> str:
        return LspCommandProcessor._request_id({"id": -1})
