/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/util/php_thrift_buffer.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/externals.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

String PhpThriftBuffer::readImpl() {
  Array args(CREATE_VECTOR1(m_size));
  return m_xin->o_invoke("read", args, -1);
}

void PhpThriftBuffer::flushImpl(CStrRef data) {
  m_xout->o_invoke("write", CREATE_VECTOR1(data), -1);
}

void PhpThriftBuffer::throwError(const char *msg, int code) {
  throw create_object("TProtocolException",
                      CREATE_VECTOR2(String(msg, CopyString), code));
}

///////////////////////////////////////////////////////////////////////////////
}
