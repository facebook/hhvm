#!/usr/bin/env python3

import contextlib
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

        method, line = line.strip().split(" ", 1)

        json_rpc_payload = f"""
{{
    "jsonrpc": "2.0",
    "id": 1,
    "method": "{method}",
    "params": {line}
}}
    """.strip()
        content_length = len(json_rpc_payload)
        return f"Content-Length: {content_length}\n\n{json_rpc_payload}"

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
