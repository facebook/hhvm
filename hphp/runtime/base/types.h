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

#ifndef incl_HPHP_TYPES_H_
#define incl_HPHP_TYPES_H_

#include <stdint.h>
#include <atomic>
#include <limits>
#include <type_traits>
#include <vector>
#include <stack>
#include <list>
#include <map>

#include "hphp/util/functional.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/util/low-ptr.h"
#include "hphp/util/mutex.h"
#include "hphp/util/thread-local.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/macros.h"
#include "hphp/runtime/base/memory-manager.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class String;
class StaticString;
class Array;
class Object;
template<typename T> class SmartObject;
class Resource;
template<typename T> class SmartResource;
class Variant;
class VarNR;
class RefData;

extern const Variant null_variant;      // uninitialized variant
extern const Variant init_null_variant; // php null
extern const VarNR null_varNR;
extern const VarNR true_varNR;
extern const VarNR false_varNR;
extern const VarNR INF_varNR;
extern const VarNR NEGINF_varNR;
extern const VarNR NAN_varNR;
extern const String null_string;
extern const Array null_array;
extern const Array empty_array_ref;
extern const StaticString array_string; // String("Array")

// Use empty_string() if you're returning String
// Use empty_string_variant() if you're returning Variant
// Or use these if you need to pass by const reference:
extern const StaticString empty_string_ref; // const StaticString&
extern const Variant empty_string_variant_ref; // const Variant&

class StringData;
class ArrayData;
class ObjectData;
class ResourceData;
class MArrayIter;

class Class;

class VariableSerializer;
class VariableUnserializer;

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_LOWPTR
constexpr bool use_lowptr = true;

typedef LowPtr<Class, uint32_t> LowClassPtr;
typedef LowPtr<const StringData, uint32_t> LowStringPtr;
#else
constexpr bool use_lowptr = false;

typedef LowPtr<Class, uintptr_t> LowClassPtr;
typedef LowPtr<const StringData, uintptr_t> LowStringPtr;
#endif

///////////////////////////////////////////////////////////////////////////////

/**
 * Many functions may elect to take "litstr" separately from "String" class.
 * This code specialization helps speed a lot by not instantiating a String
 * object to box an otherwise literal value. This also means, though not
 * obviously thus dangerous not to know, whenever a function takes a parameter
 * with type of "litstr", one can only pass in a literal string that has
 * a "permanent" memory address to be stored. To make this really clear, I
 * invented "litstr" as a typedef-ed name for "const char *" that expects a
 * literal string only. Therefore, throughout this entire runtime library,
 *
 *   litstr == literal string
 *   const char * == any C-string pointer
 *
 */

namespace Uns {
enum class Mode {
  Value = 0,
  Key = 1,
  ColValue = 2,
  ColKey = 3,
};
}

namespace Collection {
enum Type : uint16_t { // stored in ObjectData::o_subclassData
  // values must be contiguous integers (for ArrayIter::initFuncTable)
  InvalidType = 0,
  VectorType = 1,
  MapType = 2,
  SetType = 3,
  PairType = 4,
  ImmVectorType = 5,
  ImmMapType = 6,
  ImmSetType = 7,
};
const size_t MaxNumTypes = 8;

inline Type stringToType(const char* str, size_t len) {
  switch (len) {
    case 6:
      if (!strcasecmp(str, "hh\\set")) return SetType;
      if (!strcasecmp(str, "hh\\map")) return MapType;
      break;
    case 7:
      if (!strcasecmp(str, "hh\\pair")) return PairType;
      break;
    case 9:
      if (!strcasecmp(str, "hh\\vector")) return VectorType;
      if (!strcasecmp(str, "hh\\immmap")) return ImmMapType;
      if (!strcasecmp(str, "hh\\immset")) return ImmSetType;
      break;
    case 12:
      if (!strcasecmp(str, "hh\\immvector")) return ImmVectorType;
      break;
    default:
      break;
  }
  return InvalidType;
}
inline Type stringToType(const std::string& s) {
  return stringToType(s.c_str(), s.size());
}
inline bool isVectorType(Collection::Type ctype) {
  return (ctype == Collection::VectorType ||
          ctype == Collection::ImmVectorType);
}
inline bool isMapType(Collection::Type ctype) {
  return (ctype == Collection::MapType ||
          ctype == Collection::ImmMapType);
}
inline bool isSetType(Collection::Type ctype) {
  return (ctype == Collection::SetType ||
          ctype == Collection::ImmSetType);
}
inline bool isInvalidType(Collection::Type ctype) {
  return (ctype == Collection::InvalidType ||
          static_cast<size_t>(ctype) >= Collection::MaxNumTypes);
}
inline bool isMutableType(Collection::Type ctype) {
  return (ctype == Collection::VectorType ||
          ctype == Collection::MapType ||
          ctype == Collection::SetType);
}
inline bool isImmutableType(Collection::Type ctype) {
  return !isMutableType(ctype);
}

inline bool isTypeWithPossibleIntStringKeys(Collection::Type ctype) {
  return Collection::isSetType(ctype) || Collection::isMapType(ctype);
}

}

//////////////////////////////////////////////////////////////////////

typedef const char * litstr; /* literal string */

typedef const class VRefParamValue    &VRefParam;
typedef const class RefResultValue    &RefResult;

inline const Variant&
variant(RefResult v)      { return *(Variant*)&v; }
inline const Variant&
variant(const Variant& v)        { return v; }

/**
 * ref() can be used to cause strong binding
 *
 *   a = ref(b); // strong binding: now both a and b point to the same data
 *   a = b;      // weak binding: a will copy or copy-on-write
 *
 */
inline RefResult ref(const Variant& v) {
  return *(RefResultValue*)&v;
}

inline RefResult ref(Variant& v) {
  return *(RefResultValue*)&v;
}

///////////////////////////////////////////////////////////////////////////////

class GlobalNameValueTableWrapper;
class ObjectAllocatorBase;
class Profiler;
class CodeCoverage;
typedef GlobalNameValueTableWrapper GlobalVariables;

///////////////////////////////////////////////////////////////////////////////

class AccessFlags {
public:
  enum Type {
    None = 0,
    Error = 1,
    Key = 2,
    Error_Key = Error | Key,
  };
  static Type IsKey(bool s) { return s ? Key : None; }
  static Type IsError(bool e) { return e ? Error : None; }
};

#define ACCESSPARAMS_DECL AccessFlags::Type flags = AccessFlags::None
#define ACCESSPARAMS_IMPL AccessFlags::Type flags

/*
 * Program counters in the bytecode interpreter.
 *
 * Normally points to an Opcode, but has type const uchar* because
 * during a given instruction it is incremented while decoding
 * immediates and may point to arbitrary bytes.
 */
typedef const unsigned char* PC;

/*
 * Id type for various components of a unit that have to have unique
 * identifiers.  For example, function ids, class ids, string literal
 * ids.
 */
typedef int Id;
const Id kInvalidId = Id(-1);

/*
 * Translation IDs.
 *
 * These represent compilation units for the JIT, and are used to key
 * into several runtime structures for finding profiling data or
 * tracking translation information.
 */
using TransID = uint32_t;
constexpr TransID kInvalidTransID = -1u;

// Bytecode offsets.  Used for both absolute offsets and relative
// offsets.
typedef int32_t Offset;
constexpr Offset kInvalidOffset = std::numeric_limits<Offset>::max();
typedef hphp_hash_set<Offset> OffsetSet;

/*
 * Various fields in the VM's runtime have indexes that are addressed
 * using this "slot" type.  For example: methods, properties, class
 * constants.
 *
 * No slot value greater than or equal to kInvalidSlot will actually
 * be used for one of these.
 */
typedef uint32_t Slot;
const Slot kInvalidSlot = Slot(-1);

/*
 * Handles into Request Data Segment.  These are offsets from
 * RDS::tl_base.  See rds.h.
 */
namespace RDS {
  typedef uint32_t Handle;
  constexpr Handle kInvalidHandle = 0;
}

/*
 * Unique identifier for a Func*.
 */
typedef uint32_t FuncId;
constexpr FuncId InvalidFuncId = FuncId(-1LL);
constexpr FuncId DummyFuncId = FuncId(-2LL);
typedef hphp_hash_set<FuncId> FuncIdSet;

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TYPES_H_
