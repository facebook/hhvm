/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const StaticString
  s_serializedNativeDataKey(std::string("\0native", 7));

const StaticString
  s_JsonSerializable("JsonSerializable"),
  s_jsonSerialize("jsonSerialize"),
  s_serialize("serialize"),
  s_zero("\0", 1),
  s_protected_prefix("\0*\0", 3),
  s_PHP_DebugDisplay("__PHP_DebugDisplay"),
  s_PHP_Incomplete_Class("__PHP_Incomplete_Class"),
  s_PHP_Incomplete_Class_Name("__PHP_Incomplete_Class_Name"),
  s_debugInfo("__debugInfo");

/*
 * Serialize a Variant recursively.
 * The last param noQuotes indicates to serializer to not put the output in
 * double quotes (used when printing the output of a __toDebugDisplay() of
 * an object when it is a string.
 */
static void serializeVariant(const Variant&,
                             VariableSerializer *serializer,
                             bool isArrayKey = false,
                             bool skipNestCheck = false,
                             bool noQuotes = false);
static void serializeObject(const Object&, VariableSerializer*);
static void serializeObject(const ObjectData*, VariableSerializer*);
static void serializeCollection(ObjectData* obj, VariableSerializer*);
static void serializeArray(const Array&, VariableSerializer*,
                           bool isObject = false);
static void serializeArray(const ArrayData*, VariableSerializer*,
                           bool skipNestCheck = false);
static void serializeResource(const ResourceData*, VariableSerializer*);
static void serializeString(const String&, VariableSerializer*);

NEVER_INLINE ATTRIBUTE_NORETURN
static void throwNestingException() {
  throw ExtendedException("Nesting level too deep - recursive dependency?");
}

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
  switch (type) {
    case Type::DebuggerSerialize:
       m_maxLevelDebugger = g_context->debuggerSettings.printLevel;
       // fall-through
    case Type::Serialize:
    case Type::APCSerialize:
       m_arrayIds = new ReqPtrCtrMap();;
       break;
    default:
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
      if (m_option & k_JSON_PRESERVE_ZERO_FRACTION
          && strchr(buf, '.') == nullptr) {
        m_buf->append(".0");
      }
      free(buf);
    } else {
      json_set_last_error_code(json_error_codes::JSON_ERROR_INF_OR_NAN);

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
      if (isIntType(dt)) {
        write(lval);
        return;
      } else if (isDoubleType(dt)) {
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
    serializeString(v, this);
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
        serializeCollection(v.get(), this);
      } else if (v->instanceof(c_Closure::classof())) {
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
        serializeArray(props, this);
        popObjectInfo();
      }
    });
  } else {
    serializeObject(v, this);
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

void VariableSerializer::write(const Variant& v, bool isArrayKey /*= false */) {
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
    throwNestingException();
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
    if (m_indent > 0 && m_rsrcName.empty()) {
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
    } else if (!m_rsrcName.empty()) {
      m_buf->append("NULL");
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
  bool const skey = isStringType(keyCell->m_type);

  ArrayInfo &info = m_arrayInfos.back();

  switch (m_type) {
  case Type::DebuggerDump:
  case Type::PrintR: {
    indent();
    m_buf->append('[');
    if (info.is_object && skey) {
      writePropertyKey(String{keyCell->m_data.pstr});
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
        writePropertyKey(String{keyCell->m_data.pstr});
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
    if (m_rsrcName.empty()) {
      indent();
    }
    if (info.is_object && m_objCode) {
      if (m_objCode == 'O') {
        m_buf->append("))");
      } else {
        assert(m_objCode == 'V' || m_objCode == 'K');
        m_buf->append("}");
      }
    } else if (m_rsrcName.empty()) { // for rsrc, only write NULL in arrayHeader
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

static void serializeRef(const TypedValue* tv,
                         VariableSerializer* serializer,
                         bool isArrayKey) {
  assert(tv->m_type == KindOfRef);
  // Ugly, but behavior is different for serialize
  if (serializer->getType() == VariableSerializer::Type::Serialize ||
      serializer->getType() == VariableSerializer::Type::APCSerialize ||
      serializer->getType() == VariableSerializer::Type::DebuggerSerialize) {
    if (serializer->incNestedLevel(tv->m_data.pref->var())) {
      serializer->writeOverflow(tv->m_data.pref->var());
    } else {
      // Tell the inner variant to skip the nesting check for data inside
      serializeVariant(*tv->m_data.pref->var(), serializer, isArrayKey, true);
    }
    serializer->decNestedLevel(tv->m_data.pref->var());
  } else {
    serializeVariant(*tv->m_data.pref->var(), serializer, isArrayKey);
  }
}

NEVER_INLINE static
void serializeVariant(const Variant& self, VariableSerializer *serializer,
                      bool isArrayKey /* = false */,
                      bool skipNestCheck /* = false */,
                      bool noQuotes /* = false */) {
  auto tv = self.asTypedValue();

  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
      assert(!isArrayKey);
      serializer->writeNull();
      return;

    case KindOfBoolean:
      assert(!isArrayKey);
      serializer->write(tv->m_data.num != 0);
      return;

    case KindOfInt64:
      serializer->write(tv->m_data.num);
      return;

    case KindOfDouble:
      serializer->write(tv->m_data.dbl);
      return;

    case KindOfPersistentString:
    case KindOfString:
      serializer->write(tv->m_data.pstr->data(),
                        tv->m_data.pstr->size(), isArrayKey, noQuotes);
      return;

    case KindOfPersistentArray:
    case KindOfArray:
      assert(!isArrayKey);
      serializeArray(tv->m_data.parr, serializer, skipNestCheck);
      return;

    case KindOfObject:
      assert(!isArrayKey);
      serializeObject(tv->m_data.pobj, serializer);
      return;

    case KindOfResource:
      assert(!isArrayKey);
      serializeResource(tv->m_data.pres->data(), serializer);
      return;

    case KindOfRef:
      serializeRef(tv, serializer, isArrayKey);
      return;

    case KindOfClass:
      break;
  }
  not_reached();
}

static void serializeResourceImpl(const ResourceData* res,
                                  VariableSerializer* serializer) {
  serializer->pushResourceInfo(res->o_getResourceName(), res->getId());
  serializeArray(empty_array(), serializer);
  serializer->popResourceInfo();
}

static void serializeResource(const ResourceData* res,
                              VariableSerializer* serializer) {
  if (UNLIKELY(serializer->incNestedLevel((void*)res, true))) {
    serializer->writeOverflow((void*)res, true);
  } else {
    serializeResourceImpl(res, serializer);
  }
  serializer->decNestedLevel((void*)res);
}

static void serializeString(const String& str, VariableSerializer* serializer) {
  if (str) {
    serializer->write(str.data(), str.size());
  } else {
    serializer->writeNull();
  }
}

static void serializeArrayImpl(const ArrayData* arr,
                               VariableSerializer* serializer) {
  serializer->writeArrayHeader(arr->size(), arr->isVectorData());
  for (ArrayIter iter(arr); iter; ++iter) {
    serializer->writeArrayKey(iter.first());
    serializer->writeArrayValue(iter.secondRef());
  }
  serializer->writeArrayFooter();
}

static void serializeArray(const ArrayData* arr, VariableSerializer* serializer,
                           bool skipNestCheck /* = false */) {
  if (arr->size() == 0) {
    serializer->writeArrayHeader(0, arr->isVectorData());
    serializer->writeArrayFooter();
    return;
  }
  if (!skipNestCheck) {
    if (serializer->incNestedLevel((void*)arr)) {
      serializer->writeOverflow((void*)arr);
    } else {
      serializeArrayImpl(arr, serializer);
    }
    serializer->decNestedLevel((void*)arr);
  } else {
    // If isObject, the array is temporary and we should not check or save
    // its pointer.
    serializeArrayImpl(arr, serializer);
  }
}

static void serializeArray(const Array& arr, VariableSerializer* serializer,
                           bool isObject /* = false */) {
  if (!arr.isNull()) {
    serializeArray(arr.get(), serializer, isObject);
  } else {
    serializer->writeNull();
  }
}

static
void serializeCollection(ObjectData* obj, VariableSerializer* serializer) {
  int64_t sz = collections::getSize(obj);
  auto type = obj->collectionType();

  if (isMapCollection(type)) {
    serializer->pushObjectInfo(obj->getClassName(), obj->getId(), 'K');
    serializer->writeArrayHeader(sz, false);
    for (ArrayIter iter(obj); iter; ++iter) {
      serializer->writeCollectionKey(iter.first());
      serializer->writeArrayValue(iter.second());
    }
    serializer->writeArrayFooter();

  } else {
    assertx(isVectorCollection(type) ||
            isSetCollection(type) ||
            (type == CollectionType::Pair));
    serializer->pushObjectInfo(obj->getClassName(), obj->getId(), 'V');
    serializer->writeArrayHeader(sz, true);
    auto ser_type = serializer->getType();
    if (ser_type == VariableSerializer::Type::Serialize ||
        ser_type == VariableSerializer::Type::APCSerialize ||
        ser_type == VariableSerializer::Type::DebuggerSerialize ||
        ser_type == VariableSerializer::Type::VarExport ||
        ser_type == VariableSerializer::Type::PHPOutput) {
      // For the 'V' serialization format, we don't print out keys
      // for Serialize, APCSerialize, DebuggerSerialize
      for (ArrayIter iter(obj); iter; ++iter) {
        serializer->writeCollectionKeylessPrefix();
        serializer->writeArrayValue(iter.second());
      }
    } else {
      if (isSetCollection(type)) {
        for (ArrayIter iter(obj); iter; ++iter) {
          serializer->writeCollectionKeylessPrefix();
          serializer->writeArrayValue(iter.second());
        }
      } else {
        for (ArrayIter iter(obj); iter; ++iter) {
          serializer->writeCollectionKey(iter.first());
          serializer->writeArrayValue(iter.second());
        }
      }
    }
    serializer->writeArrayFooter();
  }
  serializer->popObjectInfo();
}

/* Get properties from the actual object unless we're
 * serializing for var_dump()/print_r() and the object
 * exports a __debugInfo() magic method.
 * In which case, call that and use the array it returns.
 */
inline Array getSerializeProps(const ObjectData* obj,
                               VariableSerializer* serializer) {
  if (serializer->getType() == VariableSerializer::Type::VarExport) {
    Array props = Array::Create();
    for (ArrayIter iter(obj->toArray()); iter; ++iter) {
      auto key = iter.first().toString();
      // Jump over any class attribute mangling
      if (key[0] == '\0' && key.size() > 0) {
        int sizeToCut = 0;
        do {
          sizeToCut++;
        } while (key[sizeToCut] != '\0');
        key = key.substr(sizeToCut+1);
      }
      props.setWithRef(key, iter.secondRef());
    }
    return props;
  }
  if ((serializer->getType() != VariableSerializer::Type::PrintR) &&
      (serializer->getType() != VariableSerializer::Type::VarDump)) {
    return obj->toArray();
  }
  auto cls = obj->getVMClass();
  auto debuginfo = cls->lookupMethod(s_debugInfo.get());
  if (!debuginfo) {
    // When ArrayIterator is cast to an array, it returns its array object,
    // however when it's being var_dump'd or print_r'd, it shows its properties
    if (UNLIKELY(obj->instanceof(SystemLib::s_ArrayIteratorClass))) {
      auto ret = Array::Create();
      obj->o_getArray(ret);
      return ret;
    }

    // Same with Closure, since it's a dynamic object but still has its own
    // different behavior for var_dump and cast to array
    if (UNLIKELY(obj->instanceof(c_Closure::classof()))) {
      auto ret = Array::Create();
      obj->o_getArray(ret);
      return ret;
    }

    return obj->toArray();
  }
  if (debuginfo->attrs() & (AttrPrivate|AttrProtected|
                            AttrAbstract|AttrStatic)) {
    raise_warning("%s::__debugInfo() must be public and non-static",
                  cls->name()->data());
    return obj->toArray();
  }
  Variant ret = const_cast<ObjectData*>(obj)->o_invoke_few_args(s_debugInfo, 0);
  if (ret.isArray()) {
    return ret.toArray();
  }
  if (ret.isNull()) {
    return empty_array();
  }
  raise_error("__debugInfo() must return an array");
  not_reached();
}

static void serializeObjectImpl(const ObjectData* obj,
                                VariableSerializer* serializer) {
  bool handleSleep = false;
  Variant serializableNativeData = init_null();
  Variant ret;
  auto const type = serializer->getType();

  if (obj->isCollection()) {
    serializeCollection(const_cast<ObjectData*>(obj), serializer);
    return;
  }

  if (LIKELY(type == VariableSerializer::Type::Serialize ||
             type == VariableSerializer::Type::APCSerialize)) {
    if (obj->instanceof(SystemLib::s_SerializableClass)) {
      assert(!obj->isCollection());
      Variant ret =
        const_cast<ObjectData*>(obj)->o_invoke_few_args(s_serialize, 0);
      if (ret.isString()) {
        serializer->writeSerializableObject(obj->getClassName(),
                                            ret.toString());
      } else if (ret.isNull()) {
        serializer->writeNull();
      } else {
        raise_error("%s::serialize() must return a string or NULL",
                    obj->getClassName().data());
      }
      return;
    }
    // Only serialize CPP extension type instances which can actually
    // be deserialized.  Otherwise, raise a warning and serialize
    // null.
    // Similarly, do not try to serialize WaitHandles
    // as they contain internal state via non-NativeData means.
    auto cls = obj->getVMClass();
    if ((cls->instanceCtor() && !cls->isCppSerializable()) ||
        obj->getAttribute(ObjectData::IsWaitHandle)) {
      raise_warning("Attempted to serialize unserializable builtin class %s",
                    obj->getVMClass()->preClass()->name()->data());
      Variant placeholder = init_null();
      serializeVariant(placeholder, serializer);
      return;
    }
    if (obj->getAttribute(ObjectData::HasSleep)) {
      handleSleep = true;
      ret = const_cast<ObjectData*>(obj)->invokeSleep();
    }
    if (obj->getAttribute(ObjectData::HasNativeData)) {
      auto* ndi = cls->getNativeDataInfo();
      if (ndi->isSerializable()) {
        serializableNativeData = Native::nativeDataSleep(obj);
      }
    }
  } else if (UNLIKELY(type == VariableSerializer::Type::DebuggerSerialize)) {
    // Don't try to serialize a CPP extension class which doesn't
    // support serialization. Just send the class name instead.
    if (obj->getAttribute(ObjectData::IsCppBuiltin) &&
        !obj->getVMClass()->isCppSerializable()) {
      serializer->write(obj->getClassName());
      return;
    }
  }

  if (UNLIKELY(handleSleep)) {
    assert(!obj->isCollection());
    if (ret.isArray()) {
      Array wanted = Array::Create();
      assert(isArrayType(ret.getRawType())); // can't be KindOfRef
      const Array &props = ret.asCArrRef();
      for (ArrayIter iter(props); iter; ++iter) {
        String memberName = iter.second().toString();
        String propName = memberName;
        auto obj_cls = obj->getVMClass();
        Class* ctx = obj_cls;
        auto attrMask = AttrNone;
        if (memberName.data()[0] == 0) {
          int subLen = memberName.find('\0', 1) + 1;
          if (subLen > 2) {
            if (subLen == 3 && memberName.data()[1] == '*') {
              attrMask = AttrProtected;
              memberName = memberName.substr(subLen);
            } else {
              attrMask = AttrPrivate;
              String cls = memberName.substr(1, subLen - 2);
              ctx = Unit::lookupClass(cls.get());
              if (ctx) {
                memberName = memberName.substr(subLen);
              } else {
                ctx = obj_cls;
              }
            }
          }
        }

        auto const lookup = obj_cls->getDeclPropIndex(ctx, memberName.get());
        auto const propIdx = lookup.prop;

        if (propIdx != kInvalidSlot) {
          if (lookup.accessible) {
            auto const prop = &obj->propVec()[propIdx];
            if (prop->m_type != KindOfUninit) {
              auto const attrs = obj_cls->declProperties()[propIdx].attrs;
              if (attrs & AttrPrivate) {
                memberName = concat4(s_zero, ctx->nameStr(),
                                     s_zero, memberName);
              } else if (attrs & AttrProtected) {
                memberName = concat(s_protected_prefix, memberName);
              }
              if (!attrMask || (attrMask & attrs) == attrMask) {
                wanted.set(memberName, tvAsCVarRef(prop));
                continue;
              }
            }
          }
        }
        if (!attrMask &&
            UNLIKELY(obj->getAttribute(ObjectData::HasDynPropArr))) {
          const TypedValue* prop = obj->dynPropArray()->nvGet(propName.get());
          if (prop) {
            wanted.set(propName, tvAsCVarRef(prop));
            continue;
          }
        }
        raise_notice("serialize(): \"%s\" returned as member variable from "
                     "__sleep() but does not exist", propName.data());
        wanted.set(propName, init_null());
      }
      serializer->pushObjectInfo(obj->getClassName(), obj->getId(), 'O');
      if (!serializableNativeData.isNull()) {
        wanted.set(s_serializedNativeDataKey, serializableNativeData);
      }
      serializeArray(wanted, serializer, true);
      serializer->popObjectInfo();
    } else {
      raise_notice("serialize(): __sleep should return an array only "
                   "containing the names of instance-variables to "
                   "serialize");
      serializeVariant(uninit_null(), serializer);
    }
  } else {
    if (type == VariableSerializer::Type::VarExport &&
        obj->instanceof(c_Closure::classof())) {
      serializer->write(obj->getClassName());
    } else {
      auto className = obj->getClassName();
      Array properties = getSerializeProps(obj, serializer);
      if (type == VariableSerializer::Type::DebuggerSerialize) {
        try {
           auto val = const_cast<ObjectData*>(obj)->invokeToDebugDisplay();
           if (val.isInitialized()) {
             properties.lvalAt(s_PHP_DebugDisplay).assign(val);
           }
        } catch (...) {
          raise_warning("%s::__toDebugDisplay() throws exception",
                        obj->getClassName().data());
        }
      }
      if (type == VariableSerializer::Type::DebuggerDump) {
        // Expect to display as their stringified classname.
        if (obj->instanceof(c_Closure::classof())) {
          serializer->write(obj->getVMClass()->nameStr());
          return;
        }

        // If we have a DebugDisplay prop saved, use it.
        auto const debugDispVal = obj->o_realProp(s_PHP_DebugDisplay, 0);
        if (debugDispVal) {
          serializeVariant(*debugDispVal, serializer, false, false, true);
          return;
        }
        // Otherwise compute it if we have a __toDebugDisplay method.
        auto val = const_cast<ObjectData*>(obj)->invokeToDebugDisplay();
        if (val.isInitialized()) {
          serializeVariant(val, serializer, false, false, true);
          return;
        }
      }
      if (type != VariableSerializer::Type::VarDump &&
          className.asString() == s_PHP_Incomplete_Class) {
        auto const cname = obj->o_realProp(s_PHP_Incomplete_Class_Name, 0);
        if (cname && cname->isString()) {
          serializer->pushObjectInfo(cname->toCStrRef(), obj->getId(), 'O');
          properties.remove(s_PHP_Incomplete_Class_Name, true);
          serializeArray(properties, serializer, true);
          serializer->popObjectInfo();
          return;
        }
      }
      serializer->pushObjectInfo(className, obj->getId(), 'O');
      if (!serializableNativeData.isNull()) {
        properties.set(s_serializedNativeDataKey, serializableNativeData);
      }
      serializeArray(properties, serializer, true);
      serializer->popObjectInfo();
    }
  }
}

static
void serializeObject(const ObjectData* obj, VariableSerializer* serializer) {
  if (UNLIKELY(serializer->incNestedLevel((void*)obj, true))) {
    serializer->writeOverflow((void*)obj, true);
  } else {
    serializeObjectImpl(obj, serializer);
  }
  serializer->decNestedLevel((void*)obj);
}

static
void serializeObject(const Object& obj, VariableSerializer* serializer) {
  if (obj) {
    serializeObject(obj.get(), serializer);
  } else {
    serializer->writeNull();
  }
}

}
