/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_DEBUGGER_THRIFT_BUFFER_H_
#define incl_HPHP_DEBUGGER_THRIFT_BUFFER_H_

#include "hphp/runtime/base/thrift-buffer.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/variable-serializer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Wire format and buffer for socket communication between DebuggerClient and
 * DebuggerProxy.
 */
class DebuggerThriftBuffer: public ThriftBuffer {
public:
  static const int BUFFER_SIZE = 1024;

public:
  DebuggerThriftBuffer()
    : ThriftBuffer(BUFFER_SIZE, VariableSerializer::Type::DebuggerSerialize) {}

  SmartPtr<Socket> getSocket() { return m_socket;}

  void create(SmartPtr<Socket> socket) {
    m_socket = socket;
  }
  void close() {
    m_socket->close();
  }

protected:
  virtual String readImpl();
  virtual void flushImpl(const String& data);
  virtual void throwError(const char *msg, int code);

private:
  char m_buffer[BUFFER_SIZE + 1];
  SmartPtr<Socket> m_socket;
};

///////////////////////////////////////////////////////////////////////////////

class DebuggerWireHelpers {
public:
  enum SError { // SerializationError
    NoError,
    HitLimit,
    UnknownError,
    TypeMismatch,
  };
  // Serialization functions for Array, Object, and Variant
  // Return true on success, false on error
  // On error, the result would be a special string indicating the error
  static int WireSerialize(CArrRef data, String& sdata);
  static int WireSerialize(CObjRef data, String& sdata);
  static int WireSerialize(CVarRef data, String& sdata);
  static int WireUnserialize(String& sdata, Array& data);
  static int WireUnserialize(String& sdata, Object& data);
  static int WireUnserialize(String& sdata, Variant& data);
};


///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DEBUGGER_THRIFT_BUFFER_H_
