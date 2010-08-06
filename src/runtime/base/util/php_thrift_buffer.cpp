/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

String PhpThriftBuffer::readImpl() {
  Array args(CREATE_VECTOR1(m_size));
  return m_xin->o_invoke_mil("read", args, -1);
}

void PhpThriftBuffer::flushImpl(CStrRef data) {
  m_xout->o_invoke_mil("write", CREATE_VECTOR1(data), -1);
}

///////////////////////////////////////////////////////////////////////////////
}
