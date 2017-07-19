#!/usr/bin/env python3

"""
 Takes LSP commands and communicates them with the LSP server.

 This script is useful for testing the LSP server: you can feed in a payload and
 confirm that the responses are correct.

 The LSP spec can be found here:
 https://github.com/Microsoft/language-server-protocol/blob/master/protocol.md

 Input format: the first word is the LSP method (see the LSP spec for a full
 listing).

 The second word is a string that is an element of
 [ "rw", "w" ]

 It indicates whether or not this tool should write
 the command and then read a response, or just write
 a command and not read a response.  This tool could
 do this automatically with a command lut, but it doesn't
 so that it can stay more of a testing tool and let you
 do things that aren't necessarily a good idea.

 The rest of the line is the JSON payload.

 You can comment out a line by prepending with a `#`.

 Inside a command body, you can run a python function on the argument
 to cause the string output of that function to be replaced at run-time.
 For example, for a URI, you can use

   >>> path_expand('/home/chrishahn/test')

 which generates

   file:///home/chrishahn/test

 you can also use

   >>> read_file('sample.php')

 read_file() will read the contents of 'sample.php' and
 insert them.  This is useful in conjunction with the
 textDocument/didOpen command.

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
                command, rw = command
                process_command(lsp_proc, command, rw)


def process_command(lsp_proc, command, rw):
    print_section("SENDING:", command)
    lsp_proc.send(command)
    if rw == "rw":
        response = lsp_proc.receive()
        print_section(header="LSP RESPONSE:", message=response.decode())
    else:
        print_section(header="SEND OK", message="")


def print_section(header, message):
    print(f"{header}\n{message}\n\n{'-' * 80}\n")


if __name__ == "__main__":
    main()
