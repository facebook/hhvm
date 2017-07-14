#!/usr/bin/env python3

"""
 Takes LSP commands and communicates them with the LSP server.

 This script is useful for testing the LSP server: you can feed in a payload and
 confirm that the responses are correct.

 The LSP spec can be found here:
 https://github.com/Microsoft/language-server-protocol/blob/master/protocol.md

 Input format: the first word is the LSP method (see the LSP spec for a full
 listing). The rest of the line is the JSON payload. You can also comment out a
 line by prepending with a `#`.  See 'init.txt' for example of format.

 Suggested command-line usage:

    python talk.py filename.txt
"""
import fileinput
from lspcommand import LspCommandProcessor


def main():
    with LspCommandProcessor.create() as lsp_proc:
        for line in fileinput.input():
            command = lsp_proc.build_command(line)
            if command:
                print_section("SENDING:", command)
                response = lsp_proc.send(command)
                print_section("LSP SAID:", response.decode())


def print_section(header, message):
    print(f"{header}\n{message}\n\n{'-' * 80}\n")


if __name__ == "__main__":
    main()
