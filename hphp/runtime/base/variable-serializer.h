/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/base/req-hash-map.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"

#include "hphp/util/rds-local.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Maintaining states during serialization of a variable. We use this single
 * class to uniformly serialize variables according to different formats.
 */
struct VariableSerializer {
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
    Internal, // used internally by the compiler. No compatibility guarantees.
    JSON, //json_encode()
    APCSerialize, //used in APC serialization (controlled by switch)
    DebuggerSerialize, //used by hphp debugger for client<->proxy communication
    PHPOutput, //used by compiler to output scalar values into byte code
    Last = PHPOutput,
  };

  /*
   * Set in m_option for APCSerialize to disable serializing static
   * datastructures as their address
   */
  static constexpr auto kAPC_PRIME_SERIALIZE = 1;

  /**
   * Constructor and destructor.
   */
  explicit VariableSerializer(Type type, int option = 0, int maxRecur = 3);
  ~VariableSerializer();
  VariableSerializer(const VariableSerializer&) = delete;
  VariableSerializer& operator=(const VariableSerializer&) = delete;

  // Use UnlimitSerializationScope to suspend this temporarily.
  struct SerializationLimitWrapper {
    int64_t value = StringData::MaxSize;
  };
  static RDS_LOCAL(SerializationLimitWrapper, serializationSizeLimit);

  /**
   * Top level entry function called by f_ functions.
   */
  String serialize(const_variant_ref v, bool ret, bool keepCount = false);
  String serialize(const Variant& var, bool ret, bool keepCount = false) {
    return serialize(const_variant_ref{var}, ret, keepCount);
  }
  String serializeValue(const Variant& v, bool limit);

  // Serialize with limit size of output, always return the serialized string.
  // It does not work with Serialize, JSON, APCSerialize, DebuggerSerialize.
  String serializeWithLimit(const Variant& v, int limit);

  // for ext_json
  void setDepthLimit(size_t depthLimit) { m_maxDepth = depthLimit; }
  // for ext_std_variable
  void incMaxCount() { m_maxCount++; }

  Type getType() const { return m_type; }

  // By default, for Type::Serialize, d/varrays are serialized as normal
  // arrays. This flag can override that behavior.
  void keepDVArrays() { m_keepDVArrays = true; }

  // Force Hack arrays to serialize as PHP arrays.
  void setForcePHPArrays() { m_forcePHPArrays = true; }

  // Force PHP arrays to serialize as Hack arrays. This mode is preferred.
  // In this mode, all decisions are driven by types. E.g., for JSON:
  //   - varrays and vecs are encoded as lists: [...]
  //   - darrays and dicts are encoded as maps: {...}
  void setForceHackArrays() { m_forceHackArrays = true; }

  // Emit a HAC notice on serialization of the specified kind of array.
  void setHackWarn()  { m_hackWarn = true; }
  void setDictWarn()  { m_dictWarn = true; }
  void setKeysetWarn()  { m_keysetWarn = true; }
  void setPHPWarn()   { m_phpWarn = true; }
  void setEmptyDArrayWarn()    { m_edWarn = true; }
  void setVecLikeDArrayWarn()  { m_vdWarn = true; }
  void setDictLikeDArrayWarn() { m_ddWarn = true; }

  // ignore uninitialized late init props and do not attempt to serialize them
  void setIgnoreLateInit() { m_ignoreLateInit = true; }

  // Serialize legacy bit and provenance tag, using same format as
  // Type::Internal serializer. This is only supported Type::Serialize.
  void setSerializeProvenanceAndLegacy() {
    assertx(getType() == Type::Serialize);
    m_serializeProvenanceAndLegacy = true;
  }

  // MarkedVArray and MarkedDArray are used for serialization formats, which
  // can distinguish between all 3 possible array states (unmarked varray,
  // unmarked vec, marked varray/vec).
  // In post-EvalHackArrDVArrs MarkedVArray/MarkedDArray correspond to marked
  // vec/dict.
  enum class ArrayKind { PHP, Dict, Vec, Keyset, VArray, DArray,
                         MarkedVArray, MarkedDArray };

  // One entry for each vec or dict in the value being serialized (in a
  // pre-order walk). If the bool is true, and mode is PHPOutput, the vec or
  // dict will be output like a varray or darray.
  using DVOverrides = std::vector<bool>;
  void setDVOverrides(const DVOverrides* overrides) {
    m_dvOverrides = overrides;
  }

  void setUnitFilename(const StringData* name) {
    assertx(name->isStatic());
    assertx(getType() == Type::Internal);
    m_unitFilename = name;
  }

private:
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
  void write(const_variant_ref v, bool isArrayKey = false);

  void writeNull();
  // what to write if recursive level is over limit?
  void writeOverflow(tv_rval tv);
  void writeRefCount(); // for DebugDump only

  void writeArrayHeader(int size, bool isVectorData, ArrayKind kind);
  void writeArrayKey(const Variant& key, ArrayKind kind);
  void writeArrayValue(
    const Variant& value,
    ArrayKind kind
  );
  void writeCollectionKey(
    const Variant& key,
    ArrayKind kind
  );
  void writeArrayFooter(ArrayKind kind);
  void writeSerializableObject(const String& clsname, const String& serialized);

  /**
   * Helpers.
   */
  void indent();
  void setRefCount(int count) { m_refCount = count;}
  bool incNestedLevel(tv_rval tv);
  void decNestedLevel(tv_rval tv);
  void pushObjectInfo(const String& objClass, char objCode);
  void popObjectInfo();
  void pushResourceInfo(const String& rsrcName, int rsrcId);
  void popResourceInfo();

  ArrayKind getKind(const ArrayData* arr) const;

  // The func parameter will be invoked only if there is no overflow.
  // Otherwise, writeOverflow will be invoked instead.
  void preventOverflow(const Object& v, const std::function<void()>& func);
  void writePropertyKey(const String& prop);

  // Serialize a Variant recursively.
  // The last param noQuotes indicates to serializer to not put the output in
  // double quotes (used when printing the output of a __toDebugDisplay() of
  // an object when it is a string.
  void serializeVariant(tv_rval value,
                        bool isArrayKey = false,
                        bool skipNestCheck = false,
                        bool noQuotes = false);
  void serializeObject(const Object&);
  void serializeObject(const ObjectData*);
  void serializeObjectImpl(const ObjectData* obj);
  void serializeCollection(ObjectData* obj);
  void serializeObjProps(Array&);
  void serializeArray(const ArrayData*, bool skipNestCheck = false);
  void serializeArrayImpl(const ArrayData* arr, bool isVectorData);
  void serializeResource(const ResourceData*);
  void serializeResourceImpl(const ResourceData* res);
  void serializeString(const String&);
  void serializeRFunc(const RFuncData* func);
  void serializeFunc(const Func* func);
  void serializeClass(const Class* cls);
  void serializeLazyClass(LazyClassData);
  void serializeClsMeth(ClsMethDataRef clsMeth, bool skipNestCheck = false);
  void serializeRClsMeth(RClsMethData* rclsMeth);

  Array getSerializeProps(const ObjectData* obj) const;

private:
  // Sentinel used to indicate that a member of SavedRefMap has a count but no
  // ID.
  static constexpr int NO_ID = -1;

  struct SavedRefMap {
    ~SavedRefMap();

    struct MapData {
      int m_count{0};
      int m_id{-1};
    };

    MapData& operator[](tv_rval tv) {
      auto& elm = m_mapping[*tv];
      if (!elm.m_count) tvIncRefGen(*tv);
      return elm;
    }

    const MapData& operator[](tv_rval tv) const {
      return m_mapping.at(*tv);
    }

  private:
    struct TvHash {
      std::size_t operator()(const TypedValue& tv) const {
        return pointer_hash<void>()(tv.m_data.parr);
      }
    };

    struct TvEq {
      bool operator()(const TypedValue& a, const TypedValue& b) const {
        return a.m_data.parr == b.m_data.parr;
      }
    };

    req::fast_map<TypedValue, MapData, TvHash, TvEq> m_mapping;
  };

private:
  Type m_type;
  int m_option;                  // type specific extra options
  StringBuffer *m_buf{nullptr};
  int m_indent{0};
  SavedRefMap m_refs;            // reference ids and counts for objs/arrays
  int m_valueCount{0};           // current ref index
  bool m_keepDVArrays;           // serialize d/varrays as themselves or arrays
  bool m_forcePHPArrays{false};  // serialize PHP and Hack arrays as PHP arrays
  bool m_forceHackArrays{false}; // serialize PHP and Hack arrays as Hack arrays
  bool m_serializeProvenanceAndLegacy{false}; // serialize provenance tags and
                                              // legacy bit
  bool m_hackWarn{false};        // warn when attempting on Hack arrays
  bool m_dictWarn{false};        // warn when attempting on dicts
  bool m_keysetWarn{false};        // warn when attempting on keysets
  bool m_phpWarn{false};         // warn when attempting on PHP arrays
  bool m_edWarn{false};          // warn when attempting on empty darrays
  bool m_vdWarn{false};          // warn when attempting on vec-like darrays
  bool m_ddWarn{false};          // warn when attempting on non-vec-like darrays
  bool m_ignoreLateInit{false};  // ignore uninitalized late init props
  bool m_hasHackWarned{false};   // have we already warned on Hack arrays?
  bool m_hasDictWarned{false};   // have we already warned on dicts?
  bool m_hasKeysetWarned{false};   // have we already warned on dicts?
  bool m_hasPHPWarned{false};    // have we already warned on PHP arrays?
  bool m_hasEDWarned{false};     // have we already warned on empty darrays?
  bool m_hasVDWarned{false};     // have we already warned on vec-like darrays?
  bool m_hasDDWarned{false};  // have we already warned on non-vec-like darrays?
  int m_refCount{1};             // current variable's reference count
  String m_objClass;             // for object serialization
  char m_objCode{0};             // for object serialization
  String m_rsrcName;             // for resource serialization
  int m_rsrcId{0};               // for resource serialization
  int m_maxCount;                // for max recursive levels
  int m_levelDebugger{0};        // keep track of levels for DebuggerSerialize
  int m_maxLevelDebugger{0};     // for max level of DebuggerSerialize
  size_t m_currentDepth{0};      // current depth (nasted objects/arrays)
  size_t m_maxDepth{0};          // max depth limit before an error (0 -> none)
  bool m_keyPrinted{false};

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
    char   objCode;
    String rsrcName;
    int    rsrcId;
  };
  req::vector<ObjectInfo> m_objectInfos;

  /* unitFilename should be set when we are serializing
   * an adata for a unit in the repo--it is needed to correctly
   * compress the provenance tag */
  const StringData* m_unitFilename{nullptr};
  const DVOverrides* m_dvOverrides{nullptr};
  size_t m_dvOverridesIndex{0};
};

inline String internal_serialize(const Variant& v) {
  VariableSerializer vs{VariableSerializer::Type::Internal};
  return vs.serializeValue(v, false);
}

// TODO: Move to util/folly?
template<typename T> struct TmpAssign {
  TmpAssign(T& v, const T tmp) : cur(v), save(cur) { cur = tmp; }
  ~TmpAssign() { cur = save; }
  T& cur;
  const T save;
};

struct UnlimitSerializationScope {
  static constexpr int32_t kTmpLimit = StringData::MaxSize;
  TmpAssign<int64_t> v{VariableSerializer::serializationSizeLimit->value,
                       kTmpLimit};
  TmpAssign<int64_t> rs{RuntimeOption::SerializationSizeLimit, kTmpLimit};
  TmpAssign<int32_t> rm{RuntimeOption::MaxSerializedStringSize, kTmpLimit};
};

extern const StaticString s_serializedNativeDataKey;

///////////////////////////////////////////////////////////////////////////////
}

