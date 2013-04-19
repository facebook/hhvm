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
namespace HPHP {

TypedValue* fh_gethostname(TypedValue* _rv) asm("_ZN4HPHP13f_gethostnameEv");

TypedValue* fh_gethostbyaddr(TypedValue* _rv, Value* ip_address) asm("_ZN4HPHP15f_gethostbyaddrERKNS_6StringE");

Value* fh_gethostbyname(Value* _rv, Value* hostname) asm("_ZN4HPHP15f_gethostbynameERKNS_6StringE");

TypedValue* fh_gethostbynamel(TypedValue* _rv, Value* hostname) asm("_ZN4HPHP16f_gethostbynamelERKNS_6StringE");

TypedValue* fh_getprotobyname(TypedValue* _rv, Value* name) asm("_ZN4HPHP16f_getprotobynameERKNS_6StringE");

TypedValue* fh_getprotobynumber(TypedValue* _rv, int number) asm("_ZN4HPHP18f_getprotobynumberEi");

TypedValue* fh_getservbyname(TypedValue* _rv, Value* service, Value* protocol) asm("_ZN4HPHP15f_getservbynameERKNS_6StringES2_");

TypedValue* fh_getservbyport(TypedValue* _rv, int port, Value* protocol) asm("_ZN4HPHP15f_getservbyportEiRKNS_6StringE");

TypedValue* fh_inet_ntop(TypedValue* _rv, Value* in_addr) asm("_ZN4HPHP11f_inet_ntopERKNS_6StringE");

TypedValue* fh_inet_pton(TypedValue* _rv, Value* address) asm("_ZN4HPHP11f_inet_ptonERKNS_6StringE");

TypedValue* fh_ip2long(TypedValue* _rv, Value* ip_address) asm("_ZN4HPHP9f_ip2longERKNS_6StringE");

Value* fh_long2ip(Value* _rv, int proper_address) asm("_ZN4HPHP9f_long2ipEi");

bool fh_dns_check_record(Value* host, Value* type) asm("_ZN4HPHP18f_dns_check_recordERKNS_6StringES2_");

bool fh_checkdnsrr(Value* host, Value* type) asm("_ZN4HPHP12f_checkdnsrrERKNS_6StringES2_");

TypedValue* fh_dns_get_record(TypedValue* _rv, Value* hostname, int type, TypedValue* authns, TypedValue* addtl) asm("_ZN4HPHP16f_dns_get_recordERKNS_6StringEiRKNS_14VRefParamValueES5_");

bool fh_dns_get_mx(Value* hostname, TypedValue* mxhosts, TypedValue* weights) asm("_ZN4HPHP12f_dns_get_mxERKNS_6StringERKNS_14VRefParamValueES5_");

bool fh_getmxrr(Value* hostname, TypedValue* mxhosts, TypedValue* weight) asm("_ZN4HPHP9f_getmxrrERKNS_6StringERKNS_14VRefParamValueES5_");

TypedValue* fh_socket_get_status(TypedValue* _rv, Value* stream) asm("_ZN4HPHP19f_socket_get_statusERKNS_6ObjectE");

bool fh_socket_set_blocking(Value* stream, int mode) asm("_ZN4HPHP21f_socket_set_blockingERKNS_6ObjectEi");

bool fh_socket_set_timeout(Value* stream, int seconds, int microseconds) asm("_ZN4HPHP20f_socket_set_timeoutERKNS_6ObjectEii");

void fh_header(Value* str, bool replace, int http_response_code) asm("_ZN4HPHP8f_headerERKNS_6StringEbi");

bool fh_headers_sent(TypedValue* file, TypedValue* line) asm("_ZN4HPHP14f_headers_sentERKNS_14VRefParamValueES2_");

TypedValue* fh_http_response_code(TypedValue* _rv, int response_code) asm("_ZN4HPHP20f_http_response_codeEi");

Value* fh_headers_list(Value* _rv) asm("_ZN4HPHP14f_headers_listEv");

bool fh_header_register_callback(TypedValue* callback) asm("_ZN4HPHP26f_header_register_callbackERKNS_7VariantE");

void fh_header_remove(Value* name) asm("_ZN4HPHP15f_header_removeERKNS_6StringE");

int fh_get_http_request_size() asm("_ZN4HPHP23f_get_http_request_sizeEv");

bool fh_setcookie(Value* name, Value* value, long expire, Value* path, Value* domain, bool secure, bool httponly) asm("_ZN4HPHP11f_setcookieERKNS_6StringES2_lS2_S2_bb");

bool fh_setrawcookie(Value* name, Value* value, long expire, Value* path, Value* domain, bool secure, bool httponly) asm("_ZN4HPHP14f_setrawcookieERKNS_6StringES2_lS2_S2_bb");

void fh_define_syslog_variables() asm("_ZN4HPHP25f_define_syslog_variablesEv");

bool fh_openlog(Value* ident, int option, int facility) asm("_ZN4HPHP9f_openlogERKNS_6StringEii");

bool fh_closelog() asm("_ZN4HPHP10f_closelogEv");

bool fh_syslog(int priority, Value* message) asm("_ZN4HPHP8f_syslogEiRKNS_6StringE");

} // namespace HPHP
