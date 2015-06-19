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

#include <cstdint>
#include <limits>

#include "hphp/util/low-ptr.h"
#include "hphp/runtime/base/header-kind.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class String;
class StaticString;
class Array;
class Object;
class Resource;
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
class Func;

class VariableSerializer;
class VariableUnserializer;

using LowStringPtr = LowPtr<const StringData>;

///////////////////////////////////////////////////////////////////////////////

using VRefParam = const class VRefParamValue&;
using RefResult = const class RefResultValue&;

inline const Variant& variant(RefResult v)      {
  return *(Variant*)&v;
}

inline const Variant& variant(const Variant& v) {
  return v;
}

/**
 * ref() can be used to cause strong binding.
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

struct AccessFlags {
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
using PC = const unsigned char*;

/*
 * Id type for various components of a unit that have to have unique
 * identifiers.  For example, function ids, class ids, string literal
 * ids.
 */
using Id = int;
constexpr Id kInvalidId = -1;

/*
 * Translation IDs.
 *
 * These represent compilation units for the JIT, and are used to key into
 * several runtime structures for finding profiling data or tracking
 * translation information.
 *
 * Because we often convert between JIT block IDs and TransIDs, these are
 * signed integers (blocks can have negative IDs).  However, negative block IDs
 * logically correspond to blocks without associated translations---hence,
 * negative TransID's are simply invalid.
 *
 * kInvalidTransID should be used when initializing or checking against a
 * sentinel value.  To ask if a TransID is meaningful in a translation context,
 * use isValidTransID().
 */
using TransID = int32_t;
constexpr TransID kInvalidTransID = -1;

constexpr bool isValidTransID(TransID transID) {
  return transID >= 0;
}

/*
 * Bytecode offsets.  Used for both absolute offsets and relative offsets.
 */
using Offset = int32_t;
constexpr Offset kInvalidOffset = std::numeric_limits<Offset>::max();

/*
 * Various fields in the VM's runtime have indexes that are addressed
 * using this "slot" type.  For example: methods, properties, class
 * constants.
 *
 * No slot value greater than or equal to kInvalidSlot will actually
 * be used for one of these.
 */
using Slot = uint32_t;
constexpr Slot kInvalidSlot = -1;

/*
 * Handles into Request Data Segment.  These are offsets from
 * rds::tl_base.  See rds.h.
 */
namespace rds {
  using Handle = uint32_t;
  constexpr Handle kInvalidHandle = 0;
}

/*
 * Unique identifier for a Func*.
 */
using FuncId = uint32_t;
constexpr FuncId InvalidFuncId = -1;
constexpr FuncId DummyFuncId = -2;

///////////////////////////////////////////////////////////////////////////////
}

#endif
