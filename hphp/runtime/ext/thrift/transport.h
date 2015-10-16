/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_THRIFT_TRANSPORT_H_
#define incl_HPHP_THRIFT_TRANSPORT_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/util/logger.h"
#include "hphp/util/htonll.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdexcept>

namespace HPHP { namespace thrift {
///////////////////////////////////////////////////////////////////////////////

enum TType {
  T_STOP       = 0,
  T_VOID       = 1,
  T_BOOL       = 2,
  T_BYTE       = 3,
  T_I08        = 3,
  T_I16        = 6,
  T_I32        = 8,
  T_U64        = 9,
  T_I64        = 10,
  T_DOUBLE     = 4,
  T_STRING     = 11,
  T_UTF7       = 11,
  T_STRUCT     = 12,
  T_MAP        = 13,
  T_SET        = 14,
  T_LIST       = 15,
  T_UTF8       = 16,
  T_UTF16      = 17,
  T_FLOAT      = 19,
};

extern const StaticString
  s_getTransport,
  s_flush,
  s_onewayFlush,
  s_write,
  s_putBack,
  s_read,
  s_class,
  s_key,
  s_val,
  s_elem,
  s_var,
  s_type,
  s_ktype,
  s_vtype,
  s_etype,
  s_format,
  s_collection,
  s_TSPEC,
  s_TProtocolException,
  s_TApplicationException;

const size_t SIZE = 8192;

struct PHPOutputTransport {
public:
  explicit PHPOutputTransport(const Object& protocol)
    : m_transport(protocol->o_invoke_few_args(s_getTransport, 0).toObject())
  {}

  ~PHPOutputTransport() {
    // Because this is a destructor, we might already be
    // in the process of unwinding when this function is called, so we
    // need to ensure that no exceptions can escape so that the unwinder
    // does not terminate the process.
    try {
      if (buffer_used != 0) {
        raise_warning("runtime/ext_thrift: "
                      "Output buffer has %lu unflushed bytes", buffer_used);
      }
    } catch (...) {
      handle_destructor_exception();
    }
  }

  void write(const char* data, size_t len) {
    if ((len + buffer_used) > SIZE) {
      writeBufferToTransport();
    }
    if (len > SIZE) {
      directWrite(data, len);
    } else {
      memcpy(buffer_ptr, data, len);
      buffer_used += len;
      buffer_ptr += len;
    }
  }

  void writeI64(int64_t i) {
    i = htonll(i);
    write((const char*)&i, 8);
  }

  void writeU32(uint32_t i) {
    i = htonl(i);
    write((const char*)&i, 4);
  }

  void writeI32(int32_t i) {
    i = htonl(i);
    write((const char*)&i, 4);
  }

  void writeI16(int16_t i) {
    i = htons(i);
    write((const char*)&i, 2);
  }

  void writeI8(int8_t i) {
    write((const char*)&i, 1);
  }

  void writeString(const char* str, size_t len) {
    writeU32(len);
    write(str, len);
  }

  void writeBufferToTransport() {
    if (buffer_used) {
      directWrite(buffer, buffer_used);
      buffer_ptr = buffer;
      buffer_used = 0;
    }
  }

  void flush() {
    writeBufferToTransport();
    directFlush();
  }

  void onewayFlush() {
    writeBufferToTransport();
    directOnewayFlush();
  }

private:
  void directFlush() {
    m_transport->o_invoke_few_args(s_flush, 0);
  }
  void directOnewayFlush() {
    m_transport->o_invoke_few_args(s_onewayFlush, 0);
  }
  void directWrite(const char* data, size_t len) {
    m_transport->o_invoke_few_args(s_write, 1, String(data, len, CopyString));
  }

  char buffer[SIZE];
  char* buffer_ptr{buffer};
  size_t buffer_used{0};

  Object m_transport;
};

struct PHPInputTransport {
  explicit PHPInputTransport(const Object& protocol)
    : m_transport(protocol->o_invoke_few_args(s_getTransport, 0).toObject())
  {}

  ~PHPInputTransport() {
    try {
      put_back();
    } catch (Exception &e) {
      Logger::Error("%s", e.getMessage().c_str());
    } catch (Object &e) {
      try {
        Logger::Error("%s", e.toString().c_str());
      } catch (...) {
        Logger::Error("(e.toString() failed)");
      }
    } catch (...) {
      Logger::Error("(unknown exception)");
    }
  }

  void put_back() {
    if (buffer_used) {
      m_transport->o_invoke_few_args(s_putBack,
                           1, String(buffer_ptr, buffer_used, CopyString));
    }
    buffer = String();
    buffer_used = 0;
    buffer_ptr = nullptr;
  }

  void skip(size_t len) {
    while (len) {
      size_t chunk_size = len < buffer_used ? len : buffer_used;
      if (chunk_size) {
        buffer_ptr += chunk_size;
        buffer_used -= chunk_size;
        len -= chunk_size;
      }
      if (! len) break;
      refill(len);
    }
  }

  void readBytes(void* buf, size_t len) {
    while (len) {
      size_t chunk_size = len < buffer_used ? len : buffer_used;
      if (chunk_size) {
        memcpy(buf, buffer_ptr, chunk_size);
        buffer_ptr += chunk_size;
        buffer_used -= chunk_size;
        buf = reinterpret_cast<char*>(buf) + chunk_size;
        len -= chunk_size;
      }
      if (! len) break;
      refill(len);
    }
  }

  int8_t readI8() {
    int8_t c;
    readBytes(&c, 1);
    return c;
  }

  int16_t readI16() {
    int16_t c;
    readBytes(&c, 2);
    return (int16_t)ntohs(c);
  }

  uint32_t readU32() {
    uint32_t c;
    readBytes(&c, 4);
    return (uint32_t)ntohl(c);
  }

  int32_t readI32() {
    int32_t c;
    readBytes(&c, 4);
    return (int32_t)ntohl(c);
  }

private:
  void refill(size_t len) {
    assert(buffer_used == 0);
    len = std::max<size_t>(len, SIZE);
    buffer =
      m_transport->o_invoke_few_args(s_read, 1, (int64_t)len);
    buffer_used = buffer.size();
    buffer_ptr = buffer.data();
  }

  String buffer;
  const char* buffer_ptr{nullptr};
  size_t buffer_used{0};

  Object m_transport;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif
