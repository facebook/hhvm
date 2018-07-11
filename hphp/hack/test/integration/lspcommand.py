from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from typing import Any, Iterator, Mapping, Optional, Sequence, Type, TypeVar
import contextlib
import subprocess
import uuid
import time
from hh_paths import hh_client
from jsonrpc_stream import JsonRpcStreamReader, JsonRpcStreamWriter

Json = Mapping[str, Any]
Transcript = Mapping[str, Mapping[str, Optional[Json]]]


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
        self, json_commands: Sequence[Json], request_timeout=30, notify_timeout=1
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
        return self._read_extra_responses(transcript, notify_timeout)

    def _send_commands(
        self, transcript: Transcript, commands: Sequence[Json]
    ) -> Transcript:
        for command in commands:
            self.writer.write(command)
            transcript = self._scribe(transcript, sent=command, received=None)
            # Hack: HackLSP server only connects to hh_server asynchronously.
            # We want to delay until after it's connected before testing more.
            if command["method"] == "initialize":
                time.sleep(5)

        return transcript

    def _read_request_responses(
        self, transcript: Transcript, commands: Sequence[Json], timeout_seconds: float
    ) -> Transcript:
        for _ in self._requests_in(commands):
            response = self._try_read_logged(timeout_seconds)
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

        if sent and not received:
            received = transcript[id]["received"] if id in transcript else None

        if received and not sent:
            sent = transcript[id]["sent"] if id in transcript else None

        transcript[id] = {"sent": sent, "received": received}
        return transcript

    def _transcript_id(self, sent: Optional[Json], received: Optional[Json]) -> str:
        assert sent is not None or received is not None

        def make_id(json, idgen):
            if LspCommandProcessor._has_id(json):
                return LspCommandProcessor._request_id(json)
            else:
                return idgen()

        if sent:
            return make_id(sent, LspCommandProcessor._client_notify_id)
        else:
            return make_id(received, LspCommandProcessor._server_notify_id)

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
