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

#ifndef __EVAL_PARSER_H__
#define __EVAL_PARSER_H__

#include <stack>
#include <util/parser/parser.h>
#include <runtime/eval/base/eval_base.h>
#include <runtime/eval/ast/statement.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/ast/static_statement.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/temp_expression.h>
#include <runtime/eval/ast/name.h>

#ifdef HPHP_PARSER_NS
#undef HPHP_PARSER_NS
#endif
#define HPHP_PARSER_NS Eval

#ifdef HPHP_PARSER_ERROR
#undef HPHP_PARSER_ERROR
#endif
#define HPHP_PARSER_ERROR HPHP::raise_error

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_AST_PTR(Statement);
DECLARE_AST_PTR(ClassStatement);
DECLARE_AST_PTR(FunctionStatement);
DECLARE_AST_PTR(StaticStatement);
DECLARE_AST_PTR(IfBranch);
DECLARE_AST_PTR(StatementListStatement);
DECLARE_AST_PTR(CaseStatement);
DECLARE_AST_PTR(ListElement);
DECLARE_AST_PTR(ArrayPair);
DECLARE_AST_PTR(CatchBlock);
DECLARE_AST_PTR(Parameter);
DECLARE_AST_PTR(Name);
DECLARE_AST_PTR(StaticVariable);

class Token : public ScannerToken {
public:
  enum Mode {
    None,
    SingleExpression,
    SingleStatement,
    SingleName,
    IfBranch,
    CaseStatement,
    Expression,
    ListElement,
    ArrayPair,
    CatchBlock,
    Parameter,
    Name,
    StaticVariable,
    Strings
  };

  Token() : m_mode(None), m_data(NULL) {
  }
  ~Token() {
    reset();
  }
  void reset();

  Token &operator+(const char *str) {
    m_text += str;
    return *this;
  }
  Token &operator+(const Token &token) {
    m_num += token.m_num;
    m_text += token.m_text;
    return *this;
  }
  Token *operator->() {
    return this;
  }
  void operator=(int num) {
    m_num = num;
  }
  void operator=(Token &other);

  template<class T>
  AstPtr<T> getExp() const {
    return m_exp ? m_exp->unsafe_cast<T>() : AstPtr<T>();
  }
  template<class T>
  AstPtr<T> getStmt() const {
    return m_stmt ? m_stmt->unsafe_cast<T>() : AstPtr<T>();
  }
  StatementListStatementPtr getStmtList() const;

  ExpressionPtr &exp();
  StatementPtr &stmt();
  NamePtr &name();

  Mode getMode() const { return m_mode; }

  std::vector<IfBranchPtr       > &ifBranches ();
  std::vector<ExpressionPtr     > &exprs      ();
  std::vector<CaseStatementPtr  > &cases      ();
  std::vector<ListElementPtr    > &listElems  ();
  std::vector<ArrayPairPtr      > &arrayPairs ();
  std::vector<CatchBlockPtr     > &catches    ();
  std::vector<ParameterPtr      > &params     ();
  std::vector<NamePtr           > &names      ();
  std::vector<StaticVariablePtr > &staticVars ();
  std::vector<String            > &strings    ();

private:
  ExpressionPtr m_exp;
  StatementPtr m_stmt;
  NamePtr m_name;

  class Countable {
  public:
    Countable() : m_count(1) {}
    virtual ~Countable() {}

    void dec() { if (--m_count == 0) delete this;}
    void inc() { ++m_count;}

  private:
    int m_count;
  };

  template<typename T>
  class CountableVector : public Countable {
  public:
    std::vector<T> m_vec;
  };

  Mode m_mode;
  Countable *m_data;
};

///////////////////////////////////////////////////////////////////////////////

class Parser : public ParserBase {
public:
  static StatementPtr ParseString(const char *input,
                                  std::vector<StaticStatementPtr> &statics,
                                  Block::VariableIndices &variableIndice);

  static StatementPtr ParseFile(const char *fileName,
                                std::vector<StaticStatementPtr> &statics,
                                Block::VariableIndices &variableIndices);

public:
  Parser(Scanner &scanner, const char *fileName,
         std::vector<StaticStatementPtr> &statics);

  // implementing ParserBase
  virtual bool parse();
  virtual void error(const char* fmt, ...);
  virtual bool enableXHP();
  IMPLEMENT_XHP_ATTRIBUTES;

  // result
  StatementPtr getTree() const;

  // parser handlers
  void saveParseTree(Token &tree);
  void onName(Token &out, Token &name, NameKind kind);
  void onStaticVariable(Token &out, Token *exprs, Token &var, Token *value);
  void onSimpleVariable(Token &out, Token &var);
  void onSynthesizedVariable(Token &out, Token &var);
  void onDynamicVariable(Token &out, Token &expr, bool encap);
  void onIndirectRef(Token &out, Token &refCount, Token &var);
  void onStaticMember(Token &out, Token &className, Token &name);
  void onRefDim(Token &out, Token &var, Token &offset);
  void onCallParam(Token &out, Token *params, Token &expr, bool ref);
  void onCall(Token &out, bool dynamic, Token &name, Token &params,
              Token *className);
  void onEncapsList(Token &out, int type, Token &list);
  void addEncap(Token &out, Token &list, Token &expr, int type);
  void encapRefDim(Token &out, Token &var, Token &offset);
  void encapObjProp(Token &out, Token &var, Token &name);
  void encapArray(Token &out, Token &var, Token &expr);
  void onConstantValue(Token &out, Token &constant);
  void onScalar(Token &out, int type, Token &scalar);
  void onExprListElem(Token &out, Token *exprs, Token &expr);

  void pushObject(Token &base);
  void popObject(Token &out);
  void appendMethodParams(Token &params);
  void appendProperty(Token &prop);
  void appendRefDim(Token &offset);

  void onListAssignment(Token &out, Token &vars, Token *expr);
  void onAListVar(Token &out, Token *list, Token *var);
  void onAListSub(Token &out, Token *list, Token &sublist);
  void onAssign(Token &out, Token &var, Token &expr, bool ref);
  void onAssignNew(Token &out, Token &var, Token &name, Token &args);
  void onNewObject(Token &out, Token &name, Token &args);
  void onUnaryOpExp(Token &out, Token &operand, int op, bool front);
  void onBinaryOpExp(Token &out, Token &operand1, Token &operand2, int op);
  void onQOp(Token &out, Token &exprCond, Token *expYes, Token &expNo);
  void onArray(Token &out, Token &pairs, int op = T_ARRAY);
  void onArrayPair(Token &out, Token *pairs, Token *name, Token &value,
                   bool ref);
  void onClassConst(Token &out, Token &className, Token &name, bool text);
  void onFunctionStart(Token &name);
  void onFunction(Token &out, Token &ret, Token &ref, Token &name,
                  Token &params, Token &stmt);
  void onParam(Token &out, Token *params, Token &type, Token &var,
               bool ref, Token *defValue);
  void onClassStart(int type, Token &name, Token *parent);
  void onClass(Token &out, Token &type, Token &name, Token &base,
               Token &baseInterface, Token &stmt);
  void onInterface(Token &out, Token &name, Token &base, Token &stmt);
  void onInterfaceName(Token &out, Token *names, Token &name);
  void onClassVariableModifer(Token &mod);
  void onClassVariableStart(Token &out, Token *modifiers, Token &decl,
                            Token *type);
  void onClassVariable(Token &out, Token *exprs, Token &name, Token *val);
  void onClassConstant(Token &out, Token *exprs, Token &name, Token &val);
  void onMethodStart(Token &name, Token &mods);
  void onMethod(Token &out, Token &modifiers, Token &ret, Token &ref,
                Token &name, Token &params, Token &stmt, bool reloc = true);
  void onMemberModifier(Token &out, Token *modifiers, Token &modifier);
  void onStatementListStart(Token &out);
  void addStatement(Token &out, Token &stmts, Token &new_stmt);
  void onClassStatement(Token &out, Token &stmts, Token &new_stmt) {}
  void finishStatement(Token &out, Token &stmts);
  void onBlock(Token &out, Token &stmts);
  void onIf(Token &out, Token &cond, Token &stmt, Token &elseifs,
            Token &elseStmt);
  void onElseIf(Token &out, Token &elseifs, Token &cond, Token &stmt);
  void onWhile(Token &out, Token &cond, Token &stmt);
  void onDo(Token &out, Token &stmt, Token &cond);
  void onFor(Token &out, Token &expr1, Token &expr2, Token &expr3,
             Token &stmt);
  void onSwitch(Token &out, Token &expr, Token &cases);
  void onCase(Token &out, Token &cases, Token *cond, Token &stmt);
  void onBreak(Token &out, Token *expr);
  void onContinue(Token &out, Token *expr);
  void onReturn(Token &out, Token *expr, bool checkYield = true);
  void onYield(Token &out, Token *expr);
  void onGlobal(Token &out, Token &expr);
  void onGlobalVar(Token &out, Token *exprs, Token &expr);
  void onStatic(Token &out, Token &expr);
  void onEcho(Token &out, Token &expr, bool html);
  void onUnset(Token &out, Token &expr);
  void onExpStatement(Token &out, Token &expr);
  void onForEach(Token &out, Token &arr, Token &name, Token &value,
                 Token &stmt);
  void onTry(Token &out, Token &tryStmt, Token &className, Token &var,
             Token &catchStmt, Token &catches);
  void onCatch(Token &out, Token &catches, Token &className, Token &var,
               Token &stmt);
  void onThrow(Token &out, Token &expr);

  void onClosure(Token &out, Token &ret, Token &ref, Token &params,
                 Token &cparams, Token &stmts);
  void onClosureParam(Token &out, Token *params, Token &param, bool ref);
  void onLabel(Token &out, Token &label);
  void onGoto(Token &out, Token &label, bool limited);

  void onTypeDecl(Token &out, Token &type, Token &decl);
  void onTypedVariable(Token &out, Token *exprs, Token &var, Token *value);

  ClassStatementPtr currentClass() const;

  bool haveClass() const;
  ClassStatementPtr peekClass() const;
  bool haveFunc() const;
  FunctionStatementPtr peekFunc() const;

  const Block::VariableIndices &varIndices() const {
    return m_fileBlock.varIndices();
  }
private:
  Block m_fileBlock; // for pseudomain variables
  std::vector<ExpressionPtr> m_objects; // for parsing obj prop/method calls
  typedef std::pair<ClassStatementPtr, FunctionStatementPtr> ScopePtrPair;
  std::vector<ScopePtrPair> m_scopes;
  std::vector<StaticStatementPtr> &m_staticStatements;
  int m_classVarMods;

  std::vector<int> m_foreaches;

  // parser output
  StatementPtr m_tree;

  void pushClass(ClassStatementPtr cl);
  void popClass();
  std::string getCurrentClass(); // lexical scope

  void pushFunc(FunctionStatementPtr fs);
  void popFunc();
  std::vector<bool> m_hasCallToGetArgs;
  std::vector<std::vector<StatementPtr> > m_pendingStatements;

  ExpressionPtr getDynamicVariable(ExpressionPtr exp, bool encap);
  ExpressionPtr createDynamicVariable(ExpressionPtr exp);
  NamePtr procStaticClassName(Token &className, bool text);

  void assignImpl(Token &out, Token &var, ExpressionPtr exp, bool ref);

  /**
   * evaluation order correction
   */
  TempExpressionPtr m_offset; // created by createOffset()
  ExpressionPtr createOffset(ExpressionPtr var, ExpressionPtr offset);
  void setOffset(ExpressionPtr &out, ExpressionPtr var, ExpressionPtr offset);

  bool hasType(Token &type);

  void throw_invalid_lval();
  class ParserFrameInjection : public FrameInjection {
  public:
    ParserFrameInjection(const char *func, const char *fileName);
    String getFileName() { return m_file; }
  private:
    const char *m_file;
  };
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_PARSER_H__
