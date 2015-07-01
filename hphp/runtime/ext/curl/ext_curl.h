/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_EXT_CURL_H_
#define incl_HPHP_EXT_CURL_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#if LIBCURL_VERSION_NUM >= 0x071500
extern const int64_t k_CURLINFO_LOCAL_PORT;
#endif
#if LIBCURL_VERSION_NUM >= 0x071002
extern const int64_t k_CURLOPT_TIMEOUT_MS;
extern const int64_t k_CURLOPT_CONNECTTIMEOUT_MS;
#endif

extern const int64_t k_CURLAUTH_ANY;
extern const int64_t k_CURLAUTH_ANYSAFE;
extern const int64_t k_CURLAUTH_BASIC;
extern const int64_t k_CURLAUTH_DIGEST;
extern const int64_t k_CURLAUTH_GSSNEGOTIATE;
extern const int64_t k_CURLAUTH_NTLM;
extern const int64_t k_CURLCLOSEPOLICY_CALLBACK;
extern const int64_t k_CURLCLOSEPOLICY_LEAST_RECENTLY_USED;
extern const int64_t k_CURLCLOSEPOLICY_LEAST_TRAFFIC;
extern const int64_t k_CURLCLOSEPOLICY_OLDEST;
extern const int64_t k_CURLCLOSEPOLICY_SLOWEST;
extern const int64_t k_CURLE_ABORTED_BY_CALLBACK;
extern const int64_t k_CURLE_BAD_CALLING_ORDER;
extern const int64_t k_CURLE_BAD_CONTENT_ENCODING;
extern const int64_t k_CURLE_BAD_FUNCTION_ARGUMENT;
extern const int64_t k_CURLE_BAD_PASSWORD_ENTERED;
extern const int64_t k_CURLE_COULDNT_CONNECT;
extern const int64_t k_CURLE_COULDNT_RESOLVE_HOST;
extern const int64_t k_CURLE_COULDNT_RESOLVE_PROXY;
extern const int64_t k_CURLE_FAILED_INIT;
extern const int64_t k_CURLE_FILESIZE_EXCEEDED;
extern const int64_t k_CURLE_FILE_COULDNT_READ_FILE;
extern const int64_t k_CURLE_FTP_ACCESS_DENIED;
extern const int64_t k_CURLE_FTP_BAD_DOWNLOAD_RESUME;
extern const int64_t k_CURLE_FTP_CANT_GET_HOST;
extern const int64_t k_CURLE_FTP_CANT_RECONNECT;
extern const int64_t k_CURLE_FTP_COULDNT_GET_SIZE;
extern const int64_t k_CURLE_FTP_COULDNT_RETR_FILE;
extern const int64_t k_CURLE_FTP_COULDNT_SET_ASCII;
extern const int64_t k_CURLE_FTP_COULDNT_SET_BINARY;
extern const int64_t k_CURLE_FTP_COULDNT_STOR_FILE;
extern const int64_t k_CURLE_FTP_COULDNT_USE_REST;
extern const int64_t k_CURLE_FTP_PORT_FAILED;
extern const int64_t k_CURLE_FTP_QUOTE_ERROR;
extern const int64_t k_CURLE_FTP_SSL_FAILED;
extern const int64_t k_CURLE_FTP_USER_PASSWORD_INCORRECT;
extern const int64_t k_CURLE_FTP_WEIRD_227_FORMAT;
extern const int64_t k_CURLE_FTP_WEIRD_PASS_REPLY;
extern const int64_t k_CURLE_FTP_WEIRD_PASV_REPLY;
extern const int64_t k_CURLE_FTP_WEIRD_SERVER_REPLY;
extern const int64_t k_CURLE_FTP_WEIRD_USER_REPLY;
extern const int64_t k_CURLE_FTP_WRITE_ERROR;
extern const int64_t k_CURLE_FUNCTION_NOT_FOUND;
extern const int64_t k_CURLE_GOT_NOTHING;
extern const int64_t k_CURLE_HTTP_NOT_FOUND;
extern const int64_t k_CURLE_HTTP_PORT_FAILED;
extern const int64_t k_CURLE_HTTP_POST_ERROR;
extern const int64_t k_CURLE_HTTP_RANGE_ERROR;
extern const int64_t k_CURLE_LDAP_CANNOT_BIND;
extern const int64_t k_CURLE_LDAP_INVALID_URL;
extern const int64_t k_CURLE_LDAP_SEARCH_FAILED;
extern const int64_t k_CURLE_LIBRARY_NOT_FOUND;
extern const int64_t k_CURLE_MALFORMAT_USER;
extern const int64_t k_CURLE_OBSOLETE;
extern const int64_t k_CURLE_OK;
extern const int64_t k_CURLE_OPERATION_TIMEOUTED;
extern const int64_t k_CURLE_OUT_OF_MEMORY;
extern const int64_t k_CURLE_PARTIAL_FILE;
extern const int64_t k_CURLE_READ_ERROR;
extern const int64_t k_CURLE_RECV_ERROR;
extern const int64_t k_CURLE_SEND_ERROR;
extern const int64_t k_CURLE_SHARE_IN_USE;
extern const int64_t k_CURLE_SSL_CACERT;
extern const int64_t k_CURLE_SSL_CERTPROBLEM;
extern const int64_t k_CURLE_SSL_CIPHER;
extern const int64_t k_CURLE_SSL_CONNECT_ERROR;
extern const int64_t k_CURLE_SSL_ENGINE_NOTFOUND;
extern const int64_t k_CURLE_SSL_ENGINE_SETFAILED;
extern const int64_t k_CURLE_SSL_PEER_CERTIFICATE;
extern const int64_t k_CURLE_TELNET_OPTION_SYNTAX;
extern const int64_t k_CURLE_TOO_MANY_REDIRECTS;
extern const int64_t k_CURLE_UNKNOWN_TELNET_OPTION;
extern const int64_t k_CURLE_UNSUPPORTED_PROTOCOL;
extern const int64_t k_CURLE_URL_MALFORMAT;
extern const int64_t k_CURLE_URL_MALFORMAT_USER;
extern const int64_t k_CURLE_WRITE_ERROR;
extern const int64_t k_CURLFTPAUTH_DEFAULT;
extern const int64_t k_CURLFTPAUTH_SSL;
extern const int64_t k_CURLFTPAUTH_TLS;
extern const int64_t k_CURLFTPSSL_ALL;
extern const int64_t k_CURLFTPSSL_CONTROL;
extern const int64_t k_CURLFTPSSL_NONE;
extern const int64_t k_CURLFTPSSL_TRY;
extern const int64_t k_CURLINFO_CONNECT_TIME;
extern const int64_t k_CURLINFO_CONTENT_LENGTH_DOWNLOAD;
extern const int64_t k_CURLINFO_CONTENT_LENGTH_UPLOAD;
extern const int64_t k_CURLINFO_CONTENT_TYPE;
extern const int64_t k_CURLINFO_EFFECTIVE_URL;
extern const int64_t k_CURLINFO_FILETIME;
extern const int64_t k_CURLINFO_HEADER_OUT;
extern const int64_t k_CURLINFO_HEADER_SIZE;
extern const int64_t k_CURLINFO_HTTP_CODE;
extern const int64_t k_CURLINFO_NAMELOOKUP_TIME;
extern const int64_t k_CURLINFO_PRETRANSFER_TIME;
extern const int64_t k_CURLINFO_PRIVATE;
extern const int64_t k_CURLINFO_REDIRECT_COUNT;
extern const int64_t k_CURLINFO_REDIRECT_TIME;
extern const int64_t k_CURLINFO_REQUEST_SIZE;
extern const int64_t k_CURLINFO_SIZE_DOWNLOAD;
extern const int64_t k_CURLINFO_SIZE_UPLOAD;
extern const int64_t k_CURLINFO_SPEED_DOWNLOAD;
extern const int64_t k_CURLINFO_SPEED_UPLOAD;
extern const int64_t k_CURLINFO_SSL_VERIFYRESULT;
extern const int64_t k_CURLINFO_STARTTRANSFER_TIME;
extern const int64_t k_CURLINFO_TOTAL_TIME;
extern const int64_t k_CURLMSG_DONE;
extern const int64_t k_CURLM_BAD_EASY_HANDLE;
extern const int64_t k_CURLM_BAD_HANDLE;
extern const int64_t k_CURLM_CALL_MULTI_PERFORM;
extern const int64_t k_CURLM_INTERNAL_ERROR;
extern const int64_t k_CURLM_OK;
extern const int64_t k_CURLM_OUT_OF_MEMORY;
extern const int64_t k_CURLOPT_AUTOREFERER;
extern const int64_t k_CURLOPT_BINARYTRANSFER;
extern const int64_t k_CURLOPT_BUFFERSIZE;
extern const int64_t k_CURLOPT_CAINFO;
extern const int64_t k_CURLOPT_CAPATH;
extern const int64_t k_CURLOPT_CLOSEPOLICY;
extern const int64_t k_CURLOPT_CONNECTTIMEOUT;
extern const int64_t k_CURLOPT_COOKIE;
extern const int64_t k_CURLOPT_COOKIEFILE;
extern const int64_t k_CURLOPT_COOKIEJAR;
extern const int64_t k_CURLOPT_COOKIESESSION;
extern const int64_t k_CURLOPT_CRLF;
extern const int64_t k_CURLOPT_CUSTOMREQUEST;
extern const int64_t k_CURLOPT_DNS_CACHE_TIMEOUT;
extern const int64_t k_CURLOPT_DNS_USE_GLOBAL_CACHE;
extern const int64_t k_CURLOPT_EGDSOCKET;
extern const int64_t k_CURLOPT_ENCODING;
extern const int64_t k_CURLOPT_FAILONERROR;
extern const int64_t k_CURLOPT_FILE;
extern const int64_t k_CURLOPT_FILETIME;
extern const int64_t k_CURLOPT_FOLLOWLOCATION;
extern const int64_t k_CURLOPT_FORBID_REUSE;
extern const int64_t k_CURLOPT_FRESH_CONNECT;
extern const int64_t k_CURLOPT_FTPAPPEND;
extern const int64_t k_CURLOPT_FTPLISTONLY;
extern const int64_t k_CURLOPT_FTPPORT;
extern const int64_t k_CURLOPT_FTPSSLAUTH;
extern const int64_t k_CURLOPT_FTP_CREATE_MISSING_DIRS;
extern const int64_t k_CURLOPT_FTP_SSL;
extern const int64_t k_CURLOPT_FTP_USE_EPRT;
extern const int64_t k_CURLOPT_FTP_USE_EPSV;
extern const int64_t k_CURLOPT_HEADER;
extern const int64_t k_CURLOPT_HEADERFUNCTION;
extern const int64_t k_CURLOPT_HTTP200ALIASES;
extern const int64_t k_CURLOPT_HTTPAUTH;
extern const int64_t k_CURLOPT_HTTPGET;
extern const int64_t k_CURLOPT_HTTPHEADER;
extern const int64_t k_CURLOPT_HTTPPROXYTUNNEL;
extern const int64_t k_CURLOPT_HTTP_VERSION;
extern const int64_t k_CURLOPT_INFILE;
extern const int64_t k_CURLOPT_INFILESIZE;
extern const int64_t k_CURLOPT_INTERFACE;
extern const int64_t k_CURLOPT_IPRESOLVE;
extern const int64_t k_CURLOPT_KRB4LEVEL;
extern const int64_t k_CURLOPT_LOW_SPEED_LIMIT;
extern const int64_t k_CURLOPT_LOW_SPEED_TIME;
extern const int64_t k_CURLOPT_MAXCONNECTS;
extern const int64_t k_CURLOPT_MAXREDIRS;
extern const int64_t k_CURLOPT_MUTE;
extern const int64_t k_CURLOPT_NETRC;
extern const int64_t k_CURLOPT_NOBODY;
extern const int64_t k_CURLOPT_NOPROGRESS;
extern const int64_t k_CURLOPT_NOSIGNAL;
extern const int64_t k_CURLOPT_PASSWDFUNCTION;
extern const int64_t k_CURLOPT_PORT;
extern const int64_t k_CURLOPT_POST;
extern const int64_t k_CURLOPT_POSTFIELDS;
extern const int64_t k_CURLOPT_POSTREDIR;
extern const int64_t k_CURLOPT_POSTQUOTE;
extern const int64_t k_CURLOPT_PRIVATE;
extern const int64_t k_CURLOPT_PROGRESSDATA;
extern const int64_t k_CURLOPT_PROGRESSFUNCTION;
extern const int64_t k_CURLOPT_PROXY;
extern const int64_t k_CURLOPT_PROXYAUTH;
extern const int64_t k_CURLOPT_PROXYPORT;
extern const int64_t k_CURLOPT_PROXYTYPE;
extern const int64_t k_CURLOPT_PROXYUSERPWD;
extern const int64_t k_CURLOPT_PUT;
extern const int64_t k_CURLOPT_QUOTE;
extern const int64_t k_CURLOPT_RANDOM_FILE;
extern const int64_t k_CURLOPT_RANGE;
extern const int64_t k_CURLOPT_READDATA;
extern const int64_t k_CURLOPT_READFUNCTION;
extern const int64_t k_CURLOPT_REFERER;
extern const int64_t k_CURLOPT_RESOLVE;
extern const int64_t k_CURLOPT_RESUME_FROM;
extern const int64_t k_CURLOPT_RETURNTRANSFER;
#ifdef FACEBOOK
extern const int64_t k_CURLOPT_SERVICE_NAME;
#endif
extern const int64_t k_CURLOPT_SSLCERT;
extern const int64_t k_CURLOPT_SSLCERTPASSWD;
extern const int64_t k_CURLOPT_SSLCERTTYPE;
extern const int64_t k_CURLOPT_SSLENGINE;
extern const int64_t k_CURLOPT_SSLENGINE_DEFAULT;
extern const int64_t k_CURLOPT_SSLKEY;
extern const int64_t k_CURLOPT_SSLKEYPASSWD;
extern const int64_t k_CURLOPT_SSLKEYTYPE;
extern const int64_t k_CURLOPT_SSLVERSION;
extern const int64_t k_CURLOPT_SSL_CIPHER_LIST;
extern const int64_t k_CURLOPT_SSL_VERIFYHOST;
extern const int64_t k_CURLOPT_SSL_VERIFYPEER;
extern const int64_t k_CURLOPT_STDERR;
extern const int64_t k_CURLOPT_TCP_NODELAY;
extern const int64_t k_CURLOPT_TIMECONDITION;
extern const int64_t k_CURLOPT_TIMEOUT;
extern const int64_t k_CURLOPT_TIMEVALUE;
extern const int64_t k_CURLOPT_TRANSFERTEXT;
extern const int64_t k_CURLOPT_UNRESTRICTED_AUTH;
extern const int64_t k_CURLOPT_UPLOAD;
extern const int64_t k_CURLOPT_URL;
extern const int64_t k_CURLOPT_USERAGENT;
extern const int64_t k_CURLOPT_USERPWD;
extern const int64_t k_CURLOPT_VERBOSE;
extern const int64_t k_CURLOPT_WRITEFUNCTION;
extern const int64_t k_CURLOPT_WRITEHEADER;
extern const int64_t k_CURLPROXY_HTTP;
extern const int64_t k_CURLPROXY_SOCKS5;
extern const int64_t k_CURLVERSION_NOW;
extern const int64_t k_CURL_HTTP_VERSION_1_0;
extern const int64_t k_CURL_HTTP_VERSION_1_1;
extern const int64_t k_CURL_HTTP_VERSION_NONE;
extern const int64_t k_CURL_IPRESOLVE_V4;
extern const int64_t k_CURL_IPRESOLVE_V6;
extern const int64_t k_CURL_IPRESOLVE_WHATEVER;
extern const int64_t k_CURL_NETRC_IGNORED;
extern const int64_t k_CURL_NETRC_OPTIONAL;
extern const int64_t k_CURL_NETRC_REQUIRED;
extern const int64_t k_CURL_TIMECOND_IFMODSINCE;
extern const int64_t k_CURL_TIMECOND_IFUNMODSINCE;
extern const int64_t k_CURL_TIMECOND_LASTMOD;
extern const int64_t k_CURL_VERSION_IPV6;
extern const int64_t k_CURL_VERSION_KERBEROS4;
extern const int64_t k_CURL_VERSION_LIBZ;
extern const int64_t k_CURL_VERSION_SSL;

Variant HHVM_FUNCTION(curl_init, const Variant& url = null_string);
Variant HHVM_FUNCTION(curl_init_pooled, const String& poolName,
                              const Variant& url = null_string);
Variant HHVM_FUNCTION(curl_copy_handle, const Resource& ch);
Variant HHVM_FUNCTION(curl_version, int uversion = k_CURLVERSION_NOW);
bool HHVM_FUNCTION(curl_setopt, const Resource& ch, int option, const Variant& value);
bool HHVM_FUNCTION(curl_setopt_array, const Resource& ch, const Array& options);
Variant HHVM_FUNCTION(fb_curl_getopt, const Resource& ch, int64_t opt = 0);
Variant HHVM_FUNCTION(curl_exec, const Resource& ch);
Variant HHVM_FUNCTION(curl_getinfo, const Resource& ch, int opt = 0);
Variant HHVM_FUNCTION(curl_errno, const Resource& ch);
Variant HHVM_FUNCTION(curl_error, const Resource& ch);
String HHVM_FUNCTION(curl_strerror, int code);
Variant HHVM_FUNCTION(curl_close, const Resource& ch);
void HHVM_FUNCTION(curl_reset, const Resource& ch);
Resource HHVM_FUNCTION(curl_multi_init);
Variant HHVM_FUNCTION(curl_multi_add_handle, const Resource& mh, const Resource& ch);
Variant HHVM_FUNCTION(curl_multi_remove_handle, const Resource& mh, const Resource& ch);
Variant HHVM_FUNCTION(curl_multi_exec, const Resource& mh, VRefParam still_running);
Variant HHVM_FUNCTION(curl_multi_select, const Resource& mh, double timeout = 1.0);
Variant HHVM_FUNCTION(curl_multi_getcontent, const Resource& ch);
Variant HHVM_FUNCTION(fb_curl_multi_fdset, const Resource& mh,
                              VRefParam read_fd_set,
                              VRefParam write_fd_set,
                              VRefParam exc_fd_set,
                              VRefParam max_fd = null_object);
Variant HHVM_FUNCTION(curl_multi_info_read, const Resource& mh,
                               VRefParam msgs_in_queue = null_object);
Variant HHVM_FUNCTION(curl_multi_close, const Resource& mh);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_CURL_H_
