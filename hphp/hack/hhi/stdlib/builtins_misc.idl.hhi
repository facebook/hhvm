<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
define('INF', 0);
define('NAN', 0);
function connection_aborted() { }
function connection_status() { }
function connection_timeout() { }
function constant($name) { }
function define($name, $value, $case_insensitive = false) { }
function defined($name, $autoload = true) { }
function get_browser($user_agent = null, $return_array = false) { }
function highlight_file($filename, $ret = false) { }
function show_source($filename, $ret = false) { }
function highlight_string($str, $ret = false) { }
function ignore_user_abort($setting = false) { }
function pack($format, ...) { }
function php_check_syntax($filename, &$error_message = null) { }
function php_strip_whitespace($filename) { }
function sleep($seconds) { }
function usleep($micro_seconds) { }
function time_nanosleep($seconds, $nanoseconds) { }
function time_sleep_until($timestamp) { }
function uniqid($prefix = null, $more_entropy = false) { }
function unpack($format, $data) { }
function sys_getloadavg() { }
function token_get_all($source) { }
function token_name($token) { }
function hphp_process_abort($magic) { }
function hphp_to_string($v) { }
