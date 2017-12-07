<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

namespace HH {
function server_warmup_status(): string;
function execution_context(): string;
}
const INF = 0.0;
const NAN = 0.0;
<<__PHPStdLib>>
function connection_aborted();
<<__PHPStdLib>>
function connection_status();
<<__PHPStdLib>>
function connection_timeout();
function constant($name);
function define($name, $value, $case_insensitive = false);
function defined(string $name, $autoload = true);
function get_browser($user_agent = null, $return_array = false);
<<__PHPStdLib>>
function highlight_file($filename, $ret = false);
<<__PHPStdLib>>
function show_source($filename, $ret = false);
<<__PHPStdLib>>
function highlight_string($str, $ret = false);
function ignore_user_abort($setting = false);
function pack($format, ...);
function php_check_syntax($filename, &$error_message = null);
<<__PHPStdLib>>
function php_strip_whitespace($filename);
function sleep($seconds);
function usleep($micro_seconds);
<<__PHPStdLib>>
function time_nanosleep($seconds, $nanoseconds);
function time_sleep_until($timestamp);
function uniqid($prefix = null, $more_entropy = false);
function unpack($format, $data);
<<__PHPStdLib>>
function sys_getloadavg();
<<__PHPStdLib>>
function hphp_process_abort($magic);
function hphp_to_string($v);
