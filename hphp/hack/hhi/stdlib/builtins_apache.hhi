<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace {

<<__PHPStdLib>>
function apache_note(
  string $note_name,
  HH\FIXME\MISSING_PARAM_TYPE $note_value = "",
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apache_notes(dict<string, string> $notes): void {}
<<__PHPStdLib>>
function apache_request_headers(): darray<string, string> {}
<<__PHPStdLib>>
function apache_response_headers(): darray<string, string> {}
<<__PHPStdLib>>
function apache_setenv(
  string $variable,
  string $value,
  bool $walk_to_top = false,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function getallheaders()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function virtual(
  HH\FIXME\MISSING_PARAM_TYPE $filename,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apache_get_config(): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function apache_get_rewrite_rules(): HH\FIXME\MISSING_RETURN_TYPE {}

}

namespace HH {

function get_headers_secure()[read_globals]: dict<string, vec<string>>;

}
