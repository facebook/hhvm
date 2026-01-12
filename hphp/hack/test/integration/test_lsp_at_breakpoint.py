# pyre-unsafe
from __future__ import absolute_import, division, print_function, unicode_literals

import os
from pathlib import Path

from hphp.hack.test.integration.lsp_test_base import LspTestBase
from hphp.hack.test.integration.lsptestspec import line, LspTestSpec

"""
Existence of this file indicates that a Hack notebook is stopped at a breakpoint
"""
_HHVM_IS_PAUSED_FILE: Path = Path.home() / ".vscode-sockets/hhvm-paused"


class TestLspAtBreakPoint(LspTestBase):
    def test_notebook_mode_at_breakpoint(self) -> None:
        """
        When `--notebook-mode` is passed to `hh lsp` **and HHVM is paused at a breakpoint**:
        - There should be no errors for undefined vars, see D57061779 for why
        """
        os.makedirs(os.path.dirname(_HHVM_IS_PAUSED_FILE), exist_ok=True)
        with open(_HHVM_IS_PAUSED_FILE, "w"):
            pass
        contents = """<?hh
echo $undefined_var_1; // no error
echo $undefined_var_2; // no error
1 * true;              // should error
"""
        variables = self.write_hhconf_and_naming_table()
        file_base_name = "notebook_mode.php"
        php_file_uri = self.repo_file_uri(file_base_name)
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
                                "start": {"line": 3, "character": 4},
                                "end": {"line": 3, "character": 8},
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
                                            "start": {"line": 3, "character": 0},
                                            "end": {"line": 3, "character": 8},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 3, "character": 4},
                                            "end": {"line": 3, "character": 8},
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
                                            "start": {"line": 3, "character": 0},
                                            "end": {"line": 3, "character": 8},
                                        },
                                    },
                                    "message": "Expected num because this is used in an arithmetic operation",
                                },
                                {
                                    "location": {
                                        "uri": "${php_file_uri}",
                                        "range": {
                                            "start": {"line": 3, "character": 4},
                                            "end": {"line": 3, "character": 8},
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
