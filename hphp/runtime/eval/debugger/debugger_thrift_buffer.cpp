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

#include <runtime/eval/debugger/debugger_thrift_buffer.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/variable_unserializer.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

String DebuggerThriftBuffer::readImpl() {
  assert(m_size <= BUFFER_SIZE);
  int nread = m_socket->readImpl(m_buffer, m_size);
  m_buffer[nread] = '\0';
  return String(m_buffer, nread, AttachLiteral);
}

void DebuggerThriftBuffer::flushImpl(CStrRef data) {
  m_socket->write(data);
}

void DebuggerThriftBuffer::throwError(const char *msg, int code) {
  throw Exception("Protocol Error (%d): %s", code, msg);
}

///////////////////////////////////////////////////////////////////////////////

static StaticString s_hit_limit(LITSTR_INIT("Hit serialization limit"));
static StaticString s_unknown_exp(LITSTR_INIT("Hit unknown exception"));
static StaticString s_type_mismatch(LITSTR_INIT("Type mismatch"));

template<typename T>
static inline int serializeImpl(T data, String& sdata) {
  VariableSerializer vs(VariableSerializer::DebuggerSerialize);
  try {
    sdata = vs.serialize(data, true);
  } catch (StringBufferLimitException &e) {
    sdata = s_hit_limit;
    return DebuggerWireHelpers::HitLimit;
  } catch (...) {
    sdata = s_unknown_exp;
    return DebuggerWireHelpers::UnknownError;
  }
  return DebuggerWireHelpers::NoError;
}

static inline int unserializeImpl(CStrRef sdata, Variant& data) {
  if (sdata.same(s_hit_limit)) {
    return DebuggerWireHelpers::HitLimit;
  }
  if (sdata.same(s_unknown_exp)) {
    return DebuggerWireHelpers::UnknownError;
  }
  VariableUnserializer vu(sdata.data(), sdata.size(),
                          VariableUnserializer::Serialize, true);
  try {
    data = vu.unserialize();
  } catch (Exception &e) {
    data = null_variant;
    return DebuggerWireHelpers::UnknownError;
  }
  return DebuggerWireHelpers::NoError;
}

int DebuggerWireHelpers::WireSerialize(CArrRef data, String& sdata) {
  return serializeImpl(data, sdata);
}

int DebuggerWireHelpers::WireSerialize(CObjRef data, String& sdata) {
  return serializeImpl(data, sdata);
}

int DebuggerWireHelpers::WireSerialize(CVarRef data, String& sdata) {
  return serializeImpl(data, sdata);
}

int DebuggerWireHelpers::WireUnserialize(String& sdata, Array& data) {
  Variant v;
  int ret = unserializeImpl(sdata, v);
  if (ret != NoError) {
    return ret;
  }
  if (!v.isArray() && !v.isNull()) {
    sdata = s_type_mismatch;
    return TypeMismatch;
  }
  data = v;
  return NoError;
}

int DebuggerWireHelpers::WireUnserialize(String& sdata, Object& data) {
  Variant v;
  int ret = unserializeImpl(sdata, v);
  if (ret != NoError) {
    return ret;
  }
  if (!v.isObject() && !v.isNull()) {
    sdata = s_type_mismatch;
    return TypeMismatch;
  }
  data = v;
  return NoError;
}

int DebuggerWireHelpers::WireUnserialize(String& sdata, Variant& data) {
  return unserializeImpl(sdata, data);
}

///////////////////////////////////////////////////////////////////////////////
}
