/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/base-includes.h"
#include "hphp/util/logger.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#if defined(__FreeBSD__)
# include <sys/endian.h>
#elif defined(__APPLE__)
# include <machine/endian.h>
# include <libkern/OSByteOrder.h>
#else
# include <endian.h>
# include <byteswap.h>
#endif
#include <stdexcept>

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define htolell(x) (x)
# define letohll(x) (x)
# if defined(__FreeBSD__)
#  define htonll(x) bswap64(x)
#  define ntohll(x) bswap64(x)
# elif defined(__APPLE__)
#  define htonll(x) OSSwapInt64(x)
#  define ntohll(x) OSSwapInt64(x)
# else
#  define htonll(x) bswap_64(x)
#  define ntohll(x) bswap_64(x)
# endif
#else
# if defined(__FreeBSD__)
#  define htolell(x) bswap64(x)
#  define letohll(x) bswap64(x)
# elif defined(__APPLE__)
#  define htolell(x) OSSwapInt64(x)
#  define letohll(x) OSSwapInt64(x)
# else
#  define htolell(x) bswap_64(x)
#  define letohll(x) bswap_64(x)
# endif
# define htonll(x) (x)
# define ntohll(x) (x)
#endif

namespace HPHP {
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


class PHPTransport {
public:
  static StaticString s_getTransport;
  static StaticString s_flush;
  static StaticString s_write;
  static StaticString s_putBack;
  static StaticString s_read;
  static StaticString s_class;
  static StaticString s_key;
  static StaticString s_val;
  static StaticString s_elem;
  static StaticString s_var;
  static StaticString s_type;
  static StaticString s_ktype;
  static StaticString s_vtype;
  static StaticString s_etype;
  static StaticString s_format;
  static StaticString s_collection;

public:
  Object protocol() { return p; }
  Object transport() { return t; }
protected:
  PHPTransport() {}

  void construct_with_zval(const Object& _p, size_t _buffer_size) {
    buffer = reinterpret_cast<char*>(malloc(_buffer_size));
    buffer_ptr = buffer;
    buffer_used = 0;
    buffer_size = _buffer_size;
    p = _p;
    t = p->o_invoke_few_args(s_getTransport, 0).toObject();
  }
  ~PHPTransport() {
    free(buffer);
  }

  char* buffer;
  char* buffer_ptr;
  size_t buffer_used;
  size_t buffer_size;

  Object p;
  Object t;
};


class PHPOutputTransport : public PHPTransport {
public:
  explicit PHPOutputTransport(const Object& _p, size_t _buffer_size = 8192) {
    construct_with_zval(_p, _buffer_size);
  }

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
    if ((len + buffer_used) > buffer_size) {
      writeBufferToTransport();
    }
    if (len > buffer_size) {
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

protected:
  void directFlush() {
    t->o_invoke_few_args(s_flush, 0);
  }
  void directWrite(const char* data, size_t len) {
    t->o_invoke_few_args(s_write, 1, String(data, len, CopyString));
  }
};

class PHPInputTransport : public PHPTransport {
public:
  explicit PHPInputTransport(Object _p, size_t _buffer_size = 8192) {
    construct_with_zval(_p, _buffer_size);
  }

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
      t->o_invoke_few_args(s_putBack,
                           1, String(buffer_ptr, buffer_used, CopyString));
    }
    buffer_used = 0;
    buffer_ptr = buffer;
  }

  void skip(size_t len) {
    while (len) {
      size_t chunk_size = len < buffer_used ? len : buffer_used;
      if (chunk_size) {
        buffer_ptr = reinterpret_cast<char*>(buffer_ptr) + chunk_size;
        buffer_used -= chunk_size;
        len -= chunk_size;
      }
      if (! len) break;
      refill();
    }
  }

  void readBytes(void* buf, size_t len) {
    while (len) {
      size_t chunk_size = len < buffer_used ? len : buffer_used;
      if (chunk_size) {
        memcpy(buf, buffer_ptr, chunk_size);
        buffer_ptr = reinterpret_cast<char*>(buffer_ptr) + chunk_size;
        buffer_used -= chunk_size;
        buf = reinterpret_cast<char*>(buf) + chunk_size;
        len -= chunk_size;
      }
      if (! len) break;
      refill();
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

protected:
  void refill() {
    assert(buffer_used == 0);
    String ret = t->o_invoke_few_args(s_read, 1, (int64_t)buffer_size);
    buffer_used = ret.size();
    memcpy(buffer, ret.data(), buffer_used);
    buffer_ptr = buffer;
  }

};

///////////////////////////////////////////////////////////////////////////////
}

#endif
