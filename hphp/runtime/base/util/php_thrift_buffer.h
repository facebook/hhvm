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

#ifndef __HPHP_PHP_THRIFT_BUFFER_H__
#define __HPHP_PHP_THRIFT_BUFFER_H__

#include <runtime/base/util/thrift_buffer.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Efficient thrift input/output preparation. Used by automatically generated
 * separable extension code created by running thrift compiler, for example,
 *
 *   thrift --gen hphp my_service.thrift
 */
class PhpThriftBuffer: public ThriftBuffer {
public:
  PhpThriftBuffer() : ThriftBuffer(102400) {}

  // passing in input and output transport objects
  void create(CObjRef xin, CObjRef xout) {
    m_xin = xin;
    m_xout = xout;
  }

protected:
  virtual String readImpl();
  virtual void flushImpl(CStrRef data);
  virtual void throwError(const char *msg, int code);

private:
  Object m_xin;
  Object m_xout;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_PHP_THRIFT_BUFFER_H__
