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

#ifndef incl_RUNTIME_TYPE_H_
#define incl_RUNTIME_TYPE_H_

#include "runtime/vm/bytecode.h"

namespace HPHP {
namespace VM {
namespace Transl {

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

  Location(Space spc)
  : space(spc)
  , offset(0)
  { ASSERT(spc == This); }

  Location(Space spc, int64 off)
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

  std::string pretty() const {
    char buf[1024];
    sprintf(buf, "(Location %s %lld)", spaceName(), offset);
    return std::string(buf);
  }

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

public:
  Space space;
  int64 offset;
};

struct InputInfo {
  InputInfo(const Location &l)
    : loc(l),
      dontBreak(false),
      dontGuard(l.isLiteral()),
      dontGuardInner(false)
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
      union {
        // We may have even more precise data about this set of values.
        const StringData* string; // KindOfString: The exact value.
        const ArrayData* array;   // KindOfArray: The exact value.
        const Class* klass;       // KindOfObject: A known super-class.
                                  // KindOfClass: An instance of the current
                                  //   instantiation of the preClass.  The
                                  //   exact class may differ across executions
        int64 intval;             // KindOfInt64: A literal int
        struct {
          bool boolean;           // KindOfBoolean: A literal bool
                                  // from True or False.
          bool boolValid;
        };
      };
    } m_value;
    struct {
      Iter::Type type;
    } m_iter;
  };

  inline void consistencyCheck() const {
    ASSERT(m_kind == VALUE || m_kind == ITER);
    if (m_kind == VALUE) {
      ASSERT(m_value.innerType != KindOfRef);
      ASSERT(m_value.outerType == KindOfRef ||
             m_value.innerType == KindOfInvalid);
      ASSERT(m_value.outerType == KindOfString ||
             m_value.innerType == KindOfString ||
             m_value.outerType == KindOfClass ||
             m_value.innerType == KindOfClass ||
             m_value.outerType == KindOfObject ||
             m_value.innerType == KindOfObject ||
             m_value.outerType == KindOfArray ||
             m_value.innerType == KindOfArray ||
             m_value.outerType == KindOfBoolean ||
             m_value.outerType == KindOfInt64 ||
             m_value.klass == NULL);
      ASSERT(m_value.innerType != KindOfStaticString &&
             m_value.outerType != KindOfStaticString);
    }
  }

 public:
  RuntimeType(DataType outer, DataType inner = KindOfInvalid,
              const Class* = NULL);
  RuntimeType(const StringData*);
  RuntimeType(const ArrayData*);
  RuntimeType(const Class*);
  explicit RuntimeType(bool value);
  explicit RuntimeType(int64 value);
  RuntimeType(const RuntimeType& copy);
  RuntimeType();
  RuntimeType(const Iter* iter);
  RuntimeType(Iter::Type type);

  static const int UnknownBool = -1;

  // Specializers
  RuntimeType box() const;
  RuntimeType unbox() const;
  RuntimeType setValueType(DataType vt) const;

  // Accessors
  DataType outerType() const;
  DataType innerType() const;
  DataType valueType() const;
  const Class* valueClass() const;
  const StringData* valueString() const;
  const StringData* valueStringOrNull() const;
  const ArrayData* valueArray() const;
  int valueBoolean() const;
  int64 valueInt() const;
  int64 valueGeneric() const;
  Iter::Type iterType() const;

  // Helpers for typechecking
  int typeCheckOffset() const;
  DataType typeCheckValue() const;

  bool isValue() const;
  bool isIter() const;

  bool isVagueValue() const;
  bool isVariant() const;

  bool isRefCounted() const;
  bool isUninit() const;
  bool isNull() const;
  bool isInt() const;
  bool isDouble() const;
  bool isArray() const;
  bool isBoolean() const;
  bool isString() const;
  bool isObject() const;
  bool operator==(const RuntimeType& r) const;
  RuntimeType &operator=(const RuntimeType& r);
  size_t operator()(const RuntimeType& r) const; // hash function
  std::string pretty() const;
};

} } }

#endif // incl_RUNTIME_TYPE_H_
