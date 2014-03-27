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
define('FB_UNSERIALIZE_NONSTRING_VALUE', 0);
define('FB_UNSERIALIZE_UNEXPECTED_END', 0);
define('FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE', 0);
define('FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE', 0);
define('XHPROF_FLAGS_NO_BUILTINS', 0);
define('XHPROF_FLAGS_CPU', 0);
define('XHPROF_FLAGS_MEMORY', 0);
define('XHPROF_FLAGS_VTSC', 0);
define('XHPROF_FLAGS_TRACE', 0);
define('XHPROF_FLAGS_MEASURE_XHPROF_DISABLE', 0);
define('XHPROF_FLAGS_MALLOC', 0);
function fb_thrift_serialize($thing) { }
function fb_thrift_unserialize($thing, &$success, &$errcode = null_variant) { }
function fb_serialize($thing) { }
function fb_unserialize($thing, &$success, &$errcode = null_variant) { }
function fb_compact_serialize($thing) { }
function fb_compact_unserialize($thing, &$success, &$errcode = null_variant) { }
function fb_intercept($name, $handler, $data = null_variant) { }
function fb_stubout_intercept_handler($name, $obj, $params, $data, &$done) { }
function fb_rpc_intercept_handler($name, $obj, $params, $data, &$done) { }
function fb_renamed_functions($names) { }
function fb_rename_function($orig_func_name, $new_func_name) { }
function fb_autoload_map($map, $root) { }
function fb_utf8ize(&$input) { }
function fb_utf8_strlen_deprecated($input) { }
function fb_utf8_strlen($input) { }
function fb_utf8_substr($str, $start, $length = PHP_INT_MAX) { }
function fb_call_user_func_safe($function, ...) { }
function fb_call_user_func_safe_return($function, $def, ...) { }
function fb_call_user_func_array_safe($function, $params) { }
function fb_get_code_coverage($flush) { }
function fb_enable_code_coverage() { }
function fb_disable_code_coverage() { }
function xhprof_enable($flags = 0, $args = null) { }
function xhprof_disable() { }
function xhprof_network_enable() { }
function xhprof_network_disable() { }
function xhprof_frame_begin($name) { }
function xhprof_frame_end() { }
function xhprof_run_trace($packedTrace, $flags) { }
function xhprof_sample_enable() { }
function xhprof_sample_disable() { }
function fb_load_local_databases($servers) { }
function fb_parallel_query($sql_map, $max_thread = 50, $combine_result = true, $retry_query_on_fail = true, $connect_timeout = -1, $read_timeout = -1, $timeout_in_ms = false) { }
function fb_crossall_query($sql, $max_thread = 50, $retry_query_on_fail = true, $connect_timeout = -1, $read_timeout = -1, $timeout_in_ms = false) { }
function fb_output_compression($new_value) { }
function fb_set_exit_callback($function) { }
function fb_get_flush_stat() { }
function fb_get_last_flush_size() { }
function fb_lazy_stat($filename) { }
function fb_lazy_lstat($filename) { }
function fb_lazy_realpath($filename) { }
function fb_setprofile($callback) { }
function fb_gc_collect_cycles() { }
function fb_gc_detect_cycles($filename) { }

namespace HH {

function could_include($file) { }

}
