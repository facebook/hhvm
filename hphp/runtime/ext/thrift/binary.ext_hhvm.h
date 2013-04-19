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

void fh_thrift_protocol_write_binary(Value* transportobj, Value* method_name, long msgtype, Value* request_struct, int seqid, bool strict_write) asm("_ZN4HPHP30f_thrift_protocol_write_binaryERKNS_6ObjectERKNS_6StringElS2_ib");

TypedValue* fh_thrift_protocol_read_binary(TypedValue* _rv, Value* transportobj, Value* obj_typename, bool strict_read) asm("_ZN4HPHP29f_thrift_protocol_read_binaryERKNS_6ObjectERKNS_6StringEb");

} // namespace HPHP
