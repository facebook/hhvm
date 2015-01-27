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

const int DNS_A = 1;
const int DNS_A6 = 16777216;
const int DNS_AAAA = 134217728;
const int DNS_ALL = 251713587;
const int DNS_ANY = 268435456;
const int DNS_CNAME = 16;
const int DNS_HINFO = 4096;
const int DNS_MX = 16384;
const int DNS_NAPTR = 67108864;
const int DNS_NS = 2;
const int DNS_PTR = 2048;
const int DNS_SOA = 32;
const int DNS_SRV = 33554432;
const int DNS_TXT = 32768;

const int LOG_ALERT = 1;
const int LOG_AUTH = 32;
const int LOG_AUTHPRIV = 80;
const int LOG_CONS = 2;
const int LOG_CRIT = 2;
const int LOG_CRON = 72;
const int LOG_DAEMON = 24;
const int LOG_DEBUG = 7;
const int LOG_EMERG = 0;
const int LOG_ERR = 3;
const int LOG_INFO = 6;
const int LOG_KERN = 0;
const int LOG_LOCAL0 = 128;
const int LOG_LOCAL1 = 136;
const int LOG_LOCAL2 = 144;
const int LOG_LOCAL3 = 152;
const int LOG_LOCAL4 = 160;
const int LOG_LOCAL5 = 168;
const int LOG_LOCAL6 = 176;
const int LOG_LOCAL7 = 184;
const int LOG_LPR = 48;
const int LOG_MAIL = 16;
const int LOG_NDELAY = 8;
const int LOG_NEWS = 56;
const int LOG_NOTICE = 5;
const int LOG_NOWAIT = 16;
const int LOG_ODELAY = 4;
const int LOG_PERROR = 32;
const int LOG_PID = 1;
const int LOG_SYSLOG = 40;
const int LOG_USER = 8;
const int LOG_UUCP = 64;
const int LOG_WARNING = 4;

function gethostname();
function gethostbyaddr($ip_address);
function gethostbyname($hostname);
function gethostbynamel($hostname);
function getprotobyname($name);
function getprotobynumber($number);
function getservbyname($service, $protocol);
function getservbyport($port, $protocol);
function inet_ntop($in_addr);
function inet_pton($address);
function ip2long($ip_address);
function long2ip($proper_address);
function dns_check_record($host, $type = null);
function checkdnsrr($host, $type = null);
function dns_get_record($hostname, $type = -1, &$authns = null, &$addtl = null);
function dns_get_mx($hostname, &$mxhosts, &$weights = null);
function getmxrr($hostname, &$mxhosts, &$weight = null);
function fsockopen($hostname, $port = -1, &$errnum = null, &$errstr = null, $timeout = 0.0);
function pfsockopen($hostname, $port = -1, &$errnum = null, &$errstr = null, $timeout = 0.0);
function socket_get_status($stream);
function socket_set_blocking($stream, $mode);
function socket_set_timeout($stream, $seconds, $microseconds = 0);
function header($str, $replace = true, $http_response_code = 0);
function http_response_code($response_code = 0);
function headers_list();
function get_http_request_size();
function headers_sent(&$file = null, &$line = null);
function header_register_callback($callback);
function header_remove($name = null);
function setcookie($name, $value = null, $expire = 0, $path = null, $domain = null, $secure = false, $httponly = false);
function setrawcookie($name, $value = null, $expire = 0, $path = null, $domain = null, $secure = false, $httponly = false);
function define_syslog_variables();
function openlog($ident, $option, $facility);
function closelog();
function syslog(int $priority, string $message): bool;
