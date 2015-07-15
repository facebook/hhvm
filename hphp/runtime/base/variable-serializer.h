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

#ifndef incl_HPHP_VARIABLE_SERIALIZER_H_
#define incl_HPHP_VARIABLE_SERIALIZER_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ClassInfo;

/**
 * Maintaining states during serialization of a variable. We use this single
 * class to uniformly serialize variables according to different formats.
 */
class VariableSerializer {
public:
  /**
   * Supported formats.
   */
  enum class Type {
    PrintR, //print_r()
    VarExport, //var_export()
    VarDump, //var_dump()
    DebugDump, //debug_zval_dump()
    DebuggerDump, //used by hphp debugger to obtain user visible output
    Serialize, // serialize()
    JSON, //json_encode()
    APCSerialize, //used in APC serialization (controlled by switch)
    DebuggerSerialize, //used by hphp debugger for client<->proxy communication
    PHPOutput, //used by compiler to output scalar values into byte code
  };

  /**
   * Constructor and destructor.
   */
  explicit VariableSerializer(Type type, int option = 0, int maxRecur = 3);
  ~VariableSerializer() {
    if (m_arrayIds) delete m_arrayIds;
  }

  static __thread int64_t serializationSizeLimit;

  /**
   * Top level entry function called by f_ functions.
   */
  String serialize(const Variant& v, bool ret, bool keepCount = false);
  String serializeValue(const Variant& v, bool limit);

  // Serialize with limit size of output, always return the serialized string.
  // It does not work with Serialize, JSON, APCSerialize, DebuggerSerialize.
  String serializeWithLimit(const Variant& v, int limit);

  /**
   * Type specialized output functions.
   */
  void write(bool    v);
  void write(char    v) { write((int64_t)v);}
  void write(short   v) { write((int64_t)v);}
  void write(int     v) { write((int64_t)v);}
  void write(int64_t   v);
  void write(double  v);

  void write(const char *v, int len = -1, bool isArrayKey = false,
             bool noQuotes = false);

  void write(const String& v);
  void write(const Object& v);
  void write(const Variant& v, bool isArrayKey = false);

  void writeNull();
  // what to write if recursive level is over limit?
  void writeOverflow(void* ptr, bool isObject = false);
  void writeRefCount(); // for DebugDump only

  void writeArrayHeader(int size, bool isVectorData);
  void writeArrayKey(const Variant& key);
  void writeArrayValue(const Variant& value);
  void writeCollectionKey(const Variant& key);
  void writeCollectionKeylessPrefix();
  void writeArrayFooter();
  void writeSerializableObject(const String& clsname, const String& serialized);

  /**
   * Helpers.
   */
  void indent();
  void setDepthLimit(size_t depthLimit) { m_maxDepth = depthLimit; }
  void setReferenced(bool referenced) { m_referenced = referenced;}
  void setRefCount(int count) { m_refCount = count;}
  void incMaxCount() { m_maxCount++; }
  bool incNestedLevel(void *ptr, bool isObject = false);
  void decNestedLevel(void *ptr);
  void pushObjectInfo(const String& objClass, int objId, char objCode);
  void popObjectInfo();
  void pushResourceInfo(const String& rsrcName, int rsrcId);
  void popResourceInfo();
  Type getType() const { return m_type; }

private:
  using ReqPtrCtrMap = req::hash_map<void*, int, pointer_hash<void>>;
  Type m_type;
  int m_option;                  // type specific extra options
  StringBuffer *m_buf;
  int m_indent;
  ReqPtrCtrMap m_counts;         // counting seen arrays for recursive levels
  ReqPtrCtrMap *m_arrayIds;      // reference ids for objs/arrays
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
  size_t m_currentDepth;         // current depth (nasted objects/arrays)
  size_t m_maxDepth;             // max depth limit before an error (0 -> none)

  struct ArrayInfo {
    bool is_object;     // nested arrays or objects
    bool is_vector;     // whether current array is a vector
    bool first_element; // whether this is first array element
    int  indent_delta;  // the extra indent to serialize this object
    int  size;          // the number of elements in the array
  };
  req::vector<ArrayInfo> m_arrayInfos;

  struct ObjectInfo {
    String objClass;
    int    objId;
    char   objCode;
    String rsrcName;
    int    rsrcId;
  };
  req::vector<ObjectInfo> m_objectInfos;

  // The func parameter will be invoked only if there is no overflow.
  // Otherwise, writeOverflow will be invoked instead.
  void preventOverflow(const Object& v, const std::function<void()>& func);
  void writePropertyKey(const String& prop);
};

/*
 * Serialize a Variant recursively.
 * The last param noQuotes indicates to serializer to not put the output in
 * double quotes (used when printing the output of a __toDebugDisplay() of
 * an object when it is a string.
 */
void serializeVariant(const Variant&,
                      VariableSerializer *serializer,
                      bool isArrayKey = false,
                      bool skipNestCheck = false,
                      bool noQuotes = false);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_VARIABLE_SERIALIZER_H_
