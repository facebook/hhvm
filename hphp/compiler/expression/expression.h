/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
  ExpressionPtr clone() override;                                       \
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
struct Variant;

struct Expression : Construct {
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
    RefValue  = 8,               // &exp
    InOutParameter = 0x10,       // inout exp
    ObjectContext = 0x20,        // $obj->
    InParameterExpression = 0x40,// for default value expression
    ExistContext = 0x80,         // isset(...) or empty(...) recursively
    UnsetContext = 0x100,        // Within unset(...), arr el recursively
    AssignmentLHS = 0x200,       // LHS in assignment
    InvokeArgument = 0x800,      // Invoke arguments
    RefParameter   = 0x1000,     // eg f(&$x)
    DeepReference = 0x40000,     // Value is not available for copy propagation
                                 // because it is referenced in some way
                                 // eg $b in &$b['foo']
    AccessContext = 0x80000,     // ArrayElementExpression::m_variable or
                                 // ObjectPropertyExpression::m_object
    RefAssignmentLHS = 0x100000, // LHS of a reference assignment
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
  explicit Expression(EXPRESSION_CONSTRUCTOR_BASE_PARAMETERS);

public:

  /**
   * Set this expression's context.
   */
  virtual void setContext(Context context) { m_context |= context;}
  virtual void clearContext(Context context) { m_context &= ~context;}
  void copyContext(ExpressionPtr from) { copyContext(from->m_context); }
  void copyContext(int contexts);
  ExpressionPtr replaceValue(ExpressionPtr rep, bool noWarn = false);
  void clearContext();
  int getContext() const { return m_context;}
  bool hasContext(Context context) const {
    return (m_context & context) == context;
  }
  bool hasAnyContext(int context) const {
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

  virtual ExpressionPtr getStoreVariable() const { return ExpressionPtr(); }
  void setArgNum(int n);

  /**
    * For generic walks
    */
  int getKidCount() const override {
    return 0;
  }
  ExpressionPtr getNthExpr(int n) const { return
      static_pointer_cast<Expression>(getNthKid(n)); }

  virtual bool isScalar() const { return false; }
  bool isArray() const;
  bool isCollection() const;
  virtual bool isRefable(bool /*checkError*/ = false) const { return false; }
  virtual bool getScalarValue(Variant& /*value*/) { return false; }
  bool getEffectiveScalar(Variant &value);
  virtual ExpressionPtr clone() {
    assert(false);
    return ExpressionPtr();
  }
  virtual bool isThis() const { return false;}
  virtual bool isLiteralString() const { return false;}
  virtual bool isLiteralNull() const { return false;}
  virtual std::string getLiteralString() const { return "";}
  void deepCopy(ExpressionPtr exp);

  /**
   * This is to avoid dynamic casting to ExpressionList in Parser.
   */
  virtual void addElement(ExpressionPtr exp);
  virtual void insertElement(ExpressionPtr exp, int index = 0);

  virtual ExpressionPtr preOptimize(AnalysisResultConstRawPtr /*ar*/) {
    return ExpressionPtr();
  }

  static ExpressionPtr MakeConstant(AnalysisResultConstRawPtr ar,
                                    BlockScopePtr scope,
                                    const Location::Range& r,
                                    const std::string &value);
  static ExpressionPtr MakeScalarExpression(AnalysisResultConstRawPtr ar,
                                            BlockScopePtr scope,
                                            const Location::Range& r,
                                            const Variant &value);

  bool isUnused() const { return m_unused; }
  void setUnused(bool u) { m_unused = u; }
protected:
  static bool IsIdentifier(const std::string &value);

  int m_context;
  int m_argNum;

private:
  bool m_unused;
  mutable int m_error;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXPRESSION_H_
