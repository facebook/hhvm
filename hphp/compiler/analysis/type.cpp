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

#include "hphp/compiler/analysis/type.h"
#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/expression/expression.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/runtime.h"
#include <boost/format.hpp>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// statics

TypePtr Type::Null        (new Type(Type::KindOfVoid        ));
TypePtr Type::Boolean     (new Type(Type::KindOfBoolean     ));
TypePtr Type::Int32       (new Type(Type::KindOfInt32       ));
TypePtr Type::Int64       (new Type(Type::KindOfInt64       ));
TypePtr Type::Double      (new Type(Type::KindOfDouble      ));
TypePtr Type::String      (new Type(Type::KindOfString      ));
TypePtr Type::Array       (new Type(Type::KindOfArray       ));
TypePtr Type::Object      (new Type(Type::KindOfObject      ));
TypePtr Type::Resource    (new Type(Type::KindOfResource    ));
TypePtr Type::Variant     (new Type(Type::KindOfVariant     ));

TypePtr Type::Numeric     (new Type(Type::KindOfNumeric     ));
TypePtr Type::PlusOperand (new Type(Type::KindOfPlusOperand ));
TypePtr Type::Primitive   (new Type(Type::KindOfPrimitive   ));
TypePtr Type::Sequence    (new Type(Type::KindOfSequence    ));

TypePtr Type::AutoSequence(new Type(Type::KindOfAutoSequence));
TypePtr Type::AutoObject  (new Type(Type::KindOfAutoObject  ));

TypePtr Type::Any         (new Type(Type::KindOfAny         ));
TypePtr Type::Some        (new Type(Type::KindOfSome        ));

Type::TypePtrMap Type::s_TypeHintTypes;
Type::TypePtrMap Type::s_HHTypeHintTypes;

void Type::InitTypeHintMap() {
  assert(s_TypeHintTypes.empty());
  assert(s_HHTypeHintTypes.empty());

  s_TypeHintTypes["array"] = Type::Array;
  s_TypeHintTypes["resource"] = Type::Resource;
  s_TypeHintTypes["callable"] = Type::Variant;

  s_HHTypeHintTypes["array"] = Type::Array;
  s_HHTypeHintTypes["bool"]    = Type::Boolean;
  s_HHTypeHintTypes["boolean"] = Type::Boolean;
  s_HHTypeHintTypes["int"]     = Type::Int64;
  s_HHTypeHintTypes["integer"] = Type::Int64;
  s_HHTypeHintTypes["real"]    = Type::Double;
  s_HHTypeHintTypes["double"]  = Type::Double;
  s_HHTypeHintTypes["float"]   = Type::Double;
  s_HHTypeHintTypes["string"]  = Type::String;
  s_HHTypeHintTypes["resource"] = Type::Resource;
  s_HHTypeHintTypes["callable"] = Type::Variant;
}

const Type::TypePtrMap &Type::GetTypeHintTypes(bool hhType) {
  return hhType ? s_HHTypeHintTypes : s_TypeHintTypes;
}

void Type::ResetTypeHintTypes() {
  s_TypeHintTypes.clear();
  s_HHTypeHintTypes.clear();
}

TypePtr Type::CreateObjectType(const std::string &clsname) {
  // For interfaces that support primitive types, we're pessimistic and
  // we treat it as a Variant
  if (interface_supports_array(clsname) ||
      interface_supports_string(clsname) ||
      interface_supports_int(clsname) ||
      interface_supports_double(clsname)) {
    return Type::Variant;
  }
  return TypePtr(new Type(KindOfObject, clsname));
}

TypePtr Type::GetType(KindOf kindOf, const std::string &clsname /* = "" */) {
  assert(kindOf);
  if (!clsname.empty()) {
    // For interfaces that support primitive types we're pessimistic and
    // we treat it as a Variant
    if (interface_supports_array(clsname) ||
        interface_supports_string(clsname) ||
        interface_supports_int(clsname) ||
        interface_supports_double(clsname)) {
      return Type::Variant;
    }
    return TypePtr(new Type(kindOf, clsname));
  }

  switch (kindOf) {
  case KindOfBoolean:     return Type::Boolean;
  case KindOfInt32:       return Type::Int32;
  case KindOfInt64:       return Type::Int64;
  case KindOfDouble:      return Type::Double;
  case KindOfString:      return Type::String;
  case KindOfArray:       return Type::Array;
  case KindOfVariant:     return Type::Variant;
  case KindOfObject:      return Type::Object;
  case KindOfResource:    return Type::Resource;
  case KindOfNumeric:     return Type::Numeric;
  case KindOfPrimitive:   return Type::Primitive;
  case KindOfPlusOperand: return Type::PlusOperand;
  case KindOfSequence:    return Type::Sequence;
  case KindOfSome:        return Type::Some;
  case KindOfAny:         return Type::Any;
  default:                return TypePtr(new Type(kindOf));
  }
}

TypePtr Type::Intersection(AnalysisResultConstPtr ar,
                           TypePtr from, TypePtr to) {
  // Special case: if we're casting to Some or Any, return the "from" type;
  // if we're casting to Variant, return Variant.
  if (to->m_kindOf == KindOfSome || to->m_kindOf == KindOfAny) {
    return from;
  } else if (to->m_kindOf == KindOfVariant) {
    return Variant;
  }

  int resultKind = to->m_kindOf & from->m_kindOf;
  std::string resultName = "";

  if (resultKind & KindOfObject) {
    // if they're the same, or we don't know one's name, then use
    // the other
    if (to->m_name == from->m_name || from->m_name.empty()) {
      resultName = to->m_name;
    } else if (to->m_name.empty()) {
      resultName = from->m_name;
    } else {
      // make sure there's a subclass relation
      ClassScopePtr cls = ar->findClass(from->m_name);
      if (cls) {
        if (cls->derivesFrom(ar, to->m_name, true, false)) {
          resultName = to->m_name;
        } else {
          resultKind &= ~KindOfObject;
        }
      }
    }
  }

  TypePtr res;

  // If there is overlap (for instance, they were the same, or we've narrowed
  // down something like Sequenece to be more specific), then return the
  // intersection of the types.
  if (resultKind) {
    res = GetType(resultKind, resultName);
  } else if (from->mustBe(KindOfObject) && to->m_kindOf == KindOfPrimitive) {
    // Special case Object -> Primitive: can we tostring it?
    if (!from->m_name.empty()) {
      ClassScopePtr cls = ar->findClass(from->m_name);
      if (cls && cls->findFunction(ar, "__tostring", true)) {
        res = Type::String;
      }
    }

    // Otherwise, return Int32
    res = Int32;
  } else if (from->m_kindOf == KindOfBoolean
             && to->mustBe(KindOfNumeric | KindOfArray | KindOfString)
             && !IsExactType(to->m_kindOf)) {
    res = Int32;
  } else {
    res = to;
  }

  if (from->mustBe(KindOfBoolean) && to->m_kindOf == KindOfPrimitive) {
    res = Int32;
  }

  return res;
}

bool Type::IsMappedToVariant(TypePtr t) {
  if (!t) return true;
  switch (t->m_kindOf) {
  case KindOfBoolean:
  case KindOfInt32  :
  case KindOfInt64  :
  case KindOfDouble :
  case KindOfString :
  case KindOfArray  :
  case KindOfObject :
  case KindOfResource:
    return false;
  default: break;
  }
  return true;
}

bool Type::IsCastNeeded(AnalysisResultConstPtr ar, TypePtr from, TypePtr to) {
  if (SameType(from, to)) return false;
  if (!from->m_kindOf) return true;
  if (!to->m_kindOf) return true;

  // Special case: all Sequence operations are implemented on both String and
  // Array, and vice versa, therefore no need to cast between these types.
  if ((from->m_kindOf == KindOfSequence && to->mustBe(KindOfSequence))
   || (to->m_kindOf == KindOfSequence && from->mustBe(KindOfSequence))) {
    return false;
  }

  switch (to->m_kindOf) {
  case KindOfVariant:
  case KindOfNumeric:
  case KindOfPrimitive:
  case KindOfPlusOperand:
  case KindOfSome:
  case KindOfSequence:
  case KindOfAny:
    // Currently these types are all mapped to Variant in runtime/base, and
    // that's why these casting are not needed.
    return false;
  case KindOfObject:
    if (from->m_kindOf == KindOfObject && to->m_name.empty() &&
        !from->m_name.empty()) return false;
    else return true;
  default:
    // if we don't have a specific type narrowed down, then
    // it will be a Variant at at runtime, so no cast is needed.
    return IsExactType(to->m_kindOf);
  }
}

bool Type::IsCoercionNeeded(AnalysisResultConstPtr ar, TypePtr t1, TypePtr t2) {
  if (t1->m_kindOf == KindOfSome ||
      t1->m_kindOf == KindOfAny ||
      t2->m_kindOf == KindOfSome ||
      t2->m_kindOf == KindOfAny) return true;

  // special case: we always coerce to a specific object type so we can
  // type checking properties and methods
  if (t1->m_kindOf == KindOfObject && !t1->m_name.empty() &&
      t2->m_kindOf == KindOfObject && t2->m_name.empty()) {
    return true;
  }

  return !Type::IsLegalCast(ar, t1, t2);
}

TypePtr Type::Coerce(AnalysisResultConstPtr ar, TypePtr type1, TypePtr type2) {
  if (SameType(type1, type2)) return type1;
  if (type1->m_kindOf == KindOfVariant ||
      type2->m_kindOf == KindOfVariant) return Type::Variant;
  if (type1->m_kindOf > type2->m_kindOf) {
    TypePtr tmp = type1;
    type1 = type2;
    type2 = tmp;
  }
  if (type1->m_kindOf == KindOfVoid &&
      (type2->m_kindOf == KindOfString ||
       type2->m_kindOf == KindOfArray ||
       type2->m_kindOf == KindOfObject ||
       type2->m_kindOf == KindOfResource)) {
    return type2;
  }
  if (type2->m_kindOf == KindOfSome ||
      type2->m_kindOf == KindOfAny) return type1;

  if (type2->m_kindOf & KindOfAuto) {
    if (type1->mustBe(type2->m_kindOf & ~KindOfAuto)) {
      if (!(type1->m_kindOf & Type::KindOfString)) {
        return type1;
      }
      if (type2->m_kindOf == KindOfAutoSequence) {
        return Type::Sequence;
      }
      return GetType((KindOf)(type2->m_kindOf & ~KindOfAuto));
    }
    return Type::Variant;
  }

  if (type1->mustBe(KindOfInteger)) {
    if (type2->mustBe(KindOfInteger)) {
      return type2;
    } else if (type2->mustBe(KindOfDouble)) {
      return Type::Numeric;
    }
  }

  if (type1->mustBe(Type::KindOfObject) &&
      type2->mustBe(Type::KindOfObject)) {
    if (type1->m_name.empty()) return type1;
    if (type2->m_name.empty()) return type2;
    ClassScopePtr cls1 = ar->findClass(type1->m_name);
    if (cls1 && !cls1->isRedeclaring() &&
        cls1->derivesFrom(ar, type2->m_name, true, false)) {
      return type2;
    }
    ClassScopePtr cls2 = ar->findClass(type2->m_name);
    if (cls2 && !cls2->isRedeclaring() &&
        cls2->derivesFrom(ar, type1->m_name, true, false)) {
      return type1;
    }
    if (cls1 && cls2 &&
        !cls1->isRedeclaring() && !cls2->isRedeclaring()) {
      ClassScopePtr parent =
        ClassScope::FindCommonParent(ar, type1->m_name,
                                         type2->m_name);
      if (parent) {
        return Type::CreateObjectType(parent->getName());
      }
    }
    return Type::Object;
  }

  if (type1->mustBe(type2->m_kindOf)) {
    return type2;
  }

  static_assert(Type::KindOfString < Type::KindOfArray,
                "Expected Type::KindOfString < Type::KindOfArray");
  if (type1->m_kindOf == Type::KindOfString &&
      type2->m_kindOf == Type::KindOfArray) {
    return Type::Sequence;
  }

  return Type::Variant;
}

TypePtr Type::Union(AnalysisResultConstPtr ar, TypePtr type1, TypePtr type2) {
  if (SameType(type1, type2)) {
    return type1;
  }

  int resultKind = type1->m_kindOf | type2->m_kindOf;
  if (resultKind == KindOfObject) {
    std::string resultName("");

    // if they're the same, or we don't know one's name, then use
    // the other
    if (type1->m_name == type2->m_name) {
      resultName = type1->m_name;
    } else if (type1->m_name.empty() || type2->m_name.empty()) {
      // resultName was initialized to "", so leave it as such;
      // we know it's an object but not what kind.
    } else {
      // take the superclass
      ClassScopePtr res =
        ClassScope::FindCommonParent(ar, type1->m_name,
                                         type2->m_name);
      if (res) resultName = res->getName();
    }
    return TypePtr(Type::CreateObjectType(resultName));
  }

  return GetType((KindOf)resultKind);
}

bool Type::SameType(TypePtr type1, TypePtr type2) {
  if (!type1 && !type2) return true;
  if (!type1 || !type2) return false;
  if (type1->m_kindOf == type2->m_kindOf) {
    if ((type1->m_kindOf & KindOfObject) &&
        type1->m_name != type2->m_name) return false;
    return true;
  }
  return false;
}

bool Type::SubType(AnalysisResultConstPtr ar, TypePtr type1, TypePtr type2) {
  if (!type1 && !type2) return true;
  if (!type1 || !type2) return false;
  if (type1->m_kindOf != type2->m_kindOf) return false;
  if (!(type1->m_kindOf & KindOfObject)) return true;
  // both are objects...
  if (type1->m_name == type2->m_name) return true;
  // ... with different classnames; check subtype relationship.
  ClassScopePtr cls1 = ar->findClass(type1->m_name);
  return cls1 && !cls1->isRedeclaring() &&
         cls1->derivesFrom(ar, type2->m_name, true, false);
}

bool Type::IsExactType(KindOf kindOf) {
  // clever trick thanks to mwilliams - this will evaluate
  // to true iff exactly one bit is set in kindOf
  return kindOf && !(kindOf & (kindOf-1));
}

bool Type::HasFastCastMethod(TypePtr t) {
  switch (t->getKindOf()) {
  case Type::KindOfBoolean:
  case Type::KindOfInt32:
  case Type::KindOfInt64:
  case Type::KindOfDouble:
  case Type::KindOfString:
  case Type::KindOfArray:
  case Type::KindOfObject:
  case Type::KindOfResource:
    return true;
  default: break;
  }
  return false;
}

string Type::GetFastCastMethod(
    TypePtr dst, bool allowRef, bool forConst) {
  const char *prefix0 = allowRef ? "to" : "as";
  const char *prefix1 = forConst ? "C"  : "";
  const char *prefix2 = "Ref";
  const char *type ATTRIBUTE_UNUSED;

  switch (dst->getKindOf()) {
  case Type::KindOfBoolean:
  case Type::KindOfInt32:
  case Type::KindOfInt64:
  case Type::KindOfDouble:
    prefix0 = "to";
    prefix1 = "";
    prefix2 = "Val";
    break;
  default: break;
  }

  switch (dst->getKindOf()) {
  case Type::KindOfBoolean:
    type = "Boolean";
    break;
  case Type::KindOfInt32:
  case Type::KindOfInt64:
    type = "Int64";
    break;
  case Type::KindOfDouble:
    type = "Double";
    break;
  case Type::KindOfString:
    type = "Str";
    break;
  case Type::KindOfArray:
    type = "Arr";
    break;
  case Type::KindOfObject:
    type = "Obj";
    break;
  case Type::KindOfResource:
    type = "Res";
    break;
  default:
    type = ""; // make the compiler happy
    assert(false);
    break;
  }

  return string(prefix0) + string(prefix1) + string(type) + string(prefix2);
}

/* This new IsLegalCast returns true in a few cases where the old version
 * (which was basically a hardcoded truth table) returned false; it seems
 * like "true" is in fact the right thing to return. The cases that appear
 * when compiling www are:
 *   Sequence -> Array
 *   PlusOperand -> Array
 *   String -> PlusOperand
 *   Boolean -> PlusOperand
 */

bool Type::IsLegalCast(AnalysisResultConstPtr ar, TypePtr from, TypePtr to) {
  if (!from->m_kindOf) return true;

  // since both 'from' and 'to' represent sets of types, we do
  // this by computing the set of types that we could possibly cast 'from'
  // to, and then determining whether that overlaps with 'to'.
  int canCastTo = KindOfBoolean | from->m_kindOf;

  if (from->m_kindOf & KindOfVoid) canCastTo |= KindOfVoid;

  // Boolean, Numeric, and String can all be cast among each other
  if (from->m_kindOf & (KindOfBoolean | KindOfNumeric | KindOfString)) {
    canCastTo |= KindOfNumeric | KindOfString;
  }

  if (from->m_kindOf & KindOfObject) {
    // Objects can only cast to string if they have __tostring
    if (from->m_name.empty()) {
      canCastTo |= KindOfString; // we don't know which class it is
    } else {
      ClassScopePtr cls = ar->findClass(from->m_name);
      if (!cls || cls->isRedeclaring() ||
          cls->findFunction(ar, "__tostring", true)) {
        canCastTo |= KindOfString;
      }
    }

    // can only cast between objects if there's a subclass relation
    if ((to->m_kindOf & KindOfObject)  && !to->m_name.empty() &&
        !from->m_name.empty() && to->m_name != from->m_name) {
      ClassScopePtr cls = ar->findClass(from->m_name);
      if (cls && (cls->isRedeclaring() ||
                  !cls->derivesFrom(ar, to->m_name, true, true))) {
        canCastTo &= ~KindOfObject;
      }

    }
  }

  bool overlap = (to->m_kindOf & canCastTo);
  return overlap;
}

///////////////////////////////////////////////////////////////////////////////

Type::Type(KindOf kindOf) : m_kindOf(kindOf) {
}

Type::Type(KindOf kindOf, const std::string &name)
  : m_kindOf(kindOf), m_name(name) {

  // m_name must not be empty only when this type could
  // be an object
  assert(m_name.empty() || (m_kindOf & KindOfObject));
}

bool Type::isInteger() const {
  switch (m_kindOf) {
  case KindOfInt32:
  case KindOfInt64:
    return true;
  default:
    break;
  }
  return false;
}

bool Type::isStandardObject() const {
  return m_kindOf == KindOfObject && m_name.empty();
}

bool Type::isSpecificObject() const {
  return m_kindOf == KindOfObject && !m_name.empty();
}

bool Type::isNonConvertibleType() const {
  return m_kindOf == KindOfObject || m_kindOf == KindOfResource ||
         m_kindOf == KindOfArray;
}

bool Type::isNoObjectInvolved() const {
  if (couldBe(KindOfObject)
   || couldBe(KindOfResource)
   || couldBe(KindOfArray))
    return false;
  else
    return true;
}

TypePtr Type::combinedArithmeticType(TypePtr t1, TypePtr t2) {
  KindOf kind = KindOfAny;

  if ((t1 && t1->is(Type::KindOfArray)) ||
      (t2 && t2->is(Type::KindOfArray))) {
    return TypePtr();
  }

  if (t1 && t1->isPrimitive()) {
    if (t2 && t2->isPrimitive()) {
      if (t2->getKindOf() > t1->getKindOf()) {
        kind = t2->getKindOf();
      } else {
        kind = t1->getKindOf();
      }
    } else if (t1->is(KindOfDouble)) {
      kind = KindOfDouble;
    } else {
      kind = KindOfNumeric;
    }
  } else if (t2 && t2->isPrimitive()) {
    if (t2->is(KindOfDouble)) {
      kind = KindOfDouble;
    } else {
      kind = KindOfNumeric;
    }
  } else if ((t1 && t1->mustBe(KindOfNumeric)) ||
             (t2 && t2->mustBe(KindOfNumeric))) {
    kind = KindOfNumeric;
  }

  if (kind < KindOfInt64) {
    kind = KindOfInt64;
  }

  if (kind != KindOfAny) {
    return GetType(kind);
  }

  return TypePtr();
}

///////////////////////////////////////////////////////////////////////////////

ClassScopePtr Type::getClass(AnalysisResultConstPtr ar,
                             BlockScopeRawPtr scope) const {
  if (m_name.empty()) return ClassScopePtr();
  ClassScopePtr cls = ar->findClass(m_name);
  if (cls && cls->isRedeclaring()) {
    if (!scope) {
      cls.reset();
    } else {
      cls = scope->findExactClass(cls);
    }
  }
  return cls;
}

DataType Type::getDataType() const {
  switch (m_kindOf) {
    case KindOfBoolean:     return HPHP::KindOfBoolean;
    case KindOfInt32:
    case KindOfInt64:       return HPHP::KindOfInt64;
    case KindOfDouble:      return HPHP::KindOfDouble;
    case KindOfString:      return HPHP::KindOfString;
    case KindOfArray:       return HPHP::KindOfArray;
    case KindOfObject:      return HPHP::KindOfObject;
    case KindOfResource:    return HPHP::KindOfResource;
    case KindOfNumeric:
    case KindOfPrimitive:
    case KindOfPlusOperand:
    case KindOfSequence:
    default:                return HPHP::KindOfUnknown;
  }
}

// This is similar to getDataType() except that it returns
// HPHP::KindOfNull for KindOfVoid;
DataType Type::getHhvmDataType() const {
  switch (m_kindOf) {
    case KindOfVoid:        return HPHP::KindOfNull;
    default:                return getDataType();
  }
}

std::string Type::getPHPName() {
  switch (m_kindOf) {
  case KindOfArray:       return "array";
  case KindOfObject:      return m_name;
  case KindOfResource:    return "resource";
  default: break;
  }
  return "";
}

std::string Type::toString() const {
  switch (m_kindOf) {
  case KindOfBoolean:     return "Boolean";
  case KindOfInt32:       return "Int32";
  case KindOfInt64:       return "Int64";
  case KindOfDouble:      return "Double";
  case KindOfString:      return "String";
  case KindOfArray:       return "Array";
  case KindOfVariant:     return "Variant";
  case KindOfSome:        return "Some";
  case KindOfAny:         return "Any";
  case KindOfObject:      return string("Object - ") + m_name;
  case KindOfResource:    return "Resource";
  case KindOfNumeric:     return "Numeric";
  case KindOfPrimitive:   return "Primitive";
  case KindOfPlusOperand: return "PlusOperand";
  case KindOfSequence:    return "Sequence";
  default:
    return boost::str(boost::format("[0x%x]") % m_kindOf);
  }
  return "(unknown)";
}

void Type::Dump(TypePtr type, const char *fmt /* = "%s" */) {
  printf(fmt, type ? type->toString().c_str() : "(null)");
}

void Type::Dump(ExpressionPtr exp) {
  Dump(exp->getExpectedType(), "Expected: %s\t");
  Dump(exp->getActualType(), "Actual: %s\n");
}

void Type::serialize(JSON::CodeError::OutputStream &out) const {
  out << toString();
}

void Type::serialize(JSON::DocTarget::OutputStream &out) const {
  string s("any");
  switch (m_kindOf) {
  case KindOfBoolean:     s = "boolean"; break;
  case KindOfInt32:
  case KindOfInt64:       s = "integer"; break;
  case KindOfDouble:      s = "double"; break;
  case KindOfString:      s = "string"; break;
  case KindOfArray:       s = "array"; break;
  case KindOfVariant:
  case KindOfSome:
  case KindOfAny:         s = "any"; break;
  case KindOfObject:
  {
    if (m_name.empty()) s = "object";
    else {
      ClassScopePtr c(getClass(out.analysisResult(), BlockScopeRawPtr()));
      if (c) {
        s = c->getOriginalName();
      } else {
        s = "object";
      }
    }
    break;
  }
  case KindOfResource:    s = "resource"; break;
  case KindOfNumeric:     s = "numeric"; break;
  case KindOfPrimitive:   s = "primitive"; break;
  case KindOfPlusOperand: s = "any"; break;
  case KindOfSequence:    s = "sequence"; break;
  }
  out << s;
}

void Type::count(std::map<std::string, int> &counts) {
  if (is(Type::KindOfObject)) {
    if (isSpecificObject()) {
      counts["Object - Specific"]++;
    } else {
      counts["Object"]++;
    }
  } else {
    counts[toString()]++;
  }

  if (IsExactType(m_kindOf)) {
    counts["_strong"]++;
  } else {
    counts["_weak"]++;
  }

  counts["_all"]++;
}

TypePtr Type::InferredObject(AnalysisResultConstPtr ar,
                             TypePtr type1,
                             TypePtr type2) {
  assert(type1->m_kindOf == KindOfObject);
  assert(type2->m_kindOf == KindOfObject);

  TypePtr resultType = Type::Object;
  // if they're the same, or we don't know one's name, then use
  // the other
  if (type1->m_name == type2->m_name || type1->m_name.empty()) {
    resultType = type2;
  } else if (type2->m_name.empty()) {
    resultType = type1;
  } else {
    // take the subclass
    ClassScopePtr cls1 = ar->findClass(type1->m_name);
    ClassScopePtr cls2 = ar->findClass(type2->m_name);
    bool c1ok = cls1 && !cls1->isRedeclaring();
    bool c2ok = cls2 && !cls2->isRedeclaring();

    if (c1ok && cls1->derivesFrom(ar, type2->m_name, true, false)) {
      resultType = type1;
    } else if (c2ok && cls2->derivesFrom(ar, type1->m_name, true, false)) {
      resultType = type2;
    } else if (c1ok && c2ok && cls1->derivedByDynamic() &&
               cls2->derivesFromRedeclaring()) {
      resultType = type2;
    } else {
      resultType = type1;
    }
  }
  return resultType;
}

/* We have inferred type1 and type2 as the actual types for the same
   expression.
   Check that the types are compatible (it cant be both a string and
   an integer, for example), and return the combined type. If they
   are not compatible, return a null pointer.
 */
TypePtr Type::Inferred(AnalysisResultConstPtr ar,
                       TypePtr type1, TypePtr type2) {
  if (!type1) return type2;
  if (!type2) return type1;
  KindOf k1 = type1->m_kindOf;
  KindOf k2 = type2->m_kindOf;

  if (k1 == k2) {
    return k1 == KindOfObject ?
      Type::InferredObject(ar, type1, type2) : type1;
  }

  // If one set is a subset of the other, return the subset.
  if ((k1 & k2) == k1) return type1;
  if ((k1 & k2) == k2) return type2;

  // If one type must be numeric and the other might be, then assume numeric
  if (type1->mustBe(KindOfNumeric) && type2->couldBe(KindOfNumeric)) {
    return type1;
  }
  if (type2->mustBe(KindOfNumeric) && type1->couldBe(KindOfNumeric)) {
    return type2;
  }

  // Otherwise, take the intersection
  int resultKind = type1->m_kindOf & type2->m_kindOf;
  if (resultKind == KindOfObject) {
    return Type::InferredObject(ar, type1, type2);
  }
  return resultKind ? GetType(resultKind) : TypePtr();
}
