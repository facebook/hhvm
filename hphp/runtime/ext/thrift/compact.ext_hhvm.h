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
int HPHP::f_thrift_protocol_set_compact_version(int)
_ZN4HPHP37f_thrift_protocol_set_compact_versionEi

(return value) => rax
version => rdi
*/

int fh_thrift_protocol_set_compact_version(int version) asm("_ZN4HPHP37f_thrift_protocol_set_compact_versionEi");

/*
void HPHP::f_thrift_protocol_write_compact(HPHP::Object const&, HPHP::String const&, long long, HPHP::Object const&, int)
_ZN4HPHP31f_thrift_protocol_write_compactERKNS_6ObjectERKNS_6StringExS2_i

transportobj => rdi
method_name => rsi
msgtype => rdx
request_struct => rcx
seqid => r8
*/

void fh_thrift_protocol_write_compact(Value* transportobj, Value* method_name, long long msgtype, Value* request_struct, int seqid) asm("_ZN4HPHP31f_thrift_protocol_write_compactERKNS_6ObjectERKNS_6StringExS2_i");

/*
HPHP::Variant HPHP::f_thrift_protocol_read_compact(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP30f_thrift_protocol_read_compactERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
transportobj => rsi
obj_typename => rdx
*/

TypedValue* fh_thrift_protocol_read_compact(TypedValue* _rv, Value* transportobj, Value* obj_typename) asm("_ZN4HPHP30f_thrift_protocol_read_compactERKNS_6ObjectERKNS_6StringE");


} // !HPHP

