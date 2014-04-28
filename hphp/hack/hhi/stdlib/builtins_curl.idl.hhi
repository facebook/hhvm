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
define('CURLOPT_AUTOREFERER', 0);
define('CURLOPT_BINARYTRANSFER', 0);
define('CURLOPT_COOKIESESSION', 0);
define('CURLOPT_CRLF', 0);
define('CURLOPT_DNS_USE_GLOBAL_CACHE', 0);
define('CURLOPT_FAILONERROR', 0);
define('CURLOPT_FILETIME', 0);
define('CURLOPT_FOLLOWLOCATION', 0);
define('CURLOPT_FORBID_REUSE', 0);
define('CURLOPT_FRESH_CONNECT', 0);
define('CURLOPT_HEADER', 0);
define('CURLINFO_HEADER_OUT', 0);
define('CURLOPT_HTTPGET', 0);
define('CURLOPT_MUTE', 0);
define('CURLOPT_NOBODY', 0);
define('CURLOPT_NOPROGRESS', 0);
define('CURLOPT_NOSIGNAL', 0);
define('CURLOPT_POST', 0);
define('CURLOPT_PUT', 0);
define('CURLOPT_RETURNTRANSFER', 0);
define('CURLOPT_UPLOAD', 0);
define('CURLOPT_VERBOSE', 0);
define('CURLOPT_BUFFERSIZE', 0);
define('CURLOPT_CLOSEPOLICY', 0);
define('CURLOPT_HTTP_VERSION', 0);
define('CURLOPT_HTTPAUTH', 0);
define('CURLOPT_INFILESIZE', 0);
define('CURLOPT_MAXCONNECTS', 0);
define('CURLOPT_MAXREDIRS', 0);
define('CURLOPT_PORT', 0);
define('CURLOPT_RESUME_FROM', 0);
define('CURLOPT_TIMECONDITION', 0);
define('CURLOPT_TIMEVALUE', 0);
define('CURLOPT_COOKIE', 0);
define('CURLOPT_COOKIEFILE', 0);
define('CURLOPT_COOKIEJAR', 0);
define('CURLOPT_CUSTOMREQUEST', 0);
define('CURLOPT_EGDSOCKET', 0);
define('CURLOPT_ENCODING', 0);
define('CURLOPT_INTERFACE', 0);
define('CURLOPT_POSTFIELDS', 0);
define('CURLOPT_RANGE', 0);
define('CURLOPT_REFERER', 0);
define('CURLOPT_URL', 0);
define('CURLOPT_USERAGENT', 0);
define('CURLOPT_USERPWD', 0);
define('CURLOPT_HTTPHEADER', 0);
define('CURLOPT_FILE', 0);
define('CURLOPT_INFILE', 0);
define('CURLOPT_STDERR', 0);
define('CURLOPT_WRITEHEADER', 0);
define('CURLOPT_HEADERFUNCTION', 0);
define('CURLOPT_PASSWDFUNCTION', 0);
define('CURLOPT_READFUNCTION', 0);
define('CURLOPT_WRITEFUNCTION', 0);
define('CURLOPT_HTTPPROXYTUNNEL', 0);
define('CURLOPT_PROXYAUTH', 0);
define('CURLOPT_PROXYPORT', 0);
define('CURLOPT_PROXYTYPE', 0);
define('CURLOPT_PROXY', 0);
define('CURLOPT_PROXYUSERPWD', 0);
define('CURLOPT_CONNECTTIMEOUT', 0);
define('CURLOPT_CONNECTTIMEOUT_MS', 0);
define('CURLOPT_DNS_CACHE_TIMEOUT', 0);
define('CURLOPT_LOW_SPEED_LIMIT', 0);
define('CURLOPT_LOW_SPEED_TIME', 0);
define('CURLOPT_TIMEOUT', 0);
define('CURLOPT_TIMEOUT_MS', 0);
define('CURLOPT_SSL_VERIFYPEER', 0);
define('CURLOPT_SSL_VERIFYHOST', 0);
define('CURLOPT_SSLVERSION', 0);
define('CURLOPT_CAINFO', 0);
define('CURLOPT_CAPATH', 0);
define('CURLOPT_RANDOM_FILE', 0);
define('CURLOPT_SSL_CIPHER_LIST', 0);
define('CURLOPT_SSLCERT', 0);
define('CURLOPT_SSLCERTPASSWD', 0);
define('CURLOPT_SSLCERTTYPE', 0);
define('CURLOPT_SSLENGINE', 0);
define('CURLOPT_SSLENGINE_DEFAULT', 0);
define('CURLOPT_SSLKEY', 0);
define('CURLOPT_SSLKEYPASSWD', 0);
define('CURLOPT_SSLKEYTYPE', 0);
define('CURLOPT_FB_TLS_CIPHER_SPEC', 0);
define('CURLOPT_FTP_USE_EPRT', 0);
define('CURLOPT_FTP_USE_EPSV', 0);
define('CURLOPT_FTPAPPEND', 0);
define('CURLOPT_FTPLISTONLY', 0);
define('CURLOPT_NETRC', 0);
define('CURLOPT_TRANSFERTEXT', 0);
define('CURLOPT_UNRESTRICTED_AUTH', 0);
define('CURLOPT_FTPSSLAUTH', 0);
define('CURLOPT_FTPPORT', 0);
define('CURLOPT_POSTQUOTE', 0);
define('CURLOPT_QUOTE', 0);

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
