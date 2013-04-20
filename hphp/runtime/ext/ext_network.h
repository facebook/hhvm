/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/base_includes.h>
#include <runtime/ext/ext_stream.h>
#include <syslog.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// DNS
Variant f_gethostname();
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

bool f_checkdnsrr(CStrRef host, CStrRef type = null_string);

Variant f_dns_get_record(CStrRef hostname, int type = -1, VRefParam authns = uninit_null(),
                         VRefParam addtl = uninit_null());

bool f_dns_get_mx(CStrRef hostname, VRefParam mxhosts, VRefParam weights = uninit_null());

bool f_getmxrr(CStrRef hostname, VRefParam mxhosts,
               VRefParam weight = uninit_null());

///////////////////////////////////////////////////////////////////////////////
// socket

Variant f_fsockopen(CStrRef hostname, int port = -1, VRefParam errnum = uninit_null(),
                    VRefParam errstr = uninit_null(), double timeout = 0.0);

Variant f_pfsockopen(CStrRef hostname, int port = -1, VRefParam errnum = uninit_null(),
                     VRefParam errstr = uninit_null(), double timeout = 0.0);

Variant f_socket_get_status(CObjRef stream);

bool f_socket_set_blocking(CObjRef stream, int mode);

bool f_socket_set_timeout(CObjRef stream, int seconds,
                          int microseconds = 0);

///////////////////////////////////////////////////////////////////////////////
// http

void f_header(CStrRef str, bool replace = true, int http_response_code = 0);

Variant f_http_response_code(int response_code = 0);

Array f_headers_list();

bool f_headers_sent(VRefParam file = uninit_null(), VRefParam line = uninit_null());

bool f_header_register_callback(CVarRef callback);

void f_header_remove(CStrRef name = null_string);

int f_get_http_request_size();

bool f_setcookie(CStrRef name, CStrRef value = null_string, int64_t expire = 0,
                 CStrRef path = null_string, CStrRef domain = null_string,
                 bool secure = false, bool httponly = false);

bool f_setrawcookie(CStrRef name, CStrRef value = null_string, int64_t expire = 0,
                    CStrRef path = null_string, CStrRef domain = null_string,
                    bool secure = false, bool httponly = false);

///////////////////////////////////////////////////////////////////////////////
// syslog

void f_define_syslog_variables();

bool f_openlog(CStrRef ident, int option, int facility);

bool f_closelog();

bool f_syslog(int priority, CStrRef message);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_NETWORK_H_
