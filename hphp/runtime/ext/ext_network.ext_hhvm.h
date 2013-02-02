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

/*
HPHP::Variant HPHP::f_gethostname()
_ZN4HPHP13f_gethostnameEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_gethostname(TypedValue* _rv) asm("_ZN4HPHP13f_gethostnameEv");

/*
HPHP::Variant HPHP::f_gethostbyaddr(HPHP::String const&)
_ZN4HPHP15f_gethostbyaddrERKNS_6StringE

(return value) => rax
_rv => rdi
ip_address => rsi
*/

TypedValue* fh_gethostbyaddr(TypedValue* _rv, Value* ip_address) asm("_ZN4HPHP15f_gethostbyaddrERKNS_6StringE");

/*
HPHP::String HPHP::f_gethostbyname(HPHP::String const&)
_ZN4HPHP15f_gethostbynameERKNS_6StringE

(return value) => rax
_rv => rdi
hostname => rsi
*/

Value* fh_gethostbyname(Value* _rv, Value* hostname) asm("_ZN4HPHP15f_gethostbynameERKNS_6StringE");

/*
HPHP::Variant HPHP::f_gethostbynamel(HPHP::String const&)
_ZN4HPHP16f_gethostbynamelERKNS_6StringE

(return value) => rax
_rv => rdi
hostname => rsi
*/

TypedValue* fh_gethostbynamel(TypedValue* _rv, Value* hostname) asm("_ZN4HPHP16f_gethostbynamelERKNS_6StringE");

/*
HPHP::Variant HPHP::f_getprotobyname(HPHP::String const&)
_ZN4HPHP16f_getprotobynameERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

TypedValue* fh_getprotobyname(TypedValue* _rv, Value* name) asm("_ZN4HPHP16f_getprotobynameERKNS_6StringE");

/*
HPHP::Variant HPHP::f_getprotobynumber(int)
_ZN4HPHP18f_getprotobynumberEi

(return value) => rax
_rv => rdi
number => rsi
*/

TypedValue* fh_getprotobynumber(TypedValue* _rv, int number) asm("_ZN4HPHP18f_getprotobynumberEi");

/*
HPHP::Variant HPHP::f_getservbyname(HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_getservbynameERKNS_6StringES2_

(return value) => rax
_rv => rdi
service => rsi
protocol => rdx
*/

TypedValue* fh_getservbyname(TypedValue* _rv, Value* service, Value* protocol) asm("_ZN4HPHP15f_getservbynameERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_getservbyport(int, HPHP::String const&)
_ZN4HPHP15f_getservbyportEiRKNS_6StringE

(return value) => rax
_rv => rdi
port => rsi
protocol => rdx
*/

TypedValue* fh_getservbyport(TypedValue* _rv, int port, Value* protocol) asm("_ZN4HPHP15f_getservbyportEiRKNS_6StringE");

/*
HPHP::Variant HPHP::f_inet_ntop(HPHP::String const&)
_ZN4HPHP11f_inet_ntopERKNS_6StringE

(return value) => rax
_rv => rdi
in_addr => rsi
*/

TypedValue* fh_inet_ntop(TypedValue* _rv, Value* in_addr) asm("_ZN4HPHP11f_inet_ntopERKNS_6StringE");

/*
HPHP::Variant HPHP::f_inet_pton(HPHP::String const&)
_ZN4HPHP11f_inet_ptonERKNS_6StringE

(return value) => rax
_rv => rdi
address => rsi
*/

TypedValue* fh_inet_pton(TypedValue* _rv, Value* address) asm("_ZN4HPHP11f_inet_ptonERKNS_6StringE");

/*
HPHP::Variant HPHP::f_ip2long(HPHP::String const&)
_ZN4HPHP9f_ip2longERKNS_6StringE

(return value) => rax
_rv => rdi
ip_address => rsi
*/

TypedValue* fh_ip2long(TypedValue* _rv, Value* ip_address) asm("_ZN4HPHP9f_ip2longERKNS_6StringE");

/*
HPHP::String HPHP::f_long2ip(int)
_ZN4HPHP9f_long2ipEi

(return value) => rax
_rv => rdi
proper_address => rsi
*/

Value* fh_long2ip(Value* _rv, int proper_address) asm("_ZN4HPHP9f_long2ipEi");

/*
bool HPHP::f_dns_check_record(HPHP::String const&, HPHP::String const&)
_ZN4HPHP18f_dns_check_recordERKNS_6StringES2_

(return value) => rax
host => rdi
type => rsi
*/

bool fh_dns_check_record(Value* host, Value* type) asm("_ZN4HPHP18f_dns_check_recordERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_dns_get_record(HPHP::String const&, int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP16f_dns_get_recordERKNS_6StringEiRKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
hostname => rsi
type => rdx
authns => rcx
addtl => r8
*/

TypedValue* fh_dns_get_record(TypedValue* _rv, Value* hostname, int type, TypedValue* authns, TypedValue* addtl) asm("_ZN4HPHP16f_dns_get_recordERKNS_6StringEiRKNS_14VRefParamValueES5_");

/*
bool HPHP::f_dns_get_mx(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP12f_dns_get_mxERKNS_6StringERKNS_14VRefParamValueES5_

(return value) => rax
hostname => rdi
mxhosts => rsi
weights => rdx
*/

bool fh_dns_get_mx(Value* hostname, TypedValue* mxhosts, TypedValue* weights) asm("_ZN4HPHP12f_dns_get_mxERKNS_6StringERKNS_14VRefParamValueES5_");

/*
void HPHP::f_header(HPHP::String const&, bool, int)
_ZN4HPHP8f_headerERKNS_6StringEbi

str => rdi
replace => rsi
http_response_code => rdx
*/

void fh_header(Value* str, bool replace, int http_response_code) asm("_ZN4HPHP8f_headerERKNS_6StringEbi");

/*
HPHP::Array HPHP::f_headers_list()
_ZN4HPHP14f_headers_listEv

(return value) => rax
_rv => rdi
*/

Value* fh_headers_list(Value* _rv) asm("_ZN4HPHP14f_headers_listEv");

/*
int HPHP::f_get_http_request_size()
_ZN4HPHP23f_get_http_request_sizeEv

(return value) => rax
*/

int fh_get_http_request_size() asm("_ZN4HPHP23f_get_http_request_sizeEv");

/*
bool HPHP::f_headers_sent(HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_headers_sentERKNS_14VRefParamValueES2_

(return value) => rax
file => rdi
line => rsi
*/

bool fh_headers_sent(TypedValue* file, TypedValue* line) asm("_ZN4HPHP14f_headers_sentERKNS_14VRefParamValueES2_");

/*
bool HPHP::f_header_register_callback(HPHP::Variant const&)
_ZN4HPHP26f_header_register_callbackERKNS_7VariantE

(return value) => rax
callback => rdi
*/

bool fh_header_register_callback(TypedValue* callback) asm("_ZN4HPHP26f_header_register_callbackERKNS_7VariantE");

/*
void HPHP::f_header_remove(HPHP::String const&)
_ZN4HPHP15f_header_removeERKNS_6StringE

name => rdi
*/

void fh_header_remove(Value* name) asm("_ZN4HPHP15f_header_removeERKNS_6StringE");

/*
bool HPHP::f_setcookie(HPHP::String const&, HPHP::String const&, long long, HPHP::String const&, HPHP::String const&, bool, bool)
_ZN4HPHP11f_setcookieERKNS_6StringES2_xS2_S2_bb

(return value) => rax
name => rdi
value => rsi
expire => rdx
path => rcx
domain => r8
secure => r9
httponly => st0
*/

bool fh_setcookie(Value* name, Value* value, long long expire, Value* path, Value* domain, bool secure, bool httponly) asm("_ZN4HPHP11f_setcookieERKNS_6StringES2_xS2_S2_bb");

/*
bool HPHP::f_setrawcookie(HPHP::String const&, HPHP::String const&, long long, HPHP::String const&, HPHP::String const&, bool, bool)
_ZN4HPHP14f_setrawcookieERKNS_6StringES2_xS2_S2_bb

(return value) => rax
name => rdi
value => rsi
expire => rdx
path => rcx
domain => r8
secure => r9
httponly => st0
*/

bool fh_setrawcookie(Value* name, Value* value, long long expire, Value* path, Value* domain, bool secure, bool httponly) asm("_ZN4HPHP14f_setrawcookieERKNS_6StringES2_xS2_S2_bb");


} // !HPHP

