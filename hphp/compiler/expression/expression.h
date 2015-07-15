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

#ifndef incl_HPHP_EXPRESSION_H_
#define incl_HPHP_EXPRESSION_H_

#include "hphp/util/deprecated/declare-boost-types.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/compiler/construct.h"
#include "hphp/compiler/analysis/analysis_result.h"

#define EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS                          \
  BlockScopePtr scope, const Location::Range& r,                        \
  Expression::KindOf kindOf
#define EXPRESSION_CONSTRUCTOR_BASE_PARAMETER_VALUES                    \
  scope, r, kindOf
#define EXPRESSION_CONSTRUCTOR_PARAMETERS                               \
  BlockScopePtr scope, const Location::Range& r
#define EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES(kindOf)                 \
  scope, r, Expression::KindOf##kindOf
#define EXPRESSION_CONSTRUCTOR_DERIVED_PARAMETER_VALUES                 \
  scope, r
#define DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS                       \
  void analyzeProgram(AnalysisResultPtr ar) override;                   \
  ExpressionPtr clone() override;                                       \
  void outputCodeModel(CodeGenerator &cg) override;                     \
  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) override;
#define DECLARE_EXPRESSION_VIRTUAL_FUNCTIONS                            \
  DECLARE_BASE_EXPRESSION_VIRTUAL_FUNCTIONS;                            \
  ConstructPtr getNthKid(int n) const override;                         \
  int getKidCount() const override;                                     \
  void setNthKid(int n, ConstructPtr cp) override

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Statement);
DECLARE_EXTENDED_BOOST_TYPES(Expression);
class Variant;

class Expression : public Construct {
private:
  static const char *Names[];

public:
  static const char* nameOfKind(Construct::KindOf);

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
    // Unused       0x10,
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
    StashAll = 16,
    ForceTemp = 32,
  };

  enum Error {
    NoError = 0,
    BadPassByRef = 1,
  };

protected:
  Expression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS);

public:

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
  bool hasContext(Context context) const {
    return (m_context & context) == context;
  }
  bool hasAnyContext(int context) const {
    if ((context & Declaration) == Declaration) {
      // special case Declaration because it is 2 bit fields
      if (hasContext(Declaration)) return true;
      // clear Declaration since we already checked for it
      context &= ~Declaration;
    }
    return m_context & context;
  }
  bool hasAllContext(int context) const {
    return (m_context & context) == context;
  }
  bool hasSubExpr(ExpressionPtr sub) const;
  virtual void setComment(const std::string &) {}
  virtual std::string getComment() { return ""; }
  /**
   * Set this expression's error flags.
   */
  virtual void setError(Error error) { m_error |= error;}
  virtual void clearError(Error error) { m_error &= ~error;}
  int getError() const { return m_error;}
  bool hasError(Error error) const { return m_error & error; }

  ExprClass getExprClass() const;
  virtual ExpressionPtr getStoreVariable() const { return ExpressionPtr(); }
  void setArgNum(int n);

  /**
    * For generic walks
    */
  virtual int getKidCount() const { return 0; }
  ExpressionPtr getNthExpr(int n) const { return
      static_pointer_cast<Expression>(getNthKid(n)); }

  virtual bool isScalar() const { return false; }
  bool isArray() const;
  bool isCollection() const;
  virtual bool isRefable(bool checkError = false) const { return false; }
  virtual bool getScalarValue(Variant &value) { return false; }
  bool getEffectiveScalar(Variant &value);
  virtual ExpressionPtr clone() {
    assert(false);
    return ExpressionPtr();
  }
  virtual bool isThis() const { return false;}
  virtual bool isLiteralString() const { return false;}
  virtual bool isLiteralNull() const { return false;}
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

  virtual void analyzeProgram(AnalysisResultPtr ar);

  /**
   * Called before type inference.
   */
  virtual ExpressionPtr preOptimize(AnalysisResultConstPtr ar) {
    return ExpressionPtr();
  }

  static ExpressionPtr MakeConstant(AnalysisResultConstPtr ar,
                                    BlockScopePtr scope,
                                    const Location::Range& r,
                                    const std::string &value);
  static ExpressionPtr MakeScalarExpression(AnalysisResultConstPtr ar,
                                            BlockScopePtr scope,
                                            const Location::Range& r,
                                            const Variant &value);

  static bool CheckNeededRHS(ExpressionPtr value);
  static bool CheckNeeded(ExpressionPtr variable, ExpressionPtr value);

  bool isUnused() const { return m_unused; }
  void setUnused(bool u) { m_unused = u; }

  /**
   * Correctly compute the local expression altered bit
   */
  void computeLocalExprAltered();
protected:
  static bool IsIdentifier(const std::string &value);

  int m_context;
  int m_argNum;

private:
  bool m_unused;
  mutable int m_error;

protected:
  void setDynamicByIdentifier(AnalysisResultPtr ar,
                              const std::string &value);
 private:
  static ExprClass Classes[];
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXPRESSION_H_
