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

#include <runtime/base/variable_serializer.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/complex_types.h>
#include <util/exception.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/zend/zend_string.h>
#include <math.h>
#include <cmath>
#include <runtime/base/runtime_option.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/util/request_local.h>
#include <runtime/ext/ext_json.h>
#include <runtime/ext/ext_collection.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// static strings

static StaticString s_JsonSerializable("JsonSerializable");
static StaticString s_jsonSerialize("jsonSerialize");

///////////////////////////////////////////////////////////////////////////////

VariableSerializer::VariableSerializer(Type type, int option /* = 0 */,
                                       int maxRecur /* = 3 */)
  : m_type(type), m_option(option), m_buf(NULL), m_indent(0),
    m_valueCount(0), m_referenced(false), m_refCount(1), m_maxCount(maxRecur),
    m_levelDebugger(0) {
  m_maxLevelDebugger = g_context->getDebuggerPrintLevel();
  if (type == Serialize || type == APCSerialize || type == DebuggerSerialize) {
    m_arrayIds = new PointerCounterMap();
  } else {
    m_arrayIds = NULL;
  }
}

void VariableSerializer::setObjectInfo(CStrRef objClass, int objId,
                                       char objCode) {
  ASSERT(objCode == 'O' || objCode == 'V' || objCode == 'K');
  m_objClass = objClass;
  m_objId = objId;
  m_objCode = objCode;
}

void VariableSerializer::getResourceInfo(String &rsrcName, int &rsrcId) {
  rsrcName = m_rsrcName;
  rsrcId = m_rsrcId;
}

void VariableSerializer::setResourceInfo(CStrRef rsrcName, int rsrcId) {
  m_rsrcName = rsrcName;
  m_rsrcId = rsrcId;
}

String VariableSerializer::serialize(CVarRef v, bool ret) {
  StringBuffer buf;
  m_buf = &buf;
  if (ret) {
    buf.setOutputLimit(RuntimeOption::SerializationSizeLimit);
  } else {
    buf.setOutputLimit(StringData::MaxSize);
  }
  m_valueCount = 1;
  write(v);
  if (ret) {
    return m_buf->detach();
  } else {
    String str = m_buf->detach();
    g_context->write(str);
  }
  return null_string;
}

String VariableSerializer::serializeValue(CVarRef v, bool limit) {
  StringBuffer buf;
  m_buf = &buf;
  if (limit) {
    buf.setOutputLimit(RuntimeOption::SerializationSizeLimit);
  }
  m_valueCount = 1;
  write(v);
  return m_buf->detach();
}

String VariableSerializer::serializeWithLimit(CVarRef v, int limit) {
  if (m_type == Serialize || m_type == JSON || m_type == APCSerialize ||
      m_type == DebuggerSerialize) {
    ASSERT(false);
    return null_string;
  }
  StringBuffer buf;
  m_buf = &buf;
  if (RuntimeOption::SerializationSizeLimit > 0 &&
      (limit <= 0 || limit > RuntimeOption::SerializationSizeLimit)) {
    limit = RuntimeOption::SerializationSizeLimit;
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
  case PrintR:
    if (v) m_buf->append(1);
    break;
  case VarExport:
  case PHPOutput:
  case JSON:
  case DebuggerDump:
    m_buf->append(v ? "true" : "false");
    break;
  case VarDump:
  case DebugDump:
    indent();
    m_buf->append(v ? "bool(true)" : "bool(false)");
    writeRefCount();
    m_buf->append('\n');
    break;
  case Serialize:
  case APCSerialize:
  case DebuggerSerialize:
    m_buf->append(v ? "b:1;" : "b:0;");
    break;
  default:
    ASSERT(false);
    break;
  }
}

void VariableSerializer::write(int64 v) {
  switch (m_type) {
  case PrintR:
  case VarExport:
  case PHPOutput:
  case JSON:
  case DebuggerDump:
    m_buf->append(v);
    break;
  case VarDump:
    indent();
    m_buf->append("int(");
    m_buf->append(v);
    m_buf->append(")\n");
    break;
  case DebugDump:
    indent();
    m_buf->append("long(");
    m_buf->append(v);
    m_buf->append(')');
    writeRefCount();
    m_buf->append('\n');
    break;
  case Serialize:
  case APCSerialize:
  case DebuggerSerialize:
    m_buf->append("i:");
    m_buf->append(v);
    m_buf->append(';');
    break;
  default:
    ASSERT(false);
    break;
  }
}

void VariableSerializer::write(double v) {
  switch (m_type) {
  case JSON:
    if (!std::isinf(v) && !std::isnan(v)) {
      char *buf;
      if (v == 0.0) v = 0.0; // so to avoid "-0" output
      vspprintf(&buf, 0, "%.*k", 14, v);
      m_buf->append(buf);
      free(buf);
    } else {
      // PHP issues a warning: double INF/NAN does not conform to the
      // JSON spec, encoded as 0.
      m_buf->append('0');
    }
    break;
  case VarExport:
  case PHPOutput:
  case PrintR:
  case DebuggerDump:
    {
      char *buf;
      if (v == 0.0) v = 0.0; // so to avoid "-0" output
      bool isExport = m_type == VarExport || m_type == PHPOutput;
      vspprintf(&buf, 0, isExport ? "%.*H" : "%.*G", 14, v);
      m_buf->append(buf);
      free(buf);
    }
    break;
  case VarDump:
  case DebugDump:
    {
      char *buf;
      if (v == 0.0) v = 0.0; // so to avoid "-0" output
      vspprintf(&buf, 0, "float(%.*G)", 14, v);
      indent();
      m_buf->append(buf);
      free(buf);
      writeRefCount();
      m_buf->append('\n');
    }
    break;
  case Serialize:
  case APCSerialize:
  case DebuggerSerialize:
    m_buf->append("d:");
    if (std::isnan(v)) {
      m_buf->append("NAN");
    } else if (std::isinf(v)) {
      if (v < 0) m_buf->append('-');
      m_buf->append("INF");
    } else {
      char *buf;
      if (v == 0.0) v = 0.0; // so to avoid "-0" output
      vspprintf(&buf, 0, "%.*H", 14, v);
      m_buf->append(buf);
      free(buf);
    }
    m_buf->append(';');
    break;
  default:
    ASSERT(false);
    break;
  }
}

void VariableSerializer::write(const char *v, int len /* = -1 */,
                               bool isArrayKey /* = false */) {
  if (v == NULL) v = "";
  if (len < 0) len = strlen(v);

  switch (m_type) {
  case PrintR: {
    m_buf->append(v, len);
    break;
  }
  case DebuggerDump:
  case VarExport: {
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
  case VarDump:
  case DebugDump: {
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
  case Serialize:
  case APCSerialize:
  case DebuggerSerialize:
    m_buf->append("s:");
    m_buf->append(len);
    m_buf->append(":\"");
    m_buf->append(v, len);
    m_buf->append("\";");
    break;
  case JSON: {
    if (m_option & k_JSON_NUMERIC_CHECK) {
      int64 lval; double dval;
      switch (is_numeric_string(v, len, &lval, &dval, 0)) {
        case KindOfInt64:
          write(lval);
          return;
        case KindOfDouble:
          write(dval);
          return;
        default:
          break;
      }
    }

    m_buf->appendJsonEscape(v, len, m_option);
    break;
  }
  case PHPOutput: {
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
    m_buf->append('"');
    break;
  }
  default:
    ASSERT(false);
    break;
  }
}

void VariableSerializer::write(CStrRef v) {
  if (m_type == APCSerialize && !v.isNull() && v->isStatic()) {
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

void VariableSerializer::write(CObjRef v) {
  if (!v.isNull() && m_type == JSON) {

    if (v.instanceof(s_JsonSerializable)) {
      ASSERT(!v->isCollection());
      Variant ret = v->o_invoke(s_jsonSerialize, null_array, -1);
      // for non objects or when $this is returned
      if (!ret.isObject() || (ret.isObject() && !ret.same(v))) {
        write(ret);
        return;
      }
    }
    if (incNestedLevel(v.get(), true)) {
      writeOverflow(v.get(), true);
    } else {
      if (v->isCollection()) {
        collectionSerialize(v.get(), this);
      } else {
        Array props(ArrayData::Create());
        ClassInfo::GetArray(v.get(), v->o_getClassPropTable(), props,
                            ClassInfo::GetArrayPublic);
        setObjectInfo(v->o_getClassName(), v->o_getId(), 'O');
        props.serialize(this);
      }
    }
    decNestedLevel(v.get());
  } else {
    v.serialize(this);
  }
}

void VariableSerializer::write(CVarRef v, bool isArrayKey /* = false */) {
  setReferenced(v.isReferenced());
  setRefCount(v.getRefCount());
  if (!isArrayKey && v.isObject()) {
    write(v.toObject());
    return;
  }
  v.serialize(this, isArrayKey);
}

void VariableSerializer::writeNull() {
  switch (m_type) {
  case PrintR:
    // do nothing
    break;
  case VarExport:
  case PHPOutput:
    m_buf->append("NULL");
    break;
  case VarDump:
  case DebugDump:
    indent();
    m_buf->append("NULL");
    writeRefCount();
    m_buf->append('\n');
    break;
  case Serialize:
  case APCSerialize:
  case DebuggerSerialize:
    m_buf->append("N;");
    break;
  case JSON:
  case DebuggerDump:
    m_buf->append("null");
    break;
  default:
    ASSERT(false);
    break;
  }
}

void VariableSerializer::writeOverflow(void* ptr, bool isObject /* = false */) {
  bool wasRef = m_referenced;
  setReferenced(false);
  switch (m_type) {
  case PrintR:
    if (!m_objClass.empty()) {
      m_buf->append(m_objClass);
      m_buf->append(" Object\n");
    } else {
      m_buf->append("Array\n");
    }
    m_buf->append(" *RECURSION*");
    break;
  case VarExport:
  case PHPOutput:
    throw NestingLevelTooDeepException();
  case VarDump:
  case DebugDump:
  case DebuggerDump:
    indent();
    m_buf->append("*RECURSION*\n");
    break;
  case DebuggerSerialize:
    if (m_maxLevelDebugger > 0 && m_levelDebugger > m_maxLevelDebugger) {
      // Not recursion, just cut short of print
      m_buf->append("s:12:\"...(omitted)\";", 20);
      break;
    }
    // fall through
  case Serialize:
  case APCSerialize:
    {
      ASSERT(m_arrayIds);
      PointerCounterMap::const_iterator iter = m_arrayIds->find(ptr);
      ASSERT(iter != m_arrayIds->end());
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
  case JSON:
    raise_warning("json_encode(): recursion detected");
    m_buf->append("null");
    break;
  default:
    ASSERT(false);
    break;
  }
}

void VariableSerializer::writeRefCount() {
  if (m_type == DebugDump) {
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

  switch (m_type) {
  case PrintR:
    if (!m_rsrcName.empty()) {
      m_buf->append("Resource id #");
      m_buf->append(m_rsrcId);
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
  case VarExport:
  case PHPOutput:
    if (m_indent > 0) {
      m_buf->append('\n');
      indent();
    }
    if (!m_objClass.empty()) {
      m_buf->append(m_objClass);
      m_buf->append("::__set_state(array(\n");
    } else {
      m_buf->append("array (\n");
    }
    m_indent += (info.indent_delta = 2);
    break;
  case VarDump:
  case DebugDump:
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
    if (m_type == VarDump) {
      m_buf->append(' ');
    } else {
      writeRefCount();
    }

    m_buf->append("{\n");
    m_indent += (info.indent_delta = 2);
    break;
  case Serialize:
  case APCSerialize:
  case DebuggerSerialize:
    if (!m_objClass.empty()) {
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
  case JSON:
  case DebuggerDump:
    info.is_vector =
      (m_objClass.empty() || m_objCode == 'V' || m_objCode == 'K') &&
      isVectorData;
    if (info.is_vector && m_type == JSON) {
      info.is_vector = (m_option & k_JSON_FORCE_OBJECT)
                       ? false : info.is_vector;
    }

    if (info.is_vector) {
      m_buf->append('[');
    } else {
      m_buf->append('{');
    }

    if (m_type == JSON && m_option & k_JSON_PRETTY_PRINT) {
      m_indent += (info.indent_delta = 4);
    }

    break;
  default:
    ASSERT(false);
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

void VariableSerializer::writePropertyKey(CStrRef prop) {
  const char *key = prop.data();
  int kl = prop.size();
  if (!*key && kl) {
    const char *cls = key + 1;
    if (*cls == '*') {
      ASSERT(key[2] == 0);
      m_buf->append(key + 3, kl - 3);
      const char prot[] = "\":protected";
      int o = m_type == PrintR ? 1 : 0;
      m_buf->append(prot + o, sizeof(prot) - 1 - o);
    } else {
      int l = strlen(cls);
      m_buf->append(cls + l + 1, kl - l - 2);
      int o = m_type == PrintR ? 1 : 0;
      m_buf->append("\":\"" + o, 3 - 2*o);
      m_buf->append(cls, l);
      const char priv[] = "\":private";
      m_buf->append(priv + o, sizeof(priv) - 1 - o);
    }
  } else {
    m_buf->append(prop);
    if (m_type != PrintR) m_buf->append('"');
  }
}

/* key MUST be a non-reference string or int */
void VariableSerializer::writeArrayKey(Variant key) {
  Variant::TypedValueAccessor tva = key.getTypedAccessor();
  bool skey = Variant::IsString(tva);
  if (skey && m_type == APCSerialize) {
    write(Variant::GetAsString(tva));
    return;
  }
  ArrayInfo &info = m_arrayInfos.back();
  switch (m_type) {
  case PrintR: {
    indent();
    m_buf->append('[');
    if (info.is_object && skey) {
      writePropertyKey(Variant::GetAsString(tva));
    } else {
      m_buf->append(key);
    }
    m_buf->append("] => ");
    break;
  }
  case VarExport:
  case PHPOutput:
    indent();
    write(key, true);
    m_buf->append(" => ");
    break;
  case VarDump:
  case DebugDump:
    indent();
    m_buf->append('[');
    if (!skey) {
      m_buf->append(Variant::GetInt64(tva));
    } else {
      m_buf->append('"');
      if (info.is_object) {
        writePropertyKey(Variant::GetAsString(tva));
      } else {
        m_buf->append(Variant::GetAsString(tva));
        m_buf->append('"');
      }
    }
    m_buf->append("]=>\n");
    break;
  case APCSerialize:
    ASSERT(!info.is_object);
  case Serialize:
  case DebuggerSerialize:
    write(key);
    break;
  case JSON:
  case DebuggerDump:
    if (!info.first_element) {
      m_buf->append(',');
    }
    if (m_type == JSON && m_option & k_JSON_PRETTY_PRINT) {
      m_buf->append("\n");
      indent();
    }
    if (!info.is_vector) {
      if (skey) {
        CStrRef s = Variant::GetAsString(tva);
        const char *k = s.data();
        int len = s.size();
        if (info.is_object && !*k && len) {
          while (*++k) len--;
          k++;
          len -= 2;
        }
        write(k, len);
      } else {
        m_buf->append('"');
        m_buf->append(Variant::GetInt64(tva));
        m_buf->append('"');
      }
      m_buf->append(':');
      if (m_type == JSON && m_option & k_JSON_PRETTY_PRINT) {
        m_buf->append(' ');
      }
    }
    break;
  default:
    ASSERT(false);
    break;
  }
}

void VariableSerializer::writeArrayValue(CVarRef value) {
  // Do not count referenced values after the first
  if ((m_type == Serialize || m_type == APCSerialize ||
       m_type == DebuggerSerialize) &&
      !(value.isReferenced() &&
        m_arrayIds->find(value.getRefData()) != m_arrayIds->end())) {
    m_valueCount++;
  }

  write(value);
  switch (m_type) {
  case PrintR:
    m_buf->append('\n');
    break;
  case VarExport:
  case PHPOutput:
    m_buf->append(",\n");
    break;
  default:
    break;
  }

  ArrayInfo &info = m_arrayInfos.back();
  info.first_element = false;
}

void VariableSerializer::writeArrayFooter() {
  ArrayInfo &info = m_arrayInfos.back();

  m_indent -= info.indent_delta;
  switch (m_type) {
  case PrintR:
    if (m_rsrcName.empty()) {
      indent();
      m_buf->append(")\n");
      if (m_indent > 0) {
        m_indent -= 4;
      }
    }
    break;
  case VarExport:
  case PHPOutput:
    indent();
    if (info.is_object) {
      m_buf->append("))");
    } else {
      m_buf->append(')');
    }
    break;
  case VarDump:
  case DebugDump:
    if (m_rsrcName.empty()) {
      indent();
      m_buf->append("}\n");
    }
    break;
  case Serialize:
  case APCSerialize:
  case DebuggerSerialize:
    m_buf->append('}');
    break;
  case JSON:
  case DebuggerDump:
    if (m_type == JSON && m_option & k_JSON_PRETTY_PRINT) {
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
    ASSERT(false);
    break;
  }

  m_arrayInfos.pop_back();
}

void VariableSerializer::writeSerializableObject(CStrRef clsname,
                                                 CStrRef serialized) {
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
    if (m_indent > 0 && m_type == VarDump) m_buf->append('&');
    m_referenced = false;
  }
}

bool VariableSerializer::incNestedLevel(void *ptr,
                                        bool isObject /* = false */) {
  switch (m_type) {
  case VarExport:
  case PHPOutput:
  case PrintR:
  case VarDump:
  case DebugDump:
  case JSON:
  case DebuggerDump:
    return ++m_counts[ptr] >= m_maxCount;
  case DebuggerSerialize:
    if (m_maxLevelDebugger > 0 && ++m_levelDebugger > m_maxLevelDebugger) {
      return true;
    }
    // fall through
  case Serialize:
  case APCSerialize:
    {
      ASSERT(m_arrayIds);
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
    ASSERT(false);
    break;
  }
  return false;
}

void VariableSerializer::decNestedLevel(void *ptr) {
  --m_counts[ptr];
  if (m_type == DebuggerSerialize && m_maxLevelDebugger > 0) {
    --m_levelDebugger;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
