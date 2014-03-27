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

#ifndef incl_HPHP_THRIFT_BUFFER_H_
#define incl_HPHP_THRIFT_BUFFER_H_

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/variable-serializer.h"

#include <arpa/inet.h>
#if defined(__FreeBSD__)
# include <sys/endian.h>
# elif defined(__APPLE__)
# include <machine/endian.h>
# include <libkern/OSByteOrder.h>
#else
# include <byteswap.h>
#include <map>
#include <memory>
#include <utility>
#include <vector>
#endif

#if !defined(htonll) && !defined(ntohll)

#if __BYTE_ORDER == __LITTLE_ENDIAN
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
#define htonll(x) (x)
#define ntohll(x) (x)
#endif

#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Array;
class Object;
struct Variant;

/**
 * Efficient thrift input/output preparation. Used by automatically generated
 * separable extension code created by running thrift compiler, for example,
 *
 *   thrift --gen hphp my_service.thrift
 */
class ThriftBuffer {
public:
  enum TType {
    T_STOP   = 0,
    T_VOID   = 1,
    T_BOOL   = 2,
    T_BYTE   = 3,
    T_I08    = 3,
    T_DOUBLE = 4,
    T_I16    = 6,
    T_I32    = 8,
    T_U64    = 9,
    T_I64    = 10,
    T_STRING = 11,
    T_UTF7   = 11,
    T_STRUCT = 12,
    T_MAP    = 13,
    T_SET    = 14,
    T_LIST   = 15,
    T_UTF8   = 16,
    T_UTF16  = 17,
  };

  enum TMessageType {
    CALL  = 1,
    REPLY = 2,
    EXCEPTION = 3,
  };

public:
  /**
   * Constructing with some initial size, subsequent allocation will double
   * existing size every round.
   */
  explicit ThriftBuffer(
    int size,
    VariableSerializer::Type sType = VariableSerializer::Type::Serialize);
  ~ThriftBuffer();

  void flush(); // write bytes to transport
  void reset(bool read); // get ready for reads or writes

  // input has been called with htons() call
  void nwrite(int16_t data) {
    memcpy(m_p, &data, sizeof(data));
    if ((m_p += sizeof(data)) > m_pSafe) flush();
  }

  // input has not been called with hton() calls yet
  void write(bool data) {
    *m_p = (data ? 1 : 0);
    if (++m_p > m_pSafe) flush();
  }
  void write(int8_t data) {
    *m_p = data;
    if (++m_p > m_pSafe) flush();
  }
  void write(int16_t data) {
    data = htons(data);
    memcpy(m_p, &data, sizeof(data));
    if ((m_p += sizeof(data)) > m_pSafe) flush();
  }
  void write(int32_t data) {
    data = htonl(data);
    memcpy(m_p, &data, sizeof(data));
    if ((m_p += sizeof(data)) > m_pSafe) flush();
  }
  void write(int64_t data) {
    data = htonll(data);
    memcpy(m_p, &data, sizeof(data));
    if ((m_p += sizeof(data)) > m_pSafe) flush();
  }
  void write(double data) {
    union { int64_t c; double d;} a;
    a.d = data;
    write(a.c);
  }
  void write(const String& data);

  // reads
  void read(bool &data) {
    if (m_safe) {
      data = *m_p;
      if (++m_p > m_pSafe) m_safe = false;
    } else {
      read((char*)&data, (int)sizeof(data));
    }
  }
  void read(int8_t &data) {
    if (m_safe) {
      data = *m_p;
      if (++m_p > m_pSafe) m_safe = false;
    } else {
      read((char*)&data, (int)sizeof(data));
    }
  }
  void read(int16_t &data) {
    if (m_safe) {
      memcpy(&data, m_p, sizeof(data));
      if ((m_p += sizeof(data)) > m_pSafe) m_safe = false;
    } else {
      read((char*)&data, (int)sizeof(data));
    }
    data = ntohs(data);
  }
  void read(int32_t &data) {
    if (m_safe) {
      memcpy(&data, m_p, sizeof(data));
      if ((m_p += sizeof(data)) > m_pSafe) m_safe = false;
    } else {
      read((char*)&data, (int)sizeof(data));
    }
    data = ntohl(data);
  }
  void read(int64_t &data) {
    if (m_safe) {
      memcpy(&data, m_p, sizeof(data));
      if ((m_p += sizeof(data)) > m_pSafe) m_safe = false;
    } else {
      read((char*)&data, (int)sizeof(data));
    }
    data = ntohll(data);
  }
  void read(double &data) {
    union { int64_t c; double d;} a; a.d = data;
    read(a.c);
    data = a.d;
  }
  void read(String &data) {
    int32_t size;
    read(size);
    if (size > 0 && size + 1 > 0) {
      data = String(size, ReserveString);
      char *buf = data.bufferSlice().ptr;
      read(buf, size);
      data.setSize(size);
    } else if (size) {
      throwInvalidStringSize(size);
    }
  }

  void read(std::string &data);
  void write(const std::string &data);
  void write(const char *data) { write(std::string(data ? data : ""));}

  void read(std::vector<std::string> &data);
  void write(const std::vector<std::string> &data);

  template<typename T>
  void read(std::vector<T> &data) {
    int32_t size;
    read(size);
    data.resize(size);
    for (int i = 0; i < size; i++) {
      read(data[i]);
    }
  }
  template<typename T>
  void write(const std::vector<T> &data) {
    int32_t size = data.size();
    write(size);
    for (int i = 0; i < size; i++) {
      write(data[i]);
    }
  }
  template<typename T1, typename T2>
  void read(std::pair<T1, T2> &data) {
    read(data.first);
    read(data.second);
  }
  template<typename T1, typename T2>
  void write(const std::pair<T1, T2> &data) {
    write(data.first);
    write(data.second);
  }
  template<typename T1, typename T2>
  void read(std::map<T1, T2> &data) {
    int32_t size;
    read(size);
    for (int i = 0; i < size; i++) {
      std::pair<T1, T2> entry;
      read(entry);
      data.insert(entry);
    }
  }
  template<typename T1, typename T2>
  void write(const std::map<T1, T2> &data) {
    int32_t size = data.size();
    write(size);
    for (const std::pair<T1, T2> &entry : data) {
      write(entry);
    }
  }
  template<typename T>
  void read(std::shared_ptr<T> &data) {
    bool has;
    read(has);
    if (has) {
      data = std::shared_ptr<T>(new T());
      data->recvImpl(*this);
    }
  }
  template<typename T>
  void write(const std::shared_ptr<T> &data) {
    write((bool)data);
    if (data) {
      data->sendImpl(*this);
    }
  }

  void read(Array   &data);
  void read(Object  &data);
  void read(Variant &data);
  void write(const Array& data);
  void write(const Object& data);
  void write(const Variant& data);

  void skip(int8_t type);

protected:
  virtual String readImpl() = 0;
  virtual void flushImpl(const String& data) = 0;
  virtual void throwError(const char *msg, int code) = 0;

  int   m_size;

private:
  // disabling copy constructor and assignment
  ThriftBuffer(const ThriftBuffer &sb) { assert(false); }
  ThriftBuffer &operator=(const ThriftBuffer &sb) {
    assert(false);
    return *this;
  }

  char *m_p;
  char *m_pSafe;
  char *m_pEnd;
  bool  m_safe;

  char *m_buf;

  VariableSerializer::Type m_serializerType;

  void flush(const String& data);
  void read(char *data, int len);

  void throwOutOfMemory();
  void throwInvalidStringSize(int size);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_THRIFT_BUFFER_H_
