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
const int CURLAUTH_ANY = 0;
const int CURLAUTH_ANYSAFE = 0;
const int CURLAUTH_BASIC = 0;
const int CURLAUTH_DIGEST = 0;
const int CURLAUTH_GSSNEGOTIATE = 0;
const int CURLAUTH_NTLM = 0;

const int CURLCLOSEPOLICY_CALLBACK = 0;
const int CURLCLOSEPOLICY_LEAST_RECENTLY_USED = 0;
const int CURLCLOSEPOLICY_LEAST_TRAFFIC = 0;
const int CURLCLOSEPOLICY_OLDEST = 0;
const int CURLCLOSEPOLICY_SLOWEST = 0;

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

const int CURLMSG_DONE = 1;

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
const int CURLOPT_IPRESOLVE = 0;
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
const int CURLOPT_RESOLVE = 0;
const int CURLOPT_QUOTE = 0;

const int CURLE_ABORTED_BY_CALLBACK = 0;
const int CURLE_BAD_CALLING_ORDER = 0;
const int CURLE_BAD_CONTENT_ENCODING = 0;
const int CURLE_BAD_FUNCTION_ARGUMENT = 0;
const int CURLE_BAD_PASSWORD_ENTERED = 0;
const int CURLE_COULDNT_CONNECT = 0;
const int CURLE_COULDNT_RESOLVE_HOST = 0;
const int CURLE_COULDNT_RESOLVE_PROXY = 0;
const int CURLE_FAILED_INIT = 0;
const int CURLE_FILESIZE_EXCEEDED = 0;
const int CURLE_FILE_COULDNT_READ_FILE = 0;
const int CURLE_FTP_ACCESS_DENIED = 0;
const int CURLE_FTP_BAD_DOWNLOAD_RESUME = 0;
const int CURLE_FTP_CANT_GET_HOST = 0;
const int CURLE_FTP_CANT_RECONNECT = 0;
const int CURLE_FTP_COULDNT_GET_SIZE = 0;
const int CURLE_FTP_COULDNT_RETR_FILE = 0;
const int CURLE_FTP_COULDNT_SET_ASCII = 0;
const int CURLE_FTP_COULDNT_SET_BINARY = 0;
const int CURLE_FTP_COULDNT_STOR_FILE = 0;
const int CURLE_FTP_COULDNT_USE_REST = 0;
const int CURLE_FTP_PORT_FAILED = 0;
const int CURLE_FTP_QUOTE_ERROR = 0;
const int CURLE_FTP_SSL_FAILED = 0;
const int CURLE_FTP_USER_PASSWORD_INCORRECT = 0;
const int CURLE_FTP_WEIRD_227_FORMAT = 0;
const int CURLE_FTP_WEIRD_PASS_REPLY = 0;
const int CURLE_FTP_WEIRD_PASV_REPLY = 0;
const int CURLE_FTP_WEIRD_SERVER_REPLY = 0;
const int CURLE_FTP_WEIRD_USER_REPLY = 0;
const int CURLE_FTP_WRITE_ERROR = 0;
const int CURLE_FUNCTION_NOT_FOUND = 0;
const int CURLE_GOT_NOTHING = 0;
const int CURLE_HTTP_NOT_FOUND = 0;
const int CURLE_HTTP_PORT_FAILED = 0;
const int CURLE_HTTP_POST_ERROR = 0;
const int CURLE_HTTP_RANGE_ERROR = 0;
const int CURLE_LDAP_CANNOT_BIND = 0;
const int CURLE_LDAP_INVALID_URL = 0;
const int CURLE_LDAP_SEARCH_FAILED = 0;
const int CURLE_LIBRARY_NOT_FOUND = 0;
const int CURLE_MALFORMAT_USER = 0;
const int CURLE_OBSOLETE = 0;
const int CURLE_OK = 0;
const int CURLE_OPERATION_TIMEOUTED = 0;
const int CURLE_OUT_OF_MEMORY = 0;
const int CURLE_PARTIAL_FILE = 0;
const int CURLE_READ_ERROR = 0;
const int CURLE_RECV_ERROR = 0;
const int CURLE_SEND_ERROR = 0;
const int CURLE_SHARE_IN_USE = 0;
const int CURLE_SSL_CACERT = 0;
const int CURLE_SSL_CERTPROBLEM = 0;
const int CURLE_SSL_CIPHER = 0;
const int CURLE_SSL_CONNECT_ERROR = 0;
const int CURLE_SSL_ENGINE_NOTFOUND = 0;
const int CURLE_SSL_ENGINE_SETFAILED = 0;
const int CURLE_SSL_PEER_CERTIFICATE = 0;
const int CURLE_TELNET_OPTION_SYNTAX = 0;
const int CURLE_TOO_MANY_REDIRECTS = 0;
const int CURLE_UNKNOWN_TELNET_OPTION = 0;
const int CURLE_UNSUPPORTED_PROTOCOL = 0;
const int CURLE_URL_MALFORMAT = 0;
const int CURLE_URL_MALFORMAT_USER = 0;
const int CURLE_WRITE_ERROR = 0;

const int CURLVERSION_NOW = 0;
const int CURL_VERSION_IPV6 = 0;
const int CURL_VERSION_KERBEROS4 = 0;
const int CURL_VERSION_LIBZ = 0;
const int CURL_VERSION_SSL = 0;

const int CURLPROXY_HTTP = 0;
const int CURLPROXY_SOCKS5 = 0;

const int CURL_HTTP_VERSION_1_0 = 0;
const int CURL_HTTP_VERSION_1_1 = 0;
const int CURL_HTTP_VERSION_NONE = 0;

const int CURL_IPRESOLVE_V4 = 0;
const int CURL_IPRESOLVE_V6 = 0;
const int CURL_IPRESOLVE_WHATEVER = 0;

const int CURL_NETRC_IGNORED = 0;
const int CURL_NETRC_OPTIONAL = 0;
const int CURL_NETRC_REQUIRED = 0;

const int CURL_TIMECOND_IFMODSINCE = 0;
const int CURL_TIMECOND_IFUNMODSINCE = 0;
const int CURL_TIMECOND_LASTMOD = 0;

function curl_init($url = null);
function curl_copy_handle($ch);
function curl_version($uversion = CURLVERSION_NOW);
function curl_setopt($ch, $option, $value);
function curl_setopt_array($ch, $options);
function curl_exec($ch);
function curl_getinfo($ch, $opt = 0);
function curl_errno($ch);
function curl_error($ch);
function curl_strerror($code);
function curl_close($ch);
function curl_multi_init();
function curl_multi_add_handle($mh, $ch);
function curl_multi_remove_handle($mh, $ch);
function curl_multi_exec($mh, &$still_running);
function curl_multi_select($mh, $timeout = 1.0);
function curl_multi_await($mh, float $timeout = 1.0): Awaitable<int>;
function curl_multi_getcontent($ch);
function curl_multi_info_read($mh, &$msgs_in_queue = null);
function curl_multi_close($mh);

namespace HH\Asio {
  function curl_exec(mixed $url_or_handle): Awaitable<string>;
}
