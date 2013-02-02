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

#ifndef __HPHP_VARIABLE_SERIALIZER_H__
#define __HPHP_VARIABLE_SERIALIZER_H__

#include <runtime/base/types.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/vm/class.h>
#include <runtime/vm/unit.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ClassInfo;

/**
 * Maintaining states during serialization of a variable. We use this single
 * class to uniformly serialize variables according to different formats:
 * print_r(), var_export(), var_dump(), debug_zval_dump() or serialize().
 */
class VariableSerializer {
public:
  /**
   * Supported formats.
   */
  enum Type {
    PrintR,
    VarExport,
    VarDump,
    DebugDump,
    DebuggerDump,
    Serialize,
    JSON,
    APCSerialize,
    DebuggerSerialize,
    PHPOutput,
  };

  /**
   * Constructor and destructor.
   */
  VariableSerializer(Type type, int option = 0, int maxRecur = 3);
  ~VariableSerializer() {
    if (m_arrayIds) delete m_arrayIds;
  }

  /**
   * Top level entry function called by f_ functions.
   */
  String serialize(CVarRef v, bool ret);
  String serializeValue(CVarRef v, bool limit);

  // Serialize with limit size of output, always return the serialized string.
  // It does not work with Serialize, JSON, APCSerialize, DebuggerSerialize.
  String serializeWithLimit(CVarRef v, int limit);

  /**
   * Type specialized output functions.
   */
  void write(bool    v);
  void write(char    v) { write((int64)v);}
  void write(short   v) { write((int64)v);}
  void write(int     v) { write((int64)v);}
  void write(int64   v);
  void write(double  v);
  void write(const char *v, int len = -1, bool isArrayKey = false);
  void write(CStrRef v);
  void write(CObjRef v);
  void write(CVarRef v, bool isArrayKey = false);

  void writeNull();
  // what to write if recursive level is over limit?
  void writeOverflow(void* ptr, bool isObject = false);
  void writeRefCount(); // for DebugDump only

  void writeArrayHeader(int size, bool isVectorData);
  void writeArrayKey(Variant key);
  void writeArrayValue(CVarRef value);
  void writeArrayFooter();
  void writeSerializableObject(CStrRef clsname, CStrRef serialized);

  /**
   * Helpers.
   */
  void indent();
  void setReferenced(bool referenced) { m_referenced = referenced;}
  void setRefCount(int count) { m_refCount = count;}
  void incMaxCount() { m_maxCount++; }
  bool incNestedLevel(void *ptr, bool isObject = false);
  void decNestedLevel(void *ptr);
  void setObjectInfo(CStrRef objClass, int objId, char objCode);
  void setResourceInfo(CStrRef rsrcName, int rsrcId);
  void getResourceInfo(String &rsrcName, int &rsrcId);
  Type getType() const { return m_type; }
private:
  Type m_type;
  int m_option;                  // type specific extra options
  StringBuffer *m_buf;
  int m_indent;
  PointerCounterMap m_counts;    // counting seen arrays for recursive levels
  PointerCounterMap *m_arrayIds; // reference ids for objs/arrays
  int m_valueCount;              // Current ref index
  bool m_referenced;             // mark current array element as reference
  int m_refCount;                // current variable's reference count
  String m_objClass;             // for object serialization
  int m_objId;                   // for object serialization
  char m_objCode;                // for object serialization
  String m_rsrcName;             // for resource serialization
  int m_rsrcId;                  // for resource serialization
  int m_maxCount;                // for max recursive levels
  int m_levelDebugger;           // keep track of levels for DebuggerSerialize
  int m_maxLevelDebugger;        // for max level of DebuggerSerialize

  struct ArrayInfo {
    bool is_object;     // nested arrays or objects
    bool is_vector;     // whether current array is a vector
    bool first_element; // whether this is first array element
    int  indent_delta;  // the extra indent to serialize this object
  };
  std::vector<ArrayInfo> m_arrayInfos;

  void writePropertyKey(CStrRef prop);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VARIABLE_SERIALIZER_H__
