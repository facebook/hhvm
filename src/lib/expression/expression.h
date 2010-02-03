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

#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include <lib/construct.h>
#include <lib/analysis/type.h>
#include <lib/analysis/analysis_result.h>

#define EXPRESSION_CONSTRUCTOR_PARAMETERS       \
  LocationPtr loc, Expression::KindOf kindOf
#define EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES \
  loc, kindOf
#define DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS                            \
  virtual void analyzeProgram(AnalysisResultPtr ar);                    \
  virtual ExpressionPtr clone();                                        \
  virtual ExpressionPtr preOptimize(AnalysisResultPtr ar);              \
  virtual ExpressionPtr postOptimize(AnalysisResultPtr ar);             \
  virtual TypePtr inferTypes(AnalysisResultPtr ar, TypePtr type,        \
                             bool coerce);                              \
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);      \
  virtual void outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar)

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(Expression);
class Variant;

class Expression : public Construct {
public:
  enum KindOf {
    KindOfExpressionList,
    KindOfAssignmentExpression,
    KindOfSimpleVariable,
    KindOfDynamicVariable,
    KindOfStaticMemberExpression,
    KindOfArrayElementExpression,
    KindOfDynamicFunctionCall,
    KindOfSimpleFunctionCall,
    KindOfScalarExpression,
    KindOfObjectPropertyExpression,
    KindOfObjectMethodExpression,
    KindOfListAssignment,
    KindOfNewObjectExpression,
    KindOfUnaryOpExpression,
    KindOfIncludeExpression,
    KindOfBinaryOpExpression,
    KindOfQOpExpression,
    KindOfArrayPairExpression,
    KindOfClassConstantExpression,
    KindOfParameterExpression,
    KindOfModifierExpression,
    KindOfConstantExpression,
    KindOfEncapsListExpression,
    /* KindOfCount = 22 */
  };

  enum Context {
    RValue       = 0,
    LValue       = 1,           // assignment exp; foreach stmt
    Declaration  = LValue | 2,  // global or static stmt
    NoLValueWrapper = 4,        // ok to not have lval() wrapper
    RefValue  = 8,              // &exp
    NoRefWrapper = 16,          // ok to not have ref() wrapper
    ObjectContext = 32,         // $obj->
    InParameterExpression = 64, // for default value expression
    IssetContext = 128,         // isset(...)
    AssignmentLHS = 256,        // LHS in assignment
    DeepAssignmentLHS = 512,    // LHS in assignment, deep
    UnsetContext = 1024,        // Within unset(...), arr el recursively
    InvokeArgument = 2048,      // Invoke arguments
    RefParameter   = 4096       // eg f(&$x)
  };

public:
  Expression(EXPRESSION_CONSTRUCTOR_PARAMETERS);

  /**
   * Set this expression's context.
   */
  virtual void setContext(Context context) { m_context |= context;}
  virtual void clearContext(Context context) { m_context &= ~context;}
  int getContext() const { return m_context;}
  bool hasContext(Context context) const { return m_context & context; }

  /**
   * Implementing Construct.
   */
  virtual void outputCPP(CodeGenerator &cg, AnalysisResultPtr ar);
  virtual void outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) = 0;
  void outputCPPCast(CodeGenerator &cg, AnalysisResultPtr ar);
  virtual void outputCPPDecl(CodeGenerator &cg, AnalysisResultPtr ar);

  virtual void outputCPPExistTest(CodeGenerator &cg, AnalysisResultPtr ar,
                                  int op);
  virtual void outputCPPUnset(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * Type checking without RTTI.
   */
  bool is(KindOf kindOf) const { return m_kindOf == kindOf;}
  KindOf getKindOf() const { return m_kindOf;}
  virtual bool isScalar() const { return false;}
  virtual bool isRefable() const { return false;}
  virtual bool getScalarValue(Variant &value) { return false;}
  virtual ExpressionPtr clone() {
    ASSERT(false);
    return ExpressionPtr();
  }
  virtual bool isThis() const { return false;}
  virtual bool isLiteralString() const { return false;}
  virtual std::string getLiteralString() const { return "";}
  virtual bool containsDynamicConstant(AnalysisResultPtr ar) const {
    return false;
  }
  void deepCopy(ExpressionPtr exp);

  /**
   * This is to avoid dynamic casting to ExpressionList in Parser.
   */
  virtual void addElement(ExpressionPtr exp);
  virtual void insertElement(ExpressionPtr exp, int index = 0);

  /**
   * Called before type inference.
   */
  virtual ExpressionPtr preOptimize(AnalysisResultPtr ar) = 0;

  /**
   * Called after type inference.
   */
  virtual ExpressionPtr postOptimize(AnalysisResultPtr ar) = 0;

  /**
   * Called when types need to be inferred inside this expression.
   *
   * When coerce is true, it means this expression will have to be able to
   * hold that type of data. When it's false, it means as long as this
   * expression can be converted to the type, we are fine.
   *
   * This is the key function to understand in order to understand type
   * inference. Basically, "type" parameter is "expected" type, under
   * either l-value context, when coerce == true, or r-value context, when
   * coerce == false. But it's not always l-value context that "coerce" can
   * be set to true, since for example, there are cases like foreach ($a ...)
   * that we know $a needs to be an Array for sure. Some l-value context
   * cannot set "coerce" to true, for example $a++, which doesn't actually
   * change $a's type to anything new.
   *
   * Return type is ALWAYS an r-value type that this expression is evaluated
   * to. It's always up to this expression's parent to determine whether this
   * returned type is used as a "coerce"-d one or not onto another
   * expression.
   *
   * @param type  This expression is evaluated as this type.
   * @coerce      Whether to force this expression to be that type.
   * @return      What type this expression is evaluated to.
   */
  virtual TypePtr inferTypes(AnalysisResultPtr ar, TypePtr type,
                             bool coerce) = 0;

  /**
   * Call inferTypes() and check to make sure return type is convertible
   * to specified type. If not, raise a CodeError.
   */
  virtual TypePtr inferAndCheck(AnalysisResultPtr ar, TypePtr type,
                                bool coerce);

  /**
   * Check to make sure return type is convertible to specified type.
   * If not, raise a CodeError.
   */
  TypePtr checkTypesImpl(AnalysisResultPtr ar, TypePtr expectedType,
                         TypePtr actualType, bool coerce);

  TypePtr getActualType() { return m_actualType;}
  TypePtr getExpectedType() { return m_expectedType;}
  void setActualType(TypePtr actualType) {
    m_actualType = actualType;
  }
  void setExpectedType(TypePtr expectedType) {
    m_expectedType = expectedType;
  }
  TypePtr getType();

  static ExpressionPtr makeConstant(AnalysisResultPtr ar,
                                    LocationPtr loc,
                                    const std::string &value);

protected:
  static bool isIdentifier(const std::string &value);

  KindOf m_kindOf;
  int m_context;
  TypePtr m_actualType;
  TypePtr m_expectedType; // null if the same as m_actualType

  void setTypes(TypePtr actualType, TypePtr expectedType);
  void setDynamicByIdentifier(AnalysisResultPtr ar,
                              const std::string &value);
  bool outputLineMap(CodeGenerator &cg, AnalysisResultPtr ar);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXPRESSION_H__
