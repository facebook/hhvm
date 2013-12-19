/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RUNTIME_TYPE_H_
#define incl_HPHP_RUNTIME_TYPE_H_

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/region-selection.h"

namespace HPHP {
namespace JIT {

// Location --
//   A user-program-visible, and bytecode ISA addressable place for a PHP
//   value.
struct Location {
  enum Space {
    Invalid,  // Unknown location
    Stack,    // Stack; offset == delta from top
    Local,    // Stack frame's registers; offset == local register
    Iter,     // Stack frame's iterators
    Litstr,   // Literal string pseudo-location
    Litint,   // Literal int pseudo-location
    This,     // $this in the current frame
  };

  explicit Location(Space spc)
    : space(spc)
    , offset(0)
  {
    assert(spc == This);
  }

  Location(Space spc, int64_t off)
    : space(spc)
    , offset(off)
  {}

  Location() : space(Invalid), offset(-1) {}

  int cmp(const Location &r) const {
#define CMP(field) do { \
  if (field > r.field)      { return 1; } \
  else if (field < r.field) { return -1; } } while(0)
    CMP(space);
    CMP(offset);
#undef CMP
    return 0;
  }

  bool operator==(const Location& r) const {
    return cmp(r) == 0;
  }

  bool operator!=(const Location& r) const {
    return cmp(r) != 0;
  }

  bool operator<(const Location& r) const {
    return cmp(r) < 0;
  }

  // Hash function.
  size_t operator()(const Location& l) const {
    return HPHP::hash_int64_pair(l.space, l.offset);
  }

  const char *spaceName() const {
    switch(space) {
    case Stack:  return "Stk";
    case Local:  return "Local";
    case Iter:   return "Iter";
    case Litstr: return "Litstr";
    case Litint: return "Litint";
    case This:   return "This";
    case Invalid:return "*invalid*";
    default:     not_reached();
    }
  }

  std::string pretty() const;

  bool isStack() const {
    return space == Stack;
  }

  bool isLocal() const {
    return space == Local;
  }

  bool isInvalid() const {
    return space == Invalid;
  }

  bool isValid() const {
    return !isInvalid();
  }

  bool isLiteral() const {
    return space == Litstr || space == Litint;
  }

  bool isThis() const {
    return space == This;
  }

  bool isIter() const {
    return space == Iter;
  }

  JIT::RegionDesc::Location toLocation(Offset spOffsetFromFp) const {
    typedef JIT::RegionDesc::Location L;
    switch (space) {
      case Stack: {
        auto offsetFromSp = safe_cast<uint32_t>(offset);
        return L::Stack{offsetFromSp, spOffsetFromFp - offsetFromSp};
      }
      case Local: return L::Local{safe_cast<uint32_t>(offset)};
      default:    not_reached();
    }
  }

public:
  Space space;
  int64_t offset;
};

struct InputInfo {
  explicit InputInfo(const Location &l)
    : loc(l)
    , dontBreak(false)
    , dontGuard(l.isLiteral())
    , dontGuardInner(false)
  {}

  std::string pretty() const {
    std::string p = loc.pretty();
    if (dontBreak) p += ":dc";
    if (dontGuard) p += ":dg";
    if (dontGuardInner) p += ":dgi";
    return p;
  }
  Location loc;
  /*
   * if an input is unknowable, dont break the tracelet
   * just to find its type. But still generate a guard
   * if that will tell us its type.
   */
  bool     dontBreak;
  /*
   * never break the tracelet, or generate a guard on
   * account of this input.
   */
  bool     dontGuard;
  /*
   * never guard the inner type if this input is KindOfRef
   */
  bool     dontGuardInner;
};

class InputInfos : public std::vector<InputInfo> {
 public:
  InputInfos() : needsRefCheck(false) {}

  std::string pretty() const {
    std::string retval;
    for (size_t i = 0; i < size(); i++) {
      retval += (*this)[i].pretty();
      if (i != size() - 1) {
        retval += string(" ");
      }
    }
    return retval;
  }
  bool  needsRefCheck;
};

// RuntimeType --
//
//   Symbolic description of a root location in the runtime: e.g., a stack,
//   local, global, etc.
class RuntimeType {
  enum Kind {
    VALUE,
    ITER
  } m_kind;
  union {
    struct {
      DataType outerType;
      DataType innerType;
      // Set when we want to transfer the type information to the
      // IR type system (Type object)
      union {
        const Class* knownClass;
        struct {
          bool arrayKindValid;
          ArrayData::ArrayKind arrayKind;
        };
      };
      union {
        // We may have even more precise data about this set of values.
        const StringData* string; // KindOfString: The exact value.
        const ArrayData* array;   // KindOfArray: The exact value.
        const Class* klass;       // KindOfObject: A known super-class.
                                  // KindOfClass: An instance of the current
                                  //   instantiation of the preClass.  The
                                  //   exact class may differ across executions
        int64_t intval;             // KindOfInt64: A literal int
        struct {
          bool boolean;           // KindOfBoolean: A literal bool
                                  // from True or False.
          bool boolValid;
        };
      };
    } m_value;
  };

  inline void consistencyCheck() const {
    assert(m_kind == VALUE || m_kind == ITER);
    if (m_kind == VALUE) {
      assert(m_value.innerType != KindOfRef);
      assert((m_value.outerType == KindOfRef) ==
             (m_value.innerType != KindOfNone));
      assert(m_value.outerType == KindOfString ||
             m_value.innerType == KindOfString ||
             m_value.outerType == KindOfClass ||
             m_value.innerType == KindOfClass ||
             m_value.outerType == KindOfObject ||
             m_value.innerType == KindOfObject ||
             m_value.outerType == KindOfResource ||
             m_value.innerType == KindOfResource ||
             m_value.outerType == KindOfArray ||
             m_value.innerType == KindOfArray ||
             m_value.outerType == KindOfBoolean ||
             m_value.outerType == KindOfInt64 ||
             m_value.klass == nullptr);
      assert(m_value.innerType != KindOfStaticString &&
             m_value.outerType != KindOfStaticString);
      assert((m_value.knownClass == nullptr ||
              m_value.outerType == KindOfObject ||
              (m_value.outerType == KindOfRef &&
               m_value.innerType == KindOfObject)) ||
             (!m_value.arrayKindValid ||
              m_value.outerType == KindOfArray ||
              (m_value.outerType == KindOfRef &&
               m_value.innerType == KindOfArray)));
    }
  }

  void init(DataType outer,
            DataType inner = KindOfNone,
            const Class* klass = nullptr);

 public:
  explicit RuntimeType(DataType outer, DataType inner = KindOfNone,
                       const Class* = nullptr);
  explicit RuntimeType(const StringData*);
  explicit RuntimeType(const ArrayData*);
  explicit RuntimeType(const Class*);
  explicit RuntimeType(bool value);
  explicit RuntimeType(int64_t value);
  RuntimeType(const RuntimeType& copy) = default;
  RuntimeType();
  explicit RuntimeType(const Iter* iter);
  explicit RuntimeType(ArrayIter::Type type);

  static const int UnknownBool = -1;

  // Specializers
  RuntimeType box() const;
  RuntimeType unbox() const;
  RuntimeType setValueType(DataType vt) const;
  RuntimeType setKnownClass(const Class* klass) const;
  RuntimeType setArrayKind(ArrayData::ArrayKind arrayKind) const;

  // Accessors
  DataType outerType() const;
  DataType innerType() const;
  DataType valueType() const;
  const Class* valueClass() const;
  const StringData* valueString() const;
  const StringData* valueStringOrNull() const;
  const ArrayData* valueArray() const;
  int valueBoolean() const;
  int64_t valueInt() const;
  int64_t valueGeneric() const;
  const Class* knownClass() const;
  bool hasArrayKind() const;
  ArrayData::ArrayKind arrayKind() const;

  // Helpers for typechecking
  DataType typeCheckValue() const;

  bool isValue() const;
  bool isIter() const;

  bool isVagueValue() const;
  bool isRef() const;

  bool isRefCounted() const;
  bool isUninit() const;
  bool isNull() const;
  bool isInt() const;
  bool isDouble() const;
  bool isArray() const;
  bool isBoolean() const;
  bool isString() const;
  bool isObject() const;
  bool isClass() const;
  bool hasKnownClass() const;
  bool operator==(const RuntimeType& r) const;
  RuntimeType &operator=(const RuntimeType& r) = default;
  size_t operator()(const RuntimeType& r) const; // hash function
  std::string pretty() const;
};

} }

#endif // incl_HPHP_RUNTIME_TYPE_H_
