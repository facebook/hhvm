/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <compiler/analysis/type.h>
#include <compiler/code_generator.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/expression/expression.h>

using namespace HPHP;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
// statics

TypePtr Type::Boolean(new Type(Type::KindOfBoolean  ));
TypePtr Type::Byte   (new Type(Type::KindOfByte     ));
TypePtr Type::Int16  (new Type(Type::KindOfInt16    ));
TypePtr Type::Int32  (new Type(Type::KindOfInt32    ));
TypePtr Type::Int64  (new Type(Type::KindOfInt64    ));
TypePtr Type::Double (new Type(Type::KindOfDouble   ));
TypePtr Type::String (new Type(Type::KindOfString   ));
TypePtr Type::Array  (new Type(Type::KindOfArray    ));
TypePtr Type::Variant(new Type(Type::KindOfVariant  ));

TypePtr Type::CreateType(KindOf uncertain) {
  switch (uncertain) {
  case KindOfObject:
  case KindOfNumeric:
  case KindOfPrimitive:
  case KindOfPlusOperand:
  case KindOfSequence:
  case KindOfSome:
  case KindOfAny:
    return TypePtr(new Type(uncertain));
  default:
    ASSERT(false);
  }
  return TypePtr();
}

TypePtr Type::CreateObjectType(const std::string &classname) {
  return TypePtr(new Type(KindOfObject, classname));
}

TypePtr Type::GetType(KindOf kindOf) {
  switch (kindOf) {
  case KindOfBoolean:     return Type::Boolean;
  case KindOfByte:        return Type::Byte;
  case KindOfInt16:       return Type::Int16;
  case KindOfInt32:       return Type::Int32;
  case KindOfInt64:       return Type::Int64;
  case KindOfDouble:      return Type::Double;
  case KindOfString:      return Type::String;
  case KindOfArray:       return Type::Array;
  case KindOfVariant:     return Type::Variant;
  case KindOfObject:
  case KindOfNumeric:
  case KindOfPrimitive:
  case KindOfPlusOperand:
  case KindOfSequence:
  case KindOfSome:
  case KindOfAny:         return CreateType(kindOf);
  default:
    ASSERT(false);
    break;
  }
  return TypePtr();
}

bool Type::IsLegalCast(AnalysisResultPtr ar, TypePtr from, TypePtr to) {
  if (from->m_kindOf == to->m_kindOf ||
      from->m_kindOf == KindOfVariant ||
      from->m_kindOf == KindOfSome ||
      from->m_kindOf == KindOfAny) return true;

  switch (to->m_kindOf) {
  case KindOfBoolean:
    return true;
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble:
    switch (from->m_kindOf) {
    case KindOfArray:
    case KindOfObject:
    case KindOfSequence:
      return false;
    default:
      return true;
    }
  case KindOfString:
    switch (from->m_kindOf) {
    case KindOfArray:
      return false;
    case KindOfObject:
      if (!from->m_name.empty()) {
        ClassScopePtr cls = ar->findClass(from->m_name);
        if (cls) {
          return cls->findFunction(ar, "__tostring", true);
        }
      }
      return true; // we can't really be sure
    default:
      return true;
    }
  case KindOfArray:
    return false;
  case KindOfObject:
    switch (from->m_kindOf) {
    case KindOfObject:
      if (!to->m_name.empty() && !from->m_name.empty() &&
          to->m_name != from->m_name) {
        ClassScopePtr cls = ar->findClass(from->m_name);
        if (cls) {
          return cls->derivesFrom(ar, to->m_name);
        }
      }
      return true; // we can't really be sure
    default:
      return false;
    }
  case KindOfVariant:
    return true;
  case KindOfNumeric:
    switch (from->m_kindOf) {
    case KindOfBoolean:
    case KindOfString:
    case KindOfArray:
    case KindOfObject:
    case KindOfSequence:
      return false;
    default:
      return true;
    }
  case KindOfPrimitive:
    switch (from->m_kindOf) {
    case KindOfBoolean:
    case KindOfArray:
      return false;
    case KindOfObject:
      if (!from->m_name.empty()) {
        ClassScopePtr cls = ar->findClass(from->m_name);
        if (cls) {
          return cls->findFunction(ar, "__tostring", true);
        }
      }
      return true; // we can't really be sure
    default:
      return true;
    }
  case KindOfPlusOperand:
    switch (from->m_kindOf) {
    case KindOfBoolean:
    case KindOfString:
    case KindOfObject:
      return false;
    default:
      return true;
    }
  case KindOfSequence:
    switch (from->m_kindOf) {
    case KindOfBoolean:
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfObject:
      return false;
    default:
      return true;
    }
  case KindOfSome:
  case KindOfAny:
    return true;
  default:
    ASSERT(false);
  }
  return false;
}

TypePtr Type::Cast(AnalysisResultPtr ar, TypePtr from, TypePtr to) {
  switch (to->m_kindOf) {
  case KindOfObject:
    if (from->m_kindOf == KindOfObject && to->m_name.empty() &&
        !from->m_name.empty()) return from;
    return to;
  case KindOfNumeric:
    switch (from->m_kindOf) {
    case KindOfBoolean:     return Type::Byte;
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfString:
    case KindOfSequence:    return from;
    case KindOfArray:
    case KindOfObject:      return Type::Byte;
    case KindOfVariant:     return from;
    case KindOfNumeric:
    case KindOfPrimitive:
    case KindOfPlusOperand:
    case KindOfSome:
    case KindOfAny:         return to;
    default:
      ASSERT(false);
      break;
    }
  case KindOfPrimitive:
    switch (from->m_kindOf) {
    case KindOfBoolean:     return Type::Byte;
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfString:      return from;
    case KindOfArray:       return Type::Byte;
    case KindOfObject:
      if (!from->m_name.empty()) {
        ClassScopePtr cls = ar->findClass(from->m_name);
        if (cls && cls->findFunction(ar, "__tostring", true)) {
          return Type::String;
        }
      }
      return Type::Byte;
    case KindOfVariant:     return from;
    case KindOfNumeric:     return from;
    case KindOfPrimitive:   return to;
    case KindOfPlusOperand: return CreateType(KindOfNumeric);
    case KindOfSequence:    return Type::String;
    case KindOfSome:
    case KindOfAny:         return to;
    default:
      ASSERT(false);
      break;
    }
  case KindOfPlusOperand:
    switch (from->m_kindOf) {
    case KindOfBoolean:     return Type::Byte;
    case KindOfByte:
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfString:
    case KindOfArray:       return from;
    case KindOfObject:      return Type::Array;
    case KindOfVariant:     return from;
    case KindOfNumeric:     return from;
    case KindOfPrimitive:   return CreateType(KindOfNumeric);
    case KindOfPlusOperand: return to;
    case KindOfSequence:    return Type::Array;
    case KindOfSome:
    case KindOfAny:         return to;
    default:
      ASSERT(false);
      break;
    }
  case KindOfSome:
  case KindOfAny:
    return from;
  default:
    return to;
  }
}

bool Type::IsCastNeeded(AnalysisResultPtr ar, TypePtr from, TypePtr to) {
  if (SameType(from, to)) return false;

  switch (to->m_kindOf) {
  case KindOfVariant:
  case KindOfNumeric:
  case KindOfPrimitive:
  case KindOfPlusOperand:
  case KindOfSequence:
  case KindOfSome:
  case KindOfAny:
    // Currently these types are all mapped to Variant in runtime/base, and
    // that's why these casting are not needed.
    return false;
  case KindOfObject:
    if (from->m_kindOf == KindOfObject && to->m_name.empty() &&
        !from->m_name.empty()) return false;
  default:
    break;
  }

  // All Sequence operations are implemented on String and Array classes, and
  // vice versa, therefore no need to cast between these types.
  switch (from->m_kindOf) {
  case KindOfString:
  case KindOfArray:
    if (to->m_kindOf == KindOfSequence) {
      return false;
    }
    break;
  case KindOfSequence:
    switch (to->m_kindOf) {
    case KindOfString:
    case KindOfArray:
      return false;
    default:
      break;
    }
    break;
  default:
    break;
  }

  return true;
}

bool Type::IsCoercionNeeded(AnalysisResultPtr ar, TypePtr t1, TypePtr t2) {
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

/* 
   We have inferred type1 and type2 as the actual types for the same
   expression. 
   Assert that the types are compatible (it cant be both a string and
   an integer, for example), and return the combined type.
*/
TypePtr Type::Inferred(AnalysisResultPtr ar, TypePtr type1, TypePtr type2) {
  if (!type1) return type2;
  if (!type2) return type1;
  KindOf k1 = type1->m_kindOf;
  KindOf k2 = type2->m_kindOf;

  if (k1 == k2) return type1;

  if (k1 == KindOfAny) return type2;
  if (k2 == KindOfAny) return type1;

  if (k1 == KindOfSome) return type2;
  if (k2 == KindOfSome) return type1;

  if (k1 == KindOfVariant) return type2;
  if (k2 == KindOfVariant) return type1;

  if (k1 <= KindOfDouble) {
    assert(k2 <= KindOfDouble ||
           k2 == KindOfNumeric ||
           k2 == KindOfPrimitive ||
           k2 == KindOfPlusOperand);
    return type1;
  }
  if (k2 <= KindOfDouble) {
    assert(k1 == KindOfNumeric ||
           k1 == KindOfPrimitive ||
           k1 == KindOfPlusOperand);
    return type2;
  }

  assert(k1 != KindOfObject && k2 != KindOfObject);

  if (k1 == KindOfArray) {
    assert(k2 == KindOfSequence ||
           k2 == KindOfPlusOperand);
    return type1;
  }
  if (k2 == KindOfArray) {
    assert(k1 == KindOfSequence || 
           k1 == KindOfPlusOperand);
    return type2;
  }

  if (k1 == KindOfString) {
    assert(k2 == KindOfPrimitive ||
           k2 == KindOfSequence);
    return type1;
  }
  if (k2 == KindOfString) {
    assert(k1 == KindOfPrimitive ||
           k1 == KindOfSequence);
    return type2;
  }

  if (k1 == KindOfNumeric) {
    assert(k2 == KindOfPrimitive ||
           k2 == KindOfPlusOperand);
    return type1;
  }
  if (k2 == KindOfNumeric) {
    assert(k1 == KindOfPrimitive ||
           k1 == KindOfPlusOperand);
    return type2;
  }

  if (k1 == KindOfPrimitive) {
    if (k2 == KindOfPlusOperand) {
      return Type::GetType(Type::KindOfNumeric);
    } else {
      assert(k2 == KindOfSequence);
      return Type::String;
    }
  }
  if (k2 == KindOfPrimitive) {
    if (k1 == KindOfPlusOperand) {
      return Type::GetType(Type::KindOfNumeric);
    } else {
      assert(k1 == KindOfSequence);
      return Type::String;
    }
  }
    
  if (k1 == KindOfPlusOperand) {
    assert(k2 == KindOfSequence);
    return Type::Array;
  }
  assert(k2 == KindOfPlusOperand);
  assert(k1 == KindOfSequence);
  return Type::Array;
}

TypePtr Type::Coerce(AnalysisResultPtr ar, TypePtr type1, TypePtr type2) {
  if (SameType(type1, type2)) return type1;
  if (type1->m_kindOf == KindOfVariant ||
      type2->m_kindOf == KindOfVariant) return Type::Variant;
  if (type1->m_kindOf > type2->m_kindOf) return Coerce(ar, type2, type1);
  if (type2->m_kindOf == KindOfSome ||
      type2->m_kindOf == KindOfAny) return type1;

  switch (type1->m_kindOf) {
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble:
    switch (type2->m_kindOf) {
    case KindOfInt16:
    case KindOfInt32:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfNumeric:     return type2;
    default:
      break;
    }
    break;
  case KindOfObject:
    if (type2->m_kindOf == KindOfObject) {
      if (type1->m_name.empty()) return type2;
      if (type2->m_name.empty()) return type1;
      ClassScopePtr cls1 = ar->findClass(type1->m_name);
      if (cls1 && cls1->derivesFrom(ar, type2->m_name)) {
        return type2;
      }
      ClassScopePtr cls2 = ar->findClass(type2->m_name);
      if (cls2 && cls2->derivesFrom(ar, type1->m_name)) {
        return type1;
      }
    }
    break;
  default:
    break;
  }
  return Type::Variant;
}

bool Type::SameType(TypePtr type1, TypePtr type2) {
  if (type1->m_kindOf == type2->m_kindOf) {
    if (type1->m_kindOf == KindOfObject &&
        type1->m_name != type2->m_name) return false;
    return true;
  }
  return false;
}

bool Type::IsBadTypeConversion(AnalysisResultPtr ar, TypePtr from,
                               TypePtr to, bool coercing) {
  if (!coercing) {
    return !Type::IsLegalCast(ar, from, to);
  }
  if (Type::SameType(from, to)) {
    return false;
  }
  if (from->m_kindOf == KindOfSome ||
      from->m_kindOf == KindOfAny ||
      from->m_kindOf == KindOfVariant) {
    return false;
  }
  if (to->m_kindOf == KindOfSome ||
      to->m_kindOf == KindOfAny ||
      to->m_kindOf == KindOfVariant) {
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

Type::Type(KindOf kindOf) : m_kindOf(kindOf) {
}

Type::Type(KindOf kindOf, const std::string &name)
  : m_kindOf(kindOf), m_name(name) {
}

bool Type::isInteger() const {
  switch (m_kindOf) {
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    return true;
  default:
    break;
  }
  return false;
}

bool Type::isSpecificObject() const {
  return m_kindOf == KindOfObject && !m_name.empty();
}

bool Type::isNonConvertibleType() const {
  return m_kindOf == KindOfObject || m_kindOf == KindOfArray;
}

bool Type::isNoObjectInvolved() const {
  switch (m_kindOf) {
  case KindOfVoid:
  case KindOfBoolean:
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfString:
  case KindOfNumeric:
  case KindOfPrimitive:
    return true;
  default:
    break;
  }
  return false;
}

TypePtr Type::combinedPrimType(TypePtr t1, TypePtr t2) {
  KindOf kind = KindOfAny;

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

string Type::getCPPDecl() {
  switch (m_kindOf) {
  case KindOfBoolean:     return "bool";
  case KindOfByte:        return "char";
  case KindOfInt16:       return "short";
  case KindOfInt32:       return "int";
  case KindOfInt64:       return "int64";
  case KindOfDouble:      return "double";
  case KindOfString:      return "String";
  case KindOfArray:       return "Array";
  case KindOfVariant:
  case KindOfSome:
  case KindOfAny:         return "Variant";
  case KindOfNumeric:     return "Numeric";
  case KindOfPrimitive:   return "Primitive";
  case KindOfPlusOperand: return "PlusOperand";
  case KindOfSequence:    return "Sequence";
  case KindOfObject:
    if (m_name.empty()) return "Object";
    return string("p_") + m_name;
  default:
    ASSERT(false);
  }
  return "";
}

void Type::outputCPPDecl(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg.printf(getCPPDecl().c_str());
}

void Type::outputCPPCast(CodeGenerator &cg, AnalysisResultPtr ar) {
  switch (m_kindOf) {
  case KindOfBoolean:     cg.printf("toBoolean");   break;
  case KindOfByte:        cg.printf("toByte");      break;
  case KindOfInt16:       cg.printf("toInt16");     break;
  case KindOfInt32:       cg.printf("toInt32");     break;
  case KindOfInt64:       cg.printf("toInt64");     break;
  case KindOfDouble:      cg.printf("toDouble");    break;
  case KindOfString:      cg.printf("toString");    break;
  case KindOfArray:       cg.printf("toArray");     break;
  case KindOfVariant:     cg.printf("Variant");     break;
  case KindOfNumeric:     cg.printf("Numeric");     break;
  case KindOfPrimitive:   cg.printf("Primitive");   break;
  case KindOfPlusOperand: cg.printf("PlusOperand"); break;
  case KindOfSequence:    cg.printf("Sequence");    break;
  case KindOfObject:
    if (m_name.empty()) {
      cg.printf("toObject");
    } else {
      cg.printf("p_%s", m_name.c_str());
    }
    break;
  default:
    ASSERT(false);
  }
}

const char *Type::getCPPInitializer() {
  switch (m_kindOf) {
  case KindOfBoolean:     return "false";
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:       return "0";
  case KindOfDouble:      return "0.0";
  case KindOfString:
  case KindOfArray:
  case KindOfSequence:
  case KindOfVariant:
  case KindOfSome:
  case KindOfAny:
  case KindOfObject:      return NULL;
  case KindOfNumeric:
  case KindOfPrimitive:
  case KindOfPlusOperand: return "0";
  default:
    ASSERT(false);
  }
  return NULL;
}

std::string Type::getPHPName() {
  switch (m_kindOf) {
  case KindOfArray:       return "array";
  case KindOfObject:      return m_name;
  default: break;
  }
  return "";
}

std::string Type::toString() const {
  switch (m_kindOf) {
  case KindOfBoolean:     return "Boolean";
  case KindOfByte:        return "Byte";
  case KindOfInt16:       return "Int16";
  case KindOfInt32:       return "Int32";
  case KindOfInt64:       return "Int64";
  case KindOfDouble:      return "Double";
  case KindOfString:      return "String";
  case KindOfArray:       return "Array";
  case KindOfVariant:     return "Variant";
  case KindOfSome:
  case KindOfAny:         return "Any";
  case KindOfObject:      return string("Object - ") + m_name;
  case KindOfNumeric:     return "Numeric";
  case KindOfPrimitive:   return "Primitive";
  case KindOfPlusOperand: return "PlusOperand";
  case KindOfSequence:    return "Sequence";
  default:
    ASSERT(false);
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

void Type::serialize(JSON::OutputStream &out) const {
  out << toString();
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

  switch (m_kindOf) {
  case KindOfBoolean:
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
  case KindOfDouble:
  case KindOfString:
  case KindOfArray:
  case KindOfObject:
    counts["_strong"]++;
    break;
  case KindOfVariant:
  case KindOfSome:
  case KindOfAny:
  case KindOfNumeric:
  case KindOfPrimitive:
  case KindOfPlusOperand:
  case KindOfSequence:
    counts["_weak"]++;
    break;
  default:
    ASSERT(false);
  }
  counts["_all"]++;
}
