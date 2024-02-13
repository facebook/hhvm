/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/util/logger.h"
#include "hphp/util/htonll.h"

#include <folly/lang/Bits.h>
#include <sys/types.h>
#include <stdexcept>

#include <folly/portability/Unistd.h>

namespace HPHP::thrift {
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
  s__type,
  s_collection,
  s_harray,
  s_TProtocolException,
  s_TApplicationException;

const size_t SIZE = 8192;

struct PHPOutputTransport {
public:
  explicit PHPOutputTransport(const Object& protocol)
    : m_transport(protocol->o_invoke_few_args(s_getTransport, RuntimeCoeffects::fixme(), 0).toObject())
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
    m_transport->o_invoke_few_args(s_flush, RuntimeCoeffects::fixme(), 0);
  }
  void directOnewayFlush() {
    m_transport->o_invoke_few_args(s_onewayFlush, RuntimeCoeffects::fixme(), 0);
  }
  void directWrite(const char* data, size_t len) {
    m_transport->o_invoke_few_args(s_write, RuntimeCoeffects::fixme(),
                                   1, String(data, len, CopyString));
  }

  char buffer[SIZE];
  char* buffer_ptr{buffer};
  size_t buffer_used{0};

  Object m_transport;
};

struct PHPInputTransport {
  explicit PHPInputTransport(const Object& protocol)
    : m_transport(protocol->o_invoke_few_args(s_getTransport,
                                              RuntimeCoeffects::fixme(),
                                              0).toObject())
  {}

  ~PHPInputTransport() {
    try {
      put_back();
    } catch (Exception& e) {
      Logger::Error(e.getMessage());
    } catch (Object &e) {
      try {
        Logger::Error(throwable_to_string(e.get()).toCppString());
      } catch (...) {
        Logger::Error("(e.toString() failed)");
      }
    } catch (...) {
      Logger::Error("(unknown exception)");
    }
  }

  void put_back() {
    if (buffer_used) {
      m_transport->o_invoke_few_args(s_putBack, RuntimeCoeffects::fixme(),
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

  void skipNoAdvance(size_t len) {
    DCHECK_LE(len, length());
    buffer_ptr += len;
    buffer_used -= len;
  }

  size_t length() noexcept {
    return buffer_used;
  }

  const uint8_t* data() noexcept {
    return reinterpret_cast<const uint8_t*>(buffer_ptr);
  }

  void pull(void* buf, size_t len) {
    if (LIKELY(len < buffer_used)) {
      // Fast path if we have enough data
      memcpy(buf, buffer_ptr, len);
      buffer_ptr += len;
      buffer_used -= len;
    } else {
      // Slow path at the end of the buffer
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
  }

  template<typename T>
  T readBE() {
    return folly::Endian::big(read<T>());
  }

  template<typename T>
  T readLE() {
    return folly::Endian::little(read<T>());
  }

private:
  template<typename T>
  T read() {
    T ret;
    pull(&ret, sizeof(T));
    return ret;
  }

  void refill(size_t len) {
    assertx(buffer_used == 0);
    len = std::max<size_t>(len, SIZE);
    buffer = m_transport->o_invoke_few_args(s_read, RuntimeCoeffects::fixme(),
                                            1, (int64_t)len).toString();
    buffer_used = buffer.size();
    buffer_ptr = buffer.data();
  }

  String buffer;
  const char* buffer_ptr{nullptr};
  size_t buffer_used{0};

  Object m_transport;
};

///////////////////////////////////////////////////////////////////////////////
}
