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

#include <runtime/base/util/thrift_buffer.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/variable_unserializer.h>
#include <util/logger.h>

#define INVALID_DATA 1

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

ThriftBuffer::ThriftBuffer(int size) : m_size(size), m_safe(false) {
  m_buf = (char *)malloc(m_size + 1);
  if (!m_buf) throwOutOfMemory();
  m_pEnd = m_buf + m_size;
  m_pSafe = m_pEnd - sizeof(int64) - 1;
  m_p = m_buf;
}

ThriftBuffer::~ThriftBuffer() {
  free(m_buf);
}

void ThriftBuffer::reset(bool read) {
  if (read) {
    m_pEnd = m_buf;
    m_safe = false;
  } else {
    m_pEnd = m_buf + m_size;
  }
  m_pSafe = m_pEnd - sizeof(int64) - 1;
  m_p = m_buf;
}

///////////////////////////////////////////////////////////////////////////////

void ThriftBuffer::write(CStrRef data) {
  int32 len = data.size();
  write(len);

  if (m_p + len > m_pEnd) {
    flush();
  }
  if (len > m_size) {
    flushImpl(data);
  } else {
    memcpy(m_p, data.data(), len);
    if ((m_p += len) > m_pSafe) flush();
  }
}

void ThriftBuffer::flush() {
  *m_p = '\0';
  String data(m_buf, m_p - m_buf, AttachLiteral);
  m_p = m_buf;
  flushImpl(data);
}

///////////////////////////////////////////////////////////////////////////////

void ThriftBuffer::read(char *data, int len) {
  int avail = m_pEnd - m_p;

  // still enough
  if (avail >= len) {
    if (data) memcpy(data, m_p, len);
    if ((m_p += len) > m_pSafe) m_safe = false;
    return;
  }

  if (data) memcpy(data, m_p, avail);
  len -= avail;
  data += avail;

  while (true) {
    String ret = readImpl();
    if (ret.empty()) {
      throwError("unable to read enough bytes",INVALID_DATA);
    }

    const char *rdata = ret.data();
    int rsize = ret.size();

    if (rsize >= len) {
      if (data) memcpy(data, rdata, len);
      rsize -= len;
      if (rsize) {
        memcpy(m_buf, rdata + len, rsize);
        m_pEnd = m_buf + rsize;
      } else {
        m_pEnd = m_buf;
      }
      m_pSafe = m_pEnd - sizeof(int64) - 1;
      m_p = m_buf;
      if (m_p > m_pSafe) m_safe = false;
      return; // done
    }

    if (data) memcpy(data, rdata, rsize);
    len -= rsize;
    data += rsize;
  }
}

void ThriftBuffer::skip(int8 type) {
  switch (type) {
    case T_STOP:
    case T_VOID:
      return;
    case T_STRUCT:
      while (true) {
        int8 ttype; read(ttype); // get field type
        if (ttype == T_STOP) break;
        read(NULL, 2); // skip field number, I16
        skip(ttype); // skip field payload
      }
      return;
    case T_BOOL:
    case T_BYTE:
      read(NULL, 1);
      return;
    case T_I16:
      read(NULL, 2);
      return;
    case T_I32:
      read(NULL, 4);
      return;
    case T_U64:
    case T_I64:
    case T_DOUBLE:
      read(NULL, 8);
      return;
    //case T_UTF7: // aliases T_STRING
    case T_UTF8:
    case T_UTF16:
    case T_STRING: {
      int32 len; read(len);
      read(NULL, len);
      } return;
    case T_MAP: {
      int8 keytype; read(keytype);
      int8 valtype; read(valtype);
      int32 size; read(size);
      for (int32 i = 0; i < size; ++i) {
        skip(keytype);
        skip(valtype);
      }
    } return;
    case T_LIST:
    case T_SET: {
      int8 valtype; read(valtype);
      int32 size; read(size);
      for (int32 i = 0; i < size; ++i) {
        skip(valtype);
      }
    } return;
  };

  char errbuf[128];
  sprintf(errbuf, "Unknown field type: %d", (int)type);
  throwError(errbuf, INVALID_DATA);
}

void ThriftBuffer::throwOutOfMemory() {
  throwError("out of memory", 0);
}

void ThriftBuffer::throwInvalidStringSize(int size) {
  char errbuf[128];
  sprintf(errbuf, "Negative string size: %d", (int)size);
  throwError(errbuf, INVALID_DATA);
}

///////////////////////////////////////////////////////////////////////////////

static Variant unserialize_with_no_notice(CStrRef str) {
  VariableUnserializer vu(str.data(), str.data() + str.size(),
      VariableUnserializer::Serialize, true);
  Variant v;
  try {
    v = vu.unserialize();
  } catch (Exception &e) {
    Logger::Error("unserialize(): %s", e.getMessage().c_str());
  }
  return v;
}

void ThriftBuffer::read(std::string &data) {
  String sdata;
  read(sdata);
  data = std::string(sdata.data(), sdata.size());
}

void ThriftBuffer::write(const std::string &data) {
  write(String(data.data(), data.size(), AttachLiteral));
}

void ThriftBuffer::read(std::vector<std::string> &data) {
  int32 size;
  read(size);
  data.resize(size);
  for (int i = 0; i < size; i++) {
    read(data[i]);
  }
}

void ThriftBuffer::write(const std::vector<std::string> &data) {
  int32 size = data.size();
  write(size);
  for (int i = 0; i < size; i++) {
    write(data[i]);
  }
}

void ThriftBuffer::read(Array &data) {
  String sdata;
  read(sdata);
  data = unserialize_with_no_notice(sdata).toArray();
}

void ThriftBuffer::write(CArrRef data) {
  String sdata = f_serialize(data);
  write(sdata);
}

void ThriftBuffer::read(Object &data) {
  String sdata;
  read(sdata);
  data = unserialize_with_no_notice(sdata).toObject();
}

void ThriftBuffer::write(CObjRef data) {
  String sdata = f_serialize(data);
  write(sdata);
}

void ThriftBuffer::read(Variant &data) {
  String sdata;
  read(sdata);
  data = unserialize_with_no_notice(sdata);
}

void ThriftBuffer::write(CVarRef data) {
  String sdata = f_serialize(data);
  write(sdata);
}

///////////////////////////////////////////////////////////////////////////////
}
