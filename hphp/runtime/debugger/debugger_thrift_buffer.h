/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/thrift-buffer.h"
#include "hphp/runtime/base/variable-serializer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Wire format and buffer for socket communication between DebuggerClient and
 * DebuggerProxy.
 */
struct DebuggerThriftBuffer : ThriftBuffer {
  static constexpr int BUFFER_SIZE = 1024;

  DebuggerThriftBuffer()
    : ThriftBuffer(BUFFER_SIZE, VariableSerializer::Type::DebuggerSerialize) {}

  req::ptr<Socket> getSocket() {
    return req::make<Socket>(m_socket);
  }

  void create(req::ptr<Socket> socket) {
    m_socket = socket->getData();
  }
  void close() {
    getSocket()->close();
  }

protected:
  virtual String readImpl();
  virtual void flushImpl(const String& data);
  virtual void throwError(const char* msg, int code);

private:
  char m_buffer[BUFFER_SIZE + 1];
  std::shared_ptr<SocketData> m_socket;
};

///////////////////////////////////////////////////////////////////////////////

struct DebuggerWireHelpers {
  enum SError { // SerializationError
    NoError,
    HitLimit,
    UnknownError,
    TypeMismatch,
  };

  // Serialization functions for Array, Object, and Variant
  // Return true on success, false on error
  // On error, the result would be a special string indicating the error
  static int WireSerialize(const Array& data, String& sdata);
  static int WireSerialize(const Object& data, String& sdata);
  static int WireSerialize(const Variant& data, String& sdata);
  static int WireUnserialize(String& sdata, Array& data);
  static int WireUnserialize(String& sdata, Object& data);
  static int WireUnserialize(String& sdata, Variant& data);
};


///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DEBUGGER_THRIFT_BUFFER_H_
