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

#ifndef incl_HPHP_TYPE_H_
#define incl_HPHP_TYPE_H_

#include "hphp/compiler/hphp.h"
#include "hphp/util/json.h"
#include "hphp/util/case-insensitive.h"
#include "hphp/runtime/base/types.h"


class TestCodeRun;
class TestCodeError;
struct ProgramOptions;
int process(const ProgramOptions&);

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class CodeGenerator;
DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(Type);
DECLARE_BOOST_TYPES(BlockScope);
DECLARE_BOOST_TYPES(ClassScope);

class Type : public JSON::CodeError::ISerializable,
             public JSON::DocTarget::ISerializable {
  friend class ::TestCodeRun;
  friend class ::TestCodeError;
public:
  typedef int KindOf;

  static const KindOf KindOfVoid       = 0x0001;
  static const KindOf KindOfBoolean    = 0x0002;
  static const KindOf KindOfInt32      = 0x0010;
  static const KindOf KindOfInt64      = 0x0020;
  static const KindOf KindOfDouble     = 0x0040;
  static const KindOf KindOfString     = 0x0080;
  static const KindOf KindOfArray      = 0x0100;
  static const KindOf KindOfObject     = 0x0200;   // with classname
  static const KindOf KindOfResource   = 0x0400;
  static const KindOf KindOfVariant    = 0xFFFF;

  /* This bit tells coerce that if the other type
     is already one of the specified types, it wont
     be modified.
     eg $a['foo'] = <whatever>
     If $a is already known to be string or array, it stays that way.
     If we coerce to Sequence, however, it would become Sequence, and
     hence Variant
  */
  static const KindOf KindOfAuto       = 0x0400;

  static const KindOf KindOfInteger = (KindOf)(KindOfInt64 | KindOfInt32);
  static const KindOf KindOfNumeric = (KindOf)(KindOfDouble | KindOfInteger);
  static const KindOf KindOfPrimitive = (KindOf)(KindOfNumeric | KindOfString);
  static const KindOf KindOfPlusOperand = (KindOf)(KindOfNumeric | KindOfArray);
  static const KindOf KindOfSequence = (KindOf)(KindOfString | KindOfArray);

  static const KindOf KindOfAutoSequence = (KindOf)(KindOfAuto |
                                                    KindOfSequence);
  static const KindOf KindOfAutoObject = (KindOf)(KindOfAuto | KindOfObject);

  static const KindOf KindOfSome = (KindOf)0x7FFE;
  static const KindOf KindOfAny = (KindOf)0x7FFF;
  /**
   * Inferred types: types that a variable or a constant is sure to be.
   */
  static TypePtr Null;
  static TypePtr Boolean;
  static TypePtr Int32;
  static TypePtr Int64;
  static TypePtr Double;
  static TypePtr String;
  static TypePtr Array;
  static TypePtr Object;
  static TypePtr Resource;
  static TypePtr Variant;

  static TypePtr Numeric;
  static TypePtr PlusOperand;
  static TypePtr Primitive;
  static TypePtr Sequence;

  static TypePtr AutoSequence;
  static TypePtr AutoObject;

  static TypePtr Any;
  static TypePtr Some;

  typedef hphp_string_imap<TypePtr> TypePtrMap;
  static const TypePtrMap &GetTypeHintTypes(bool hhType);

  /**
   * Uncertain types: types that are ambiguous yet.
   */
  static TypePtr CreateObjectType(const std::string &classname);

  /**
   * For inferred, return static type objects; for uncertain, create new
   * ones.
   */
  static TypePtr GetType(KindOf kindOf,
                         const std::string &clsname = "");

  /**
   * Whether a type can be used as another type.
   */
  static bool IsLegalCast(AnalysisResultConstPtr ar, TypePtr from, TypePtr to);

  /**
   * Find the intersection between two sets of types.
   */
  static TypePtr Intersection(AnalysisResultConstPtr ar,
                              TypePtr from, TypePtr to);

  /**
   * Whether or not this type is mapped to type Variant
   * in the runtime
   */
  static bool IsMappedToVariant(TypePtr t);

  /**
   * Whether or not a cast is needed during code generation.
   */
  static bool IsCastNeeded(AnalysisResultConstPtr ar, TypePtr from, TypePtr to);

  /**
   * When a variable's type is t1, and it's used as t2, do we need to
   * coerce variable's type? Normally, if t2 can be legally casted to t1,
   * this returns false.
   */
  static bool IsCoercionNeeded(AnalysisResultConstPtr ar,
                               TypePtr t1, TypePtr t2);

  /**
   * When a variable is assigned with two types, what type a variable
   * should be?
   */
  static TypePtr Coerce(AnalysisResultConstPtr ar,
                        TypePtr type1, TypePtr type2);
  static TypePtr Union(AnalysisResultConstPtr ar, TypePtr type1, TypePtr type2);

  /**
   * When two types have been inferred for an expression, what type
   * should it be?
   */
  static TypePtr Inferred(AnalysisResultConstPtr ar,
                          TypePtr type1, TypePtr type2);

  /**
   * When two object types have been inferred for an expression, what type
   * should it be?
   */
  static TypePtr InferredObject(AnalysisResultConstPtr ar,
                                TypePtr type1,
                                TypePtr type2);

  /**
   * Whether or not two types are the same.
   */
  static bool SameType(TypePtr type1, TypePtr type2);

  /**
   * Return true if SameType(type1,type2) or if type1 and type2
   * are objects and type1 derives from type2.
   */
  static bool SubType(AnalysisResultConstPtr ar, TypePtr type1, TypePtr type2);

  /**
   * Testing type conversion for constants.
   */
  static bool IsExactType(KindOf kindOf);

  static bool HasFastCastMethod(TypePtr t);

  /**
   *  Returns the name of the method used to fast cast from
   *  variant to dst
   */
  static std::string GetFastCastMethod(
      TypePtr dst, bool allowRef, bool forConst);

private:
  Type(KindOf kindOf, const std::string &name);

public:
  /**
   * KindOf testing.
   */
  explicit Type(KindOf kindOf);
  bool is(KindOf kindOf) const { return m_kindOf == kindOf;}
  bool isExactType() const { return IsExactType(m_kindOf); }
  bool mustBe(KindOf kindOf) const { return !(m_kindOf & ~kindOf); }
  bool couldBe(KindOf kindOf) const { return m_kindOf & kindOf; }
  bool isSubsetOf(TypePtr t) const {
    return m_kindOf != t->m_kindOf && mustBe(t->m_kindOf);
  }
  KindOf getKindOf() const { return m_kindOf;}
  bool isInteger() const;
  bool isStandardObject() const;
  bool isSpecificObject() const;
  bool isNonConvertibleType() const; // other types cannot convert to them
  bool isPrimitive() const {
    return IsExactType(m_kindOf) && (m_kindOf <= KindOfDouble) &&
      (m_kindOf != KindOfVoid);
  }
  bool isNoObjectInvolved() const;
  const std::string &getName() const { return m_name;}
  static TypePtr combinedArithmeticType(TypePtr t1, TypePtr t2);

  ClassScopePtr getClass(AnalysisResultConstPtr ar,
                         BlockScopeRawPtr scope) const;

  DataType getDataType() const;
  DataType getHhvmDataType() const;

  /**
   * Type hint names in PHP.
   */
  std::string getPHPName();

  /**
   * Debug dump.
   */
  std::string toString() const;
  static void Dump(TypePtr type, const char *fmt = "%s");
  static void Dump(ExpressionPtr exp);

  /**
   * Implements JSON::CodeError::ISerializable.
   */
  virtual void serialize(JSON::CodeError::OutputStream &out) const;

  /**
   * Implements JSON::DocTarget::ISerializable.
   */
  virtual void serialize(JSON::DocTarget::OutputStream &out) const;

  /**
   * For stats reporting.
   */
  void count(std::map<std::string, int> &counts);

  /**
   * Must not be invoked concurrently
   */
  static void InitTypeHintMap();
private:

  /**
   * Must not be invoked concurrently
   */
  static void ResetTypeHintTypes();

  static TypePtrMap s_TypeHintTypes;
  static TypePtrMap s_HHTypeHintTypes;

  const KindOf m_kindOf;
  const std::string m_name;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_TYPE_H_
