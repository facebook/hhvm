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
const int CURLINFO_CONNECT_TIME = 0;
const int CURLINFO_CONTENT_LENGTH_DOWNLOAD = 0;
const int CURLINFO_CONTENT_LENGTH_UPLOAD = 0;
const int CURLINFO_CONTENT_TYPE = 0;
const int CURLINFO_EFFECTIVE_URL = 0;
const int CURLINFO_FILETIME = 0;
const int CURLINFO_HEADER_OUT = 0;
const int CURLINFO_HEADER_SIZE = 0;
const int CURLINFO_HTTP_CODE = 0;
const int CURLINFO_LOCAL_PORT = 0;
const int CURLINFO_NAMELOOKUP_TIME = 0;
const int CURLINFO_PRETRANSFER_TIME = 0;
const int CURLINFO_REDIRECT_COUNT = 0;
const int CURLINFO_REDIRECT_TIME = 0;
const int CURLINFO_REQUEST_SIZE = 0;
const int CURLINFO_SIZE_DOWNLOAD = 0;
const int CURLINFO_SIZE_UPLOAD = 0;
const int CURLINFO_SPEED_DOWNLOAD = 0;
const int CURLINFO_SPEED_UPLOAD = 0;
const int CURLINFO_SSL_VERIFYRESULT = 0;
const int CURLINFO_STARTTRANSFER_TIME = 0;
const int CURLINFO_TOTAL_TIME = 0;
const int CURLM_BAD_EASY_HANDLE = 0;
const int CURLM_BAD_HANDLE = 0;
const int CURLM_CALL_MULTI_PERFORM = 0;
const int CURLM_INTERNAL_ERROR = 0;
const int CURLM_OK = 0;
const int CURLM_OUT_OF_MEMORY = 0;
const int CURLOPT_AUTOREFERER = 0;
const int CURLOPT_BINARYTRANSFER = 0;
const int CURLOPT_COOKIESESSION = 0;
const int CURLOPT_CRLF = 0;
const int CURLOPT_DNS_USE_GLOBAL_CACHE = 0;
const int CURLOPT_FAILONERROR = 0;
const int CURLOPT_FILETIME = 0;
const int CURLOPT_FOLLOWLOCATION = 0;
const int CURLOPT_FORBID_REUSE = 0;
const int CURLOPT_FRESH_CONNECT = 0;
const int CURLOPT_HEADER = 0;
const int CURLOPT_HTTPGET = 0;
const int CURLOPT_MUTE = 0;
const int CURLOPT_NOBODY = 0;
const int CURLOPT_NOPROGRESS = 0;
const int CURLOPT_NOSIGNAL = 0;
const int CURLOPT_POST = 0;
const int CURLOPT_PUT = 0;
const int CURLOPT_RETURNTRANSFER = 0;
const int CURLOPT_UPLOAD = 0;
const int CURLOPT_VERBOSE = 0;
const int CURLOPT_BUFFERSIZE = 0;
const int CURLOPT_CLOSEPOLICY = 0;
const int CURLOPT_HTTP_VERSION = 0;
const int CURLOPT_HTTPAUTH = 0;
const int CURLOPT_INFILESIZE = 0;
const int CURLOPT_MAXCONNECTS = 0;
const int CURLOPT_MAXREDIRS = 0;
const int CURLOPT_PORT = 0;
const int CURLOPT_RESUME_FROM = 0;
const int CURLOPT_TIMECONDITION = 0;
const int CURLOPT_TIMEVALUE = 0;
const int CURLOPT_COOKIE = 0;
const int CURLOPT_COOKIEFILE = 0;
const int CURLOPT_COOKIEJAR = 0;
const int CURLOPT_CUSTOMREQUEST = 0;
const int CURLOPT_EGDSOCKET = 0;
const int CURLOPT_ENCODING = 0;
const int CURLOPT_INTERFACE = 0;
const int CURLOPT_POSTFIELDS = 0;
const int CURLOPT_RANGE = 0;
const int CURLOPT_REFERER = 0;
const int CURLOPT_URL = 0;
const int CURLOPT_USERAGENT = 0;
const int CURLOPT_USERPWD = 0;
const int CURLOPT_HTTPHEADER = 0;
const int CURLOPT_FILE = 0;
const int CURLOPT_INFILE = 0;
const int CURLOPT_STDERR = 0;
const int CURLOPT_WRITEHEADER = 0;
const int CURLOPT_HEADERFUNCTION = 0;
const int CURLOPT_PASSWDFUNCTION = 0;
const int CURLOPT_READFUNCTION = 0;
const int CURLOPT_WRITEFUNCTION = 0;
const int CURLOPT_HTTPPROXYTUNNEL = 0;
const int CURLOPT_PROXYAUTH = 0;
const int CURLOPT_PROXYPORT = 0;
const int CURLOPT_PROXYTYPE = 0;
const int CURLOPT_PROXY = 0;
const int CURLOPT_PROXYUSERPWD = 0;
const int CURLOPT_CONNECTTIMEOUT = 0;
const int CURLOPT_CONNECTTIMEOUT_MS = 0;
const int CURLOPT_DNS_CACHE_TIMEOUT = 0;
const int CURLOPT_LOW_SPEED_LIMIT = 0;
const int CURLOPT_LOW_SPEED_TIME = 0;
const int CURLOPT_TIMEOUT = 0;
const int CURLOPT_TIMEOUT_MS = 0;
const int CURLOPT_SSL_VERIFYPEER = 0;
const int CURLOPT_SSL_VERIFYHOST = 0;
const int CURLOPT_SSLVERSION = 0;
const int CURLOPT_CAINFO = 0;
const int CURLOPT_CAPATH = 0;
const int CURLOPT_RANDOM_FILE = 0;
const int CURLOPT_SSL_CIPHER_LIST = 0;
const int CURLOPT_SSLCERT = 0;
const int CURLOPT_SSLCERTPASSWD = 0;
const int CURLOPT_SSLCERTTYPE = 0;
const int CURLOPT_SSLENGINE = 0;
const int CURLOPT_SSLENGINE_DEFAULT = 0;
const int CURLOPT_SSLKEY = 0;
const int CURLOPT_SSLKEYPASSWD = 0;
const int CURLOPT_SSLKEYTYPE = 0;
const int CURLOPT_FB_TLS_CIPHER_SPEC = 0;
const int CURLOPT_FTP_USE_EPRT = 0;
const int CURLOPT_FTP_USE_EPSV = 0;
const int CURLOPT_FTPAPPEND = 0;
const int CURLOPT_FTPLISTONLY = 0;
const int CURLOPT_NETRC = 0;
const int CURLOPT_TRANSFERTEXT = 0;
const int CURLOPT_UNRESTRICTED_AUTH = 0;
const int CURLOPT_FTPSSLAUTH = 0;
const int CURLOPT_FTPPORT = 0;
const int CURLOPT_POSTQUOTE = 0;
const int CURLOPT_QUOTE = 0;

function curl_init($url = null) { }
function curl_copy_handle($ch) { }
function curl_version($uversion = CURLVERSION_NOW) { }
function curl_setopt($ch, $option, $value) { }
function curl_setopt_array($ch, $options) { }
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
function curl_multi_getcontent($ch) { }
function curl_multi_info_read($mh, &$msgs_in_queue = null) { }
function curl_multi_close($mh) { }
