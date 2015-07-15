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

#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/util/exception.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"
#include <cmath>
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/utf8-decode.h"
#include "hphp/runtime/ext/json/JSON_parser.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// static strings

const StaticString
  s_JsonSerializable("JsonSerializable"),
  s_jsonSerialize("jsonSerialize");

///////////////////////////////////////////////////////////////////////////////

VariableSerializer::VariableSerializer(Type type, int option /* = 0 */,
                                       int maxRecur /* = 3 */)
  : m_type(type)
  , m_option(option)
  , m_buf(nullptr)
  , m_indent(0)
  , m_valueCount(0)
  , m_referenced(false)
  , m_refCount(1)
  , m_objId(0)
  , m_objCode(0)
  , m_rsrcId(0)
  , m_maxCount(maxRecur)
  , m_levelDebugger(0)
  , m_currentDepth(0)
  , m_maxDepth(0)
{
  m_maxLevelDebugger = g_context->debuggerSettings.printLevel;
  if (type == Type::Serialize ||
      type == Type::APCSerialize ||
      type == Type::DebuggerSerialize) {
    m_arrayIds = new ReqPtrCtrMap();
  } else {
    m_arrayIds = nullptr;
  }
}

void VariableSerializer::pushObjectInfo(const String& objClass, int objId,
                                        char objCode) {
  assert(objCode == 'O' || objCode == 'V' || objCode == 'K');
  m_objectInfos.emplace_back(
    ObjectInfo { m_objClass, m_objId, m_objCode, m_rsrcName, m_rsrcId }
  );
  m_objClass = objClass;
  m_objId = objId;
  m_objCode = objCode;
  m_rsrcName.reset();
  m_rsrcId = 0;
}

void VariableSerializer::pushResourceInfo(const String& rsrcName, int rsrcId) {
  m_objectInfos.emplace_back(
    ObjectInfo { m_objClass, m_objId, m_objCode, m_rsrcName, m_rsrcId }
  );
  m_objClass.reset();
  m_objId = 0;
  m_objCode = 0;
  m_rsrcName = rsrcName;
  m_rsrcId = rsrcId;
}

void VariableSerializer::popObjectInfo() {
  ObjectInfo &info = m_objectInfos.back();
  m_objClass = info.objClass;
  m_objId = info.objId;
  m_objCode = info.objCode;
  m_rsrcName = info.rsrcName;
  m_rsrcId = info.rsrcId;
  m_objectInfos.pop_back();
}

__thread int64_t VariableSerializer::serializationSizeLimit =
  StringData::MaxSize;

void VariableSerializer::popResourceInfo() {
  popObjectInfo();
}

String VariableSerializer::serialize(const Variant& v, bool ret,
                                     bool keepCount /* = false */) {
  StringBuffer buf;
  m_buf = &buf;
  if (ret) {
    buf.setOutputLimit(serializationSizeLimit);
  } else {
    buf.setOutputLimit(StringData::MaxSize);
  }
  m_valueCount = keepCount ? m_valueCount + 1 : 1;
  write(v);
  if (ret) {
    return m_buf->detach();
  } else {
    String str = m_buf->detach();
    g_context->write(str);
  }
  return String();
}

String VariableSerializer::serializeValue(const Variant& v, bool limit) {
  StringBuffer buf;
  m_buf = &buf;
  if (limit) {
    buf.setOutputLimit(serializationSizeLimit);
  }
  m_valueCount = 1;
  write(v);
  return m_buf->detach();
}

String VariableSerializer::serializeWithLimit(const Variant& v, int limit) {
  if (m_type == Type::Serialize || m_type == Type::JSON ||
      m_type == Type::APCSerialize || m_type == Type::DebuggerSerialize) {
    assert(false);
    return String();
  }
  StringBuffer buf;
  m_buf = &buf;
  if (serializationSizeLimit > 0 &&
      (limit <= 0 || limit > serializationSizeLimit)) {
    limit = serializationSizeLimit;
  }
  buf.setOutputLimit(limit);
  //Does not need m_valueCount, which is only useful with the unsupported types
  try {
    write(v);
  } catch (StringBufferLimitException &e) {
    return e.m_result;
  }
  return m_buf->detach();
}

///////////////////////////////////////////////////////////////////////////////

void VariableSerializer::write(bool v) {
  switch (m_type) {
  case Type::PrintR:
    if (v) m_buf->append(1);
    break;
  case Type::VarExport:
  case Type::PHPOutput:
  case Type::JSON:
  case Type::DebuggerDump:
    m_buf->append(v ? "true" : "false");
    break;
  case Type::VarDump:
  case Type::DebugDump:
    indent();
    m_buf->append(v ? "bool(true)" : "bool(false)");
    writeRefCount();
    m_buf->append('\n');
    break;
  case Type::Serialize:
  case Type::APCSerialize:
  case Type::DebuggerSerialize:
    m_buf->append(v ? "b:1;" : "b:0;");
    break;
  default:
    assert(false);
    break;
  }
}

void VariableSerializer::write(int64_t v) {
  switch (m_type) {
  case Type::PrintR:
  case Type::VarExport:
  case Type::PHPOutput:
  case Type::JSON:
  case Type::DebuggerDump:
    m_buf->append(v);
    break;
  case Type::VarDump:
    indent();
    m_buf->append("int(");
    m_buf->append(v);
    m_buf->append(")\n");
    break;
  case Type::DebugDump:
    indent();
    m_buf->append("long(");
    m_buf->append(v);
    m_buf->append(')');
    writeRefCount();
    m_buf->append('\n');
    break;
  case Type::Serialize:
  case Type::APCSerialize:
  case Type::DebuggerSerialize:
    m_buf->append("i:");
    m_buf->append(v);
    m_buf->append(';');
    break;
  default:
    assert(false);
    break;
  }
}

void VariableSerializer::write(double v) {
  auto const precision = 14;
  auto const serde_precision = 17;

  switch (m_type) {
  case Type::JSON:
    if (!std::isinf(v) && !std::isnan(v)) {
      char *buf;
      vspprintf(&buf, 0, "%.*k", precision, v);
      m_buf->append(buf);
      free(buf);
    } else {
      if (std::isnan(v) || std::isinf(v)) {
        json_set_last_error_code(json_error_codes::JSON_ERROR_INF_OR_NAN);
      }

      m_buf->append('0');
    }
    break;
  case Type::VarExport:
  case Type::PHPOutput:
  case Type::PrintR:
  case Type::DebuggerDump:
    {
      char *buf;
      bool isExport = m_type == Type::VarExport || m_type == Type::PHPOutput;
      vspprintf(&buf, 0, isExport ? "%.*H" : "%.*G", precision, v);
      m_buf->append(buf);
      // In PHPOutput mode, we always want doubles to parse as
      // doubles, so make sure there's a decimal point.
      if (m_type == Type::PHPOutput && strpbrk(buf, ".E") == nullptr) {
        m_buf->append(".0");
      }
      free(buf);
    }
    break;
  case Type::VarDump:
  case Type::DebugDump:
    {
      char *buf;
      vspprintf(&buf, 0, "float(%.*G)", precision, v);
      indent();
      m_buf->append(buf);
      free(buf);
      writeRefCount();
      m_buf->append('\n');
    }
    break;
  case Type::Serialize:
  case Type::APCSerialize:
  case Type::DebuggerSerialize:
    m_buf->append("d:");
    if (std::isnan(v)) {
      m_buf->append("NAN");
    } else if (std::isinf(v)) {
      if (v < 0) m_buf->append('-');
      m_buf->append("INF");
    } else {
      char *buf;
      vspprintf(&buf, 0, "%.*H", serde_precision, v);
      m_buf->append(buf);
      free(buf);
    }
    m_buf->append(';');
    break;
  default:
    assert(false);
    break;
  }
}

uint16_t reverse16(uint16_t us) {
  return
    ((us & 0xf) << 12)       | (((us >> 4) & 0xf) << 8) |
    (((us >> 8) & 0xf) << 4) | ((us >> 12) & 0xf);
}

// Potentially need to escape all control characters (< 32) and also "\/<>&'@%
static const bool jsonNoEscape[128] = {
  false, false, false, false, false, false, false, false,
  false, false, false, false, false, false, false, false,
  false, false, false, false, false, false, false, false,
  false, false, false, false, false, false, false, false,
  true,  true,  false, true,  true,  false, false, false,
  true,  true,  true,  true,  true,  true,  true,  false,
  true,  true,  true,  true,  true,  true,  true,  true,
  true,  true,  true,  true,  false, true,  false, true,
  false, true,  true,  true,  true,  true,  true,  true,
  true,  true,  true,  true,  true,  true,  true,  true,
  true,  true,  true,  true,  true,  true,  true,  true,
  true,  true,  true,  true,  false, true,  true,  true,
  true,  true,  true,  true,  true,  true,  true,  true,
  true,  true,  true,  true,  true,  true,  true,  true,
  true,  true,  true,  true,  true,  true,  true,  true,
  true,  true,  true,  true,  true,  true,  true,  true,
};

static void appendJsonEscape(StringBuffer& sb,
                             const char *s,
                             int len,
                             int options) {
  if (len == 0) {
    sb.append("\"\"", 2);
    return;
  }

  static const char digits[] = "0123456789abcdef";

  auto const start = sb.size();
  sb.append('"');

  // Do a fast path for ASCII characters that don't need escaping
  int pos = 0;
  do {
    int c = s[pos];
    if (UNLIKELY((unsigned char)c >= 128 || !jsonNoEscape[c])) {
      goto utf8_decode;
    }
    sb.append((char)c);
    pos++;
  } while (pos < len);
  sb.append('"');
  return;

utf8_decode:
  UTF8To16Decoder decoder(s + pos, len - pos, options & k_JSON_FB_LOOSE);
  for (;;) {
    int c = options & k_JSON_UNESCAPED_UNICODE ? decoder.decodeAsUTF8()
                                               : decoder.decode();
    if (c == UTF8_END) {
      sb.append('"');
      break;
    }
    if (c == UTF8_ERROR) {
      json_set_last_error_code(json_error_codes::JSON_ERROR_UTF8);
      // discard the part that has been already decoded.
      sb.resize(start);
      sb.append("null", 4);
      break;
    }
    assert(c >= 0);
    unsigned short us = (unsigned short)c;
    switch (us) {
    case '"':
      if (options & k_JSON_HEX_QUOT) {
        sb.append("\\u0022", 6);
      } else {
        sb.append("\\\"", 2);
      }
      break;
    case '\\': sb.append("\\\\", 2); break;
    case '/':
      if (options & k_JSON_UNESCAPED_SLASHES) {
        sb.append('/');
      } else {
        sb.append("\\/", 2);
      }
      break;
    case '\b': sb.append("\\b", 2);  break;
    case '\f': sb.append("\\f", 2);  break;
    case '\n': sb.append("\\n", 2);  break;
    case '\r': sb.append("\\r", 2);  break;
    case '\t': sb.append("\\t", 2);  break;
    case '<':
      if (options & k_JSON_HEX_TAG || options & k_JSON_FB_EXTRA_ESCAPES) {
        sb.append("\\u003C", 6);
      } else {
        sb.append('<');
      }
      break;
    case '>':
      if (options & k_JSON_HEX_TAG) {
        sb.append("\\u003E", 6);
      } else {
        sb.append('>');
      }
      break;
    case '&':
      if (options & k_JSON_HEX_AMP) {
        sb.append("\\u0026", 6);
      } else {
        sb.append('&');
      }
      break;
    case '\'':
      if (options & k_JSON_HEX_APOS) {
        sb.append("\\u0027", 6);
      } else {
        sb.append('\'');
      }
      break;
    case '@':
      if (options & k_JSON_FB_EXTRA_ESCAPES) {
        sb.append("\\u0040", 6);
      } else {
        sb.append('@');
      }
      break;
    case '%':
      if (options & k_JSON_FB_EXTRA_ESCAPES) {
        sb.append("\\u0025", 6);
      } else {
        sb.append('%');
      }
      break;
    default:
      if (us >= ' ' &&
          ((options & k_JSON_UNESCAPED_UNICODE) || (us & 127) == us)) {
        sb.append((char)us);
      } else {
        sb.append("\\u", 2);
        us = reverse16(us);
        sb.append(digits[us & ((1 << 4) - 1)]); us >>= 4;
        sb.append(digits[us & ((1 << 4) - 1)]); us >>= 4;
        sb.append(digits[us & ((1 << 4) - 1)]); us >>= 4;
        sb.append(digits[us & ((1 << 4) - 1)]);
      }
      break;
    }
  }
}

void VariableSerializer::write(const char *v, int len /* = -1 */,
                               bool isArrayKey /* = false */,
                               bool noQuotes /* = false */) {
  if (v == nullptr) v = "";
  if (len < 0) len = strlen(v);

  switch (m_type) {
  case Type::PrintR: {
    m_buf->append(v, len);
    break;
  }
  case Type::VarExport: {
    m_buf->append('\'');
    const char *p = v;
    for (int i = 0; i < len; i++, p++) {
      const char c = *p;
      // adapted from Zend php_var_export and php_addcslashes
      if (c == '\0') {
        m_buf->append("' . \"\\0\" . '");
        continue;
      } else if (c == '\'' || c == '\\') {
        m_buf->append('\\');
      }
      m_buf->append(c);
    }
    m_buf->append('\'');
    break;
  }
  case Type::VarDump:
  case Type::DebugDump: {
    indent();
    m_buf->append("string(");
    m_buf->append(len);
    m_buf->append(") \"");
    m_buf->append(v, len);
    m_buf->append('"');
    writeRefCount();
    m_buf->append('\n');
    break;
  }
  case Type::Serialize:
  case Type::APCSerialize:
  case Type::DebuggerSerialize:
    m_buf->append("s:");
    m_buf->append(len);
    m_buf->append(":\"");
    m_buf->append(v, len);
    m_buf->append("\";");
    break;
  case Type::JSON: {
    if ((m_option & k_JSON_NUMERIC_CHECK) && !isArrayKey) {
      int64_t lval; double dval;
      auto dt = is_numeric_string(v, len, &lval, &dval, 0);
      if (IS_INT_TYPE(dt)) {
        write(lval);
        return;
      } else if (IS_DOUBLE_TYPE(dt)) {
        write(dval);
        return;
      }
    }
    appendJsonEscape(*m_buf, v, len, m_option);
    break;
  }
  case Type::DebuggerDump:
  case Type::PHPOutput: {
    if (!noQuotes)
      m_buf->append('"');
    for (int i = 0; i < len; ++i) {
      const unsigned char c = v[i];
      switch (c) {
        case '\n': m_buf->append("\\n"); break;
        case '\r': m_buf->append("\\r"); break;
        case '\t': m_buf->append("\\t"); break;
        case '\\': m_buf->append("\\\\"); break;
        case '$':  m_buf->append("\\$"); break;
        case '"':  m_buf->append("\\\""); break;
        default: {
          if (c >= ' ' && c <= '~') {
            // The range [' ', '~'] contains only printable characters
            // and we've already handled special cases above
            m_buf->append(c);
          } else {
            char buf[5];
            snprintf(buf, sizeof(buf), "\\%03o", c);
            m_buf->append(buf);
          }
        }
      }
    }
    if (!noQuotes)
      m_buf->append('"');
    break;
  }
  default:
    assert(false);
    break;
  }
}

void VariableSerializer::write(const String& v) {
  if (m_type == Type::APCSerialize && !v.isNull() && v.get()->isStatic()) {
    union {
      char buf[8];
      StringData *sd;
    } u;
    u.sd = v.get();
    m_buf->append("S:");
    m_buf->append(u.buf, 8);
    m_buf->append(';');
  } else {
    v.serialize(this);
  }
}

void VariableSerializer::write(const Object& v) {
  if (!v.isNull() && m_type == Type::JSON) {

    if (v.instanceof(s_JsonSerializable)) {
      assert(!v->isCollection());
      Variant ret = v->o_invoke_few_args(s_jsonSerialize, 0);
      // for non objects or when $this is not returned
      if (!ret.isObject() || (ret.isObject() && !same(ret, v))) {
        if (ret.isArray() || ret.isObject()) {
          preventOverflow(v, [&ret, this]() {
            write(ret);
          });
        } else {
          // Don't need to check for overflows if ret is of primitive type
          // because the depth does not change.
          write(ret);
        }
        return;
      }
    }
    preventOverflow(v, [&v, this]() {
      if (v->isCollection()) {
        collections::serialize(v.get(), this);
      } else if (v->instanceof(SystemLib::s_ClosureClass)) {
        // We serialize closures as "{}" in JSON mode to be compatible
        // with PHP. And issue a warning in HipHop syntax.
        if (RuntimeOption::EnableHipHopSyntax) {
          m_buf->append("null");
          json_set_last_error_code(
            json_error_codes::JSON_ERROR_UNSUPPORTED_TYPE);
          return;
        }
        m_buf->append("{}");
      } else {
        auto props = v->toArray(true);
        pushObjectInfo(v->getClassName(), v->getId(), 'O');
        props.serialize(this);
        popObjectInfo();
      }
    });
  } else {
    v.serialize(this);
  }
}

void VariableSerializer::preventOverflow(const Object& v,
                                         const std::function<void()>& func) {
  if (incNestedLevel(v.get(), true)) {
    writeOverflow(v.get(), true);
  } else {
    func();
  }
  decNestedLevel(v.get());
}

void VariableSerializer::write(const Variant& v, bool isArrayKey /* = false */) {
  setReferenced(v.isReferenced());
  if (m_type == Type::DebugDump) {
    setRefCount(v.getRefCount());
  }
  if (!isArrayKey && v.isObject()) {
    write(v.toObject());
    return;
  }
  serializeVariant(v, this, isArrayKey);
}

void VariableSerializer::writeNull() {
  switch (m_type) {
  case Type::PrintR:
    // do nothing
    break;
  case Type::VarExport:
  case Type::PHPOutput:
    m_buf->append("NULL");
    break;
  case Type::VarDump:
  case Type::DebugDump:
    indent();
    m_buf->append("NULL");
    writeRefCount();
    m_buf->append('\n');
    break;
  case Type::Serialize:
  case Type::APCSerialize:
  case Type::DebuggerSerialize:
    m_buf->append("N;");
    break;
  case Type::JSON:
  case Type::DebuggerDump:
    m_buf->append("null");
    break;
  default:
    assert(false);
    break;
  }
}

void VariableSerializer::writeOverflow(void* ptr, bool isObject /* = false */) {
  bool wasRef = m_referenced;
  setReferenced(false);
  switch (m_type) {
  case Type::PrintR:
    if (!m_objClass.empty()) {
      m_buf->append(m_objClass);
      m_buf->append(" Object\n");
    } else {
      m_buf->append("Array\n");
    }
    m_buf->append(" *RECURSION*");
    break;
  case Type::VarExport:
  case Type::PHPOutput:
    throw ExtendedException("Nesting level too deep - recursive dependency?");
  case Type::VarDump:
  case Type::DebugDump:
  case Type::DebuggerDump:
    indent();
    m_buf->append("*RECURSION*\n");
    break;
  case Type::DebuggerSerialize:
    if (m_maxLevelDebugger > 0 && m_levelDebugger > m_maxLevelDebugger) {
      // Not recursion, just cut short of print
      m_buf->append("s:12:\"...(omitted)\";", 20);
      break;
    }
    // fall through
  case Type::Serialize:
  case Type::APCSerialize:
    {
      assert(m_arrayIds);
      ReqPtrCtrMap::const_iterator iter = m_arrayIds->find(ptr);
      assert(iter != m_arrayIds->end());
      int id = iter->second;
      if (isObject) {
        m_buf->append("r:");
        m_buf->append(id);
        m_buf->append(';');
      } else if (wasRef) {
        m_buf->append("R:");
        m_buf->append(id);
        m_buf->append(';');
      } else {
        m_buf->append("N;");
      }
    }
    break;
  case Type::JSON:
    json_set_last_error_code(json_error_codes::JSON_ERROR_RECURSION);
    m_buf->append("null");
    break;
  default:
    assert(false);
    break;
  }
}

void VariableSerializer::writeRefCount() {
  if (m_type == Type::DebugDump) {
    m_buf->append(" refcount(");
    m_buf->append(m_refCount);
    m_buf->append(')');
    m_refCount = 1;
  }
}

void VariableSerializer::writeArrayHeader(int size, bool isVectorData) {
  m_arrayInfos.push_back(ArrayInfo());
  ArrayInfo &info = m_arrayInfos.back();
  info.first_element = true;
  info.indent_delta = 0;
  info.size = size;

  switch (m_type) {
  case Type::DebuggerDump:
  case Type::PrintR:
    if (!m_rsrcName.empty()) {
      m_buf->append("Resource id #");
      m_buf->append(m_rsrcId);
      if (m_type == Type::DebuggerDump) {
        m_buf->append(" of type ");
        m_buf->append(m_rsrcName);
      }
      break;
    } else if (!m_objClass.empty()) {
      m_buf->append(m_objClass);
      m_buf->append(" Object\n");
    } else {
      m_buf->append("Array\n");
    }
    if (m_indent > 0) {
      m_indent += 4;
      indent();
    }
    m_buf->append("(\n");
    m_indent += (info.indent_delta = 4);
    break;
  case Type::VarExport:
  case Type::PHPOutput:
    if (m_indent > 0) {
      m_buf->append('\n');
      indent();
    }
    if (!m_objClass.empty()) {
      m_buf->append(m_objClass);
      if (m_objCode == 'O') {
        m_buf->append("::__set_state(array(\n");
      } else {
        assert(m_objCode == 'V' || m_objCode == 'K');
        m_buf->append(" {\n");
      }
    } else {
      m_buf->append("array (\n");
    }
    m_indent += (info.indent_delta = 2);
    break;
  case Type::VarDump:
  case Type::DebugDump:
    indent();
    if (!m_rsrcName.empty()) {
      m_buf->append("resource(");
      m_buf->append(m_rsrcId);
      m_buf->append(") of type (");
      m_buf->append(m_rsrcName);
      m_buf->append(")\n");
      break;
    } else if (!m_objClass.empty()) {
      m_buf->append("object(");
      m_buf->append(m_objClass);
      m_buf->append(")#");
      m_buf->append(m_objId);
      m_buf->append(' ');
    } else {
      m_buf->append("array");
    }
    m_buf->append('(');
    m_buf->append(size);
    m_buf->append(')');

    // ...so to strictly follow PHP's output
    if (m_type == Type::VarDump) {
      m_buf->append(' ');
    } else {
      writeRefCount();
    }

    m_buf->append("{\n");
    m_indent += (info.indent_delta = 2);
    break;
  case Type::Serialize:
  case Type::APCSerialize:
  case Type::DebuggerSerialize:
    if (!m_rsrcName.empty() && m_type == Type::DebuggerSerialize) {
      m_buf->append("L:");
      m_buf->append(m_rsrcId);
      m_buf->append(":");
      m_buf->append((int)m_rsrcName.size());
      m_buf->append(":\"");
      m_buf->append(m_rsrcName);
      m_buf->append("\"{");
    } else if (!m_objClass.empty()) {
      m_buf->append(m_objCode);
      m_buf->append(":");
      m_buf->append((int)m_objClass.size());
      m_buf->append(":\"");
      m_buf->append(m_objClass);
      m_buf->append("\":");
      m_buf->append(size);
      m_buf->append(":{");
    } else {
      m_buf->append("a:");
      m_buf->append(size);
      m_buf->append(":{");
    }
    break;
  case Type::JSON:
    info.is_vector =
      (m_objClass.empty() || m_objCode == 'V' || m_objCode == 'K') &&
      isVectorData;
    if (info.is_vector && m_type == Type::JSON) {
      info.is_vector = (m_option & k_JSON_FORCE_OBJECT)
                       ? false : info.is_vector;
    }

    if (info.is_vector) {
      m_buf->append('[');
    } else {
      m_buf->append('{');
    }

    if (m_type == Type::JSON && (m_option & k_JSON_PRETTY_PRINT) &&
        info.size > 0) {
      m_buf->append("\n");
      m_indent += (info.indent_delta = 4);
    }

    break;
  default:
    assert(false);
    break;
  }

  // ...so we don't mess up next array output
  if (!m_objClass.empty() || !m_rsrcName.empty()) {
    m_objClass.clear();
    info.is_object = true;
  } else {
    info.is_object = false;
  }
}

void VariableSerializer::writePropertyKey(const String& prop) {
  const char *key = prop.data();
  int kl = prop.size();
  if (!*key && kl) {
    const char *cls = key + 1;
    if (*cls == '*') {
      assert(key[2] == 0);
      m_buf->append(key + 3, kl - 3);
      const char prot[] = "\":protected";
      int o = m_type == Type::PrintR ? 1 : 0;
      m_buf->append(prot + o, sizeof(prot) - 1 - o);
    } else {
      int l = strlen(cls);
      m_buf->append(cls + l + 1, kl - l - 2);
      int o = m_type == Type::PrintR ? 1 : 0;
      m_buf->append(&"\":\""[o], 3 - 2*o);
      m_buf->append(cls, l);
      const char priv[] = "\":private";
      m_buf->append(priv + o, sizeof(priv) - 1 - o);
    }
  } else {
    m_buf->append(prop);
    if (m_type != Type::PrintR && m_type != Type::DebuggerDump) {
      m_buf->append('"');
    }
  }
}

/* key MUST be a non-reference string or int */
void VariableSerializer::writeArrayKey(const Variant& key) {
  auto const keyCell = tvAssertCell(key.asTypedValue());
  bool const skey = IS_STRING_TYPE(keyCell->m_type);

  ArrayInfo &info = m_arrayInfos.back();

  switch (m_type) {
  case Type::DebuggerDump:
  case Type::PrintR: {
    indent();
    m_buf->append('[');
    if (info.is_object && skey) {
      writePropertyKey(keyCell->m_data.pstr);
    } else {
      m_buf->append(key);
    }
    m_buf->append("] => ");
    break;
  }

  case Type::VarExport:
  case Type::PHPOutput:
    indent();
    write(key, true);
    m_buf->append(" => ");
    break;

  case Type::VarDump:
  case Type::DebugDump:
    indent();
    m_buf->append('[');
    if (!skey) {
      m_buf->append(keyCell->m_data.num);
    } else {
      m_buf->append('"');
      if (info.is_object) {
        writePropertyKey(keyCell->m_data.pstr);
      } else {
        m_buf->append(keyCell->m_data.pstr);
        m_buf->append('"');
      }
    }
    m_buf->append("]=>\n");
    break;

  case Type::APCSerialize:
    if (skey) {
      write(StrNR(keyCell->m_data.pstr).asString());
      return;
    }

  case Type::Serialize:
  case Type::DebuggerSerialize:
    write(key);
    break;

  case Type::JSON:
    if (!info.first_element) {
      m_buf->append(',');
    }
    if (UNLIKELY(m_option & k_JSON_PRETTY_PRINT)) {
      if (!info.first_element) {
        m_buf->append("\n");
      }
      indent();
    }
    if (!info.is_vector) {
      if (skey) {
        auto const sdata = keyCell->m_data.pstr;
        const char *k = sdata->data();
        int len = sdata->size();
        if (info.is_object && !*k && len) {
          while (*++k) len--;
          k++;
          len -= 2;
        }
        write(k, len, true);
      } else {
        m_buf->append('"');
        m_buf->append(keyCell->m_data.num);
        m_buf->append('"');
      }
      m_buf->append(':');
      if (UNLIKELY(m_option & k_JSON_PRETTY_PRINT)) {
        m_buf->append(' ');
      }
    }
    break;

  default:
    assert(false);
    break;
  }
}

void VariableSerializer::writeCollectionKey(const Variant& key) {
  if (m_type == Type::Serialize || m_type == Type::APCSerialize ||
      m_type == Type::DebuggerSerialize) {
    m_valueCount++;
  }
  writeArrayKey(key);
}

void VariableSerializer::writeCollectionKeylessPrefix() {
  switch (m_type) {
  case Type::PrintR:
  case Type::VarExport:
  case Type::PHPOutput:
  case Type::DebuggerDump:
    indent();
    break;
  case Type::VarDump:
  case Type::DebugDump:
  case Type::APCSerialize:
  case Type::Serialize:
  case Type::DebuggerSerialize:
    break;
  case Type::JSON: {
    ArrayInfo &info = m_arrayInfos.back();
    if (!info.first_element) {
      m_buf->append(',');
    }
    if (m_type == Type::JSON && m_option & k_JSON_PRETTY_PRINT) {
      if (!info.first_element) {
        m_buf->append("\n");
      }
      indent();
    }
    break;
  }
  default:
    assert(false);
    break;
  }
}

void VariableSerializer::writeArrayValue(const Variant& value) {
  switch (m_type) {
  case Type::Serialize:
  case Type::APCSerialize:
  case Type::DebuggerSerialize:
  // Do not count referenced values after the first
    if (!(value.isReferenced() &&
          m_arrayIds->find(value.getRefData()) != m_arrayIds->end())) {
      m_valueCount++;
    }
    write(value);
    break;

  case Type::DebuggerDump:
  case Type::PrintR:
    write(value);
    m_buf->append('\n');
    break;

  case Type::VarExport:
  case Type::PHPOutput:
    write(value);
    m_buf->append(",\n");
    break;

  default:
    write(value);
    break;
  }

  ArrayInfo &info = m_arrayInfos.back();
  info.first_element = false;
}

void VariableSerializer::writeArrayFooter() {
  ArrayInfo &info = m_arrayInfos.back();

  m_indent -= info.indent_delta;
  switch (m_type) {
  case Type::DebuggerDump:
  case Type::PrintR:
    if (m_rsrcName.empty()) {
      indent();
      m_buf->append(")\n");
      if (m_indent > 0) {
        m_indent -= 4;
      }
    }
    break;
  case Type::VarExport:
  case Type::PHPOutput:
    indent();
    if (info.is_object && m_objCode) {
      if (m_objCode == 'O') {
        m_buf->append("))");
      } else {
        assert(m_objCode == 'V' || m_objCode == 'K');
        m_buf->append("}");
      }
    } else {
      m_buf->append(')');
    }
    break;
  case Type::VarDump:
  case Type::DebugDump:
    if (m_rsrcName.empty()) {
      indent();
      m_buf->append("}\n");
    }
    break;
  case Type::Serialize:
  case Type::APCSerialize:
  case Type::DebuggerSerialize:
    m_buf->append('}');
    break;
  case Type::JSON:
    if (m_type == Type::JSON && (m_option & k_JSON_PRETTY_PRINT) &&
        info.size > 0) {
      m_buf->append("\n");
      indent();
    }
    if (info.is_vector) {
      m_buf->append(']');
    } else {
      m_buf->append('}');
    }
    break;
  default:
    assert(false);
    break;
  }

  m_arrayInfos.pop_back();
}

void VariableSerializer::writeSerializableObject(const String& clsname,
                                                 const String& serialized) {
  m_buf->append("C:");
  m_buf->append(clsname.size());
  m_buf->append(":\"");
  m_buf->append(clsname.data(), clsname.size());
  m_buf->append("\":");
  m_buf->append(serialized.size());
  m_buf->append(":{");
  m_buf->append(serialized.data(), serialized.size());
  m_buf->append('}');
}

///////////////////////////////////////////////////////////////////////////////

void VariableSerializer::indent() {
  for (int i = 0; i < m_indent; i++) {
    m_buf->append(' ');
  }
  if (m_referenced) {
    if (m_indent > 0 && m_type == Type::VarDump) m_buf->append('&');
    m_referenced = false;
  }
}

bool VariableSerializer::incNestedLevel(void *ptr,
                                        bool isObject /* = false */) {
  ++m_currentDepth;

  switch (m_type) {
  case Type::VarExport:
  case Type::PHPOutput:
  case Type::PrintR:
  case Type::VarDump:
  case Type::DebugDump:
  case Type::DebuggerDump:
    return ++m_counts[ptr] >= m_maxCount;
  case Type::JSON:
    if (m_currentDepth > m_maxDepth) {
      json_set_last_error_code(json_error_codes::JSON_ERROR_DEPTH);
    }

    return ++m_counts[ptr] >= m_maxCount;
  case Type::DebuggerSerialize:
    if (m_maxLevelDebugger > 0 && ++m_levelDebugger > m_maxLevelDebugger) {
      return true;
    }
    // fall through
  case Type::Serialize:
  case Type::APCSerialize:
    {
      assert(m_arrayIds);
      int ct = ++m_counts[ptr];
      if (m_arrayIds->find(ptr) != m_arrayIds->end() &&
          (m_referenced || isObject)) {
        return true;
      } else {
        (*m_arrayIds)[ptr] = m_valueCount;
      }
      return ct >= (m_maxCount - 1);
    }
    break;
  default:
    assert(false);
    break;
  }
  return false;
}

void VariableSerializer::decNestedLevel(void *ptr) {
  --m_currentDepth;
  --m_counts[ptr];
  if (m_type == Type::DebuggerSerialize && m_maxLevelDebugger > 0) {
    --m_levelDebugger;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
