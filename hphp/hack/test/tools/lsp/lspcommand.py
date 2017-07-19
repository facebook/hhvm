#!/usr/bin/env python3

import contextlib
import json
import os
import re
import subprocess


class LspCommandProcessor:
    def __init__(self, proc):
        self.proc = proc

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
        yield cls(proc)

        proc.stdin.close()
        proc.stdout.close()
        proc.stderr.close()

    def send(self, command):
        self._write_command(command)

    def receive(self):
        return self._read_response()

    # decodes a compressed LSP command into a JSON
    # payload suitable to be sent to the LSP.
    #
    # commands that start with a "#" are considered comments
    # and will return None for the built command.
    def build_command(self, line):
        line = line.strip()

        if self._is_empty_line(line) or self._is_comment(line):
            return None

        method, rw, line = line.strip().split(" ", 2)

        line = self._eval_replacements(line)

        json_rpc_payload = f"""
{{
    "jsonrpc": "2.0",
    "id": 1,
    "method": "{method}",
    "params": {line}
}}
    """.strip()
        content_length = len(json_rpc_payload)
        return (f"Content-Length: {content_length}\n\n{json_rpc_payload}", rw)

    def _eval_replacements(self, encoded_json):
        decoded_json = json.loads(encoded_json)
        return json.dumps(self._eval_json(decoded_json))

    def _eval_json(self, json):
        if isinstance(json, dict):
            return {k: self._eval_json(v) for k, v in json.items()}
        elif isinstance(json, list):
            return [self._eval_json(i) for i in list]
        elif isinstance(json, str):
            match = re.match(r'>>>(.*)', json)
            if match is None:
                return json
            return eval(match.group(1))  # noqa: P204
        else:
            return json

    def _write_command(self, command):
        self.proc.stdin.write(command.encode())
        self.proc.stdin.flush()

    def _read_content_length(self):
        # read the 'Content-Length:' line and absorb the newline
        # after it
        length_line = self.proc.stdout.readline().decode()
        self.proc.stdout.read(len("\n\n"))

        # get the content length as an integer for the
        # rest of the package
        parts = length_line.split(":", 1)
        return (int(parts[1].strip()))

    def _read_content(self, length):
        return self.proc.stdout.read(length)

    def _read_response(self):
        length = self._read_content_length()
        return self._read_content(length)

    def _is_empty_line(self, line):
        return (not line) or (line == "\n")

    def _is_comment(self, line):
        return line.startswith("#")


# string replacement methods meant to be called
# within a command processing script.
def path_expand(path):
    return "file://" + os.path.abspath(path)


def read_file(file):
    with open(file, "r") as f:
        return f.read()
