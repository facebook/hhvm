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

#include "hphp/util/thread-local.h"
#include "hphp/util/mutex.h"
#include "hphp/util/functional.h"
#include "hphp/util/hash-map-typedefs.h"
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
extern const Array empty_array;

class StringData;
class ArrayData;
class ObjectData;
class ResourceData;
class MArrayIter;

class VariableSerializer;
class VariableUnserializer;

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

}

//////////////////////////////////////////////////////////////////////

typedef const char * litstr; /* literal string */

typedef const class VRefParamValue    &VRefParam;
typedef const class RefResultValue    &RefResult;
typedef const class VariantStrongBind &CVarStrongBind;
typedef const class VariantWithRefBind&CVarWithRefBind;

inline CVarStrongBind
strongBind(const Variant& v)     { return *(VariantStrongBind*)&v; }
inline CVarStrongBind
strongBind(RefResult v)   { return *(VariantStrongBind*)&v; }
inline CVarWithRefBind
withRefBind(const Variant& v)    { return *(VariantWithRefBind*)&v; }

inline const Variant&
variant(CVarStrongBind v) { return *(Variant*)&v; }
inline const Variant&
variant(CVarWithRefBind v){ return *(Variant*)&v; }
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

class Class;

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
typedef hphp_hash_set<FuncId> FuncIdSet;

/*
 * Special types that are not relevant to the runtime as a whole.
 * The order for public/protected/private matters in numerous places.
 *
 * Attr unions are directly stored as integers in .hhbc repositories, so
 * incompatible changes here require a schema version bump.
 *
 * AttrTrait on a method means that the method is NOT a constructor,
 * even though it may look like one
 *
 * AttrNoOverride (WholeProgram only) on a class means its not extended
 * and on a method means that no extending class defines the method.
 *
 * AttrVariadicByRef indicates a function is a builtin that takes
 * variadic arguments, where the arguments are either by ref or
 * optionally by ref.  (It is equivalent to having ClassInfo's
 * (RefVariableArguments | MixedVariableArguments).)
 *
 * AttrMayUseVV indicates that a function may need to use a VarEnv or
 * varargs (aka extraArgs) at run time.
 *
 * AttrPhpLeafFn indicates a function does not make any explicit calls
 * to other php functions.  It may still call other user-level
 * functions via re-entry (e.g. for destructors or autoload), and it
 * may make calls to builtins using FCallBuiltin.
 *
 * AttrBuiltin is set on builtin functions - whether c++ or php
 *
 * AttrAllowOverride is set on builtin functions that can be replaced
 *   by user implementations
 *
 * AttrSkipFrame is set to indicate that the frame should be ignored
 *   when searching for the context (eg array_map evaluates its
 *   callback in the context of its caller).
 */
enum Attr {
  AttrNone          = 0,         // class  property  method  //
  AttrReference     = (1 <<  0), //                     X    //
  AttrPublic        = (1 <<  1), //            X        X    //
  AttrProtected     = (1 <<  2), //            X        X    //
  AttrPrivate       = (1 <<  3), //            X        X    //
  AttrStatic        = (1 <<  4), //            X        X    //
  AttrAbstract      = (1 <<  5), //    X                X    //
  AttrFinal         = (1 <<  6), //    X                X    //
  AttrInterface     = (1 <<  7), //    X                     //
  AttrPhpLeafFn     = (1 <<  7), //                     X    //
  AttrTrait         = (1 <<  8), //    X                X    //
  AttrNoInjection   = (1 <<  9), //                     X    //
  AttrUnique        = (1 << 10), //    X                X    //
  AttrDynamicInvoke = (1 << 11), //                     X    //
  AttrNoExpandTrait = (1 << 12), //    X                     //
  AttrNoOverride    = (1 << 13), //    X                X    //
  AttrClone         = (1 << 14), //                     X    //
  AttrVariadicByRef = (1 << 15), //                     X    //
  AttrMayUseVV      = (1 << 16), //                     X    //
  AttrPersistent    = (1 << 17), //    X                X    //
  AttrDeepInit      = (1 << 18), //            X             //
  AttrHot           = (1 << 19), //                     X    //
  AttrBuiltin       = (1 << 20), //    X                X    //
  AttrAllowOverride = (1 << 21), //                     X    //
  AttrSkipFrame     = (1 << 22), //                     X    //
  AttrNative        = (1 << 23), //                     X    //
  AttrHPHPSpecific  = (1 << 25), //                     X    //
  AttrIsFoldable    = (1 << 26), //                     X    //
  AttrNoFCallBuiltin= (1 << 27), //                     X    //
};

inline Attr operator|(Attr a, Attr b) { return Attr((int)a | (int)b); }

inline const char* attrToVisibilityStr(Attr attr) {
  return (attr & AttrPrivate)   ? "private"   :
         (attr & AttrProtected) ? "protected" : "public";
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TYPES_H_
