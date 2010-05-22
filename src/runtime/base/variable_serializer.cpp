/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <runtime/base/class_info.h>
#include <math.h>
#include <runtime/base/runtime_option.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

VariableSerializer::VariableSerializer(Type type, int option /* = 0 */)
  : m_type(type), m_option(option), m_out(NULL), m_indent(0),
    m_valueCount(0), m_referenced(false), m_refCount(1), m_maxCount(3),
    m_outputLimit(0) {
}

void VariableSerializer::setObjectInfo(const char *objClass, int objId) {
  m_objClass = objClass;
  m_objId = objId;
}

void VariableSerializer::getResourceInfo(std::string &rsrcName, int &rsrcId) {
  rsrcName = m_rsrcName;
  rsrcId = m_rsrcId;
}

void VariableSerializer::setResourceInfo(const char *rsrcName, int rsrcId) {
  m_rsrcName = rsrcName;
  m_rsrcId = rsrcId;
}

Variant VariableSerializer::serialize(CVarRef v, bool ret) {
  std::ostringstream oss;
  if (ret) {
    m_out = &oss;
    m_outputLimit = RuntimeOption::SerializationSizeLimit;
  } else {
    m_out = &g_context->out();
  }
  m_initPos = m_out->tellp();
  m_valueCount = 1;
  if (m_type == VarDump && v.isContagious()) *m_out << '&';
  write(v);
  if (ret) {
    return String(oss.str()); // TODO: fix this one extra string copy here
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void VariableSerializer::write(bool v) {
  switch (m_type) {
  case PrintR:
    if (v) *m_out << 1;
    break;
  case VarExport:
  case JSON:
    *m_out << (v ? "true" : "false");
    break;
  case VarDump:
  case DebugDump:
    indent();
    *m_out << (v ? "bool(true)" : "bool(false)");
    writeRefCount();
    *m_out << '\n';
    break;
  case Serialize:
    *m_out << "b:" << (v ? 1 : 0) << ';';
    break;
  default:
    ASSERT(false);
    break;
  }
  checkOutputSize();
}

void VariableSerializer::write(int64 v) {
  switch (m_type) {
  case PrintR:
  case VarExport:
  case JSON:
    *m_out << v;
    break;
  case VarDump:
    indent();
    *m_out << "int(" << v << ")\n";
    break;
  case DebugDump:
    indent();
    *m_out << "long(" << v << ')';
    writeRefCount();
    *m_out << '\n';
    break;
  case Serialize:
    *m_out << "i:" << v << ';';
    break;
  default:
    ASSERT(false);
    break;
  }
  checkOutputSize();
}

void VariableSerializer::write(double v) {
  switch (m_type) {
  case JSON:
    if (!isinf(v) && !isnan(v)) {
      char *buf;
      if (v == 0.0) v = 0.0; // so to avoid "-0" output
      vspprintf(&buf, 0, "%.*k", 14, v);
      *m_out << buf;
      free(buf);
    } else {
      // PHP issues a warning: double INF/NAN does not conform to the
      // JSON spec, encoded as 0.
      *m_out << '0';
    }
    break;
  case VarExport:
  case PrintR:
    {
      char *buf;
      if (v == 0.0) v = 0.0; // so to avoid "-0" output
      vspprintf(&buf, 0, "%.*G", 14, v);
      *m_out << buf;
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
      *m_out << buf;
      free(buf);
      writeRefCount();
      *m_out << '\n';
    }
    break;
  case Serialize:
    *m_out << "d:";
    if (isnan(v)) {
      *m_out << "NAN";
    } else if (isinf(v)) {
      *m_out << "INF";
    } else {
      char *buf;
      if (v == 0.0) v = 0.0; // so to avoid "-0" output
      vspprintf(&buf, 0, "%.*G", 14, v);
      *m_out << buf;
      free(buf);
    }
    *m_out << ';';
    break;
  default:
    ASSERT(false);
    break;
  }
  checkOutputSize();
}

void VariableSerializer::write(litstr v, int len /* = -1 */,
                               bool isArrayKey /* = false */) {
  switch (m_type) {
  case PrintR: {
    if (len < 0) len = strlen(v);
    const char *p = v;
    for (int i = 0; i < len; i++) {
      *m_out << *p++;
    }
    break;
  }
  case VarExport: {
    if (len < 0) len = strlen(v);
    *m_out << '\'';
    const char *p = v;
    for (int i = 0; i < len; i++, p++) {
      const char c = *p;
      // adapted from Zend php_var_export and php_addcslashes
      if (c == '\'' || c == '\\' || (!isArrayKey && c == '\0')) {
        if ((unsigned char) c < 32 || (unsigned char) c > 126) {
          *m_out << '\\';
          char buffer[4];
          sprintf(buffer, "%03o", (unsigned char)c);
          *m_out << buffer;
          continue;
        } else {
          *m_out << '\\';
        }
      }
      *m_out << c;
    }
    *m_out << '\'';
    break;
  }
  case VarDump:
  case DebugDump: {
    if (v == NULL) v = "";
    if (len < 0) len = strlen(v);
    indent();
    *m_out << "string(" << len << ") \"";
    const char *p = v;
    for (int i = 0; i < len; i++) {
      *m_out << *p++;
    }
    *m_out << '"';
    writeRefCount();
    *m_out << '\n';
    break;
  }
  case Serialize:
    if (len < 0) {
      len = strlen(v);
      *m_out << "s:" << len << ":\"" << v << "\";";
    } else {
      *m_out << "s:" << len << ":\"";
      const char *p = v;
      for (int i = 0; i < len; i++) {
        *m_out << *p++;
      }
      *m_out << "\";";
    }
    break;
  case JSON:
    {
      if (len < 0) len = strlen(v);
      char *escaped = string_json_escape(v, len, m_option);
      *m_out << escaped;
      free(escaped);
    }
    break;
  default:
    ASSERT(false);
    break;
  }
  checkOutputSize();
}

void VariableSerializer::write(CStrRef v) {
  v.serialize(this);
}

void VariableSerializer::write(CArrRef v) {
  v.serialize(this);
}

void VariableSerializer::write(CObjRef v) {
  if (!v.isNull() && m_type == JSON) {
    Array props = v->o_toArray();
    ClassInfo::PropertyVec properties;
    ClassInfo::GetClassProperties(properties, v->o_getClassName());
    for (ClassInfo::PropertyVec::const_iterator iter = properties.begin();
         iter != properties.end(); ++iter) {
      if ((*iter)->attribute & ClassInfo::IsProtected) {
        props.remove((*iter)->name);
      }
    }
    // Remove private props
    for (ArrayIter it(props); !it.end(); it.next()) {
      if (it.first().toString().charAt(0) == '\0') {
        props.remove(it.first());
      }
    }
    setObjectInfo(v->o_getClassName(), v->o_getId());
    props.serialize(this);
  } else {
    v.serialize(this);
  }
}

void VariableSerializer::write(CVarRef v, bool isArrayKey /* = false */) {
  if (!isArrayKey && v.isObject()) {
    write(v.toObject());
    return;
  }
  setReferenced(v.isReferenced());
  setRefCount(v.getRefCount());
  v.serialize(this, isArrayKey);
}

void VariableSerializer::writeNull() {
  switch (m_type) {
  case PrintR:
    // do nothing
    break;
  case VarExport:
    *m_out << "NULL";
    break;
  case VarDump:
  case DebugDump:
    indent();
    *m_out << "NULL";
    writeRefCount();
    *m_out << '\n';
    break;
  case Serialize:
    *m_out << "N;";
    break;
  case JSON:
    *m_out << "null";
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
    *m_out << "*RECURSION*";
    break;
  case VarExport:
    throw NestingLevelTooDeepException();
  case VarDump:
  case DebugDump:
    indent();
    *m_out << "*RECURSION*\n";
    break;
  case Serialize:
    {
      PointerCounterMap::const_iterator iter = m_arrayIds.find(ptr);
      ASSERT(iter != m_arrayIds.end());
      int id = iter->second;
      if (isObject) {
        *m_out << "r:" << id << ";";
      } else if (wasRef) {
        *m_out << "R:" << id << ";";
      } else {
        *m_out << "N;";
      }
    }
    break;
  case JSON:
    *m_out << "null";
    break;
  default:
    ASSERT(false);
    break;
  }
}

void VariableSerializer::writeRefCount() {
  if (m_type == DebugDump) {
    *m_out << " refcount(" << m_refCount << ')';
    m_refCount = 1;
  }
}

void VariableSerializer::writeArrayHeader(const ArrayData *arr, int size) {
  m_arrayInfos.resize(m_arrayInfos.size() + 1);
  ArrayInfo &info = m_arrayInfos.back();
  info.first_element = true;
  info.is_vector = m_objClass.empty() && arr->isVectorData();
  info.indent_delta = 0;

  switch (m_type) {
  case PrintR:
    if (!m_objClass.empty()) {
      *m_out << m_objClass << " Object\n";
    } else {
      *m_out << "Array\n";
    }
    if (m_indent > 0) {
      m_indent += 4;
      indent();
    }
    *m_out << "(\n";
    m_indent += (info.indent_delta = 4);
    break;
  case VarExport:
    if (m_indent > 0) {
      *m_out << '\n';
      indent();
    }
    if (!m_objClass.empty()) {
      *m_out << m_objClass << "::__set_state(array(\n";
    } else {
      *m_out << "array (\n";
    }
    m_indent += (info.indent_delta = 2);
    break;
  case VarDump:
  case DebugDump:
    indent();
    if (!m_rsrcName.empty()) {
      *m_out << "resource(" << m_rsrcId << ") of type (" << m_rsrcName << ")\n";
      break;
    } else if (!m_objClass.empty()) {
      *m_out << "object(" << m_objClass << ")#" << m_objId << " ";
    } else {
      *m_out << "array";
    }
    *m_out << "(" << size << ")";

    // ...so to strictly follow PHP's output
    if (m_type == VarDump) {
      *m_out << " ";
    } else {
      writeRefCount();
    }

    *m_out << "{\n";
    m_indent += (info.indent_delta = 2);
    break;
  case Serialize:
    if (!m_objClass.empty()) {
      *m_out << "O:" << m_objClass.size() << ":\"" << m_objClass << "\":"
             << size << ":{";
    } else {
      *m_out << "a:" << size << ":{";
    }
    break;
  case JSON:
    if (info.is_vector) {
      *m_out << "[";
    } else {
      *m_out << "{";
    }
    break;
  default:
    ASSERT(false);
    break;
  }

  // ...so we don't mess up next array output
  if (!m_objClass.empty() || !m_rsrcName.empty()) {
    if (!m_objClass.empty()) {
      info.class_info = ClassInfo::FindClass(m_objClass.c_str());
    }
    m_objClass.clear();
    info.is_object = true;
  } else {
    info.is_object = false;
  }
}

void VariableSerializer::writePropertyPrivacy(const char *prop,
                                              const ClassInfo *cls) {
  if (!cls) return;
  const ClassInfo *origCls = cls;
  ClassInfo::PropertyInfo *p = cls->getPropertyInfo(prop);
  while (!p && cls && cls->getParentClass()) {
    cls = ClassInfo::FindClass(cls->getParentClass());
    if (cls) p = cls->getPropertyInfo(prop);
  }
  if (!p) return;
  ClassInfo::Attribute a = p->attribute;
  if (a & ClassInfo::IsProtected) {
    *m_out << ":protected";
  } else if (a & ClassInfo::IsPrivate && cls == origCls) {
    *m_out << ":private";
  }
}

void VariableSerializer::writeSerializedProperty(CStrRef prop,
                                                 const ClassInfo *cls) {
  String res = prop;
  const ClassInfo *origCls = cls;
  if (cls) {
    ClassInfo::PropertyInfo *p = cls->getPropertyInfo(prop.c_str());
    // Try to find defining class
    while (!p && cls && cls->getParentClass()) {
      cls = ClassInfo::FindClass(cls->getParentClass());
      if (cls) p = cls->getPropertyInfo(prop);
    }
    if (p) {
      const ClassInfo *dcls = p->owner;
      ClassInfo::Attribute a = p->attribute;
      if (a & ClassInfo::IsProtected) {
        res = String("\0*\0", 3, AttachLiteral) + prop;
      } else if (a & ClassInfo::IsPrivate && cls == origCls) {
        const char *clsname = dcls->getName();
        int clsLen = strlen(clsname);
        int headerLen = clsLen + 2;
        int totalLen = headerLen + prop.size() + 1;
        char *buf = (char*)malloc(totalLen);
        buf[0] = '\0';
        memcpy(buf + 1, clsname, clsLen);
        buf[clsLen + 1] = '\0';
        memcpy(buf + headerLen, prop.c_str(), prop.size());
        buf[totalLen - 1] = '\0';
        res = String(buf, totalLen - 1, AttachString);
      }
    }
  }
  write(res);
}

void VariableSerializer::writeArrayKey(const ArrayData *arr, Variant key) {
  ArrayInfo &info = m_arrayInfos.back();
  const ClassInfo *cls = info.class_info;
  if (info.is_object) {
    String ks(key.toString());
    if (ks.charAt(0) == '\0') {
      int span = ks.find('\0', 1);
      ASSERT(span != String::npos);
      String cl(ks.substr(1, span - 1));
      cls = ClassInfo::FindClass(cl);
      ASSERT(cls);
      key = ks.substr(span + 1);
    }
  }
  switch (m_type) {
  case PrintR: {
    indent();
    *m_out << '[';
      String keyStr = key.toString();
      const char *p = keyStr;
      int len  = keyStr.length();
      for (int i = 0; i < len; i++) {
        *m_out << *p++;
      }
      if (info.is_object) writePropertyPrivacy(keyStr.c_str(), cls);
      *m_out << "] => ";
    break;
  }
  case VarExport:
    indent();
    write(key, true);
    *m_out << " => ";
    break;
  case VarDump:
  case DebugDump:
    indent();
    if (key.isNumeric()) {
      *m_out << '[' << (const char *)key.toString() << "]=>\n";
    } else {
      *m_out << "[\"";
      String keyStr = key.toString();
      const char *p = keyStr;
      int len  = keyStr.length();
      for (int i = 0; i < len; i++) {
        *m_out << *p++;
      }
      if (info.is_object) writePropertyPrivacy(keyStr.c_str(), cls);
      *m_out << "\"]=>\n";
    }
    break;
  case Serialize:
    if (info.is_object) {
      writeSerializedProperty(key.toString(), cls);
    } else {
      write(key);
    }
    break;
  case JSON:
    if (!info.first_element) {
      *m_out << ",";
    }
    if (!info.is_vector) {
      write(key.toString());
      *m_out << ":";
    }
    break;
  default:
    ASSERT(false);
    break;
  }
}

void VariableSerializer::writeArrayValue(const ArrayData *arr, CVarRef value) {
  // Do not count referenced values after the first
  if (m_type == Serialize &&
      !(value.isReferenced() &&
        m_arrayIds.find(value.getVariantData()) != m_arrayIds.end()))
    m_valueCount++;

  write(value);
  switch (m_type) {
  case PrintR:
    *m_out << '\n';
    break;
  case VarExport:
    *m_out << ",\n";
    break;
  default:
    break;
  }

  ArrayInfo &info = m_arrayInfos.back();
  info.first_element = false;
}

void VariableSerializer::writeArrayFooter(const ArrayData *arr) {
  ArrayInfo &info = m_arrayInfos.back();

  m_indent -= info.indent_delta;
  switch (m_type) {
  case PrintR:
    indent();
    *m_out << ")\n";
    if (m_indent > 0) {
      m_indent -= 4;
    }
    break;
  case VarExport:
    indent();
    if (info.is_object) {
      *m_out << "))";
    } else {
      *m_out << ')';
    }
    break;
  case VarDump:
  case DebugDump:
    if (m_rsrcName.empty()) {
      indent();
      *m_out << "}\n";
    }
    break;
  case Serialize:
    *m_out << '}';
    break;
  case JSON:
    if (info.is_vector) {
      *m_out << "]";
    } else {
      *m_out << "}";
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
  *m_out << "C:" << clsname.size() << ":\"";
  const char *p = clsname.data();
  for (int i = 0; i < clsname.size(); i++) {
    *m_out << *p++;
  }
  *m_out << "\":" << serialized.size() << ":{";
  p = serialized.data();
  for (int i = 0; i < serialized.size(); i++) {
    *m_out << *p++;
  }
  *m_out << "}";
}

///////////////////////////////////////////////////////////////////////////////

void VariableSerializer::indent() {
  for (int i = 0; i < m_indent; i++) {
    *m_out << ' ';
  }
  if (m_referenced) {
    if (m_indent > 0) *m_out << '&';
    m_referenced = false;
  }
}

bool VariableSerializer::incNestedLevel(void *ptr,
                                        bool isObject /* = false */) {
  switch (m_type) {
  case VarExport:
  case PrintR:
  case VarDump:
  case DebugDump:
    return ++m_counts[ptr] >= m_maxCount;
  case Serialize:
    {
      int ct = ++m_counts[ptr];
      if (m_arrayIds.find(ptr) != m_arrayIds.end() &&
          (m_referenced || isObject)) {
        return true;
      } else {
        m_arrayIds[ptr] = m_valueCount;
      }
      return ct >= (m_maxCount - 1);
    }
    break;
  case JSON:
    return ++m_counts[ptr] >= m_maxCount;
  default:
    ASSERT(false);
    break;
  }
  return false;
}

void VariableSerializer::decNestedLevel(void *ptr) {
  --m_counts[ptr];
}

void VariableSerializer::checkOutputSize() {
  if (m_outputLimit > 0 && m_out->tellp() - m_initPos > m_outputLimit) {
    raise_error("Value too large for serialization");
  }
}

///////////////////////////////////////////////////////////////////////////////
}
