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
HPHP::Object HPHP::f_memcache_connect(HPHP::String const&, int, int, int)
_ZN4HPHP18f_memcache_connectERKNS_6StringEiii

(return value) => rax
_rv => rdi
host => rsi
port => rdx
timeout => rcx
timeoutms => r8
*/

Value* fh_memcache_connect(Value* _rv, Value* host, int port, int timeout, int timeoutms) asm("_ZN4HPHP18f_memcache_connectERKNS_6StringEiii");

/*
HPHP::Object HPHP::f_memcache_pconnect(HPHP::String const&, int, int, int)
_ZN4HPHP19f_memcache_pconnectERKNS_6StringEiii

(return value) => rax
_rv => rdi
host => rsi
port => rdx
timeout => rcx
timeoutms => r8
*/

Value* fh_memcache_pconnect(Value* _rv, Value* host, int port, int timeout, int timeoutms) asm("_ZN4HPHP19f_memcache_pconnectERKNS_6StringEiii");

/*
bool HPHP::f_memcache_add(HPHP::Object const&, HPHP::String const&, HPHP::Variant const&, int, int)
_ZN4HPHP14f_memcache_addERKNS_6ObjectERKNS_6StringERKNS_7VariantEii

(return value) => rax
memcache => rdi
key => rsi
var => rdx
flag => rcx
expire => r8
*/

bool fh_memcache_add(Value* memcache, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP14f_memcache_addERKNS_6ObjectERKNS_6StringERKNS_7VariantEii");

/*
bool HPHP::f_memcache_set(HPHP::Object const&, HPHP::String const&, HPHP::Variant const&, int, int)
_ZN4HPHP14f_memcache_setERKNS_6ObjectERKNS_6StringERKNS_7VariantEii

(return value) => rax
memcache => rdi
key => rsi
var => rdx
flag => rcx
expire => r8
*/

bool fh_memcache_set(Value* memcache, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP14f_memcache_setERKNS_6ObjectERKNS_6StringERKNS_7VariantEii");

/*
bool HPHP::f_memcache_replace(HPHP::Object const&, HPHP::String const&, HPHP::Variant const&, int, int)
_ZN4HPHP18f_memcache_replaceERKNS_6ObjectERKNS_6StringERKNS_7VariantEii

(return value) => rax
memcache => rdi
key => rsi
var => rdx
flag => rcx
expire => r8
*/

bool fh_memcache_replace(Value* memcache, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP18f_memcache_replaceERKNS_6ObjectERKNS_6StringERKNS_7VariantEii");

/*
HPHP::Variant HPHP::f_memcache_get(HPHP::Object const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_memcache_getERKNS_6ObjectERKNS_7VariantERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
memcache => rsi
key => rdx
flags => rcx
*/

TypedValue* fh_memcache_get(TypedValue* _rv, Value* memcache, TypedValue* key, TypedValue* flags) asm("_ZN4HPHP14f_memcache_getERKNS_6ObjectERKNS_7VariantERKNS_14VRefParamValueE");

/*
bool HPHP::f_memcache_delete(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP17f_memcache_deleteERKNS_6ObjectERKNS_6StringEi

(return value) => rax
memcache => rdi
key => rsi
expire => rdx
*/

bool fh_memcache_delete(Value* memcache, Value* key, int expire) asm("_ZN4HPHP17f_memcache_deleteERKNS_6ObjectERKNS_6StringEi");

/*
long long HPHP::f_memcache_increment(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP20f_memcache_incrementERKNS_6ObjectERKNS_6StringEi

(return value) => rax
memcache => rdi
key => rsi
offset => rdx
*/

long long fh_memcache_increment(Value* memcache, Value* key, int offset) asm("_ZN4HPHP20f_memcache_incrementERKNS_6ObjectERKNS_6StringEi");

/*
long long HPHP::f_memcache_decrement(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP20f_memcache_decrementERKNS_6ObjectERKNS_6StringEi

(return value) => rax
memcache => rdi
key => rsi
offset => rdx
*/

long long fh_memcache_decrement(Value* memcache, Value* key, int offset) asm("_ZN4HPHP20f_memcache_decrementERKNS_6ObjectERKNS_6StringEi");

/*
bool HPHP::f_memcache_close(HPHP::Object const&)
_ZN4HPHP16f_memcache_closeERKNS_6ObjectE

(return value) => rax
memcache => rdi
*/

bool fh_memcache_close(Value* memcache) asm("_ZN4HPHP16f_memcache_closeERKNS_6ObjectE");

/*
bool HPHP::f_memcache_debug(bool)
_ZN4HPHP16f_memcache_debugEb

(return value) => rax
onoff => rdi
*/

bool fh_memcache_debug(bool onoff) asm("_ZN4HPHP16f_memcache_debugEb");

/*
HPHP::Variant HPHP::f_memcache_get_version(HPHP::Object const&)
_ZN4HPHP22f_memcache_get_versionERKNS_6ObjectE

(return value) => rax
_rv => rdi
memcache => rsi
*/

TypedValue* fh_memcache_get_version(TypedValue* _rv, Value* memcache) asm("_ZN4HPHP22f_memcache_get_versionERKNS_6ObjectE");

/*
bool HPHP::f_memcache_flush(HPHP::Object const&, int)
_ZN4HPHP16f_memcache_flushERKNS_6ObjectEi

(return value) => rax
memcache => rdi
timestamp => rsi
*/

bool fh_memcache_flush(Value* memcache, int timestamp) asm("_ZN4HPHP16f_memcache_flushERKNS_6ObjectEi");

/*
bool HPHP::f_memcache_setoptimeout(HPHP::Object const&, int)
_ZN4HPHP23f_memcache_setoptimeoutERKNS_6ObjectEi

(return value) => rax
memcache => rdi
timeoutms => rsi
*/

bool fh_memcache_setoptimeout(Value* memcache, int timeoutms) asm("_ZN4HPHP23f_memcache_setoptimeoutERKNS_6ObjectEi");

/*
long long HPHP::f_memcache_get_server_status(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP28f_memcache_get_server_statusERKNS_6ObjectERKNS_6StringEi

(return value) => rax
memcache => rdi
host => rsi
port => rdx
*/

long long fh_memcache_get_server_status(Value* memcache, Value* host, int port) asm("_ZN4HPHP28f_memcache_get_server_statusERKNS_6ObjectERKNS_6StringEi");

/*
bool HPHP::f_memcache_set_compress_threshold(HPHP::Object const&, int, double)
_ZN4HPHP33f_memcache_set_compress_thresholdERKNS_6ObjectEid

(return value) => rax
memcache => rdi
threshold => rsi
min_savings => xmm0
*/

bool fh_memcache_set_compress_threshold(Value* memcache, int threshold, double min_savings) asm("_ZN4HPHP33f_memcache_set_compress_thresholdERKNS_6ObjectEid");

/*
HPHP::Array HPHP::f_memcache_get_stats(HPHP::Object const&, HPHP::String const&, int, int)
_ZN4HPHP20f_memcache_get_statsERKNS_6ObjectERKNS_6StringEii

(return value) => rax
_rv => rdi
memcache => rsi
type => rdx
slabid => rcx
limit => r8
*/

Value* fh_memcache_get_stats(Value* _rv, Value* memcache, Value* type, int slabid, int limit) asm("_ZN4HPHP20f_memcache_get_statsERKNS_6ObjectERKNS_6StringEii");

/*
HPHP::Array HPHP::f_memcache_get_extended_stats(HPHP::Object const&, HPHP::String const&, int, int)
_ZN4HPHP29f_memcache_get_extended_statsERKNS_6ObjectERKNS_6StringEii

(return value) => rax
_rv => rdi
memcache => rsi
type => rdx
slabid => rcx
limit => r8
*/

Value* fh_memcache_get_extended_stats(Value* _rv, Value* memcache, Value* type, int slabid, int limit) asm("_ZN4HPHP29f_memcache_get_extended_statsERKNS_6ObjectERKNS_6StringEii");

/*
bool HPHP::f_memcache_set_server_params(HPHP::Object const&, HPHP::String const&, int, int, int, bool, HPHP::Variant const&)
_ZN4HPHP28f_memcache_set_server_paramsERKNS_6ObjectERKNS_6StringEiiibRKNS_7VariantE

(return value) => rax
memcache => rdi
host => rsi
port => rdx
timeout => rcx
retry_interval => r8
status => r9
failure_callback => st0
*/

bool fh_memcache_set_server_params(Value* memcache, Value* host, int port, int timeout, int retry_interval, bool status, TypedValue* failure_callback) asm("_ZN4HPHP28f_memcache_set_server_paramsERKNS_6ObjectERKNS_6StringEiiibRKNS_7VariantE");

/*
bool HPHP::f_memcache_add_server(HPHP::Object const&, HPHP::String const&, int, bool, int, int, int, bool, HPHP::Variant const&, int)
_ZN4HPHP21f_memcache_add_serverERKNS_6ObjectERKNS_6StringEibiiibRKNS_7VariantEi

(return value) => rax
memcache => rdi
host => rsi
port => rdx
persistent => rcx
weight => r8
timeout => r9
retry_interval => st0
status => st8
failure_callback => st16
timeoutms => st24
*/

bool fh_memcache_add_server(Value* memcache, Value* host, int port, bool persistent, int weight, int timeout, int retry_interval, bool status, TypedValue* failure_callback, int timeoutms) asm("_ZN4HPHP21f_memcache_add_serverERKNS_6ObjectERKNS_6StringEibiiibRKNS_7VariantEi");


} // !HPHP

