<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH {
function server_warmup_status(): string;
function execution_context(): string;
}
namespace {
const INF = 0.0;
const NAN = 0.0;
<<__PHPStdLib>>
function connection_aborted();
<<__PHPStdLib>>
function connection_status();
<<__PHPStdLib>>
function connection_timeout();
<<__PHPStdLib>>
function constant($name);
<<__PHPStdLib>>
function define($name, $value, $case_insensitive = false);
<<__PHPStdLib>>
function defined(string $name, $autoload = true);
<<__PHPStdLib>>
function get_browser($user_agent = null, $return_array = false);
<<__PHPStdLib>>
function highlight_file($filename, $ret = false);
<<__PHPStdLib>>
function show_source($filename, $ret = false);
<<__PHPStdLib>>
function highlight_string($str, $ret = false);
<<__PHPStdLib>>
function ignore_user_abort($setting = false);
<<__PHPStdLib, __Rx>>
function pack($format, ...);
<<__PHPStdLib>>
function php_check_syntax($filename, &$error_message = null);
<<__PHPStdLib>>
function php_strip_whitespace($filename);
<<__PHPStdLib>>
function sleep($seconds);
<<__PHPStdLib>>
function usleep($micro_seconds);
<<__PHPStdLib>>
function time_nanosleep($seconds, $nanoseconds);
<<__PHPStdLib>>
function time_sleep_until($timestamp);
<<__PHPStdLib>>
function uniqid($prefix = null, $more_entropy = false);
<<__PHPStdLib, __Rx>>
function unpack($format, $data);
<<__PHPStdLib>>
function sys_getloadavg();
<<__PHPStdLib>>
function hphp_process_abort($magic);
<<__PHPStdLib, __Rx>>
function hphp_to_string($v);
}
