#!/usr/bin/env python3

import contextlib
import json
import os
import re
import subprocess
import uuid
from jsonrpc_stream import JsonRpcStreamReader
from jsonrpc_stream import JsonRpcStreamWriter

class LspCommandProcessor:
    def __init__(self, proc, reader, writer):
        self.proc = proc
        self.reader = reader
        self.writer = writer

    @classmethod
    @contextlib.contextmanager
    def create(cls):
        # yes shell = True is generally a bad idea, but
        # in this case we want to pick up your environment entirely because
        # hack depends heavily on it to work
        proc = subprocess.Popen('hh_client lsp',
                                shell=True,
                                stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)

        reader = JsonRpcStreamReader(proc.stdout)
        writer = JsonRpcStreamWriter(proc.stdin)
        yield cls(proc, reader, writer)

        proc.stdin.close()
        proc.stdout.close()
        proc.stderr.close()

    @staticmethod
    def parse_commands(raw_data):
        raw_json = json.loads(raw_data)
        return [LspCommandProcessor._eval_json(command) for command in raw_json]

    # request_timeout is the number of seconds to wait for responses
    # that we expect the server to send back.  this timeout can
    # be longer because it typically won't be hit.
    #
    # notify_timeout is the number of seconds to wait for responses
    # from the server that aren't caused by a request.  these could
    # be errors or server notifications.
    def communicate(self,
                    json_commands,
                    request_timeout=30,
                    notify_timeout=1):

        transcript = self._send_commands({}, json_commands)

        # we are expecting at least one response per request sent so
        # we read these giving the server more time to respond with them.
        transcript = self._read_request_responses(transcript,
                                                  json_commands,
                                                  request_timeout)

        # because it's possible the server sent us notifications
        # along with responses we need to try to keep reading
        # from the stream to get anything that might be left.
        return self._read_extra_responses(transcript, notify_timeout)

    def _send_commands(self, transcript, commands):
        for command in commands:
            self.writer.write(command)
            transcript = self._scribe(transcript, sent=command, received=None)

        return transcript

    def _read_request_responses(self, transcript, commands, timeout_seconds):
        for _ in self._requests_in(commands):
            response = self.reader.try_read(timeout_seconds)
            transcript = self._scribe(transcript, sent=None, received=response)
        return transcript

    def _read_extra_responses(self, transcript, timeout_seconds):
        while True:
            response = self.reader.try_read(timeout_seconds)
            if not response:
                break
            transcript = self._scribe(transcript, sent=None, received=response)
        return transcript

    def _scribe(self, transcript, sent, received):
        transcript = dict(transcript)

        id = self._transcript_id(sent, received)

        if sent and not received:
            received = transcript[id]['received'] if id in transcript else None

        if received and not sent:
            sent = transcript[id]['sent'] if id in transcript else None

        transcript[id] = {"sent": sent, "received": received}

        return transcript

    def _transcript_id(self, sent, received):
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

    def _requests_in(self, commands):
        return [c for c in commands if LspCommandProcessor._has_id(c)]

    @staticmethod
    def _has_id(json):
        return 'id' in json

    @staticmethod
    def _client_notify_id():
        return LspCommandProcessor._notify_id('NOTIFY_CLIENT_TO_SERVER_')

    @staticmethod
    def _server_notify_id():
        return LspCommandProcessor._notify_id('NOTIFY_SERVER_TO_CLIENT_')

    @staticmethod
    def _notify_id(prefix):
        return prefix + str(uuid.uuid4())

    @staticmethod
    def _request_id(json_command):
        return 'REQUEST_' + str(json_command['id'])

    @staticmethod
    def _eval_json(json):
        if isinstance(json, dict):
            return {k: LspCommandProcessor._eval_json(v) for k, v in json.items()}
        elif isinstance(json, list):
            return [LspCommandProcessor._eval_json(i) for i in list]
        elif isinstance(json, str):
            match = re.match(r'>>>(.*)', json)
            if match is None:
                return json
            return eval(match.group(1))  # noqa: P204
        else:
            return json


# string replacement methods meant to be called
# within a command processing script.
def path_expand(path):
    return os.path.abspath(path)


def file_uri(path):
    return "file://" + path_expand(path)


def read_file(file):
    with open(file, "r") as f:
        return f.read()
