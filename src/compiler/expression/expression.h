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

#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include <compiler/construct.h>
#include <compiler/analysis/type.h>
#include <compiler/analysis/analysis_result.h>

#define EXPRESSION_CONSTRUCTOR_PARAMETERS       \
  BlockScopePtr scope, LocationPtr loc, Expression::KindOf kindOf
#define EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES \
  scope, loc, kindOf
#define DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS                       \
  virtual void analyzeProgram(AnalysisResultPtr ar);                    \
  virtual ExpressionPtr clone();                                        \
  virtual TypePtr inferTypes(AnalysisResultPtr ar, TypePtr type,        \
                             bool coerce);                              \
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);      \
  virtual void outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar)
#define DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS                            \
  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;                            \
  virtual ConstructPtr getNthKid(int n) const;                          \
  virtual int getKidCount() const;                                      \
  virtual void setNthKid(int n, ConstructPtr cp)

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(Expression);
class Variant;

#define DECLARE_EXPRESSION_TYPES(x)             \
  x(ExpressionList, None),                      \
    x(AssignmentExpression, Store),             \
    x(SimpleVariable, Load),                    \
    x(DynamicVariable, Load),                   \
    x(StaticMemberExpression, Load),            \
    x(ArrayElementExpression, Load),            \
    x(DynamicFunctionCall, Call),               \
    x(SimpleFunctionCall, Call),                \
    x(ScalarExpression, None),                  \
    x(ObjectPropertyExpression, Load),          \
    x(ObjectMethodExpression, Call),            \
    x(ListAssignment, Store),                   \
    x(NewObjectExpression, Call),               \
    x(UnaryOpExpression, Update),               \
    x(IncludeExpression, Call),                 \
    x(BinaryOpExpression, Update),              \
    x(QOpExpression, None),                     \
    x(ArrayPairExpression, None),               \
    x(ClassConstantExpression, Const),          \
    x(ParameterExpression, None),               \
    x(ModifierExpression, None),                \
    x(ConstantExpression, Const),               \
    x(EncapsListExpression, None),              \
    x(ClosureExpression, None)

class Expression : public Construct {
public:
#define DEC_EXPR_ENUM(x,t) KindOf##x
  enum KindOf {
    DECLARE_EXPRESSION_TYPES(DEC_EXPR_ENUM)
  };
  static const char *Names[];
  enum ExprClass {
    None,
    Load = 1,
    Store = 2,
    Update = 3,
    Const = 4,
    Call = 8
  };

  enum Context {
    NoContext    = 0,
    RValue       = 0,
    LValue       = 1,            // assignment exp; foreach stmt
    Declaration  = LValue | 2,   // global or static stmt, or delayed var
    NoLValueWrapper = 4,         // ok to not have lval() wrapper
    RefValue  = 8,               // &exp
    NoRefWrapper = 0x10,         // ok to not have ref() wrapper
    ObjectContext = 0x20,        // $obj->
    InParameterExpression = 0x40,// for default value expression
    ExistContext = 0x80,         // isset(...) or empty(...) recursively
    UnsetContext = 0x100,        // Within unset(...), arr el recursively
    AssignmentLHS = 0x200,       // LHS in assignment
    DeepAssignmentLHS = 0x400,   // LHS in assignment, deep
    InvokeArgument = 0x800,      // Invoke arguments
    RefParameter   = 0x1000,     // eg f(&$x)
    OprLValue = 0x2000,          // Lhs of op=, or operand of ++,--
    DeepOprLValue = 0x4000,      // LHS of op=, or operand of ++,--, deep
    DeadStore = 0x8000,          // This is an assignment, op=, or ++/--
                                 // which can be killed
    CondExpr = 0x10000,          // Used by alias manager to track expressions
                                 // which are conditionally executed
    AssignmentRHS = 0x20000,     // RHS in assignment
    DeepReference = 0x40000,     // Value is not available for copy propagation
                                 // because it is referenced in some way
                                 // eg $b in &$b['foo']
    AccessContext = 0x80000,     // ArrayElementExpression::m_variable or
                                 // ObjectPropertyExpression::m_object
    RefAssignmentLHS = 0x100000, // LHS of a reference assignment
    ReturnContext = 0x200000,    // Return expression
  };

  enum Order {
    FixOrder  = 1,
    StashVars = 2,
    StashKidVars = 4,
    StashByRef = 8,
    StashAll = 16
  };

  enum Error {
    NoError = 0,
    BadPassByRef = 1,
  };

public:
  Expression(EXPRESSION_CONSTRUCTOR_PARAMETERS);

  /**
   * Set this expression's context.
   */
  virtual void setContext(Context context) { m_context |= context;}
  virtual void clearContext(Context context) { m_context &= ~context;}
  void copyContext(ExpressionPtr from) { copyContext(from->m_context); }
  void copyContext(int contexts);
  ExpressionPtr replaceValue(ExpressionPtr rep);
  void clearContext();
  int getContext() const { return m_context;}
  bool hasContext(Context context) const { return (m_context & context) == context; }
  bool hasAnyContext(int context) const {
    if ((context & Declaration) == Declaration) {
      // special case Declaration because it is 2 bit fields
      if (hasContext(Declaration)) return true;
      // clear Declaration since we already checked for it
      context &= ~Declaration;
    }
    return m_context & context;
  }
  bool hasAllContext(int context) const { return (m_context & context) == context; }
  bool hasSubExpr(ExpressionPtr sub) const;
  virtual void setComment(const std::string &) {}
  /**
   * Set this expression's error flags.
   */
  virtual void setError(Error error) { m_error |= error;}
  virtual void clearError(Error error) { m_error &= ~error;}
  int getError() const { return m_error;}
  bool hasError(Error error) const { return m_error & error; }

  ExprClass getExprClass() const;
  void setArgNum(int n);

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
  virtual void preOutputStash(CodeGenerator &cg, AnalysisResultPtr ar,
                              int state);
  virtual bool outputCPPUnneeded(CodeGenerator &cg, AnalysisResultPtr ar);
  bool outputCPPBegin(CodeGenerator &cg, AnalysisResultPtr ar);
  bool outputCPPEnd(CodeGenerator &cg, AnalysisResultPtr ar);
  virtual bool preOutputCPP(CodeGenerator &cg, AnalysisResultPtr ar,
                            int state);
  bool preOutputOffsetLHS(CodeGenerator &cg, AnalysisResultPtr ar,
                          int state);
  bool hasCPPTemp() const { return !m_cppTemp.empty(); }
  const std::string &cppTemp() const { return m_cppTemp; }
  std::string genCPPTemp(CodeGenerator &cg, AnalysisResultPtr ar);
  void setCPPTemp(const std::string &s) { m_cppTemp = s; }
  BlockScopeRawPtr getOriginalScope();
  ClassScopeRawPtr getOriginalClass();
  FunctionScopeRawPtr getOriginalFunction();
  std::string originalClassName(CodeGenerator &cg, bool withComma);

  /**
    * For generic walks
    */
  virtual int getKidCount() const { return 0; }
  ExpressionPtr getNthExpr(int n) const { return
      boost::static_pointer_cast<Expression>(getNthKid(n)); }

  /**
   * For cse & canonicalization
   */
  virtual unsigned getCanonHash() const;
  virtual bool canonCompare(ExpressionPtr e) const;
  void setCanonID(unsigned id) { m_canon_id = id; }
  unsigned getCanonID() const { return m_canon_id; }
  void setCanonPtr(ExpressionPtr e) { m_canonPtr = e; }
  ExpressionPtr getCanonPtr() const {
    return m_context & (LValue|RefValue|UnsetContext|DeepReference) ?
      ExpressionPtr() : m_canonPtr;
  }
  ExpressionPtr getCanonLVal() const {
    return m_canonPtr;
  }

  /**
   * Type checking without RTTI.
   */
  bool is(KindOf kindOf) const { return m_kindOf == kindOf;}
  KindOf getKindOf() const { return m_kindOf;}
  virtual bool isTemporary() const { return false; }
  virtual bool isScalar() const { return false; }
  virtual bool isRefable(bool checkError = false) const { return false; }
  virtual bool getScalarValue(Variant &value) { return false; }
  bool getEffectiveScalar(Variant &value);
  virtual ExpressionPtr clone() {
    ASSERT(false);
    return ExpressionPtr();
  }
  virtual bool isThis() const { return false;}
  virtual bool isLiteralString() const { return false;}
  virtual bool isLiteralNull() const { return false;}
  bool isUnquotedScalar() const;
  virtual std::string getLiteralString() const { return "";}
  virtual bool containsDynamicConstant(AnalysisResultPtr ar) const {
    return false;
  }
  void deepCopy(ExpressionPtr exp);
  virtual ExpressionPtr unneeded();
  virtual ExpressionPtr unneededHelper();

  /**
   * This is to avoid dynamic casting to ExpressionList in Parser.
   */
  virtual void addElement(ExpressionPtr exp);
  virtual void insertElement(ExpressionPtr exp, int index = 0);

  virtual void analyzeProgram(AnalysisResultPtr ar);                    \

  /**
   * Called before type inference.
   */
  virtual ExpressionPtr preOptimize(AnalysisResultConstPtr ar) {
    return ExpressionPtr();
  }

  /**
   * Called after type inference.
   */
  virtual ExpressionPtr postOptimize(AnalysisResultConstPtr ar) {
    return ExpressionPtr();
  }

  /**
   * Find other types that have been inferred for this expression,
   * and combine them with inType to form a new, tighter type.
   */
  TypePtr propagateTypes(AnalysisResultConstPtr ar, TypePtr inType);

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
  TypePtr checkTypesImpl(AnalysisResultConstPtr ar, TypePtr expectedType,
                         TypePtr actualType, bool coerce);

  TypePtr getActualType() { return m_actualType;}
  TypePtr getExpectedType() { return m_expectedType;}
  TypePtr getImplementedType() { return m_implementedType;}
  void setActualType(TypePtr actualType) {
    m_actualType = actualType;
  }
  void setExpectedType(TypePtr expectedType) {
    m_expectedType = expectedType;
  }
  void setImplementedType(TypePtr implementedType) {
    m_implementedType = implementedType;
  }
  TypePtr getType();
  TypePtr getCPPType();

  static ExpressionPtr MakeConstant(AnalysisResultConstPtr ar,
                                    BlockScopePtr scope,
                                    LocationPtr loc,
                                    const std::string &value);
  static ExpressionPtr MakeScalarExpression(AnalysisResultConstPtr ar,
                                            BlockScopePtr scope,
                                            LocationPtr loc,
                                            const Variant &value);
  static void CheckPassByReference(AnalysisResultPtr ar,
                                   ExpressionPtr param);

  static bool CheckNeededRHS(ExpressionPtr value);
  static bool CheckNeeded(ExpressionPtr variable, ExpressionPtr value);

  void fixExpectedType(AnalysisResultConstPtr ar);
  bool outputCPPArithArg(CodeGenerator &cg, AnalysisResultPtr ar, bool arrayOk);
  bool isUnused() const { return m_unused; }
  void setUnused(bool u) { m_unused = u; }
  ExpressionPtr fetchReplacement();
  void setReplacement(ExpressionPtr rep) { m_replacement = rep; }

  /**
   * Correctly compute the local expression altered bit
   */
  void computeLocalExprAltered();

protected:
  static bool IsIdentifier(const std::string &value);

  KindOf m_kindOf;
  int m_context;
  TypePtr m_actualType;
  TypePtr m_expectedType; // null if the same as m_actualType
  TypePtr m_implementedType; // null if the same as m_actualType
  std::string m_cppTemp;
  int m_argNum;

  TypePtr inferAssignmentTypes(AnalysisResultPtr ar, TypePtr type,
                               bool coerce, ExpressionPtr variable,
                               ExpressionPtr value = ExpressionPtr());
  void setTypes(AnalysisResultConstPtr ar, TypePtr actualType,
                TypePtr expectedType);
  void setDynamicByIdentifier(AnalysisResultPtr ar,
                              const std::string &value);
  bool outputLineMap(CodeGenerator &cg, AnalysisResultPtr ar,
                     bool force = false);

 private:
  static ExprClass Classes[];
  void outputCPPInternal(CodeGenerator &cg, AnalysisResultPtr ar);

  BlockScopeRawPtr m_originalScope;
  bool m_originalScopeSet;
  unsigned m_canon_id;
  ExpressionPtr m_canonPtr;
  mutable int m_error;
  bool m_unused;
  ExpressionPtr m_replacement;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXPRESSION_H__
