<?hh // decl     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function apache_note($note_name, $note_value = "") { }
<<__PHPStdLib>>
function apache_request_headers(): array<string, string> { }
<<__PHPStdLib>>
function apache_response_headers(): array<string, string> { }
<<__PHPStdLib>>
function apache_setenv($variable, $value, $walk_to_top = false) { }
<<__PHPStdLib>>
function getallheaders() { }
<<__PHPStdLib>>
function virtual($filename) { }
<<__PHPStdLib>>
function apache_get_config() { }
<<__PHPStdLib>>
function apache_get_rewrite_rules() { }
