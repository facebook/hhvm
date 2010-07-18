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

#include <runtime/eval/debugger/debugger_thrift_buffer.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

String DebuggerThriftBuffer::readImpl() {
  char buf[1024 + 1];
  ASSERT(m_size <= 1024);
  int nread = m_socket->readImpl(buf, m_size);
  buf[nread] = '\0';
  return String(buf, nread, AttachLiteral);
}

void DebuggerThriftBuffer::flushImpl(CStrRef data) {
  m_socket->write(data);
}

///////////////////////////////////////////////////////////////////////////////
}
