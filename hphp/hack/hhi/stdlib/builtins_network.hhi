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

const int DNS_A;
const int DNS_A6;
const int DNS_AAAA;
const int DNS_ALL;
const int DNS_ANY;
const int DNS_CAA;
const int DNS_CNAME;
const int DNS_HINFO;
const int DNS_MX;
const int DNS_NAPTR;
const int DNS_NS;
const int DNS_PTR;
const int DNS_SOA;
const int DNS_SRV;
const int DNS_TXT;

const int LOG_ALERT;
const int LOG_AUTH;
const int LOG_AUTHPRIV;
const int LOG_CONS;
const int LOG_CRIT;
const int LOG_CRON;
const int LOG_DAEMON;
const int LOG_DEBUG;
const int LOG_EMERG;
const int LOG_ERR;
const int LOG_INFO;
const int LOG_KERN;
const int LOG_LOCAL0;
const int LOG_LOCAL1;
const int LOG_LOCAL2;
const int LOG_LOCAL3;
const int LOG_LOCAL4;
const int LOG_LOCAL5;
const int LOG_LOCAL6;
const int LOG_LOCAL7;
const int LOG_LPR;
const int LOG_MAIL;
const int LOG_NDELAY;
const int LOG_NEWS;
const int LOG_NOTICE;
const int LOG_NOWAIT;
const int LOG_ODELAY;
const int LOG_PERROR;
const int LOG_PID;
const int LOG_SYSLOG;
const int LOG_USER;
const int LOG_UUCP;
const int LOG_WARNING;

<<__PHPStdLib>>
function gethostname()[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gethostbyaddr(string $ip_address): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gethostbyname(string $hostname): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function gethostbynamel(string $hostname): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getprotobyname(string $name): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getprotobynumber(int $number): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getservbyname(
  string $service,
  string $protocol,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getservbyport(
  int $port,
  string $protocol,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function inet_ntop(string $in_addr)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function inet_ntop_folly(string $in_addr)[]: ?string;
<<__PHPStdLib>>
function inet_ntop_nullable(string $in_addr)[]: ?string;
<<__PHPStdLib>>
function inet_pton(string $address)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function ip2long(string $ip_address)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function long2ip(string $proper_address)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function dns_check_record(
  HH\FIXME\MISSING_PARAM_TYPE $host,
  HH\FIXME\MISSING_PARAM_TYPE $type = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function checkdnsrr(
  string $host,
  string $type = "MX",
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function dns_get_record(
  string $hostname,
  int $type,
  inout HH\FIXME\MISSING_PARAM_TYPE $authns,
  inout HH\FIXME\MISSING_PARAM_TYPE $addtl,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function dns_get_mx(
  HH\FIXME\MISSING_PARAM_TYPE $hostname,
  inout HH\FIXME\MISSING_PARAM_TYPE $mxhosts,
  inout HH\FIXME\MISSING_PARAM_TYPE $weights,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function getmxrr(
  string $hostname,
  inout HH\FIXME\MISSING_PARAM_TYPE $mxhosts,
  inout HH\FIXME\MISSING_PARAM_TYPE $weight,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function fsockopen(
  string $hostname,
  int $port,
  inout HH\FIXME\MISSING_PARAM_TYPE $errnum,
  inout HH\FIXME\MISSING_PARAM_TYPE $errstr,
  float $timeout = 0.0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pfsockopen(
  string $hostname,
  int $port,
  inout HH\FIXME\MISSING_PARAM_TYPE $errnum,
  inout HH\FIXME\MISSING_PARAM_TYPE $errstr,
  float $timeout = 0.0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function socket_get_status(
  HH\FIXME\MISSING_PARAM_TYPE $stream,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function socket_set_blocking(
  HH\FIXME\MISSING_PARAM_TYPE $stream,
  HH\FIXME\MISSING_PARAM_TYPE $mode,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function socket_set_timeout(
  HH\FIXME\MISSING_PARAM_TYPE $stream,
  HH\FIXME\MISSING_PARAM_TYPE $seconds,
  HH\FIXME\MISSING_PARAM_TYPE $microseconds = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function header(
  string $str,
  bool $replace = true,
  int $http_response_code = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function http_response_code(
  int $response_code = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function headers_list()[read_globals]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function get_http_request_size(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function headers_sent(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function headers_sent_with_file_line(
  inout HH\FIXME\MISSING_PARAM_TYPE $file,
  inout HH\FIXME\MISSING_PARAM_TYPE $line,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function header_register_callback(
  HH\FIXME\MISSING_PARAM_TYPE $callback,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function header_remove(
  HH\FIXME\MISSING_PARAM_TYPE $name = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function setcookie(
  string $name,
  string $value = "",
  int $expire = 0,
  string $path = "",
  string $domain = "",
  bool $secure = false,
  bool $httponly = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function setrawcookie(
  string $name,
  string $value = "",
  int $expire = 0,
  string $path = "",
  string $domain = "",
  bool $secure = false,
  bool $httponly = false,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function define_syslog_variables(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function openlog(
  string $ident,
  int $option,
  int $facility,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function closelog(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function syslog(int $priority, string $message): bool;

}

namespace HH {

function parse_cookies(
  string $header_value,
)[]: dict<arraykey, mixed>;

}
