/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_NETWORK_H_
#define incl_HPHP_EXT_NETWORK_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/ext_stream.h"
#include "hphp/util/network.h"
#include <syslog.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// DNS
Variant f_gethostname();
Variant f_gethostbyaddr(const String& ip_address);
String f_gethostbyname(const String& hostname);
Variant f_gethostbynamel(const String& hostname);
Variant f_getprotobyname(const String& name);
Variant f_getprotobynumber(int number);
Variant f_getservbyname(const String& service, const String& protocol);
Variant f_getservbyport(int port, const String& protocol);
Variant f_inet_ntop(const String& in_addr);
Variant f_inet_pton(const String& address);
Variant f_ip2long(const String& ip_address);
String f_long2ip(int proper_address);

bool f_dns_check_record(const String& host, const String& type = null_string);

bool f_checkdnsrr(const String& host, const String& type = null_string);

Variant f_dns_get_record(const String& hostname, int type = -1, VRefParam authns = uninit_null(),
                         VRefParam addtl = uninit_null());

bool f_dns_get_mx(const String& hostname, VRefParam mxhosts, VRefParam weights = uninit_null());

bool f_getmxrr(const String& hostname, VRefParam mxhosts,
               VRefParam weight = uninit_null());

///////////////////////////////////////////////////////////////////////////////
// socket

Variant sockopen_impl(const Util::HostURL &hosturl,
                      VRefParam errnum, VRefParam errstr,
                      double timeout = -1.0, bool persistent = false);
Variant f_fsockopen(const String& hostname, int port = -1,
                    VRefParam errnum = uninit_null(),
                    VRefParam errstr = uninit_null(), double timeout = -1.0);

Variant f_pfsockopen(const String& hostname, int port = -1,
                     VRefParam errnum = uninit_null(),
                     VRefParam errstr = uninit_null(), double timeout = -1.0);

Variant f_socket_get_status(CResRef stream);

bool f_socket_set_blocking(CResRef stream, int mode);

bool f_socket_set_timeout(CResRef stream, int seconds,
                          int microseconds = 0);

///////////////////////////////////////////////////////////////////////////////
// http

void f_header(const String& str, bool replace = true, int http_response_code = 0);

Variant f_http_response_code(int response_code = 0);

Array f_headers_list();

bool f_headers_sent(VRefParam file = uninit_null(), VRefParam line = uninit_null());

bool f_header_register_callback(CVarRef callback);

void f_header_remove(const String& name = null_string);

int f_get_http_request_size();

bool f_setcookie(const String& name, const String& value = null_string, int64_t expire = 0,
                 const String& path = null_string, const String& domain = null_string,
                 bool secure = false, bool httponly = false);

bool f_setrawcookie(const String& name, const String& value = null_string, int64_t expire = 0,
                    const String& path = null_string, const String& domain = null_string,
                    bool secure = false, bool httponly = false);

///////////////////////////////////////////////////////////////////////////////
// syslog

void f_define_syslog_variables();

bool f_openlog(const String& ident, int option, int facility);

bool f_closelog();

bool f_syslog(int priority, const String& message);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_NETWORK_H_
