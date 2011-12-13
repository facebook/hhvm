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

namespace HPHP {
namespace VM {
namespace Transl {

// Location --
//   A user-program-visible, and bytecode ISA addressable place for a PHP
//   value.
struct Location {
  enum Space {
    Invalid,  // Unknown location;
    Stack,    // Stack; offset == delta from top
    Local,    // Stack frame's registers; offset == local register
    Iter,     // Stack frame's iterators
  };
  Space space;
  int offset;

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

  Location(Space spc, int off) : space(spc), offset(off) { }

  // Hash function.
  size_t operator()(const Location& l) const {
    return HPHP::hash_int64_pair(l.space, l.offset);
  }

  Location() : space(Invalid), offset(-1) { }

  const char *spaceName() const {
    switch(space) {
    case Stack:  return "Stk";
    case Local:  return "Local";
    case Iter:   return "Iter";
    case Invalid:
    default:     return "*invalid*";
    }
  }

  std::string pretty() const {
    char buf[1024];
    sprintf(buf, "(Location %s %d)", spaceName(), offset);
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
};

// RuntimeType --
//
//   Symbolic description of a root location in the runtime: e.g., a stack,
//   local, global, etc. There is a hard-coded three level hierarchy of
//   indirection: the most indirect types represent homes of vars pointing to
//   primitive types. The home layer of indirection is handled a bit
//   differently, because homes can never be referred to by other homes or
//   vars, and because homes have associated runtime locations which can be
//   precisely computed at runtime.
class RuntimeType {
  enum Kind {
    VALUE,
    HOME,
    ITER
  } m_kind;
  union {
    struct {
      DataType outerType;
      DataType innerType;
      union {
        // We may have even more precise data about this set of values.
        const StringData* string; // KindOfString: The exact value.
        const Class* klass;       // KindOfObject: A known super-class.
                                  // KindOfClass: An instance of the current
                                  //   instantiation of the preClass.  The
                                  //   exact class may differ across executions
      };
    } m_value;
    struct {
      Iter::Type type;
    } m_iter;
  };
  // Cannot be part of the union above since Location has a constructor.
  Location m_homeLoc;

  inline void consistencyCheck() const {
    ASSERT(m_kind == VALUE || m_kind == HOME || m_kind == ITER);
    if (m_kind == VALUE) {
      ASSERT(m_homeLoc == Location());
      ASSERT(m_value.outerType != KindOfHome);
      ASSERT(m_value.innerType != KindOfHome);
      ASSERT(m_value.innerType != KindOfVariant);
      ASSERT(m_value.outerType == KindOfVariant ||
             m_value.innerType == KindOfInvalid);
      ASSERT(m_value.outerType == KindOfString ||
             m_value.innerType == KindOfString ||
             m_value.outerType == KindOfClass ||
             m_value.innerType == KindOfClass ||
             m_value.outerType == KindOfObject ||
             m_value.innerType == KindOfObject ||
             m_value.klass == NULL);
    }
  }

 public:
  RuntimeType(DataType outer, DataType inner = KindOfInvalid, const Class* = NULL);
  RuntimeType(const StringData*);
  RuntimeType(const Class*);
  RuntimeType(const RuntimeType& copy);
  RuntimeType(const Location& l);
  RuntimeType();
  RuntimeType(const Iter* iter);

  // Specializers
  //   E.g., RuntimeType(KindOfInt64).box().homeAt(Location(Local, 0))
  //     represents a home for the Int64 at location 0.
  RuntimeType box() const;
  RuntimeType unbox() const;
  RuntimeType setValueType(DataType vt) const;

  // Accessors
  DataType outerType() const;
  DataType innerType() const;
  DataType valueType() const;
  const Class* valueClass() const;
  const StringData* valueString() const;
  Iter::Type iterType() const;

  // Helpers for typechecking
  int typeCheckOffset() const;
  DataType typeCheckValue() const;

  bool isValue() const;
  bool isHome() const;
  bool isIter() const;

  bool isVagueValue() const;
  bool isVariant() const;

  Location homeLocation() const;
  bool isRefCounted() const;
  bool isNull() const;
  bool isInt() const;
  bool isString() const;
  bool operator==(const RuntimeType& r) const;
  RuntimeType &operator=(const RuntimeType& r);
  size_t operator()(const RuntimeType& r) const; // hash function
  std::string pretty() const;
};

} } }

#endif // incl_RUNTIME_TYPE_H_
