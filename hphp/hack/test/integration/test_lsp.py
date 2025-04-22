# pyre-strict
# flake8: noqa: B950

from __future__ import absolute_import, division, print_function, unicode_literals

import os
from pathlib import Path
from typing import Union

from hphp.hack.test.integration.lsp_test_base import LspTestBase
from hphp.hack.test.integration.lsptestspec import line, LspTestSpec, NoResponse

"""
Existence of this file indicates that a Hack notebook is stopped at a breakpoint
"""
_HHVM_IS_PAUSED_FILE: Path = Path.home() / ".vscode-sockets/hhvm-paused"


class TestLsp(LspTestBase):
    def test_init_shutdown(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        self.load_and_run("initialize_shutdown", variables)

    def test_optional_param_completion(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("optional_param_completion.php"))
        spec = (
            self.initialize_spec(LspTestSpec("optional_param_completion"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .notification(
                comment="Insert the beginning of a method call",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 4, "character": 0},
                                "end": {"line": 4, "character": 0},
                            },
                            "text": "$mfc->doSt",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete method",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 4, "character": 10},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "doStuff",
                            "kind": 2,
                            "detail": "function(int $x, optional int $y): void",
                            "sortText": "doStuff",
                            "insertTextFormat": 2,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 4, "character": 6},
                                    "end": {"line": 4, "character": 10},
                                },
                                # We don't want to require the user to provide optional arguments, so
                                # only insert $x, not $y.
                                "newText": "doStuff(${1:\\$x})",
                            },
                            "data": {
                                "fullname": "doStuff",
                                "filename": "${root_path}/optional_param_completion.php",
                                "line": 8,
                                "char": 19,
                                "base_class": "\\MyFooCompletion",
                            },
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_all_optional_params_completion(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("all_optional_params_completion.php"))
        spec = (
            self.initialize_spec(LspTestSpec("all_optional_params_completion"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .notification(
                comment="Insert the beginning of a method call",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 4, "character": 0},
                                "end": {"line": 4, "character": 0},
                            },
                            "text": "$mfc->doSt",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete method",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 4, "character": 10},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "doStuff",
                            "kind": 2,
                            "detail": "function(optional int $x, optional int $y): void",
                            "sortText": "doStuff",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 4, "character": 6},
                                    "end": {"line": 4, "character": 10},
                                },
                                "newText": "doStuff()",
                            },
                            "data": {
                                "fullname": "doStuff",
                                "filename": "${root_path}/all_optional_params_completion.php",
                                "line": 8,
                                "char": 19,
                                "base_class": "\\MyFooCompletionOptional",
                            },
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_completion(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("completion.php"))
        spec = (
            self.initialize_spec(LspTestSpec("ide_completion"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                comment="LocalIndex doesn't have any names in its SymbolIndex, so let's index these files...",
                method="workspace/didChangeWatchedFiles",
                params={
                    "changes": [
                        {
                            "uri": "file://${root_path}/completion.php",
                            "type": 2,
                        },
                        {
                            "uri": "file://${root_path}/xhp_class_definitions.php",
                            "type": 2,
                        },
                        {
                            "uri": "file://${root_path}/completion_extras_namespace.php",
                            "type": 2,
                        },
                    ]
                },
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .notification(
                comment="Add '$x = $point1['' to test autocomplete for shapes",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 22, "character": 0},
                                "end": {"line": 22, "character": 0},
                            },
                            "text": "$x = $point1['",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after user types a shape",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 22, "character": 14},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "'x'",
                            "kind": 12,
                            "detail": "literal",
                            "sortText": "'x'",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 22, "character": 13},
                                    "end": {"line": 22, "character": 14},
                                },
                                "newText": "'x'",
                            },
                            "data": {
                                "fullname": "'x'",
                                "filename": "${root_path}/completion.php",
                                "line": 22,
                                "char": 19,
                            },
                        },
                        {
                            "label": "'y'",
                            "kind": 12,
                            "detail": "literal",
                            "sortText": "'y'",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 22, "character": 13},
                                    "end": {"line": 22, "character": 14},
                                },
                                "newText": "'y'",
                            },
                            "data": {
                                "fullname": "'y'",
                                "filename": "${root_path}/completion.php",
                                "line": 22,
                                "char": 30,
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add automatically closed apostrophes when typing a shape key, the way visual studio code does it",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 22, "character": 0},
                                "end": {"line": 22, "character": 14},
                            },
                            "text": "$x = $point1['']",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after a shape, with VS Code automatically closed apostrophes",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 22, "character": 14},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "'x",
                            "kind": 12,
                            "detail": "literal",
                            "sortText": "'x",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 22, "character": 13},
                                    "end": {"line": 22, "character": 13},
                                },
                                "newText": "'x",
                            },
                            "data": {
                                "fullname": "'x'",
                                "filename": "${root_path}/completion.php",
                                "line": 22,
                                "char": 19,
                            },
                        },
                        {
                            "label": "'y",
                            "kind": 12,
                            "detail": "literal",
                            "sortText": "'y",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 22, "character": 13},
                                    "end": {"line": 22, "character": 13},
                                },
                                "newText": "'y",
                            },
                            "data": {
                                "fullname": "'y'",
                                "filename": "${root_path}/completion.php",
                                "line": 22,
                                "char": 30,
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 0},
                            },
                            "text": "$x = <",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 6},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 6},
                                    "end": {"line": 3, "character": 6},
                                },
                                "newText": "ab:cd:alpha>$0</ab:cd:alpha>",
                            },
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "sortText": "ab:cd:text",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 6},
                                    "end": {"line": 3, "character": 6},
                                },
                                "newText": "ab:cd:text>$0</ab:cd:text>",
                            },
                            "data": {"fullname": ":ab:cd:text"},
                        },
                        {
                            "label": "xhp:enum-attribute",
                            "kind": 7,
                            "detail": "class",
                            "sortText": "xhp:enum-attribute",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 6},
                                    "end": {"line": 3, "character": 6},
                                },
                                "newText": "xhp:enum-attribute>$0</xhp:enum-attribute>",
                            },
                            "data": {"fullname": ":xhp:enum-attribute"},
                        },
                        {
                            "label": "xhp:generic",
                            "kind": 7,
                            "detail": "class",
                            "sortText": "xhp:generic",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 6},
                                    "end": {"line": 3, "character": 6},
                                },
                                "newText": "xhp:generic>$0</xhp:generic>",
                            },
                            "data": {"fullname": ":xhp:generic"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <a'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 6},
                            },
                            "text": "$x = <a",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <a'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 7},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 6},
                                    "end": {"line": 3, "character": 7},
                                },
                                "newText": "ab:cd:alpha>$0</ab:cd:alpha>",
                            },
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "sortText": "ab:cd:text",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 6},
                                    "end": {"line": 3, "character": 7},
                                },
                                "newText": "ab:cd:text>$0</ab:cd:text>",
                            },
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 7},
                            },
                            "text": "$x = <ab:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 9},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 6},
                                    "end": {"line": 3, "character": 9},
                                },
                                "newText": "ab:cd:alpha>$0</ab:cd:alpha>",
                            },
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "sortText": "ab:cd:text",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 6},
                                    "end": {"line": 3, "character": 9},
                                },
                                "newText": "ab:cd:text>$0</ab:cd:text>",
                            },
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text '",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 9},
                            },
                            "text": "$x = <ab:cd:text ",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text '",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 17},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "width",
                            "kind": 5,
                            "detail": "?int",
                            "sortText": "width",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 17},
                                    "end": {"line": 3, "character": 17},
                                },
                                "newText": "width",
                            },
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": "color",
                            "kind": 5,
                            "detail": "?string",
                            "sortText": "color",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 17},
                                    "end": {"line": 3, "character": 17},
                                },
                                "newText": "color",
                            },
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text w'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 17},
                            },
                            "text": "$x = <ab:cd:text w",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text w'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 18},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "width",
                            "kind": 5,
                            "detail": "?int",
                            "sortText": "width",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 17},
                                    "end": {"line": 3, "character": 18},
                                },
                                "newText": "width",
                            },
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": "color",
                            "kind": 5,
                            "detail": "?string",
                            "sortText": "color",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 17},
                                    "end": {"line": 3, "character": 18},
                                },
                                "newText": "color",
                            },
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = new :'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 18},
                            },
                            "text": "$x = new :",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = new :'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 10},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "sortText": ":ab:cd:alpha",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 9},
                                    "end": {"line": 3, "character": 10},
                                },
                                "newText": ":ab:cd:alpha",
                            },
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": ":ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "sortText": ":ab:cd:text",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 9},
                                    "end": {"line": 3, "character": 10},
                                },
                                "newText": ":ab:cd:text",
                            },
                            "data": {"fullname": ":ab:cd:text"},
                        },
                        {
                            "label": ":xhp:enum-attribute",
                            "kind": 7,
                            "detail": "class",
                            "sortText": ":xhp:enum-attribute",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 9},
                                    "end": {"line": 3, "character": 10},
                                },
                                "newText": ":xhp:enum-attribute",
                            },
                            "data": {"fullname": ":xhp:enum-attribute"},
                        },
                        {
                            "label": ":xhp:generic",
                            "kind": 7,
                            "detail": "class",
                            "sortText": ":xhp:generic",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 9},
                                    "end": {"line": 3, "character": 10},
                                },
                                "newText": ":xhp:generic",
                            },
                            "data": {"fullname": ":xhp:generic"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = new :a'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 10},
                            },
                            "text": "$x = new :a",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = new :a'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 11},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "sortText": ":ab:cd:alpha",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 9},
                                    "end": {"line": 3, "character": 11},
                                },
                                "newText": ":ab:cd:alpha",
                            },
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": ":ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "sortText": ":ab:cd:text",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 9},
                                    "end": {"line": 3, "character": 11},
                                },
                                "newText": ":ab:cd:text",
                            },
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            # Note that this request should match the result in the previous example
            .request(
                line=line(),
                comment="autocomplete resolving after '$x = new :a'",
                method="completionItem/resolve",
                params={
                    "label": ":ab:cd:alpha",
                    "kind": 7,
                    "detail": "class",
                    "insertText": ":ab:cd:alpha",
                    "insertTextFormat": 1,
                    "data": {"fullname": ":ab:cd:alpha"},
                },
                result={
                    "label": ":ab:cd:alpha",
                    "kind": 7,
                    "detail": "class",
                    "documentation": {
                        "kind": "markdown",
                        "value": ":ab:cd:alpha docblock",
                    },
                    "insertText": ":ab:cd:alpha",
                    "insertTextFormat": 1,
                    "data": {"fullname": ":ab:cd:alpha"},
                },
                powered_by="serverless_ide",
            )
            # Try the same thing again, but this time without "new", instead using "<xhp" style
            .notification(
                comment="Add '$x = <a'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 11},
                            },
                            "text": "$x = <a",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <a'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 7},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "ab:cd:alpha",
                            "kind": 7,
                            "detail": "class",
                            "sortText": "ab:cd:alpha",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 6},
                                    "end": {"line": 3, "character": 7},
                                },
                                "newText": "ab:cd:alpha>$0</ab:cd:alpha>",
                            },
                            "data": {"fullname": ":ab:cd:alpha"},
                        },
                        {
                            "label": "ab:cd:text",
                            "kind": 7,
                            "detail": "class",
                            "sortText": "ab:cd:text",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 6},
                                    "end": {"line": 3, "character": 7},
                                },
                                "newText": "ab:cd:text>$0</ab:cd:text>",
                            },
                            "data": {"fullname": ":ab:cd:text"},
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="autocomplete resolving after '$x = <a'",
                method="completionItem/resolve",
                params={
                    "label": "ab:cd:alpha",
                    "kind": 7,
                    "detail": "class",
                    "insertText": "ab:cd:alpha",
                    "insertTextFormat": 1,
                    "data": {"fullname": ":ab:cd:alpha"},
                },
                result={
                    "label": "ab:cd:alpha",
                    "kind": 7,
                    "detail": "class",
                    "documentation": {
                        "kind": "markdown",
                        "value": ":ab:cd:alpha docblock",
                    },
                    "insertText": "ab:cd:alpha",
                    "insertTextFormat": 1,
                    "data": {"fullname": ":ab:cd:alpha"},
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text/>; $y = $x->'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 7},
                            },
                            "text": "$x = <ab:cd:text/>; $y = $x->",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text/>; $y = $x->'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 29},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":width",
                            "kind": 5,
                            "detail": "?int",
                            "sortText": ":width",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 29},
                                    "end": {"line": 3, "character": 29},
                                },
                                "newText": ":width",
                            },
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": ":color",
                            "kind": 5,
                            "detail": "?string",
                            "sortText": ":color",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 29},
                                    "end": {"line": 3, "character": 29},
                                },
                                "newText": ":color",
                            },
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$x = <ab:cd:text/>; $y = $x->:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 29},
                            },
                            "text": "$x = <ab:cd:text/>; $y = $x->:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$x = <ab:cd:text/>; $y = $x->:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 30},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": ":width",
                            "kind": 5,
                            "detail": "?int",
                            "sortText": ":width",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 29},
                                    "end": {"line": 3, "character": 30},
                                },
                                "newText": ":width",
                            },
                            "data": {
                                "fullname": ":width",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 5,
                                "char": 27,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                        {
                            "label": ":color",
                            "kind": 5,
                            "detail": "?string",
                            "sortText": ":color",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 29},
                                    "end": {"line": 3, "character": 30},
                                },
                                "newText": ":color",
                            },
                            "data": {
                                "fullname": ":color",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 5,
                                "char": 13,
                                "base_class": "\\:ab:cd:text",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'test_fun'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 30},
                            },
                            "text": "test_fun",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'test_fun'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 8},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "test_function",
                            "kind": 3,
                            "detail": "function",
                            "sortText": "test_function",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 0},
                                    "end": {"line": 3, "character": 8},
                                },
                                "newText": "test_function",
                            },
                            "data": {"fullname": "test_function"},
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="autocomplete resolving after 'test_fun'",
                method="completionItem/resolve",
                params={
                    "label": "test_function",
                    "kind": 3,
                    "detail": "function(): void",
                    "insertText": "test_function",
                    "insertTextFormat": 1,
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "fullname": "test_function",
                        "line": 8,
                        "char": 10,
                    },
                },
                result={
                    "label": "test_function",
                    "kind": 3,
                    "detail": "function(): void",
                    "documentation": {
                        "kind": "markdown",
                        "value": "test_function docblock.",
                    },
                    "insertText": "test_function",
                    "insertTextFormat": 1,
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "fullname": "test_function",
                        "line": 8,
                        "char": 10,
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'switch (Elsa::Alonso) { case Elsa:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 8},
                            },
                            "text": "switch (Elsa::Alonso) { case Elsa:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'switch (Elsa::Alonso) { case Elsa:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 34},
                },
                result={"isIncomplete": False, "items": []},
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'switch (Elsa::Alonso) { case Elsa::'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 34},
                            },
                            "text": "switch (Elsa::Alonso) { case Elsa::",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'switch (Elsa::Alonso) { case Elsa::'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 35},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "class",
                            "kind": 21,
                            "detail": "class<this>",
                            "sortText": "class",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 35},
                                    "end": {"line": 3, "character": 35},
                                },
                                "newText": "class",
                            },
                            "data": {
                                "fullname": "class",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 3,
                                "char": 6,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "Bard",
                            "kind": 21,
                            "detail": "Elsa",
                            "sortText": "Bard",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 35},
                                    "end": {"line": 3, "character": 35},
                                },
                                "newText": "Bard",
                            },
                            "data": {
                                "fullname": "Bard",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 3,
                                "char": 12,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "Alonso",
                            "kind": 21,
                            "detail": "Elsa",
                            "sortText": "Alonso",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 35},
                                    "end": {"line": 3, "character": 35},
                                },
                                "newText": "Alonso",
                            },
                            "data": {
                                "fullname": "Alonso",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 3,
                                "char": 12,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "isValid",
                            "kind": 2,
                            "detail": "function(mixed $value)[]: bool",
                            "sortText": "isValid",
                            "insertTextFormat": 2,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 35},
                                    "end": {"line": 3, "character": 35},
                                },
                                "newText": "isValid(${1:\\$value})",
                            },
                            "data": {
                                "fullname": "isValid",
                                "filename": "/tmp/catg/tmpykiuvuh4/hhi_7a9178f/BuiltinEnum.hhi",
                                "line": 51,
                                "char": 34,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "getValues",
                            "kind": 2,
                            "detail": "function()[]: dict<string, Elsa>",
                            "sortText": "getValues",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 35},
                                    "end": {"line": 3, "character": 35},
                                },
                                "newText": "getValues()",
                            },
                            "data": {
                                "fullname": "getValues",
                                "filename": "/tmp/catg/tmpykiuvuh4/hhi_7a9178f/BuiltinEnum.hhi",
                                "line": 34,
                                "char": 34,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "getNames",
                            "kind": 2,
                            "detail": "function()[]: dict<Elsa, string>",
                            "sortText": "getNames",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 35},
                                    "end": {"line": 3, "character": 35},
                                },
                                "newText": "getNames()",
                            },
                            "data": {
                                "fullname": "getNames",
                                "filename": "/tmp/catg/tmpykiuvuh4/hhi_7a9178f/BuiltinEnum.hhi",
                                "line": 43,
                                "char": 34,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "coerce",
                            "kind": 2,
                            "detail": "function(mixed $value)[]: ?Elsa",
                            "sortText": "coerce",
                            "insertTextFormat": 2,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 35},
                                    "end": {"line": 3, "character": 35},
                                },
                                "newText": "coerce(${1:\\$value})",
                            },
                            "data": {
                                "fullname": "coerce",
                                "filename": "/tmp/catg/tmpykiuvuh4/hhi_7a9178f/BuiltinEnum.hhi",
                                "line": 58,
                                "char": 34,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "assertAll",
                            "kind": 2,
                            "detail": "function(Traversable<mixed> $values)[]: Container<Elsa>",
                            "sortText": "assertAll",
                            "insertTextFormat": 2,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 35},
                                    "end": {"line": 3, "character": 35},
                                },
                                "newText": "assertAll(${1:\\$values})",
                            },
                            "data": {
                                "fullname": "assertAll",
                                "filename": "/tmp/catg/tmpykiuvuh4/hhi_7a9178f/BuiltinEnum.hhi",
                                "line": 72,
                                "char": 34,
                                "base_class": "\\Elsa",
                            },
                        },
                        {
                            "label": "assert",
                            "kind": 2,
                            "detail": "function(mixed $value)[]: Elsa",
                            "sortText": "assert",
                            "insertTextFormat": 2,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 35},
                                    "end": {"line": 3, "character": 35},
                                },
                                "newText": "assert(${1:\\$value})",
                            },
                            "data": {
                                "fullname": "assert",
                                "filename": "/tmp/catg/tmpykiuvuh4/hhi_7a9178f/BuiltinEnum.hhi",
                                "line": 65,
                                "char": 34,
                                "base_class": "\\Elsa",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'switch (Elsa::Alonso) { case Elsa::Alonso:'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 35},
                            },
                            "text": "switch (Elsa::Alonso) { case Elsa::Alonso:",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'switch (Elsa::Alonso) { case Elsa::Alonso:'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 42},
                },
                result={"isIncomplete": False, "items": []},
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'TestNS\\'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 42},
                            },
                            "text": "TestNS\\",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'TestNS\\'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 7},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "test_func",
                            "kind": 3,
                            "detail": "function",
                            "sortText": "test_func",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 7},
                                    "end": {"line": 3, "character": 7},
                                },
                                "newText": "test_func",
                            },
                            "data": {"fullname": "TestNS\\test_func"},
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '$cc = new CompletionClass(); $cc->interfa'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 7},
                            },
                            "text": "$cc = new CompletionClass(); $cc->interfa",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '$cc = new CompletionClass(); $cc->interfa'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 41},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "interfaceDocBlockMethod",
                            "kind": 2,
                            "detail": "function(): void",
                            "sortText": "interfaceDocBlockMethod",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 34},
                                    "end": {"line": 3, "character": 41},
                                },
                                "newText": "interfaceDocBlockMethod()",
                            },
                            "data": {
                                "fullname": "interfaceDocBlockMethod",
                                "filename": "${root_path}/completion.php",
                                "line": 18,
                                "char": 19,
                                "base_class": "\\CompletionClass",
                            },
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="autocomplete resolving after '$cc = new CompletionClass(); $cc->interfa'",
                method="completionItem/resolve",
                params={
                    "label": "interfaceDocBlockMethod",
                    "kind": 2,
                    "detail": "function(): void",
                    "insertTextFormat": 1,
                    "textEdit": {
                        "range": {
                            "start": {"line": 3, "character": 34},
                            "end": {"line": 3, "character": 41},
                        },
                        "newText": "interfaceDocBlockMethod",
                    },
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "fullname": "interfaceDocBlockMethod",
                        "line": 18,
                        "char": 19,
                    },
                },
                result={
                    "label": "interfaceDocBlockMethod",
                    "kind": 2,
                    "detail": "function(): void",
                    "insertTextFormat": 1,
                    "textEdit": {
                        "range": {
                            "start": {"line": 3, "character": 34},
                            "end": {"line": 3, "character": 41},
                        },
                        "newText": "interfaceDocBlockMethod",
                    },
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "fullname": "interfaceDocBlockMethod",
                        "line": 18,
                        "char": 19,
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'DeprecatedClass::'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 41},
                            },
                            "text": "DeprecatedClass::",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after 'DeprecatedClass::'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 17},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "class",
                            "kind": 21,
                            "detail": "class<this>",
                            "sortText": "class",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 17},
                                    "end": {"line": 3, "character": 17},
                                },
                                "newText": "class",
                            },
                            "data": {
                                "fullname": "class",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 8,
                                "char": 13,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "test_do_not_use",
                            "kind": 2,
                            "detail": "function(): void",
                            "sortText": "~test_do_not_use",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 17},
                                    "end": {"line": 3, "character": 17},
                                },
                                "newText": "test_do_not_use()",
                            },
                            "data": {
                                "fullname": "test_do_not_use",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 12,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "getName",
                            "kind": 2,
                            "detail": "function(): void",
                            "sortText": "getName",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 17},
                                    "end": {"line": 3, "character": 17},
                                },
                                "newText": "getName()",
                            },
                            "data": {
                                "fullname": "getName",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 9,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "getAttributes_DO_NOT_USE",
                            "kind": 2,
                            "detail": "function(): void",
                            "sortText": "~getAttributes_DO_NOT_USE",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 17},
                                    "end": {"line": 3, "character": 17},
                                },
                                "newText": "getAttributes_DO_NOT_USE()",
                            },
                            "data": {
                                "fullname": "getAttributes_DO_NOT_USE",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 11,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                        {
                            "label": "__getLoader",
                            "kind": 2,
                            "detail": "function(): void",
                            "sortText": "~__getLoader",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 17},
                                    "end": {"line": 3, "character": 17},
                                },
                                "newText": "__getLoader()",
                            },
                            "data": {
                                "fullname": "__getLoader",
                                "filename": "${root_path}/completion_extras.php",
                                "line": 10,
                                "char": 26,
                                "base_class": "\\DeprecatedClass",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add 'call_lambda(3, $m'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 30, "character": 0},
                                "end": {"line": 30, "character": 0},
                            },
                            "text": "  call_lambda(3, $m",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete results for 'call_lambda(3, $m'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 30, "character": 19},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "$mylambda",
                            "kind": 6,
                            "detail": "(function(int $n): int)",
                            "sortText": "$mylambda",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 30, "character": 17},
                                    "end": {"line": 30, "character": 19},
                                },
                                "newText": "$mylambda",
                            },
                            "data": {
                                "fullname": "$mylambda",
                                "filename": "${root_path}/completion.php",
                                "line": 30,
                                "char": 15,
                            },
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="resolve autocompletion for $mylambda'",
                method="completionItem/resolve",
                params={
                    "label": "$mylambda",
                    "kind": 6,
                    "detail": "(function(int $n): int)",
                    "insertTextFormat": 1,
                    "textEdit": {
                        "range": {
                            "start": {"line": 30, "character": 17},
                            "end": {"line": 30, "character": 19},
                        },
                        "newText": "$mylambda",
                    },
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "fullname": "function",
                        "line": 30,
                        "char": 15,
                    },
                },
                result={
                    "label": "$mylambda",
                    "kind": 6,
                    "detail": "(function(int $n): int)",
                    "insertTextFormat": 1,
                    "textEdit": {
                        "range": {
                            "start": {"line": 30, "character": 17},
                            "end": {"line": 30, "character": 19},
                        },
                        "newText": "$mylambda",
                    },
                    "data": {
                        "filename": "${root_path}/completion.php",
                        "fullname": "function",
                        "line": 30,
                        "char": 15,
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '<xhp:enum-attribute enum-attribute={}'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 17},
                            },
                            "text": "<xhp:enum-attribute enum-attribute={}",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '<xhp:enum-attribute enum-attribute={'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 36},
                    "context": {"triggerKind": 2, "triggerCharacter": "{"},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "MyEnum::TYPE_C",
                            "kind": 13,
                            "detail": "enum",
                            "sortText": "MyEnum::TYPE_C",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 36},
                                    "end": {"line": 3, "character": 36},
                                },
                                "newText": "MyEnum::TYPE_C",
                            },
                            "data": {
                                "fullname": "MyEnum::TYPE_C",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 13,
                                "char": 14,
                                "base_class": "\\MyEnum",
                            },
                        },
                        {
                            "label": "MyEnum::TYPE_B",
                            "kind": 13,
                            "detail": "enum",
                            "sortText": "MyEnum::TYPE_B",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 36},
                                    "end": {"line": 3, "character": 36},
                                },
                                "newText": "MyEnum::TYPE_B",
                            },
                            "data": {
                                "fullname": "MyEnum::TYPE_B",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 13,
                                "char": 14,
                                "base_class": "\\MyEnum",
                            },
                        },
                        {
                            "label": "MyEnum::TYPE_A",
                            "kind": 13,
                            "detail": "enum",
                            "sortText": "MyEnum::TYPE_A",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 36},
                                    "end": {"line": 3, "character": 36},
                                },
                                "newText": "MyEnum::TYPE_A",
                            },
                            "data": {
                                "fullname": "MyEnum::TYPE_A",
                                "filename": "${root_path}/xhp_class_definitions.php",
                                "line": 13,
                                "char": 14,
                                "base_class": "\\MyEnum",
                            },
                        },
                    ],
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '1 is strin'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 37},
                            },
                            "text": "1 is strin",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="autocomplete after '1 is strin'",
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 10},
                },
                result={
                    "isIncomplete": False,
                    "items": [
                        {
                            "label": "string",
                            "kind": 25,
                            "detail": "builtin",
                            "documentation": {
                                "kind": "markdown",
                                "value": "A sequence of characters.",
                            },
                            "sortText": "string",
                            "insertTextFormat": 1,
                            "textEdit": {
                                "range": {
                                    "start": {"line": 3, "character": 5},
                                    "end": {"line": 3, "character": 10},
                                },
                                "newText": "string",
                            },
                            "data": {"fullname": "string"},
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="autocomplete resolving after '1 is strin'",
                method="completionItem/resolve",
                params={
                    "data": {"fullname": "string"},
                    "detail": "builtin",
                    "documentation": {
                        "kind": "markdown",
                        "value": "A sequence of characters.",
                    },
                    "insertText": "string",
                    "insertTextFormat": 1,
                    "kind": 25,
                    "label": "string",
                    "sortText": "string",
                },
                result={
                    "data": {"fullname": "string"},
                    "detail": "builtin",
                    "documentation": {
                        "kind": "markdown",
                        "value": "A sequence of characters.",
                    },
                    "insertText": "string",
                    "insertTextFormat": 1,
                    "kind": 25,
                    "label": "string",
                    "sortText": "string",
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_definition(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("definition.php"))
        spec = (
            self.initialize_spec(LspTestSpec("serverless_ide_definition"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="call to `b_definition`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 10},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 6, "character": 9},
                            "end": {"line": 6, "character": 21},
                        },
                        "title": "b_definition",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new BB(1)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 29, "character": 13},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 11, "character": 18},
                            "end": {"line": 11, "character": 29},
                        },
                        "title": "BB::__construct",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new CC(1)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 30, "character": 13},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 14, "character": 6},
                            "end": {"line": 14, "character": 8},
                        },
                        "title": "CC",
                    },
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 11, "character": 18},
                            "end": {"line": 11, "character": 29},
                        },
                        "title": "BB::__construct",
                    },
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new DD(1)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 31, "character": 13},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 17, "character": 6},
                            "end": {"line": 17, "character": 8},
                        },
                        "title": "DD",
                    },
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 11, "character": 18},
                            "end": {"line": 11, "character": 29},
                        },
                        "title": "BB::__construct",
                    },
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new EE(1)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 32, "character": 13},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 21, "character": 18},
                            "end": {"line": 21, "character": 29},
                        },
                        "title": "EE::__construct",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new FF(1)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 33, "character": 13},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 26, "character": 6},
                            "end": {"line": 26, "character": 8},
                        },
                        "title": "FF",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="call to `new TakesString(HasString::MyString)`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 45, "character": 23},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 40, "character": 6},
                            "end": {"line": 40, "character": 15},
                        },
                        "title": "HasString",
                    }
                ],
                powered_by="serverless_ide",
            )
            .notification(
                comment="make local, unsaved change to the file",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}", "version": 2},
                    "contentChanges": [
                        {
                            "text": "test",
                            "range": {
                                "start": {"line": 3, "character": 9},
                                "end": {"line": 3, "character": 21},
                            },
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="call to `test` instead of `b_definition`",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 10},
                },
                result=[
                    {
                        "uri": "file://${root_path}/definition.php",
                        "range": {
                            "start": {"line": 28, "character": 9},
                            "end": {"line": 28, "character": 13},
                        },
                        "title": "test",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_overridden_definition(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("override.php"))

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_overridden_definition"),
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="find overridden method from trait. It's arbitrary which one we pick. This test embodies the current implementation.",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 13, "character": 5},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 7, "character": 18},
                            "end": {"line": 7, "character": 21},
                        },
                        "title": "MyTrait::foo",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="find overridden static method. It's arbitrary which one we pick. This test embodies the current implementation.",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 5},
                },
                result=[
                    {
                        "uri": "file://${root_path}/override.php",
                        "range": {
                            "start": {"line": 23, "character": 25},
                            "end": {"line": 23, "character": 28},
                        },
                        "title": "C2::bar",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="find overridden interface method",
                method="textDocument/definition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 35, "character": 5},
                },
                result=[
                    {
                        "uri": "file://${root_path}/override.php",
                        "range": {
                            "start": {"line": 32, "character": 18},
                            "end": {"line": 32, "character": 22},
                        },
                        "title": "I1::quux",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_document_symbol(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("definition.php"))
        spec = (
            self.initialize_spec(LspTestSpec("serverless_ide_document_symbol"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="documentSymbol call",
                method="textDocument/documentSymbol",
                params={"textDocument": {"uri": "${php_file_uri}"}},
                result=[
                    {
                        "name": "First",
                        "kind": 14,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 50, "character": 18},
                                "end": {"line": 50, "character": 47},
                            },
                        },
                        "containerName": "MyEnumClass",
                    },
                    {
                        "name": "MyEnumClass",
                        "kind": 10,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 49, "character": 0},
                                "end": {"line": 52, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "testClassMemberInsideConstructorInvocation",
                        "kind": 12,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 44, "character": 0},
                                "end": {"line": 46, "character": 1},
                            },
                        },
                        "detail": "function(): void",
                    },
                    {
                        "name": "MyString",
                        "kind": 14,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 41, "character": 8},
                                "end": {"line": 41, "character": 29},
                            },
                        },
                        "detail": 'MyString = "myString"',
                        "containerName": "HasString",
                    },
                    {
                        "name": "HasString",
                        "kind": 5,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 40, "character": 0},
                                "end": {"line": 42, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "__construct",
                        "kind": 6,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 37, "character": 2},
                                "end": {"line": 37, "character": 43},
                            },
                        },
                        "detail": "function(string $s)",
                        "containerName": "TakesString",
                    },
                    {
                        "name": "TakesString",
                        "kind": 5,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 36, "character": 0},
                                "end": {"line": 38, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "FF",
                        "kind": 5,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 26, "character": 0},
                                "end": {"line": 26, "character": 11},
                            },
                        },
                    },
                    {
                        "name": "__construct",
                        "kind": 6,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 21, "character": 2},
                                "end": {"line": 23, "character": 3},
                            },
                        },
                        "detail": "function()",
                        "containerName": "EE",
                    },
                    {
                        "name": "EE",
                        "kind": 5,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 20, "character": 0},
                                "end": {"line": 24, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "CC",
                        "kind": 5,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 14, "character": 0},
                                "end": {"line": 15, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "__construct",
                        "kind": 6,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 11, "character": 2},
                                "end": {"line": 11, "character": 40},
                            },
                        },
                        "detail": "function(int $i)",
                        "containerName": "BB",
                    },
                    {
                        "name": "BB",
                        "kind": 5,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 10, "character": 0},
                                "end": {"line": 12, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "a_definition",
                        "kind": 12,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 2, "character": 0},
                                "end": {"line": 4, "character": 1},
                            },
                        },
                        "detail": "function(): int",
                    },
                    {
                        "name": "b_definition",
                        "kind": 12,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 6, "character": 0},
                                "end": {"line": 8, "character": 1},
                            },
                        },
                        "detail": "function(): int",
                    },
                    {
                        "name": "DD",
                        "kind": 5,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 17, "character": 0},
                                "end": {"line": 18, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "test",
                        "kind": 12,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 28, "character": 0},
                                "end": {"line": 34, "character": 1},
                            },
                        },
                        "detail": "function(): void",
                    },
                    {
                        "name": "MyEnumClassKind",
                        "kind": 5,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 48, "character": 0},
                                "end": {"line": 48, "character": 24},
                            },
                        },
                    },
                    {
                        "name": "Second",
                        "kind": 14,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 51, "character": 18},
                                "end": {"line": 51, "character": 48},
                            },
                        },
                        "containerName": "MyEnumClass",
                    },
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_type_definition(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("type_definition.php"))
        spec = (
            self.initialize_spec(LspTestSpec("serverless_ide_type_definition"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="Conditional Type Definition of HH or II",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 32, "character": 2},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 2, "character": 6},
                            "end": {"line": 2, "character": 8},
                        },
                        "title": "\\HH",
                    },
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 12, "character": 6},
                            "end": {"line": 12, "character": 8},
                        },
                        "title": "\\LL",
                    },
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Standard Class Definition",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 40, "character": 2},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 2, "character": 6},
                            "end": {"line": 2, "character": 8},
                        },
                        "title": "\\HH",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Class Type Definition with Casting",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 41, "character": 2},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 2, "character": 6},
                            "end": {"line": 2, "character": 8},
                        },
                        "title": "\\HH",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Primitive Type Definition",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 42, "character": 2},
                },
                result=[],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Function Return Type Definition",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 43, "character": 2},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 12, "character": 6},
                            "end": {"line": 12, "character": 8},
                        },
                        "title": "\\LL",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="Function definition with primitive return type",
                method="textDocument/typeDefinition",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 44, "character": 2},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 22, "character": 9},
                            "end": {"line": 22, "character": 29},
                        },
                        "title": "(function(): int)",
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_hover(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))
        spec = (
            self.initialize_spec(LspTestSpec("serverless_ide_hover"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="hover over function invocation",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 16},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function b_hover(): int"},
                        "---",
                        "A comment describing b_hover.",
                    ],
                    "range": {
                        "start": {"line": 3, "character": 9},
                        "end": {"line": 3, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over string literal outside call",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 25, "character": 12},  # 9 - 16
                },
                result={"contents": [{"language": "hack", "value": "string"}]},
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over string literal inside call",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 20},  # 16 - 29
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "string"},
                        "---",
                        {"language": "hack", "value": "Parameter: $s"},
                    ]
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over int literal inside call",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 32},  # 31 - 33
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "int"},
                        "---",
                        {"language": "hack", "value": "Parameter: $i"},
                    ]
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over constant reference",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 15, "character": 19},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "THE_ANSWER"},
                        "---",
                        "A comment describing THE_ANSWER",
                    ],
                    "range": {
                        "start": {"line": 15, "character": 9},
                        "end": {"line": 15, "character": 19},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over whitespace",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 1},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over a comment",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 1, "character": 4},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover past the end of a line",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 100},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover past the end of a file",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 300, "character": 0},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over class with copyright docblock",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 37, "character": 15},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "final class CopyrightClass"},
                        "---",
                        "Testing copyright removal",
                    ],
                    "range": {
                        "start": {"line": 37, "character": 2},
                        "end": {"line": 37, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over class with generated docblock",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 58, "character": 15},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "final class GeneratedClass"},
                        "---",
                        "Testing generated text removal",
                    ],
                    "range": {
                        "start": {"line": 58, "character": 2},
                        "end": {"line": 58, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over an primitive attribute in an xhp literal",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 62, "character": 25},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "attribute ?string name"},
                        "---",
                        ":xhp:enum-attribute::name docblock",
                    ],
                    "range": {
                        "start": {"line": 62, "character": 22},
                        "end": {"line": 62, "character": 26},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over a nonprimitive attribute in an xhp literal",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 62, "character": 36},
                },
                result={
                    "contents": [
                        {
                            "language": "hack",
                            "value": "attribute ?MyEnum enum-attribute",
                        }
                    ],
                    "range": {
                        "start": {"line": 62, "character": 33},
                        "end": {"line": 62, "character": 47},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover over a generic attribute in an xhp literal",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 63, "character": 16},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "attribute ?ID<EntSomething> id"}
                    ],
                    "range": {
                        "start": {"line": 63, "character": 15},
                        "end": {"line": 63, "character": 17},
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Add '<xhp:enum-attribute name' to test hover for incomplete xhp attribute",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 69, "character": 0},
                                "end": {"line": 69, "character": 0},
                            },
                            "text": "<xhp:enum-attribute name",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="hover over an attribute in an xhp literal without a value",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 69, "character": 22},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "attribute ?string name"},
                        "---",
                        ":xhp:enum-attribute::name docblock",
                    ],
                    "range": {
                        "start": {"line": 69, "character": 20},
                        "end": {"line": 69, "character": 24},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_file_touched_on_disk(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_file_on_disk_change"),
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .notification(
                method="workspace/didChangeWatchedFiles",
                params={"changes": [{"uri": "${php_file_uri}", "type": 2}]},
            )
            .request(
                line=line(),
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 16},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function b_hover(): int"},
                        "---",
                        "A comment describing b_hover.",
                    ],
                    "range": {
                        "start": {"line": 3, "character": 9},
                        "end": {"line": 3, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_file_hover_with_errors(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover_with_errors.php"))
        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_file_hover_with_errors"),
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="Totally normal hover",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 14, "character": 37},
                },
                result={
                    "contents": [
                        "Defined in `HoverWithErrorsClass`",
                        "---",
                        {
                            "language": "hack",
                            "value": "public static function staticMethod(string $z): void",
                        },
                        "---",
                        'During testing, we\'ll remove the "public" tag from this '
                        "method\n"
                        "to ensure that we can still get IDE services",
                    ],
                    "range": {
                        "end": {"character": 39, "line": 14},
                        "start": {"character": 27, "line": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="Remove the 'public' visibility modifier which triggers AST->AAST errors",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 10, "character": 2},
                                "end": {"line": 10, "character": 8},
                            },
                            "text": "",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="Hover should still work even if visibility modifier has been removed",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 14, "character": 37},
                },
                result={
                    "contents": [
                        "Defined in `HoverWithErrorsClass`",
                        "---",
                        {
                            "language": "hack",
                            "value": "public static function staticMethod(string $z): void",
                        },
                        "---",
                        'During testing, we\'ll remove the "public" tag from this '
                        "method\n"
                        "to ensure that we can still get IDE services",
                    ],
                    "range": {
                        "end": {"character": 39, "line": 14},
                        "start": {"character": 27, "line": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_formatting(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("messy.php"))
        spec = (
            self.initialize_spec(LspTestSpec("formatting"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                method="textDocument/formatting",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "options": {"tabSize": 5, "insertSpaces": True},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 3, "character": 0},
                            "end": {"line": 11, "character": 0},
                        },
                        "newText": '     $a = "this";\n\n'
                        + '     $b = "is";\n\n'
                        + '     $c = "messy";\n\n'
                        + '     $d = ".";\n'
                        + '     return "$a"."$b"."$c"."d";\n',
                    }
                ],
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_rangeformatting(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("messy.php"))
        spec = (
            self.initialize_spec(LspTestSpec("range_formatting"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                method="textDocument/rangeFormatting",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "range": {
                        "start": {"line": 3, "character": 0},
                        "end": {"line": 4, "character": 0},
                    },
                    "options": {"tabSize": 5, "insertSpaces": True},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 3, "character": 0},
                            "end": {"line": 4, "character": 0},
                        },
                        "newText": '     $a = "this";\n',
                    }
                ],
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_ontypeformatting(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("ontypeformatting.php"))

        spec = (
            self.initialize_spec(LspTestSpec("ontypeformatting"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                method="textDocument/onTypeFormatting",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 9, "character": 58},
                    "ch": ";",
                    "options": {"tabSize": 2, "insertSpaces": True},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 5, "character": 23},
                            "end": {"line": 9, "character": 58},
                        },
                        "newText": "{\n  test_otf(\n"
                        + "    '1234567890',\n"
                        + "    '1234567890',\n"
                        + "    '1234567890',\n"
                        + "    '1234567890',\n"
                        + "    '1234567890',\n"
                        + "    '1234567890',\n  );",
                    }
                ],
            )
            .request(
                line=line(),
                method="textDocument/onTypeFormatting",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 15, "character": 23},
                    "ch": "}",
                    "options": {"tabSize": 2, "insertSpaces": True},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 15, "character": 0},
                            "end": {"line": 15, "character": 23},
                        },
                        "newText": "function otf(): void {}",
                    }
                ],
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables)

    def test_top_level_def_name_at_pos(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("completion.php"))
        spec = (
            self.initialize_spec(LspTestSpec("top_level_def_name_at_pos"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                method="custom/topLevelDefNameAtPos",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 17, "character": 18},
                },
                result="CompletionClass",
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                method="custom/topLevelDefNameAtPos",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 12, "character": 1},
                },
                result="CompletionInterface",
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                method="custom/topLevelDefNameAtPos",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 16, "character": 0},
                },
                result="CompletionClass",
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                method="custom/topLevelDefNameAtPos",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 18, "character": 1},
                },
                result="CompletionClass",
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_did_change(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("didchange.php"))
        spec = (
            self.initialize_spec(LspTestSpec("did_change"))
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={"uri": "${php_file_uri}", "diagnostics": []},
            )
            .notification(
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 7, "character": 11},
                                "end": {"line": 7, "character": 12},
                            },
                            "text": "a",
                        }
                    ],
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 7, "character": 11},
                                "end": {"line": 7, "character": 11},
                            },
                            "severity": 1,
                            "code": 1002,
                            "source": "Hack",
                            "message": "A semicolon ; is expected here.",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                comment="Hack appears to clear out diagnostics before shutting down",
                method="textDocument/publishDiagnostics",
                params={"uri": "${php_file_uri}", "diagnostics": []},
            )
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_fixme(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("fixme.php"))
        spec = (
            self.initialize_spec(LspTestSpec("fixme"))
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={"uri": "${php_file_uri}", "diagnostics": []},
            )
            .notification(
                comment="disable the first fixme by turning 'HH_FIXME' into 'NO_FIXME'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 5},
                                "end": {"line": 3, "character": 7},
                            },
                            "text": "NO",
                        }
                    ],
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 4, "character": 9},
                                "end": {"line": 4, "character": 10},
                            },
                            "severity": 1,
                            "code": 4110,
                            "source": "Hack",
                            "message": "Invalid return type",
                            "relatedInformation": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 2, "character": 25},
                                            "end": {"line": 2, "character": 31},
                                        },
                                    },
                                    "message": "Expected string",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 4, "character": 9},
                                            "end": {"line": 4, "character": 10},
                                        },
                                    },
                                    "message": "But got int",
                                },
                            ],
                            "relatedLocations": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 2, "character": 25},
                                            "end": {"line": 2, "character": 31},
                                        },
                                    },
                                    "message": "Expected string",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 4, "character": 9},
                                            "end": {"line": 4, "character": 10},
                                        },
                                    },
                                    "message": "But got int",
                                },
                            ],
                        }
                    ],
                },
            )
            .notification(
                comment="restore the first fixme by turning 'NO_FIXME' back 'HH_FIXME'",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 3, "character": 5},
                                "end": {"line": 3, "character": 7},
                            },
                            "text": "HH",
                        }
                    ],
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={"uri": "${php_file_uri}", "diagnostics": []},
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_go_to_implementation(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("go_to_implementation.php"))
        self.test_driver.start_hh_server()
        self.test_driver.run_check()
        spec = (
            self.initialize_spec(LspTestSpec("test_go_to_implementation"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .wait_for_hh_server_ready()
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="go to implementation: abstract class",
                method="textDocument/implementation",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 1, "character": 17},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 7, "character": 6},
                            "end": {"line": 7, "character": 9},
                        },
                    }
                ],
            )
            .request(
                line=line(),
                comment="go to implementation: interface",
                method="textDocument/implementation",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 13, "character": 13},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 17, "character": 6},
                            "end": {"line": 17, "character": 9},
                        },
                    }
                ],
            )
            .request(
                line=line(),
                comment="go to implementation: trait",
                method="textDocument/implementation",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 23, "character": 10},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 30, "character": 6},
                            "end": {"line": 30, "character": 16},
                        },
                    }
                ],
            )
            .request(
                line=line(),
                comment="go to implementation: method",
                method="textDocument/implementation",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 19, "character": 18},
                },
                result=[
                    {
                        "uri": "${php_file_uri}",
                        "range": {
                            "start": {"line": 8, "character": 18},
                            "end": {"line": 8, "character": 22},
                        },
                    }
                ],
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_signature_help(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("signaturehelp.php"))
        spec = (
            self.initialize_spec(LspTestSpec("test_signature_help"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="signature help for 0-argument constructor"
                " (left of opening paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 18, "character": 18},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 0-argument constructor",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 18, "character": 19},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function __construct(): void",
                            "documentation": "Constructor with doc block",
                            "parameters": [],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 0-argument constructor"
                " (right of closing paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 18, "character": 20},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (left of opening paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 19, "character": 20},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (right of opening paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 19, "character": 21},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function instanceMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (left of first comma)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 19, "character": 22},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function instanceMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 1,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (right of first comma)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 19, "character": 23},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function instanceMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 1,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (left of closing paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 19, "character": 24},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function instanceMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 1,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument instance method"
                " (right of closing paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 19, "character": 25},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument generic instance method"
                " (left of opening paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 20, "character": 19},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument generic instance method"
                " (right of opening paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 20, "character": 20},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function genericMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Generic instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument generic instance method"
                " (left of first comma)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 20, "character": 21},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function genericMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Generic instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 1,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument generic instance method"
                " (right of first comma)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 20, "character": 23},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function genericMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Generic instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 1,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument generic instance method"
                " (left of closing paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 20, "character": 23},
                },
                result={
                    "signatures": [
                        {
                            "label": "public function genericMethod"
                            "(int $x1, int $x2): void",
                            "documentation": "Generic instance method with doc block",
                            "parameters": [{"label": "$x1"}, {"label": "$x2"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 1,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument generic instance method"
                " (right of closing paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 20, "character": 25},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 1-argument static method"
                " (left of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 21, "character": 23},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 1-argument static method"
                " (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 21, "character": 24},
                },
                result={
                    "signatures": [
                        {
                            "label": "public static function staticMethod"
                            "(string $z): void",
                            "documentation": "Static method with doc block",
                            "parameters": [{"label": "$z"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument global function"
                " (left of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 22, "character": 17},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument global function"
                " (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 22, "character": 18},
                },
                result={
                    "signatures": [
                        {
                            "label": "function global_function"
                            "(string $s, int $x): void",
                            "documentation": "Global function with doc block",
                            "parameters": [{"label": "$s"}, {"label": "$x"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 1-argument namespace-aliased global"
                " function (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 23, "character": 26},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 1-argument namespace-aliased global"
                " function (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 23, "character": 26},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 1-argument namespace-aliased global"
                " function (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 23, "character": 27},
                },
                result={
                    "signatures": [
                        {
                            "label": "function aliased_global_func(string $s): void",
                            "documentation": "Namespace-aliased function with doc block",
                            "parameters": [{"label": "$s"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 1-argument namespace-aliased global"
                " function (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 23, "character": 28},
                },
                result={
                    "signatures": [
                        {
                            "label": "function aliased_global_func(string $s): void",
                            "documentation": "Namespace-aliased function with doc block",
                            "parameters": [{"label": "$s"}],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument function with params"
                " (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 24, "character": 30},
                },
                result={
                    "signatures": [
                        {
                            "label": "function test_signature_help_params1("
                            "\n  string $param1,\n  string $param2\n): void",
                            "documentation": "comment describing the method"
                            "\n@param $param1 info1"
                            "\n@param param2 info2",
                            "parameters": [
                                {"label": "$param1", "documentation": "info1"},
                                {"label": "$param2", "documentation": "info2"},
                            ],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument function with params"
                " (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 25, "character": 30},
                },
                result={
                    "signatures": [
                        {
                            "label": "function test_signature_help_params2("
                            "\n  string $param1,\n  string $param2\n): void",
                            "documentation": "comment describing the method"
                            "\n@param $param1 info1",
                            "parameters": [
                                {"label": "$param1", "documentation": "info1"},
                                {"label": "$param2"},
                            ],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for 2-argument function with params"
                " (right of open paren)",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 30},
                },
                result={
                    "signatures": [
                        {
                            "label": "function test_signature_help_params3("
                            "\n  string $param1,\n  string $param2\n): string",
                            "documentation": "@param $param1 info1"
                            "\n               for param1"
                            "\n@param $param2   info2"
                            "\n@return the string"
                            "\n        'hack'",
                            "parameters": [
                                {
                                    "label": "$param1",
                                    "documentation": "info1 for param1",
                                },
                                {"label": "$param2", "documentation": "info2"},
                            ],
                        }
                    ],
                    "activeSignature": 0,
                    "activeParameter": 0,
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_signature_help_lambda(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("signaturehelp_lambda.php"))
        spec = (
            self.initialize_spec(
                LspTestSpec("test_serverless_ide_signature_help_lambda"),
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="signature help for a normal function call",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 8, "character": 29},
                },
                result={
                    "activeParameter": 0,
                    "activeSignature": 0,
                    "signatures": [
                        {
                            "label": "function test_lambda_sighelp(\n"
                            "  string $str,\n"
                            "  (function(string): int) $f\n"
                            "): int",
                            "parameters": [{"label": "$str"}, {"label": "$f"}],
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for normal function call within a lambda",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 9, "character": 21},
                },
                result={
                    "activeParameter": 0,
                    "activeSignature": 0,
                    "signatures": [
                        {
                            "label": "function normal_test_func(string $str): void",
                            "parameters": [{"label": "$str"}],
                        }
                    ],
                },
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for text within a lambda, left side of an open paren",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 10, "character": 15},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="signature help for text within a lambda, right side of an open paren",
                method="textDocument/signatureHelp",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 10, "character": 16},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_rename_ok(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("rename.php"))
        variables.update(
            {
                "rename2_file_uri": self.repo_file_uri("rename2.php"),
                "rename2_file": self.read_repo_file("rename2.php"),
            }
        )
        self.test_driver.start_hh_server()
        self.test_driver.run_check()
        self.load_and_run("rename_ok", variables)

    def test_rename_with_server(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("rename.php"))
        self.test_driver.start_hh_server()
        self.test_driver.run_check()
        self.load_and_run("rename_with_server", variables)

    def test_rename_in_interface(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("rename_in_interface.php"))
        self.test_driver.start_hh_server()
        self.test_driver.run_check()

        spec = (
            self.initialize_spec(LspTestSpec("rename_in_interface"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .wait_for_hh_server_ready()
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                method="textDocument/rename",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 19},
                    "newName": "previouslyCalledSomeMethod",
                },
                result={
                    "changes": {
                        "${php_file_uri}": [
                            {
                                "range": {
                                    "start": {"line": 3, "character": 18},
                                    "end": {"line": 3, "character": 28},
                                },
                                "newText": "previouslyCalledSomeMethod",
                            },
                        ]
                    }
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_rename_let(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        file_base_name = "rename_let.php"
        php_file_uri = self.repo_file_uri(file_base_name)
        contents = """<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>
function f(): void {
  $z = 3;
  let $xyz: int = $z;
  $xyz;
}"""
        variables.update({"php_file_uri": php_file_uri, "contents": contents})
        self.test_driver.start_hh_server()
        self.test_driver.run_check()

        spec = (
            self.initialize_spec(LspTestSpec("rename_let"))
            .write_to_disk(
                comment="create file ${file_base_name}",
                uri="${php_file_uri}",
                contents="${contents}",
                notify=False,
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .wait_for_hh_server_ready()
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${contents}",
                    }
                },
            )
            .request(
                line=line(),
                method="textDocument/rename",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 4, "character": 6},
                    "newName": "$ab",
                },
                result={
                    "changes": {
                        "${php_file_uri}": [
                            {
                                "range": {
                                    "start": {"line": 4, "character": 6},
                                    "end": {"line": 4, "character": 10},
                                },
                                "newText": "$ab",
                            },
                            {
                                "range": {
                                    "start": {"line": 5, "character": 2},
                                    "end": {"line": 5, "character": 6},
                                },
                                "newText": "$ab",
                            },
                        ]
                    }
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)
        self.run_spec(spec, variables)

    def test_references_ok(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("references.php"))
        self.test_driver.start_hh_server()
        self.test_driver.run_check()
        self.load_and_run("references_ok", variables)

    def test_references_server_cancel(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("references.php"))
        self.test_driver.start_hh_server()
        self.test_driver.run_check()
        self.load_and_run("references_server_cancel", variables)

    def test_references_client_cancel(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("references.php"))
        self.test_driver.start_hh_server()
        self.load_and_run("references_client_cancel", variables)

    def test_references_client_retry_after_cancel(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("references.php"))
        self.test_driver.start_hh_server()
        self.load_and_run("references_client_retry_after_cancel", variables)

    def test_references_client_double_cancel(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("references.php"))
        self.test_driver.start_hh_server()
        self.load_and_run("references_client_double_cancel", variables)

    def test_references_with_server(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("references.php"))
        self.test_driver.start_hh_server()
        self.test_driver.run_check()
        self.load_and_run("references_with_server", variables)

    def test_references_partial_with_server(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("references.php"))
        self.test_driver.start_hh_server()
        self.test_driver.run_check()
        self.load_and_run("references_partial_with_server", variables)

    def test_references_partial_with_server_2(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("references.php"))
        self.test_driver.start_hh_server()
        self.test_driver.run_check()
        self.load_and_run("references_partial_with_server_2", variables)

    def test_references_no_server(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("references.php"))
        self.load_and_run("references_no_server", variables)

    def test_non_existing_method(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("nomethod.php"))
        self.load_and_run("nomethod", variables)

    def test_bad_call(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("bad_call.php"))
        self.load_and_run("bad_call", variables)

    def test_code_action_missing_method(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("code_action_missing_method.php"))

        spec = (
            self.initialize_spec(LspTestSpec("code_action_missing_method"))
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                comment="make local, unsaved change to the file",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}", "version": 2},
                    "contentChanges": [
                        {
                            "text": """\
<?hh

class ClassWithFooBar {
  public function foobar(): void {}
}

function call_method(ClassWithFooBar $mc): void {
  $mc->foobaz();
}
"""
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="get actions",
                method="textDocument/codeAction",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "range": {
                        "start": {"line": 7, "character": 7},
                        "end": {"line": 7, "character": 13},
                    },
                    "context": {
                        "diagnostics": [
                            {
                                "range": {
                                    "start": {"line": 7, "character": 7},
                                    "end": {"line": 7, "character": 13},
                                },
                                "severity": 1,
                                "code": 4053,
                                "source": "Hack",
                                "message": "No instance method foobaz in ClassWithFooBar",
                                "relatedInformation": [
                                    {
                                        "location": {
                                            "uri": "${php_file_uri}",
                                            "range": {
                                                "start": {"line": 3, "character": 18},
                                                "end": {"line": 3, "character": 24},
                                            },
                                        },
                                        "message": "Did you mean foobar instead?",
                                    },
                                    {
                                        "location": {
                                            "uri": "${php_file_uri}",
                                            "range": {
                                                "start": {"line": 6, "character": 21},
                                                "end": {"line": 6, "character": 36},
                                            },
                                        },
                                        "message": "This is why I think it is an object of type ClassWithFooBar",
                                    },
                                    {
                                        "location": {
                                            "uri": "${php_file_uri}",
                                            "range": {
                                                "start": {"line": 2, "character": 6},
                                                "end": {"line": 2, "character": 21},
                                            },
                                        },
                                        "message": "Declaration of ClassWithFooBar is here",
                                    },
                                ],
                                "relatedLocations": [
                                    {
                                        "location": {
                                            "uri": "${php_file_uri}",
                                            "range": {
                                                "start": {"line": 3, "character": 18},
                                                "end": {"line": 3, "character": 24},
                                            },
                                        },
                                        "message": "Did you mean foobar instead?",
                                    },
                                    {
                                        "location": {
                                            "uri": "${php_file_uri}",
                                            "range": {
                                                "start": {"line": 6, "character": 21},
                                                "end": {"line": 6, "character": 36},
                                            },
                                        },
                                        "message": "This is why I think it is an object of type ClassWithFooBar",
                                    },
                                    {
                                        "location": {
                                            "uri": "${php_file_uri}",
                                            "range": {
                                                "start": {"line": 2, "character": 6},
                                                "end": {"line": 2, "character": 21},
                                            },
                                        },
                                        "message": "Declaration of ClassWithFooBar is here",
                                    },
                                ],
                            }
                        ]
                    },
                },
                result=[
                    {
                        "title": "Change to ->foobar",
                        "kind": "quickfix",
                        "diagnostics": [],
                        "data": {
                            "originalRequest": {
                                "textDocument": {"uri": "${php_file_uri}"},
                                "range": {
                                    "start": {"line": 7, "character": 7},
                                    "end": {"line": 7, "character": 13},
                                },
                                "context": {
                                    "diagnostics": [
                                        {
                                            "range": {
                                                "start": {"line": 7, "character": 7},
                                                "end": {"line": 7, "character": 13},
                                            },
                                            "severity": 1,
                                            "code": 4053,
                                            "source": "Hack",
                                            "message": "No instance method foobaz in ClassWithFooBar",
                                            "relatedInformation": [
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 3,
                                                                "character": 18,
                                                            },
                                                            "end": {
                                                                "line": 3,
                                                                "character": 24,
                                                            },
                                                        },
                                                    },
                                                    "message": "Did you mean foobar instead?",
                                                },
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 6,
                                                                "character": 21,
                                                            },
                                                            "end": {
                                                                "line": 6,
                                                                "character": 36,
                                                            },
                                                        },
                                                    },
                                                    "message": "This is why I think it is an object of type ClassWithFooBar",
                                                },
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 2,
                                                                "character": 6,
                                                            },
                                                            "end": {
                                                                "line": 2,
                                                                "character": 21,
                                                            },
                                                        },
                                                    },
                                                    "message": "Declaration of ClassWithFooBar is here",
                                                },
                                            ],
                                            "relatedLocations": [
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 3,
                                                                "character": 18,
                                                            },
                                                            "end": {
                                                                "line": 3,
                                                                "character": 24,
                                                            },
                                                        },
                                                    },
                                                    "message": "Did you mean foobar instead?",
                                                },
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 6,
                                                                "character": 21,
                                                            },
                                                            "end": {
                                                                "line": 6,
                                                                "character": 36,
                                                            },
                                                        },
                                                    },
                                                    "message": "This is why I think it is an object of type ClassWithFooBar",
                                                },
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 2,
                                                                "character": 6,
                                                            },
                                                            "end": {
                                                                "line": 2,
                                                                "character": 21,
                                                            },
                                                        },
                                                    },
                                                    "message": "Declaration of ClassWithFooBar is here",
                                                },
                                            ],
                                        }
                                    ]
                                },
                            }
                        },
                    },
                    {
                        "title": "Extract into variable",
                        "kind": "refactor",
                        "diagnostics": [],
                        "data": {
                            "originalRequest": {
                                "textDocument": {"uri": "${php_file_uri}"},
                                "range": {
                                    "start": {"line": 7, "character": 7},
                                    "end": {"line": 7, "character": 13},
                                },
                                "context": {
                                    "diagnostics": [
                                        {
                                            "range": {
                                                "start": {"line": 7, "character": 7},
                                                "end": {"line": 7, "character": 13},
                                            },
                                            "severity": 1,
                                            "code": 4053,
                                            "source": "Hack",
                                            "message": "No instance method foobaz in ClassWithFooBar",
                                            "relatedInformation": [
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 3,
                                                                "character": 18,
                                                            },
                                                            "end": {
                                                                "line": 3,
                                                                "character": 24,
                                                            },
                                                        },
                                                    },
                                                    "message": "Did you mean foobar instead?",
                                                },
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 6,
                                                                "character": 21,
                                                            },
                                                            "end": {
                                                                "line": 6,
                                                                "character": 36,
                                                            },
                                                        },
                                                    },
                                                    "message": "This is why I think it is an object of type ClassWithFooBar",
                                                },
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 2,
                                                                "character": 6,
                                                            },
                                                            "end": {
                                                                "line": 2,
                                                                "character": 21,
                                                            },
                                                        },
                                                    },
                                                    "message": "Declaration of ClassWithFooBar is here",
                                                },
                                            ],
                                            "relatedLocations": [
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 3,
                                                                "character": 18,
                                                            },
                                                            "end": {
                                                                "line": 3,
                                                                "character": 24,
                                                            },
                                                        },
                                                    },
                                                    "message": "Did you mean foobar instead?",
                                                },
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 6,
                                                                "character": 21,
                                                            },
                                                            "end": {
                                                                "line": 6,
                                                                "character": 36,
                                                            },
                                                        },
                                                    },
                                                    "message": "This is why I think it is an object of type ClassWithFooBar",
                                                },
                                                {
                                                    "location": {
                                                        "uri": "${php_file_uri}",
                                                        "range": {
                                                            "start": {
                                                                "line": 2,
                                                                "character": 6,
                                                            },
                                                            "end": {
                                                                "line": 2,
                                                                "character": 21,
                                                            },
                                                        },
                                                    },
                                                    "message": "Declaration of ClassWithFooBar is here",
                                                },
                                            ],
                                        }
                                    ]
                                },
                            }
                        },
                    },
                    {
                        "title": "Fix Hack error inline - No instance method `foobaz` in `ClassWithFooBar`",
                        "kind": "",
                        "diagnostics": [],
                        "edit": {"changes": {}},
                        "command": {
                            "title": "Fix Hack error inline - No instance method `foobaz` in `ClassWithFooBar`",
                            "command": "code-compose.show-inline-chat",
                            "arguments": [
                                {
                                    "entrypoint": "FixLintErrorCodeAction",
                                    "predefinedPrompt": {
                                        "command": "Fix Hack error inline",
                                        "displayPrompt": "Fix inline - No instance method `foobaz` in `ClassWithFooBar`",
                                        "userPrompt": "Given the following snippet of Hack code that is part of the file:\n<SNIPPET>\n```hack\n 7 | function call_method(ClassWithFooBar $mc): void {\n 8 |   $mc->[DIAGNOSTIC_START]foobar[DIAGNOSTIC_END]();\n 9 | }\n```\n</SNIPPET>\n<DIAGNOSTIC>\nerror: Typing[4053] No instance method foobaz in ClassWithFooBar\n\nFile code_action_missing_method.php, line 8, character 8 - line 8, character 13:\n\n 5 | }\n 6 | \n 7 | function call_method(ClassWithFooBar $mc): void {\n 8 |   $mc->»foobar«();\n 9 | }\n\nDid you mean foobar instead?\n\nFile code_action_missing_method.php, line 4, character 19 - line 4, character 24:\n\n 1 | <?hh\n 2 | \n 3 | class ClassWithFooBar {\n 4 |   public function »foobar«(): void {}\n 5 | }\n 6 | \n 7 | function call_method(ClassWithFooBar $mc): void {\n\nThis is why I think it is an object of type ClassWithFooBar\n\nFile code_action_missing_method.php, line 7, character 22 - line 7, character 36:\n\n 5 | }\n 6 | \n 7 | function call_method(»ClassWithFooBar« $mc): void {\n 8 |   $mc->foobar();\n 9 | }\n\nDeclaration of ClassWithFooBar is here\n\nFile code_action_missing_method.php, line 3, character 7 - line 3, character 21:\n\n 1 | <?hh\n 2 | \n 3 | class »ClassWithFooBar« {\n 4 |   public function foobar(): void {}\n 5 | }\n 6 | \n 7 | function call_method(ClassWithFooBar $mc): void {\n\n\n</DIAGNOSTIC>\nEdit <SNIPPET> in a way that would fix that lint.\n   If there are multiple ways to fix this issue, please return in the code section the most strightforward one that is part of <SNIPPET>,\n   any further suggestions can be added in the explanation section.",
                                        "description": "Fix Hack error inline",
                                        "model": "iCodeLlama 3.1 70B",
                                    },
                                    "overrideSelection": {
                                        "start": {"line": 6, "character": 0},
                                        "end": {"line": 8, "character": 1},
                                    },
                                    "webviewStartLine": 6,
                                    "extras": {
                                        "lineAgnosticHash": "6c30c21bea1128b0",
                                        "legacyUserPrompt": "Given the following snippet of Hack code that is part of the file:\n<SNIPPET>\n```hack\n 7 | function call_method(ClassWithFooBar $mc): void {\n 8 |   $mc->[DIAGNOSTIC_START]foobar[DIAGNOSTIC_END]();\n 9 | }\n```\n</SNIPPET>\n<DIAGNOSTIC>\nNo instance method `foobaz` in `ClassWithFooBar`\n</DIAGNOSTIC>\n<HINT>\nDid you mean `foobar` instead?\nlocation uri:${root_path}/code_action_missing_method.php\n</HINT>\n<HINT>\nThis is why I think it is an object of type ClassWithFooBar\nlocation uri:${root_path}/code_action_missing_method.php\n</HINT>\n<HINT>\nDeclaration of `ClassWithFooBar` is here\nlocation uri:${root_path}/code_action_missing_method.php\n</HINT>\nEdit <SNIPPET> in a way that would fix that lint.\n   If there are multiple ways to fix this issue, please return in the code section the most strightforward one that is part of <SNIPPET>,\n   any further suggestions can be added in the explanation section.",
                                    },
                                }
                            ],
                        },
                        "data": {"isAI": True},
                    },
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_code_action_flip_around_comma(self) -> None:
        """This test is mainly for testing lazy code action resolution:
        - The server returns a code action with neither 'edit' nor 'command' field
        - The client must send `codeAction/resolve`
        - The server then replies with a complete code action
        """
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("code_action_flip_around_comma.php"))

        spec = (
            self.initialize_spec(LspTestSpec("code_action_flip_around_comma"))
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .request(
                line=line(),
                comment="get actions",
                method="textDocument/codeAction",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "range": {
                        "start": {"line": 3, "character": 10},
                        "end": {"line": 3, "character": 10},
                    },
                    "context": {"diagnostics": []},
                },
                result=[
                    {
                        "title": "Flip around comma",
                        "kind": "refactor",
                        "diagnostics": [],
                        "data": {
                            "originalRequest": {
                                "textDocument": {"uri": "${php_file_uri}"},
                                "range": {
                                    "start": {"line": 3, "character": 10},
                                    "end": {"line": 3, "character": 10},
                                },
                                "context": {"diagnostics": []},
                            }
                        },
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="resolve code action",
                method="codeAction/resolve",
                params={
                    "title": "Flip around comma",
                    "data": {
                        "originalRequest": {
                            "textDocument": {"uri": "${php_file_uri}"},
                            "range": {
                                "start": {"line": 3, "character": 10},
                                "end": {"line": 3, "character": 10},
                            },
                            "context": {"diagnostics": []},
                        },
                    },
                    "kind": "refactor",
                    "diagnostics": [],
                },
                result={
                    "title": "Flip around comma",
                    "kind": "refactor",
                    "diagnostics": [],
                    "edit": {
                        "changes": {
                            "${php_file_uri}": [
                                {
                                    "range": {
                                        "start": {"line": 3, "character": 6},
                                        "end": {"line": 3, "character": 19},
                                    },
                                    "newText": '"b", "a", "c"',
                                }
                            ]
                        }
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_code_action_content_modified(self) -> None:
        """Test that we handle the following situation gracefully:
        - client sends textDocument/codeAction
        - server sends back a partially-resolved code action
        - client sends codeAction/resolve with a *different* position s.t.
        the server can't find a code action with the given position and title.
        We should reply with an LSP error `ContentModified`, per
        https://github.com/microsoft/language-server-protocol/issues/1738
        """
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("code_action_flip_around_comma.php"))

        spec = (
            self.initialize_spec(LspTestSpec("code_action_flip_around_comma"))
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .request(
                line=line(),
                comment="get actions",
                method="textDocument/codeAction",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "range": {
                        "start": {"line": 3, "character": 10},
                        "end": {"line": 3, "character": 10},
                    },
                    "context": {"diagnostics": []},
                },
                result=[
                    {
                        "title": "Flip around comma",
                        "kind": "refactor",
                        "diagnostics": [],
                        "data": {
                            "originalRequest": {
                                "textDocument": {"uri": "${php_file_uri}"},
                                "range": {
                                    "start": {"line": 3, "character": 10},
                                    "end": {"line": 3, "character": 10},
                                },
                                "context": {"diagnostics": []},
                            },
                        },
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="resolve code action",
                method="codeAction/resolve",
                params={
                    "title": "Flip around comma",
                    "data": {
                        "originalRequest": {
                            "textDocument": {"uri": "${php_file_uri}"},
                            "range": {
                                "start": {"line": 3, "character": 2},
                                "end": {"line": 3, "character": 3},
                            },
                            "context": {"diagnostics": []},
                        },
                    },
                    "kind": "refactor",
                    "diagnostics": [],
                },
                result={
                    "code": -32801,
                    "message": "Expected the code action requested with codeAction/resolve to be findable.\nNote: This error message may be caused by the source text changing between\nwhen the code action menu pops up and when the user selects the code action.\nIn such cases we may not be able to find a code action at the same location with\nthe same title, so cannot resolve the code action.\n        ",
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_hierarchy_file_change_on_disk(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("incremental_derived.php"))
        changed_php_file_uri = self.repo_file("incremental_base.php")
        variables.update({"changed_php_file_uri": changed_php_file_uri})

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_hierarchy_file_change_on_disk"),
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="hover before change to class hierarchy should be `int`",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 7, "character": 14},
                },
                result={
                    "contents": [
                        "Defined in `BaseClassIncremental`",
                        "---",
                        {
                            "language": "hack",
                            "value": "public function foo(): int",
                        },
                    ],
                    "range": {
                        "start": {"line": 7, "character": 12},
                        "end": {"line": 7, "character": 15},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                uri=changed_php_file_uri,
                contents="""\
<?hh
class BaseClassIncremental {
  public function foo(): string { return ''; }
}
""",
                notify=True,
            )
            .request(
                line=line(),
                comment="hover after change to class hierarchy should be `string`",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 7, "character": 14},
                },
                result={
                    "contents": [
                        "Defined in `BaseClassIncremental`",
                        "---",
                        {
                            "language": "hack",
                            "value": "public function foo(): string",
                        },
                    ],
                    "range": {
                        "start": {"line": 7, "character": 12},
                        "end": {"line": 7, "character": 15},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables)

    def test_incrementality(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("incrementality_use.php"))
        parent_php_file_uri = self.repo_file("incrementality_parent.php")
        variables.update({"parent_php_file_uri": parent_php_file_uri})
        child_php_file_uri = self.repo_file("incrementality_child.php")
        variables.update({"child_php_file_uri": child_php_file_uri})

        spec = (
            self.initialize_spec(
                LspTestSpec("incrementality"),
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${parent_php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": self.read_repo_file("incrementality_parent.php"),
                    }
                },
            )
            .request(
                line=line(),
                comment="hover before change to class hierarchy should be `int`",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 6},
                },
                result={
                    "contents": [
                        "Defined in `Parentt`",
                        "---",
                        {
                            "language": "hack",
                            "value": "public function m(int $_): void",
                        },
                    ],
                    "range": {
                        "start": {"line": 3, "character": 6},
                        "end": {"line": 3, "character": 7},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                uri=parent_php_file_uri,
                contents="""\
<?hh

class Parentt {
  public function m(string $_): void {}
}
""",
                notify=True,
            )
            .request(
                line=line(),
                comment="hover after change to class hierarchy should be `string`",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 6},
                },
                result={
                    "contents": [
                        "Defined in `Parentt`",
                        "---",
                        {
                            "language": "hack",
                            "value": "public function m(string $_): void",
                        },
                    ],
                    "range": {
                        "start": {"line": 3, "character": 6},
                        "end": {"line": 3, "character": 7},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables)

    def test_serverless_ide_decl_in_unsaved_buffer_changed(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_decl_in_unsaved_buffer_changed"),
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="hover over function invocation",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 16},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function b_hover(): int"},
                        "---",
                        "A comment describing b_hover.",
                    ],
                    "range": {
                        "start": {"line": 3, "character": 9},
                        "end": {"line": 3, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="make local, unsaved change to the file",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}", "version": 2},
                    "contentChanges": [
                        {
                            "text": """\
<?hh
// comment
function a_hover(): int {
  return b_hover();
}
// A comment describing b_hover differently.
function b_hover(): string {
  return 42;
}
"""
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="another hover over function invocation, should be string now",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 16},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function b_hover(): string"},
                        "---",
                        "A comment describing b_hover differently.",
                    ],
                    "range": {
                        "start": {"line": 3, "character": 9},
                        "end": {"line": 3, "character": 16},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables)

    def test_serverless_ide_decl_two_unsaved_buffers(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("unsaved1.php"))
        variables.update({"unsaved2_file_uri": self.repo_file_uri("unsaved2.php")})

        spec = (
            self.initialize_spec(
                LspTestSpec("test_serverless_ide_decl_two_unsaved_buffers"),
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                comment="open 'unsaved1.php', since we'll be hovering in it",
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .notification(
                comment="open 'unsaved2.php' with a bool-returning signature, different from disk",
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${unsaved2_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": """\
<?hh
function unsaved_bar(): bool { return true; }
""",
                    }
                },
            )
            .request(
                line=line(),
                comment="hover 'unsaved1.php' is with respect to disk contents of 'unsaved2.php'",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 1, "character": 39},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function unsaved_bar(): int"},
                    ],
                    "range": {
                        "start": {"line": 1, "character": 34},
                        "end": {"line": 1, "character": 45},
                    },
                },
                powered_by="serverless_ide",
            )
            .notification(
                comment="change signature in 'unsaved2.php' to return string",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${unsaved2_file_uri}", "version": 2},
                    "contentChanges": [
                        {
                            "text": """\
<?hh
function unsaved_bar(): string { return "hello"; }
"""
                        }
                    ],
                },
            )
            .request(
                line=line(),
                comment="this is a dummy hover in 'unsaved2.php' just to ensure its decl is cached",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${unsaved2_file_uri}"},
                    "position": {"line": 0, "character": 0},
                },
                result=None,
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="hover 'unsaved1.php' is still with respect to disk contents of 'unsaved2.php'",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 1, "character": 39},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function unsaved_bar(): int"},
                    ],
                    "range": {
                        "start": {"line": 1, "character": 34},
                        "end": {"line": 1, "character": 45},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                comment="save signature in 'unsaved2' to return string",
                uri=variables["unsaved2_file_uri"],
                contents="""\
<?hh
function unsaved_bar(): string { return "hello"; }
""",
                notify=True,
            )
            .request(
                line=line(),
                comment="hover 'unsaved1.php' gets new disk contents of 'unsaved2.php'",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 1, "character": 39},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function unsaved_bar(): string"},
                    ],
                    "range": {
                        "start": {"line": 1, "character": 34},
                        "end": {"line": 1, "character": 45},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables)

    def test_hover_without_file_open(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))

        spec = (
            self.initialize_spec(
                LspTestSpec("test_hover_without_file_open"),
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .request(
                line=line(),
                comment="hover before file_open will fail",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 20},
                },
                result=None,
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="hover after file_open will succeed",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 20},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "string"},
                        "---",
                        {"language": "hack", "value": "Parameter: $s"},
                    ]
                },
                powered_by="serverless_ide",
            )
            .notification(
                method="textDocument/didClose",
                params={"textDocument": {"uri": "${php_file_uri}"}},
            )
            .request(
                line=line(),
                comment="hover after file_close will fail",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 26, "character": 20},
                },
                result=None,
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables)

    def test_serverless_ide_highlight(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("highlight.php"))
        spec = (
            self.initialize_spec(LspTestSpec("serverless_ide_highlight"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .request(
                line=line(),
                comment="document highlight, id 2",
                method="textDocument/documentHighlight",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 10},
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 3, "character": 9},
                            "end": {"line": 3, "character": 20},
                        }
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="shutdown, id 3",
                method="shutdown",
                params={},
                result=None,
            )
        )
        self.run_spec(spec, variables)

    def test_status_running(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))

        spec = (
            self.initialize_spec(
                LspTestSpec("status_running"),
                has_status_capability=True,
            )
            .ignore_requests(
                comment="Ignore all status requests not explicitly waited for in the test",
                method="window/showStatus",
                params=None,
            )
            .start_hh_server("starting")
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "message": "<ROOT>\n\nHack IDE support is ready\n\nhh_server is ready",
                    "shortMessage": "Hack",
                    "type": 3,
                },
                result=NoResponse(),
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_status_stopped(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_status_stopped"),
                has_status_capability=True,
            )
            .ignore_requests(
                comment="Ignore all status requests not explicitly waited for in the test",
                method="window/showStatus",
                params=None,
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "type": 1,
                    "message": "<ROOT>\n\nHack IDE support is ready\n\nhh_server is stopped. Try running `hh` at the command-line.",
                    "shortMessage": "Hack: hh_server stopped",
                },
                result=NoResponse(),
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_standalone_status(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))
        spec = (
            self.initialize_spec(
                LspTestSpec("test_standalone_status"),
                has_status_capability=True,
            )
            .ignore_requests(
                comment="Ignore all status requests not explicitly waited for in the test",
                method="window/showStatus",
                params=None,
            )
            .wait_for_server_request(
                comment="standalone status upon startup when it starts with hh_server stopped",
                method="window/showStatus",
                params={
                    "message": "<ROOT>\n\nHack IDE support is ready\n\nhh_server is stopped. Try running `hh` at the command-line.",
                    "shortMessage": "Hack: hh_server stopped",
                    "type": 1,
                },
                result=NoResponse(),
            )
            .start_hh_server("Restart HH Server")
            .wait_for_server_request(
                comment="standalone status when hh_server transitions to starting up",
                method="window/showStatus",
                params={
                    "message": "<ROOT>\n\nHack IDE support is ready\n\nhh_server is ready",
                    "shortMessage": "Hack",
                    "type": 3,
                },
                result=NoResponse(),
            )
            .stop_hh_server("Shutdown HH Server")
            .wait_for_server_request(
                comment="standalone status when hh_server transitions to stopped",
                method="window/showStatus",
                params={
                    "message": "<ROOT>\n\nHack IDE support is ready\n\nhh_server is stopped. Try running `hh` at the command-line.",
                    "shortMessage": "Hack: hh_server stopped",
                    "type": 1,
                },
                result=NoResponse(),
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_standalone_errors(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))
        errors_a_uri = self.repo_file_uri("errors_a.php")
        errors_b_uri = self.repo_file_uri("errors_b.php")
        variables.update({"errors_a_uri": errors_a_uri, "errors_b_uri": errors_b_uri})

        spec = (
            self.initialize_spec(
                LspTestSpec("test_standalone_errors"),
                has_status_capability=True,
            )
            .ignore_requests(
                comment="Ignore all status requests not explicitly waited for in the test",
                method="window/showStatus",
                params=None,
            )
            .write_to_disk(
                comment="create file errors_a.php",
                uri="${errors_a_uri}",
                contents="<?hh\nfunction aaa(): int { return 1 }\n",
                notify=False,
            )
            .write_to_disk(
                comment="create file errors_b.php",
                uri="${errors_b_uri}",
                contents="<?hh\nfunction bbb(): int { return 2 }\n",
                notify=False,
            )
            .notification(
                comment="open errors_b.php",
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${errors_b_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "<?hh\nfunction bbb(): int { return 2 }\n",
                    }
                },
            )
            .notification(
                comment="open errors_a.php",
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${errors_a_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "<?hh\nfunction aaa(): int { return 1 }\n",
                    }
                },
            )
            .notification(
                comment="edit errors_a.php",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${errors_a_uri}", "version": 1},
                    "contentChanges": [
                        {
                            "text": "<?hh\n\n\n\nfunction aaa(): int { return 1 }\n",
                        }
                    ],
                },
            )
            .wait_for_notification(
                comment="standalone should report a squiggle in errors_a.php from serverless",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${errors_a_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 4, "character": 31},
                                "end": {"line": 4, "character": 31},
                            },
                            "severity": 1,
                            "code": 1002,
                            "source": "Hack",
                            "message": "A semicolon ; is expected here.",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .start_hh_server("start HH Server")
            .wait_for_notification(
                comment="standalone should report a squiggle in errors_b.php from errors.bin",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${errors_b_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 31},
                                "end": {"line": 1, "character": 31},
                            },
                            "severity": 1,
                            "code": 1002,
                            "source": "Hack",
                            "message": "A semicolon ; is expected here.",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .start_hh_server("start HH Server")
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                comment="standalone should clean up squiggles - a",
                method="textDocument/publishDiagnostics",
                params={"uri": "${errors_a_uri}", "diagnostics": []},
            )
            .wait_for_notification(
                comment="standalone should clean up squiggles - b",
                method="textDocument/publishDiagnostics",
                params={"uri": "${errors_b_uri}", "diagnostics": []},
            )
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_live_squiggles(self) -> None:
        """This tests that "live squiggles" (those from clientIdeDaemon) are correctly
        produced by didOpen, didChange, codeAction and publishDiagnostics."""
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))
        errors_a_uri = self.repo_file_uri("errors_a.php")
        errors_b_uri = self.repo_file_uri("errors_b.php")
        variables.update({"errors_a_uri": errors_a_uri, "errors_b_uri": errors_b_uri})

        spec = (
            self.initialize_spec(
                LspTestSpec("test_live_squiggles"),
            )
            .write_to_disk(
                comment="create file errors_a.php",
                uri="${errors_a_uri}",
                contents="<?hh\nfunction aaa() { }\n",
                notify=False,
            )
            .notification(
                comment="open errors_a.php",
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${errors_a_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "<?hh\nfunction aaa() { }\n",
                    }
                },
            )
            .wait_for_notification(
                comment="didOpen should report a live squiggle in errors_a.php",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${errors_a_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 9},
                                "end": {"line": 1, "character": 12},
                            },
                            "severity": 1,
                            "code": 4030,
                            "source": "Hack",
                            "message": "Was expecting a return type hint",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .notification(
                comment="change errors_a.php",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${errors_a_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 0},
                                "end": {"line": 1, "character": 0},
                            },
                            "text": "\n",
                        }
                    ],
                },
            )
            .wait_for_notification(
                comment="didChange should update live squiggles in errors_a.php (one line down from what we got in didOpen)",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${errors_a_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 2, "character": 9},
                                "end": {"line": 2, "character": 12},
                            },
                            "severity": 1,
                            "code": 4030,
                            "source": "Hack",
                            "message": "Was expecting a return type hint",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .write_to_disk(
                comment="create file errors_b.php and send didChangeWatchedFiles",
                uri="${errors_b_uri}",
                contents="<?hh\nfunction bbb(): int { return 2; }\n",
                notify=True,
            )
            .request(
                line=line(),
                comment="send codeAction to trigger errors to be refreshed",
                method="textDocument/codeAction",
                params={
                    "textDocument": {"uri": "${errors_a_uri}"},
                    "range": {
                        "start": {"line": 0, "character": 0},
                        "end": {"line": 0, "character": 0},
                    },
                    "context": {"diagnostics": []},
                },
                result=[],
                powered_by="serverless_ide",
            )
            .wait_for_notification(
                comment="codeAction should update live squiggles in errors_a.php (same as what we had from didChange)",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${errors_a_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 2, "character": 9},
                                "end": {"line": 2, "character": 12},
                            },
                            "severity": 1,
                            "code": 4030,
                            "source": "Hack",
                            "message": "Was expecting a return type hint",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .notification(
                method="textDocument/didClose",
                params={"textDocument": {"uri": "${errors_a_uri}"}},
            )
            .wait_for_notification(
                comment="didClose should update live squiggles in (unsaved) errors_a.php back to what they were on disk",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${errors_a_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 9},
                                "end": {"line": 1, "character": 12},
                            },
                            "severity": 1,
                            "code": 4030,
                            "source": "Hack",
                            "message": "Was expecting a return type hint",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                comment="shutdown should clear out live squiggles",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${errors_a_uri}",
                    "diagnostics": [],
                },
            )
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_squiggle_sequence(self) -> None:
        """This tests that diagnostics come at the right time"""
        variables = self.write_hhconf_and_naming_table()
        squiggle_sequence_uri = self.repo_file_uri("squiggle_sequence.php")
        variables.update({"squiggle_sequence_uri": squiggle_sequence_uri})

        spec = (
            self.initialize_spec(
                LspTestSpec("test_squiggle_sequence"),
            )
            .write_to_disk(
                uri="${squiggle_sequence_uri}",
                contents="<?hh\nfunction squiggle_sequence_foo(): void {\n\n}\n",
                notify=False,
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${squiggle_sequence_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "<?hh\nfunction squiggle_sequence_foo(): void {\n\n}\n",
                    }
                },
            )
            .request(
                line=line(),
                method="textDocument/codeAction",
                params={
                    "textDocument": {"uri": "${squiggle_sequence_uri}"},
                    "range": {
                        "start": {"line": 2, "character": 0},
                        "end": {"line": 2, "character": 0},
                    },
                    "context": {"diagnostics": []},
                },
                result=[],
                powered_by="serverless_ide",
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${squiggle_sequence_uri}",
                    "diagnostics": [],
                },
            )
            .notification(
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${squiggle_sequence_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 2, "character": 0},
                                "end": {"line": 2, "character": 0},
                            },
                            "text": "squiggle_sequence_f",
                        }
                    ],
                },
            )
            .request(
                line=line(),
                method="textDocument/completion",
                params={
                    "textDocument": {"uri": "${squiggle_sequence_uri}"},
                    "position": {"line": 2, "character": 11},
                },
                result={
                    "isIncomplete": False,
                    "items": [],
                },
                powered_by="serverless_ide",
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${squiggle_sequence_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 2, "character": 19},
                                "end": {"line": 3, "character": 0},
                            },
                            "severity": 1,
                            "code": 1002,
                            "source": "Hack",
                            "message": "A semicolon ; is expected here.",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        }
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_parsing_squiggles_priority(self) -> None:
        """This tests that parsing squiggles suppress typing squiggles from clientIdeDaemon"""
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))
        errors_a_uri = self.repo_file_uri("errors_a.php")
        variables.update({"errors_a_uri": errors_a_uri})

        spec = (
            self.initialize_spec(
                LspTestSpec("test_parsing_squiggles_priority"),
            )
            .write_to_disk(
                comment="create file errors_a.php",
                uri="${errors_a_uri}",
                contents="<?hh\nfunction aaa(): int { return $undefined }\n",
                notify=False,
            )
            .notification(
                comment="open errors_a.php",
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${errors_a_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "<?hh\nfunction aaa(): int { return $undefined }\n",
                    }
                },
            )
            .wait_for_notification(
                comment="didOpen should report only a parsing squiggle in errors_a.php",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${errors_a_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 40},
                                "end": {"line": 1, "character": 40},
                            },
                            "severity": 1,
                            "code": 1002,
                            "source": "Hack",
                            "message": "A semicolon ; is expected here.",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .notification(
                comment="change errors_a.php",
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${errors_a_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 39},
                                "end": {"line": 1, "character": 39},
                            },
                            "text": ";",
                        }
                    ],
                },
            )
            .wait_for_notification(
                comment="didChange should update live squiggles in errors_a.php (now revealing the typing error)",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${errors_a_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 29},
                                "end": {"line": 1, "character": 39},
                            },
                            "severity": 1,
                            "code": 2050,
                            "source": "Hack",
                            "message": "Variable $undefined is undefined, or not always defined.",
                            "relatedLocations": [],
                            "relatedInformation": [],
                        }
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                comment="shutdown should clear out live squiggles",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${errors_a_uri}",
                    "diagnostics": [],
                },
            )
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_falls_back_to_full_index(self) -> None:
        # Test recovery behavior when we fail to load a naming table, but we're
        # permitted to fall back to the full index naming table build.
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))
        assert "naming_table_saved_state_path" in variables
        variables["naming_table_saved_state_path"] = "/tmp/nonexistent"

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_falls_back_to_full_index"),
                has_status_capability=True,
                wait_for_init_done=False,
            )
            .ignore_requests(
                comment="Ignore all status requests not explicitly waited for in the test",
                method="window/showStatus",
                params=None,
            )
            .wait_for_server_request(
                comment="standalone status upon startup when it starts with hh_server stopped",
                method="window/showStatus",
                params={
                    "message": "<ROOT>\n\nHack IDE support is ready\n\nhh_server is stopped. Try running `hh` at the command-line.",
                    "shortMessage": "Hack: hh_server stopped",
                    "type": 1,
                },
                result=NoResponse(),
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(
            spec,
            variables,
        )

    def test_serverless_ide_failed_to_load_saved_state_no_full_index(self) -> None:
        # This test examines the failure behavior when the naming table is
        # non-existent and we are *not* falling back to full index.
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("hover.php"))
        assert "naming_table_saved_state_path" in variables
        variables["naming_table_saved_state_path"] = "/tmp/nonexistent"

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_status_failed_to_load_saved_state"),
                has_status_capability=True,
                wait_for_init_done=False,
            )
            .ignore_requests(
                comment="Ignore all status requests not explicitly waited for in the test",
                method="window/showStatus",
                params=None,
            )
            .wait_for_notification(
                method="window/logMessage",
                params={
                    "type": 1,
                    "message": "A watchman fault stops Hack from working.\nThis is not a bug in Hack or VSCode.\nConsider consulting Watchman Users group for advice.\nMore details: http://dummy/HH_TEST_MODE",
                },
            )
            .wait_for_notification(
                method="window/showMessage",
                params={
                    "type": 1,
                    "message": "A watchman fault stops Hack from working. See Output\u203aHack for details.",
                },
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "message": "<ROOT>\n\nA watchman fault stops Hack from working. See Output\u203aHack for details.\n\nhh_server is stopped. Try running `hh` at the command-line.",
                    "shortMessage": "Hack: watchman fault",
                    "type": 1,
                },
                result=NoResponse(),
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        # By setting `fall_back_to_full_index` to `False`, the IDE will give up once it fails to load a saved state instead of attempting the naming table build.
        self.run_spec(
            spec,
            variables,
            fall_back_to_full_index=False,
        )

    def test_workspace_symbol(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("didchange.php"))
        spec = (
            self.initialize_spec(LspTestSpec("test_workspace_symbol"))
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .notification(
                comment="LocalIndex doesn't have any names in its SymbolIndex, so let's index these files...",
                method="workspace/didChangeWatchedFiles",
                params={
                    "changes": [
                        {
                            "uri": "file://${root_path}/completion_extras_namespace.php",
                            "type": 2,
                        }
                    ]
                },
            )
            .request(
                line=line(),
                comment="Look up symbols",
                method="workspace/symbol",
                params={"query": "TestNS\\test"},
                result=[
                    {
                        "name": "TestNS\\test_func",
                        "kind": 12,
                        "location": {
                            "uri": "file://${root_path}/completion_extras_namespace.php",
                            "range": {
                                "start": {"line": 4, "character": 9},
                                "end": {"line": 4, "character": 25},
                            },
                        },
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_naming_error1(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("didchange.php"))
        variables.update(
            {
                "main_file": self.repo_file("main.php"),
                "main_file_contents": """\
<?hh
function main(): int {
    return aaa();
}
""",
                "file_a": self.repo_file("a.php"),
                "file_b": self.repo_file("b.php"),
            }
        )
        spec = (
            self.initialize_spec(LspTestSpec("serverless_ide_naming_error1"))
            .write_to_disk(
                uri="${main_file}", contents="${main_file_contents}", notify=True
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${main_file}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${main_file_contents}",
                    }
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "file://${main_file}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 2, "character": 11},
                                "end": {"line": 2, "character": 14},
                            },
                            "severity": 1,
                            "code": 2049,
                            "source": "Hack",
                            "message": "Unbound name: aaa (a global function)",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        },
                        {
                            "range": {
                                "start": {"line": 2, "character": 11},
                                "end": {"line": 2, "character": 14},
                            },
                            "severity": 1,
                            "code": 4107,
                            "source": "Hack",
                            "message": "Unbound name (typing): aaa",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        },
                    ],
                },
            )
            .request(
                line=line(),
                comment="Ensure that hover over `aaa` works even when the name is not yet defined",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${main_file}"},
                    "position": {"line": 2, "character": 13},
                },
                result={
                    "contents": [{"language": "hack", "value": "nothing"}],
                    "range": {
                        "start": {"line": 2, "character": 11},
                        "end": {"line": 2, "character": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                comment="create file A",
                uri="${file_a}",
                contents="""\
<?hh
function aaa(): int {
    return 1;
}
""",
                notify=True,
            )
            .request(
                line=line(),
                comment="Ensure that hover over `aaa` works when there are no naming errors",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${main_file}"},
                    "position": {"line": 2, "character": 13},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function aaa(): int"},
                    ],
                    "range": {
                        "start": {"line": 2, "character": 11},
                        "end": {"line": 2, "character": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                comment="create file B",
                uri="${file_b}",
                contents="""\
<?hh
function aaa(): string {
    return "foo";
}
""",
                notify=True,
            )
            .request(
                line=line(),
                comment="Ensure that hover over `aaa` works even when there is a duplicate name",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${main_file}"},
                    "position": {"line": 2, "character": 13},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function aaa(): int"},
                    ],
                    "range": {
                        "start": {"line": 2, "character": 11},
                        "end": {"line": 2, "character": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .write_to_disk(
                comment="delete file A", uri="${file_a}", contents=None, notify=True
            )
            .request(
                line=line(),
                comment="Now that we've fixed the error, hover should work.",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${main_file}"},
                    "position": {"line": 2, "character": 13},
                },
                result={
                    "contents": [
                        {"language": "hack", "value": "function aaa(): string"},
                    ],
                    "range": {
                        "start": {"line": 2, "character": 11},
                        "end": {"line": 2, "character": 14},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={"uri": "file://${main_file}", "diagnostics": []},
            )
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_naming_error2(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("naming_error_caller.php"))
        variables.update(
            {
                "contents": self.read_repo_file("naming_error_declaration.php"),
                "original": self.repo_file("naming_error_declaration.php"),
                "copy": self.repo_file("naming_error_copy.php"),
            }
        )
        spec = (
            self.initialize_spec(LspTestSpec("serverless_ide_naming_error2"))
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={"uri": "${php_file_uri}", "diagnostics": []},
            )
            .write_to_disk(
                comment="create copy",
                uri="${copy}",
                contents="${contents}",
                notify=True,
            )
            .write_to_disk(
                comment="delete copy", uri="${copy}", contents=None, notify=True
            )
            .request(
                line=line(),
                comment="hover should work fine after making copy then deleting copy.",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 15},
                },
                result={
                    "contents": [
                        {
                            "language": "hack",
                            "value": "function naming_error_declaration(): void",
                        },
                    ],
                    "range": {
                        "start": {"line": 3, "character": 2},
                        "end": {"line": 3, "character": 26},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_naming_error3(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update(self.setup_php_file("naming_error_caller.php"))
        variables.update(
            {
                "contents": self.read_repo_file("naming_error_declaration.php"),
                "original": self.repo_file("naming_error_declaration.php"),
                "copy": self.repo_file("naming_error_copy.php"),
            }
        )
        spec = (
            self.initialize_spec(LspTestSpec("serverless_ide_naming_error3"))
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={"uri": "${php_file_uri}", "diagnostics": []},
            )
            .write_to_disk(
                comment="create copy",
                uri="${copy}",
                contents="${contents}",
                notify=True,
            )
            .write_to_disk(
                comment="delete original", uri="${original}", contents=None, notify=True
            )
            .request(
                line=line(),
                comment="hover should work fine after making copy then deleting original.",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "position": {"line": 3, "character": 15},
                },
                result={
                    "contents": [
                        {
                            "language": "hack",
                            "value": "function naming_error_declaration(): void",
                        },
                    ],
                    "range": {
                        "start": {"line": 3, "character": 2},
                        "end": {"line": 3, "character": 26},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_serverless_ide_requests_before_init(self) -> None:
        variables = self.write_hhconf_and_naming_table()

        spec = (
            self.initialize_spec(
                LspTestSpec("test_serverless_ide_requests_before_init"),
                has_status_capability=True,
                wait_for_init_done=False,
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .ignore_requests(
                method="window/showStatus",
                params={
                    "type": 2,
                    "message": "<ROOT>\n\nHack IDE support is initializing (loading saved state)\n\nhh_server is stopped. Try running `hh` at the command-line.",
                    "shortMessage": "Hack: initializing",
                },
            )
            .ignore_requests(
                method="window/showStatus",
                params={
                    "type": 2,
                    "message": "<ROOT>\n\nHack is working on IDE requests\n\nhh_server is stopped. Try running `hh` at the command-line.",
                    "shortMessage": "Hack: hh_server stopped",
                },
            )
            .write_to_disk(
                notify=True,
                uri="file://${root_path}/beforeInit1.php",
                contents="<?hh\nfunction beforeInit1(): int {\n  return 42;\n}\n",
            )
            .notification(
                comment="open a file before init has finished",
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "file://${root_path}/beforeInit2.php",
                        "languageId": "hack",
                        "version": 1,
                        "text": "<?hh\nfunction beforeInit2(): void {\n  $foo = beforeInit1();\n}\n",
                    }
                },
            )
            .request(
                line=line(),
                comment="hover before init will fail",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "file://${root_path}/beforeInit2.php"},
                    "position": {"line": 2, "character": 4},
                },
                result=None,
            )
            .request(
                line=line(),
                comment="documentSymbol before init will succeed",
                method="textDocument/documentSymbol",
                params={"textDocument": {"uri": "file://${root_path}/beforeInit2.php"}},
                result=[
                    {
                        "name": "beforeInit2",
                        "kind": 12,
                        "location": {
                            "uri": "file://${root_path}/beforeInit2.php",
                            "range": {
                                "start": {"line": 1, "character": 0},
                                "end": {"line": 3, "character": 1},
                            },
                        },
                        "detail": "function(): void",
                    }
                ],
                powered_by="serverless_ide",
            )
            .wait_for_notification(
                comment="wait for clientIdeDaemon to init",
                method="telemetry/event",
                params={"type": 4, "message": "[client-ide] Finished init: ok"},
            )
            .wait_for_server_request(
                method="window/showStatus",
                params={
                    "type": 1,
                    "message": "<ROOT>\n\nHack IDE support is ready\n\nhh_server is stopped. Try running `hh` at the command-line.",
                    "shortMessage": "Hack: hh_server stopped",
                },
                result=NoResponse(),
            )
            .request(
                line=line(),
                comment="hover after init will succeed",
                method="textDocument/hover",
                params={
                    "textDocument": {"uri": "file://${root_path}/beforeInit2.php"},
                    "position": {"line": 2, "character": 4},
                },
                result={
                    "contents": [{"language": "hack", "value": "int"}],
                    "range": {
                        "start": {"line": 2, "character": 2},
                        "end": {"line": 2, "character": 6},
                    },
                },
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )

        self.run_spec(spec, variables)

    def test_serverless_ide_workspace_symbol(self) -> None:
        variables = self.write_hhconf_and_naming_table()

        spec = (
            self.initialize_spec(LspTestSpec("serverless_ide_workspace_symbol"))
            .notification(
                comment="LocalIndex doesn't have any names in its SymbolIndex, so let's index this file...",
                method="workspace/didChangeWatchedFiles",
                params={
                    "changes": [
                        {"uri": "file://${root_path}/definition.php", "type": 2}
                    ]
                },
            )
            .request(
                line=line(),
                comment="workspace symbol call, global (derived from SymbolIndex)",
                method="workspace/symbol",
                params={"query": "TakesString"},
                result=[
                    {
                        "name": "TakesString",
                        "kind": 5,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 36, "character": 6},
                                "end": {"line": 36, "character": 17},
                            },
                        },
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                comment="workspace symbol call, member (derived from SymbolIndex followed by naming-table)",
                method="workspace/symbol",
                params={"query": "TakesString::"},
                result=[
                    {
                        "name": "__construct",
                        "kind": 6,
                        "location": {
                            "uri": "file://${root_path}/definition.php",
                            "range": {
                                "start": {"line": 37, "character": 18},
                                "end": {"line": 37, "character": 29},
                            },
                        },
                    }
                ],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_errors_before_init(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update({"php_file_uri": self.repo_file_uri("php_file.php")})

        spec = (
            self.initialize_spec(
                LspTestSpec("errors_before_init"),
                wait_for_init_done=False,
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "<?hh\nfunction f(): int { return 1 }\n",
                    }
                },
            )
            .wait_for_notification(
                method="telemetry/event",
                params={"type": 4, "message": "[client-ide] Finished init: ok"},
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 29},
                                "end": {"line": 1, "character": 29},
                            },
                            "severity": 1,
                            "code": 1002,
                            "source": "Hack",
                            "message": "A semicolon ; is expected here.",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        }
                    ],
                },
            )
            .request(
                comment="the codeAction request will push diagnostics if they've not already been pushed",
                line=line(),
                method="textDocument/codeAction",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "range": {
                        "start": {"line": 0, "character": 0},
                        "end": {"line": 0, "character": 0},
                    },
                    "context": {"diagnostics": []},
                },
                result=[],
                powered_by="serverless_ide",
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={"uri": "${php_file_uri}", "diagnostics": []},
            )
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_skip_errors(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        variables.update({"php_file_uri": self.repo_file_uri("php_file.php")})

        spec = (
            self.initialize_spec(
                LspTestSpec("skip_errors"),
                wait_for_init_done=False,
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "<?hh\nfunction f(): int { return 1 }\n",
                    }
                },
            )
            .notification(
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 0},
                                "end": {"line": 1, "character": 30},
                            },
                            "text": "function f(): int { return 12 }",
                        }
                    ],
                },
            )
            .notification(
                method="textDocument/didChange",
                params={
                    "textDocument": {"uri": "${php_file_uri}"},
                    "contentChanges": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 0},
                                "end": {"line": 1, "character": 31},
                            },
                            "text": "function f(): int { return 123 }",
                        }
                    ],
                },
            )
            .wait_for_notification(
                method="telemetry/event",
                params={"type": 4, "message": "[client-ide] Finished init: ok"},
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 31},
                                "end": {"line": 1, "character": 31},
                            },
                            "severity": 1,
                            "code": 1002,
                            "source": "Hack",
                            "message": "A semicolon ; is expected here.",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        }
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={"uri": "${php_file_uri}", "diagnostics": []},
            )
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_toplevel_statements(self) -> None:
        """
        regression test for:
        - T170543625: presence of a top-level statement shouldn't suppress type errors in the rest of the file
        - T170543963: type errors within top-level statements should be shown
        """
        variables = self.write_hhconf_and_naming_table()
        file_base_name = "toplevel_statements.php"
        php_file_uri = self.repo_file_uri(file_base_name)
        contents = """<?hh

$x = 3;
$x + 1;     // OK
$x + true; // should be an error here

function expect_type_errors_inside(): void {
  3 + true; // should be an error here
}
"""
        variables.update({"php_file_uri": php_file_uri, "contents": contents})
        spec = (
            self.initialize_spec(LspTestSpec("toplevel_statements"))
            .write_to_disk(
                comment="create file ${file_base_name}",
                uri="${php_file_uri}",
                contents="${contents}",
                notify=False,
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${contents}",
                    }
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 2, "character": 0},
                                "end": {"line": 2, "character": 7},
                            },
                            "severity": 1,
                            "code": 2128,
                            "source": "Hack",
                            "message": "Hack does not support top level statements. Use the __EntryPoint attribute on a function instead",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        },
                        {
                            "range": {
                                "start": {"line": 3, "character": 0},
                                "end": {"line": 3, "character": 7},
                            },
                            "severity": 1,
                            "code": 2128,
                            "source": "Hack",
                            "message": "Hack does not support top level statements. Use the __EntryPoint attribute on a function instead",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        },
                        {
                            "range": {
                                "start": {"line": 4, "character": 0},
                                "end": {"line": 4, "character": 10},
                            },
                            "severity": 1,
                            "code": 2128,
                            "source": "Hack",
                            "message": "Hack does not support top level statements. Use the __EntryPoint attribute on a function instead",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        },
                        {
                            "range": {
                                "start": {"line": 4, "character": 5},
                                "end": {"line": 4, "character": 9},
                            },
                            "severity": 1,
                            "code": 4429,
                            "source": "Hack",
                            "message": "Typing error",
                            "relatedInformation": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 4, "character": 0},
                                            "end": {"line": 4, "character": 9},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 4, "character": 5},
                                            "end": {"line": 4, "character": 9},
                                        },
                                    },
                                    "message": "But got bool",
                                },
                            ],
                            "relatedLocations": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 4, "character": 0},
                                            "end": {"line": 4, "character": 9},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 4, "character": 5},
                                            "end": {"line": 4, "character": 9},
                                        },
                                    },
                                    "message": "But got bool",
                                },
                            ],
                        },
                        {
                            "range": {
                                "start": {"line": 7, "character": 6},
                                "end": {"line": 7, "character": 10},
                            },
                            "severity": 1,
                            "code": 4429,
                            "source": "Hack",
                            "message": "Typing error",
                            "relatedInformation": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 7, "character": 2},
                                            "end": {"line": 7, "character": 10},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 7, "character": 6},
                                            "end": {"line": 7, "character": 10},
                                        },
                                    },
                                    "message": "But got bool",
                                },
                            ],
                            "relatedLocations": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 7, "character": 2},
                                            "end": {"line": 7, "character": 10},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 7, "character": 6},
                                            "end": {"line": 7, "character": 10},
                                        },
                                    },
                                    "message": "But got bool",
                                },
                            ],
                        },
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                comment="shutdown should clear out live squiggles",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [],
                },
            )
        )
        self.run_spec(spec, variables)

    def test_toplevel_statements_await(self) -> None:
        """
        regression test for:
        - T171537862: presence of a top-level await shouldn't suppress type errros in the rest of the file
        - T170614550: correct type errors for top-level statements (`$await gen_int()` + true should error)
        """
        variables = self.write_hhconf_and_naming_table()
        file_base_name = "toplevel_statements.php"
        php_file_uri = self.repo_file_uri(file_base_name)
        contents = """<?hh
$b = await async_bool();
$b + 1; // should be a type error here
async function async_bool(): Awaitable<bool> {
    return true;
}

function expect_type_errors_inside(): void {
3 + true; // should be a type error here
}
"""
        variables.update({"php_file_uri": php_file_uri, "contents": contents})
        spec = (
            self.initialize_spec(LspTestSpec("toplevel_statements"))
            .write_to_disk(
                comment="create file ${file_base_name}",
                uri="${php_file_uri}",
                contents="${contents}",
                notify=False,
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${contents}",
                    }
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 1, "character": 0},
                                "end": {"line": 1, "character": 24},
                            },
                            "severity": 1,
                            "code": 2128,
                            "source": "Hack",
                            "message": "Hack does not support top level statements. Use the __EntryPoint attribute on a function instead",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        },
                        {
                            "range": {
                                "start": {"line": 2, "character": 0},
                                "end": {"line": 2, "character": 7},
                            },
                            "severity": 1,
                            "code": 2128,
                            "source": "Hack",
                            "message": "Hack does not support top level statements. Use the __EntryPoint attribute on a function instead",
                            "relatedInformation": [],
                            "relatedLocations": [],
                        },
                        {
                            "range": {
                                "start": {"line": 2, "character": 0},
                                "end": {"line": 2, "character": 2},
                            },
                            "severity": 1,
                            "code": 4429,
                            "source": "Hack",
                            "message": "Typing error",
                            "relatedInformation": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 2, "character": 0},
                                            "end": {"line": 2, "character": 6},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 3, "character": 39},
                                            "end": {"line": 3, "character": 43},
                                        },
                                    },
                                    "message": "But got bool",
                                },
                            ],
                            "relatedLocations": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 2, "character": 0},
                                            "end": {"line": 2, "character": 6},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 3, "character": 39},
                                            "end": {"line": 3, "character": 43},
                                        },
                                    },
                                    "message": "But got bool",
                                },
                            ],
                        },
                        {
                            "range": {
                                "start": {"line": 8, "character": 4},
                                "end": {"line": 8, "character": 8},
                            },
                            "severity": 1,
                            "code": 4429,
                            "source": "Hack",
                            "message": "Typing error",
                            "relatedInformation": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 8, "character": 0},
                                            "end": {"line": 8, "character": 8},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 8, "character": 4},
                                            "end": {"line": 8, "character": 8},
                                        },
                                    },
                                    "message": "But got bool",
                                },
                            ],
                            "relatedLocations": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 8, "character": 0},
                                            "end": {"line": 8, "character": 8},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 8, "character": 4},
                                            "end": {"line": 8, "character": 8},
                                        },
                                    },
                                    "message": "But got bool",
                                },
                            ],
                        },
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                comment="shutdown should clear out live squiggles",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [],
                },
            )
        )
        self.run_spec(spec, variables)

    def test_notebook_mode(self) -> None:
        """
        When `--notebook-mode` is passed to `hh lsp`:
        - There should be NO parse errors for top-level statements
        - There should be NO error for top-level await
        - If HHVM is paused at a breakpoint, there should be no undefined vars
        when not stopped at a breakpoint
        """
        try:
            # Ensure magic file not present.
            # See also `test_notebook_mode_at_breakpoint`, where we test the behavior when
            # the magic file is present.
            os.remove(_HHVM_IS_PAUSED_FILE)
        except FileNotFoundError:
            pass
        variables = self.write_hhconf_and_naming_table()
        file_base_name = "notebook_mode.php"
        php_file_uri = self.repo_file_uri(file_base_name)
        contents = r"""<?hh

async function gen_bool(): Awaitable<bool> {
    return true;
}
$s = "";                // OK top-level statement, because we pass '--notebook-mode' to the LSP
$s + 3;                 // Error, expected num but got string
$i = await gen_bool();  // OK top-level await, because we pass '--notebook-mode' to the LSP
$i + 3;                 // Error, expected num but got bool
echo $undefined_var;
class :el {
  public function __construct(
    public darray<string,mixed> $x, // Attributes
    public varray<mixed> $y, // Children
    public string $z, // Filename
    public int $s, // Line number
  ) {}
}
<el />;

"""
        variables.update({"php_file_uri": php_file_uri, "contents": contents})
        spec = (
            self.initialize_spec(LspTestSpec("notebook_mode"))
            .write_to_disk(
                comment="create file ${file_base_name}",
                uri="${php_file_uri}",
                contents="${contents}",
                notify=False,
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${contents}",
                    }
                },
            )
            .wait_for_notification(
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [
                        {
                            "range": {
                                "start": {"line": 9, "character": 5},
                                "end": {"line": 9, "character": 19},
                            },
                            "severity": 1,
                            "code": 2050,
                            "source": "Hack",
                            "message": "Variable $undefined_var is undefined, or not always defined.",
                            "relatedInformation": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 5, "character": 0},
                                            "end": {"line": 5, "character": 7},
                                        },
                                    },
                                    "message": "Did you mean $s instead?",
                                }
                            ],
                            "relatedLocations": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 5, "character": 0},
                                            "end": {"line": 5, "character": 7},
                                        },
                                    },
                                    "message": "Did you mean $s instead?",
                                }
                            ],
                        },
                        {
                            "range": {
                                "start": {"line": 6, "character": 0},
                                "end": {"line": 6, "character": 2},
                            },
                            "severity": 1,
                            "code": 4429,
                            "source": "Hack",
                            "message": "Typing error",
                            "relatedInformation": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 6, "character": 0},
                                            "end": {"line": 6, "character": 6},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 5, "character": 5},
                                            "end": {"line": 5, "character": 7},
                                        },
                                    },
                                    "message": "But got string",
                                },
                            ],
                            "relatedLocations": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 6, "character": 0},
                                            "end": {"line": 6, "character": 6},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 5, "character": 5},
                                            "end": {"line": 5, "character": 7},
                                        },
                                    },
                                    "message": "But got string",
                                },
                            ],
                        },
                        {
                            "range": {
                                "start": {"line": 8, "character": 0},
                                "end": {"line": 8, "character": 2},
                            },
                            "severity": 1,
                            "code": 4429,
                            "source": "Hack",
                            "message": "Typing error",
                            "relatedInformation": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 8, "character": 0},
                                            "end": {"line": 8, "character": 6},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 2, "character": 37},
                                            "end": {"line": 2, "character": 41},
                                        },
                                    },
                                    "message": "But got bool",
                                },
                            ],
                            "relatedLocations": [
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 8, "character": 0},
                                            "end": {"line": 8, "character": 6},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 2, "character": 37},
                                            "end": {"line": 2, "character": 41},
                                        },
                                    },
                                    "message": "But got bool",
                                },
                            ],
                        },
                    ],
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .wait_for_notification(
                comment="shutdown should clear out live squiggles",
                method="textDocument/publishDiagnostics",
                params={
                    "uri": "${php_file_uri}",
                    "diagnostics": [],
                },
            )
        )
        self.run_spec(spec, variables, lsp_extra_args=["--notebook-mode"])

    def test_file_outline_detail(self) -> None:
        """
        Test that file outline symbols contain [the `detail` field](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#documentSymbol), where applicable.
        Afaict this field isn't user-visible in VSCode. Our use case is to provide additional context to AI code assist tooling.
        The key piece of information not already provided is the type hints from the definition site, but we also include additional information that may help tooling,
        such as parameter names of functions and methods.
        """
        variables = self.write_hhconf_and_naming_table()
        file_base_name = "file_outline.php"
        php_file_uri = self.repo_file_uri(file_base_name)
        contents = """<?hh
class CC {
  public static abstract final readonly int $prop;
  const type T = int;
  const vec<int> NUMBERS = vec[1, 2, 3];
  public readonly function foo(inout arraykey $x) {
    $x = 1;
  }
  public static async function bar(): Awaitable<void> {}
}

function baz<T>(readonly T $x): readonly T {
  return $x;
}
"""
        variables.update({"php_file_uri": php_file_uri, "contents": contents})
        spec = (
            self.initialize_spec(LspTestSpec("notebook_mode"))
            .write_to_disk(
                comment="create file ${file_base_name}",
                uri="${php_file_uri}",
                contents="${contents}",
                notify=False,
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${contents}",
                    }
                },
            )
            .request(
                line=line(),
                method="textDocument/documentSymbol",
                params={"textDocument": {"uri": "${php_file_uri}"}},
                result=[
                    {
                        "name": "baz",
                        "kind": 12,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 11, "character": 0},
                                "end": {"line": 13, "character": 1},
                            },
                        },
                        "detail": "function<T>(readonly T $x): readonly T",
                    },
                    {
                        "name": "bar",
                        "kind": 6,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 8, "character": 2},
                                "end": {"line": 8, "character": 56},
                            },
                        },
                        "detail": "function(): Awaitable<void>",
                        "containerName": "CC",
                    },
                    {
                        "name": "NUMBERS",
                        "kind": 14,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 4, "character": 17},
                                "end": {"line": 4, "character": 39},
                            },
                        },
                        "detail": "vec<int> NUMBERS = vec[1, 2, 3]",
                        "containerName": "CC",
                    },
                    {
                        "name": "prop",
                        "kind": 7,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 2, "character": 44},
                                "end": {"line": 2, "character": 49},
                            },
                        },
                        "detail": "readonly int $prop",
                        "containerName": "CC",
                    },
                    {
                        "name": "CC",
                        "kind": 5,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 1, "character": 0},
                                "end": {"line": 9, "character": 1},
                            },
                        },
                    },
                    {
                        "name": "T",
                        "kind": 5,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 3, "character": 2},
                                "end": {"line": 3, "character": 21},
                            },
                        },
                        "detail": "const type T = int;",
                        "containerName": "CC",
                    },
                    {
                        "name": "foo",
                        "kind": 6,
                        "location": {
                            "uri": "${php_file_uri}",
                            "range": {
                                "start": {"line": 5, "character": 2},
                                "end": {"line": 7, "character": 3},
                            },
                        },
                        "detail": "function(inout arraykey $x)",
                        "containerName": "CC",
                    },
                ],
                powered_by="serverless_ide",
            )
            .request(
                line=line(),
                method="shutdown",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                    }
                },
                result=None,
            )
        )
        self.run_spec(spec, variables)

    def run_serverless_ide_will_save_wait_until(
        self,
        variables: dict[str, str],
        filename: str,
        expected_result: list[dict[str, Union[dict[str, dict[str, int]], str]]],
    ) -> None:
        variables.update(self.setup_php_file(filename))

        spec = (
            self.initialize_spec(
                LspTestSpec("serverless_ide_will_save_wait_until_" + filename),
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${php_file}",
                    }
                },
            )
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .request(
                line=line(),
                comment="willSaveWaitUntil",
                method="textDocument/willSaveWaitUntil",
                params={"textDocument": {"uri": "${php_file_uri}"}, "reason": 1},
                result=expected_result,
            )
            .request(
                line=line(),
                comment="shutdown",
                method="shutdown",
                params={},
                result=None,
            )
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables)

    def test_initialize_will_save_wait_until_is_disabled(self) -> None:
        variables = self.write_hhconf_and_naming_table()
        spec = (
            LspTestSpec("test_initialize_will_save_wait_until_is_disabled")
            .ignore_notifications(method="telemetry/event")
            .ignore_notifications(method="textDocument/publishDiagnostics")
            .ignore_requests(
                method="client/registerCapability",
                params={
                    "registrations": [
                        {
                            "id": "did-change-watched-files",
                            "method": "workspace/didChangeWatchedFiles",
                            "registerOptions": {
                                "watchers": [
                                    {
                                        "globPattern": "**/*.{php,phpt,hack,hackpartial,hck,hh,hhi,xhp}",
                                        "kind": 7,
                                    }
                                ]
                            },
                        }
                    ]
                },
            )
            .request(
                line=line(),
                method="initialize",
                params={
                    "initializationOptions": {},
                    "processId": None,
                    "rootPath": "${root_path}",
                    "capabilities": {},
                },
                result={
                    "capabilities": {
                        "textDocumentSync": {
                            "openClose": True,
                            "change": 2,
                            "willSave": False,
                            "willSaveWaitUntil": False,
                            "save": {"includeText": False},
                        },
                        "hoverProvider": True,
                        "completionProvider": {
                            "resolveProvider": True,
                            "triggerCharacters": [
                                "$",
                                ">",
                                "\\",
                                ":",
                                "<",
                                "[",
                                "'",
                                '"',
                                "{",
                                "#",
                            ],
                        },
                        "signatureHelpProvider": {"triggerCharacters": ["(", ","]},
                        "definitionProvider": True,
                        "typeDefinitionProvider": True,
                        "referencesProvider": True,
                        "documentHighlightProvider": True,
                        "documentSymbolProvider": True,
                        "workspaceSymbolProvider": True,
                        "codeActionProvider": {"resolveProvider": True},
                        "documentFormattingProvider": True,
                        "documentRangeFormattingProvider": True,
                        "documentOnTypeFormattingProvider": {
                            "firstTriggerCharacter": ";",
                            "moreTriggerCharacter": ["}"],
                        },
                        "renameProvider": True,
                        "implementationProvider": True,
                        "rageProvider": True,
                        "experimental": {"snippetTextEdit": True, "autoCloseJsx": True},
                    }
                },
            )
            .request(line=line(), method="shutdown", params={}, result=None)
            .notification(method="exit", params={})
        )
        self.run_spec(spec, variables, lsp_extra_args=["--disable-format-on-save"])

    def test_serverless_ide_will_save_wait_until_no_format(
        self,
    ) -> None:
        variables = self.write_hhconf_and_naming_table()

        for filename in [
            "unformatted_with_format_and_generated.php",
            "unformatted_with_noformat.php",
        ]:
            self.run_serverless_ide_will_save_wait_until(variables, filename, [])

    def test_serverless_ide_will_save_wait_until_format(
        self,
    ) -> None:
        variables = self.write_hhconf_and_naming_table()

        for filename, expected_result in [
            (
                "formatted.php",
                [
                    {
                        "range": {
                            "start": {"line": 0, "character": 0},
                            "end": {"line": 0, "character": 0},
                        },
                        "newText": "",
                    }
                ],
            ),
            (
                "unformatted.php",
                [
                    {
                        "range": {
                            "start": {"line": 2, "character": 0},
                            "end": {"line": 5, "character": 0},
                        },
                        "newText": 'function unformatted(): string {\n  return "this file should be formatted on save";\n}\n',
                    }
                ],
            ),
            (
                "unformatted_and_partially_generated.php",
                [
                    {
                        "range": {
                            "start": {"line": 5, "character": 0},
                            "end": {"line": 6, "character": 0},
                        },
                        "newText": "  return true;\n",
                    }
                ],
            ),
        ]:
            self.run_serverless_ide_will_save_wait_until(
                variables, filename, expected_result
            )

    def test_will_save_wait_until_newlines_at_top(self) -> None:
        """
        Test our handling of textDocument/willSaveWaitUntil:
        we shouldn't have any extra newlines at the top (T188437747)
        """
        variables = self.write_hhconf_and_naming_table()
        file_base_name = "will_save_wait_until_newlines_at_top.php"
        php_file_uri = self.repo_file_uri(file_base_name)
        contents = """<?hh



class C2 {}
"""
        variables.update({"php_file_uri": php_file_uri, "contents": contents})
        spec = (
            self.initialize_spec(LspTestSpec("notebook_mode"))
            .write_to_disk(
                comment="create file ${file_base_name}",
                uri="${php_file_uri}",
                contents="${contents}",
                notify=False,
            )
            .notification(
                method="textDocument/didOpen",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                        "languageId": "hack",
                        "version": 1,
                        "text": "${contents}",
                    }
                },
            )
            .request(
                line=line(),
                method="textDocument/willSaveWaitUntil",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                    },
                    # TextDocumentSaveReason.Manual (user saved)
                    # https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocumentSaveReason
                    "reason": 1,
                },
                result=[
                    {
                        "range": {
                            "start": {"line": 1, "character": 0},
                            "end": {"line": 3, "character": 0},
                        },
                        "newText": "",
                    }
                ],
            )
            .request(
                line=line(),
                method="shutdown",
                params={
                    "textDocument": {
                        "uri": "${php_file_uri}",
                    }
                },
                result=None,
            )
        )
        self.run_spec(spec, variables)
