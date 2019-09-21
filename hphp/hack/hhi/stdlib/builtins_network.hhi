<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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

<<__PHPStdLib>>
function gethostname();
<<__PHPStdLib>>
function gethostbyaddr(string $ip_address);
<<__PHPStdLib>>
function gethostbyname(string $hostname);
<<__PHPStdLib>>
function gethostbynamel(string $hostname);
<<__PHPStdLib>>
function getprotobyname(string $name);
<<__PHPStdLib>>
function getprotobynumber(int $number);
<<__PHPStdLib>>
function getservbyname(string $service, string $protocol);
<<__PHPStdLib>>
function getservbyport(int $port, string $protocol);
<<__PHPStdLib, __Rx>>
function inet_ntop(string $in_addr);
<<__PHPStdLib, __Rx>>
function inet_pton(string $address);
<<__PHPStdLib, __Rx>>
function ip2long(string $ip_address);
<<__PHPStdLib, __Rx>>
function long2ip(string $proper_address);
<<__PHPStdLib>>
function dns_check_record($host, $type = null);
<<__PHPStdLib>>
function checkdnsrr(string $host, string $type = "MX");
<<__PHPStdLib>>
function dns_get_record(string $hostname, int $type, inout $authns, inout $addtl);
<<__PHPStdLib>>
function dns_get_mx($hostname, inout $mxhosts, inout $weights);
<<__PHPStdLib>>
function getmxrr(string $hostname, inout $mxhosts, inout $weight);
<<__PHPStdLib>>
function fsockopen(string $hostname, int $port, inout $errnum, inout $errstr, float $timeout = 0.0);
<<__PHPStdLib>>
function pfsockopen(string $hostname, int $port, inout $errnum, inout $errstr, float $timeout = 0.0);
<<__PHPStdLib>>
function socket_get_status($stream);
<<__PHPStdLib>>
function socket_set_blocking($stream, $mode);
<<__PHPStdLib>>
function socket_set_timeout($stream, $seconds, $microseconds = 0);
<<__PHPStdLib>>
function header(string $str, bool $replace = true, int $http_response_code = 0);
<<__PHPStdLib>>
function http_response_code(int $response_code = 0);
<<__PHPStdLib>>
function headers_list();
<<__PHPStdLib>>
function get_http_request_size();
<<__PHPStdLib>>
function headers_sent();
<<__PHPStdLib>>
function headers_sent_with_file_line(inout $file, inout $line);
<<__PHPStdLib>>
function header_register_callback($callback);
<<__PHPStdLib>>
function header_remove($name = null);
<<__PHPStdLib>>
function setcookie(string $name, string $value = "", int $expire = 0, string $path = "", string $domain = "", bool $secure = false, bool $httponly = false);
<<__PHPStdLib>>
function setrawcookie(string $name, string $value = "", int $expire = 0, string $path = "", string $domain = "", bool $secure = false, bool $httponly = false);
<<__PHPStdLib>>
function define_syslog_variables();
<<__PHPStdLib>>
function openlog(string $ident, int $option, int $facility);
<<__PHPStdLib>>
function closelog();
<<__PHPStdLib>>
function syslog(int $priority, string $message): bool;
