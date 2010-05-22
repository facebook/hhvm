/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __EXT_NETWORK_H__
#define __EXT_NETWORK_H__

#include <runtime/base/base_includes.h>
#include <runtime/ext/ext_stream.h>
#include <syslog.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// DNS

Variant f_gethostbyaddr(CStrRef ip_address);
String f_gethostbyname(CStrRef hostname);
Variant f_gethostbynamel(CStrRef hostname);
Variant f_getprotobyname(CStrRef name);
Variant f_getprotobynumber(int number);
Variant f_getservbyname(CStrRef service, CStrRef protocol);
Variant f_getservbyport(int port, CStrRef protocol);
Variant f_inet_ntop(CStrRef in_addr);
Variant f_inet_pton(CStrRef address);
Variant f_ip2long(CStrRef ip_address);
String f_long2ip(int proper_address);

bool f_dns_check_record(CStrRef host, CStrRef type = null_string);

inline bool f_checkdnsrr(CStrRef host, CStrRef type = null_string) {
  return f_dns_check_record(host, type);
}

Variant f_dns_get_record(CStrRef hostname, int type = -1, Variant authns = null,
                         Variant addtl = null);

bool f_dns_get_mx(CStrRef hostname, Variant mxhosts, Variant weights = null);

inline bool f_getmxrr(CStrRef hostname, Variant mxhosts,
                      Variant weight = null) {
  return f_dns_get_mx(hostname, ref(mxhosts), weight);
}

///////////////////////////////////////////////////////////////////////////////
// socket

Variant f_fsockopen(CStrRef hostname, int port = -1, Variant errnum = null,
                    Variant errstr = null, double timeout = 0.0);

Variant f_pfsockopen(CStrRef hostname, int port = -1, Variant errnum = null,
                     Variant errstr = null, double timeout = 0.0);

inline Array f_socket_get_status(CObjRef stream) {
  return f_stream_get_meta_data(stream);
}

inline bool f_socket_set_blocking(CObjRef stream, int mode) {
  return f_stream_set_blocking(stream, mode);
}

inline bool f_socket_set_timeout(CObjRef stream, int seconds,
                                 int microseconds = 0) {
  return f_stream_set_timeout(stream, seconds, microseconds);
}

///////////////////////////////////////////////////////////////////////////////
// http

void f_header(CStrRef str, bool replace = true, int http_response_code = 0);

Array f_headers_list();

bool f_headers_sent(Variant file = null, Variant line = null);

void f_header_remove(CStrRef name = null_string);

bool f_setcookie(CStrRef name, CStrRef value = null_string, int expire = 0,
                 CStrRef path = null_string, CStrRef domain = null_string,
                 bool secure = false, bool httponly = false);

bool f_setrawcookie(CStrRef name, CStrRef value = null_string, int expire = 0,
                    CStrRef path = null_string, CStrRef domain = null_string,
                    bool secure = false, bool httponly = false);

///////////////////////////////////////////////////////////////////////////////
// syslog

inline void f_define_syslog_variables() {
  // do nothing, since all variables are defined as constants already
}

inline void f_openlog(CStrRef ident, int option, int facility) {
  openlog(ident.data(), option, facility);
}

inline void f_closelog() {
  closelog();
}

inline void f_syslog(int priority, CStrRef message) {
  syslog(priority, "%s", message.data());
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_NETWORK_H__
