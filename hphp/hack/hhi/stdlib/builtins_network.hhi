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
function gethostname() { }
function gethostbyaddr($ip_address) { }
function gethostbyname($hostname) { }
function gethostbynamel($hostname) { }
function getprotobyname($name) { }
function getprotobynumber($number) { }
function getservbyname($service, $protocol) { }
function getservbyport($port, $protocol) { }
function inet_ntop($in_addr) { }
function inet_pton($address) { }
function ip2long($ip_address) { }
function long2ip($proper_address) { }
function dns_check_record($host, $type = null) { }
function checkdnsrr($host, $type = null) { }
function dns_get_record($hostname, $type = -1, &$authns = null, &$addtl = null) { }
function dns_get_mx($hostname, &$mxhosts, &$weights = null) { }
function getmxrr($hostname, &$mxhosts, &$weight = null) { }
function fsockopen($hostname, $port = -1, &$errnum = null, &$errstr = null, $timeout = 0.0) { }
function pfsockopen($hostname, $port = -1, &$errnum = null, &$errstr = null, $timeout = 0.0) { }
function socket_get_status($stream) { }
function socket_set_blocking($stream, $mode) { }
function socket_set_timeout($stream, $seconds, $microseconds = 0) { }
function header($str, $replace = true, $http_response_code = 0) { }
function http_response_code($response_code = 0) { }
function headers_list() { }
function get_http_request_size() { }
function headers_sent(&$file = null, &$line = null) { }
function header_register_callback($callback) { }
function header_remove($name = null) { }
function setcookie($name, $value = null, $expire = 0, $path = null, $domain = null, $secure = false, $httponly = false) { }
function setrawcookie($name, $value = null, $expire = 0, $path = null, $domain = null, $secure = false, $httponly = false) { }
function define_syslog_variables() { }
function openlog($ident, $option, $facility) { }
function closelog() { }
function syslog($priority, $message) { }
