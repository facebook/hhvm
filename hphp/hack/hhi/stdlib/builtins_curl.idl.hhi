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
define('CURLINFO_LOCAL_PORT', 0);
define('CURLOPT_TIMEOUT_MS', 0);
define('CURLOPT_CONNECTTIMEOUT_MS', 0);
function curl_init($url = null) { }
function curl_copy_handle($ch) { }
function curl_version($uversion = CURLVERSION_NOW) { }
function curl_setopt($ch, $option, $value) { }
function curl_setopt_array($ch, $options) { }
function fb_curl_getopt($ch, $opt = 0) { }
function curl_exec($ch) { }
function curl_getinfo($ch, $opt = 0) { }
function curl_errno($ch) { }
function curl_error($ch) { }
function curl_close($ch) { }
function curl_multi_init() { }
function curl_multi_add_handle($mh, $ch) { }
function curl_multi_remove_handle($mh, $ch) { }
function curl_multi_exec($mh, &$still_running) { }
function curl_multi_select($mh, $timeout = 1.0) { }
function fb_curl_multi_fdset($mh, &$read_fd_set, &$write_fd_set, &$exc_fd_set, &$max_fd = null_object) { }
function curl_multi_getcontent($ch) { }
function curl_multi_info_read($mh, &$msgs_in_queue = null) { }
function curl_multi_close($mh) { }
