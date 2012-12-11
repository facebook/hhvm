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

#ifndef __HPHP_PARSER_H__
#define __HPHP_PARSER_H__

#include <runtime/base/util/exceptions.h>
#include <util/parser/parser.h>
#include <compiler/construct.h>

#ifdef HPHP_PARSER_NS
#undef HPHP_PARSER_NS
#endif
#define HPHP_PARSER_NS Compiler

#ifdef HPHP_PARSER_ERROR
#undef HPHP_PARSER_ERROR
#endif
#define HPHP_PARSER_ERROR(fmt, p, args...)                              \
  do {                                                                  \
    if (!HPHP::hhvm) {                                                  \
      HPHP::Logger::Error(fmt " %s", ##args, (p)->getMessage(true).c_str()); \
    }                                                                   \
    throw HPHP::ParseTimeFatalException((p)->file(), (p)->line1(),      \
                                        fmt, ##args);                   \
  } while (0)


namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(Location);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(BlockScope);

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////
// scanner

class Token : public ScannerToken {
public:
  ExpressionPtr exp;
  StatementPtr stmt;

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
  void operator=(Token &other) {
    ScannerToken::operator=(other);
    exp = other.exp;
    stmt = other.stmt;
  }
  void reset() {
    exp.reset();
    stmt.reset();
    ScannerToken::reset();
  }
};

///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Parser);
class Parser : public ParserBase {
public:
  static StatementListPtr ParseString(CStrRef input, AnalysisResultPtr ar,
                                      const char *fileName = NULL,
                                      bool lambdaMode = false);

public:
  Parser(Scanner &scanner, const char *fileName,
         AnalysisResultPtr ar, int fileSize = 0);

  // implementing ParserBase
  virtual bool parseImpl();
  bool parse();
  virtual void error(const char* fmt, ...);
  virtual bool enableXHP();
  virtual bool enableHipHopSyntax();
  virtual bool enableFinallyStatement();
  IMPLEMENT_XHP_ATTRIBUTES;

  virtual void fatal(Location *loc, const char *msg);
  std::string errString();

  // result
  StatementListPtr getTree() const { return m_tree;}

  // parser handlers
  void saveParseTree(Token &tree);
  void onName(Token &out, Token &name, NameKind kind);
  void onVariable(Token &out, Token *exprs, Token &var, Token *value,
                  bool constant = false,
                  const std::string &docComment = "");
  void onStaticVariable(Token &out, Token *exprs, Token &var, Token *value);
  void onClassVariableModifer(Token &mod) {}
  void onClassVariableStart(Token &out, Token *modifiers, Token &decl,
                            Token *type);
  void onClassVariable(Token &out, Token *exprs, Token &var, Token *value);
  void onClassConstant(Token &out, Token *exprs, Token &var, Token &value);
  void onSimpleVariable(Token &out, Token &var);
  void onSynthesizedVariable(Token &out, Token &var) {
    onSimpleVariable(out, var);
  }
  void onDynamicVariable(Token &out, Token &expr, bool encap);
  void onIndirectRef(Token &out, Token &refCount, Token &var);
  void onStaticMember(Token &out, Token &cls, Token &name);
  void onRefDim(Token &out, Token &var, Token &offset);
  void onCallParam(Token &out, Token *params, Token &expr, bool ref);
  void onCall(Token &out, bool dynamic, Token &name, Token &params,
              Token *cls, bool fromCompiler = false);
  void onEncapsList(Token &out, int type, Token &list);
  void addEncap(Token &out, Token *list, Token &expr, int type);
  void encapRefDim(Token &out, Token &var, Token &offset);
  void encapObjProp(Token &out, Token &var, Token &name);
  void encapArray(Token &out, Token &var, Token &expr);
  void onConstantValue(Token &out, Token &constant);
  void onScalar(Token &out, int type, Token &scalar);
  void onExprListElem(Token &out, Token *exprs, Token &expr);

  void onObjectProperty(Token &out, Token &base, Token &prop);
  void onObjectMethodCall(Token &out, Token &base, Token &prop, Token &params);

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
  void onEmptyCollection(Token &out);
  void onCollectionPair(Token &out, Token *pairs, Token *name, Token &value);
  void onUserAttribute(Token &out, Token *attrList, Token &name, Token &value);
  void onClassConst(Token &out, Token &cls, Token &name, bool text);
  void fixStaticVars();
  void onFunctionStart(Token &name, bool doPushComment = true);
  void onFunction(Token &out, Token &ret, Token &ref, Token &name,
                  Token &params, Token &stmt, Token *attr);
  void onParam(Token &out, Token *params, Token &type, Token &var,
               bool ref, Token *defValue, Token *attr);
  void onClassStart(int type, Token &name);
  void onClass(Token &out, int type, Token &name, Token &base,
               Token &baseInterface, Token &stmt, Token *attr);
  void onInterface(Token &out, Token &name, Token &base, Token &stmt,
                   Token *attr);
  void onInterfaceName(Token &out, Token *names, Token &name);
  void onTraitUse(Token &out, Token &traits, Token &rules);
  void onTraitName(Token &out, Token *names, Token &name);
  void onTraitRule(Token &out, Token &stmtList, Token &newStmt);
  void onTraitPrecRule(Token &out, Token &className, Token &methodName,
                       Token &otherClasses);
  void onTraitAliasRuleStart(Token &out, Token &className, Token &methodName);
  void onTraitAliasRuleModify(Token &out, Token &rule, Token &accessModifiers,
                         Token &newMethodName);
  void onMethodStart(Token &name, Token &mods, bool doPushComment = true);
  void onMethod(Token &out, Token &modifiers, Token &ret, Token &ref,
                Token &name, Token &params, Token &stmt, Token *attr,
                bool reloc = true);
  void onMemberModifier(Token &out, Token *modifiers, Token &modifier);
  void onStatementListStart(Token &out);
  void addStatement(Token &out, Token &stmts, Token &new_stmt);
  void onClassStatement(Token &out, Token &stmts, Token &new_stmt) {
    addStatement(out, stmts, new_stmt);
  }
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
  void onYield(Token &out, Token *expr, bool assign);
  void onYieldBreak(Token &out);
  void onGlobal(Token &out, Token &expr);
  void onGlobalVar(Token &out, Token *exprs, Token &expr);
  void onStatic(Token &out, Token &expr);
  void onEcho(Token &out, Token &expr, bool html);
  void onUnset(Token &out, Token &expr);
  void onExpStatement(Token &out, Token &expr);
  void onForEachStart();
  void onForEach(Token &out, Token &arr, Token &name, Token &value,
                 Token &stmt);
  void onTry(Token &out, Token &tryStmt, Token &className, Token &var,
             Token &catchStmt, Token &catches, Token &finallyStmt);
  void onTry(Token &out, Token &tryStmt, Token &finallyStmt);
  void onCatch(Token &out, Token &catches, Token &className, Token &var,
               Token &stmt);
  void onFinally(Token &out, Token &stmt);
  void onThrow(Token &out, Token &expr);

  void onClosure(Token &out, Token &ret, Token &ref, Token &params,
                 Token &cparams, Token &stmts);
  void onClosureParam(Token &out, Token *params, Token &param, bool ref);
  void onLabel(Token &out, Token &label);
  void onGoto(Token &out, Token &label, bool limited);

  virtual void invalidateGoto(TStatementPtr stmt, GotoError error);
  virtual void invalidateLabel(TStatementPtr stmt);

  virtual TStatementPtr extractStatement(ScannerToken *stmt);

  FileScopePtr getFileScope() { return m_file; }

private:
  struct FunctionContext {
    FunctionContext()
      : isNotGenerator(false)
      , isGenerator(false)
      , numYields(0)
      , numForeaches(0)
    {}

    // mark this function as generator; returns true on success
    bool setIsGenerator() {
      if (!isNotGenerator) isGenerator = true;
      return !isNotGenerator;
    }

    // mark this function as non-generator; returns true on success
    bool setIsNotGenerator() {
      if (!isGenerator) isNotGenerator = true;
      return !isGenerator;
    }

    void checkFinalAssertions() {
      ASSERT(!isGenerator || !isNotGenerator);
      ASSERT(foreachHasYield.empty());
    }

    bool isNotGenerator;  // function determined to not be a generator
    bool isGenerator;     // function determined to be a generator
    int numYields;        // number of plain yield statements seen so far
    int numForeaches;     // number of foreach statements seen so far
    std::vector<bool> foreachHasYield;  // whether open foreach has yield
 };

  AnalysisResultPtr m_ar;
  FileScopePtr m_file;
  std::vector<std::string> m_comments; // for docComment stack
  std::vector<BlockScopePtrVec> m_scopes;
  std::vector<FunctionContext> m_funcContexts;
  std::vector<std::vector<StatementPtr> > m_prependingStatements;
  std::string m_clsName; // for T_CLASS_C inside a closure
  std::string m_funcName;
  bool m_inTrait;

  // parser output
  StatementListPtr m_tree;
  std::string m_error;

  std::vector<bool> m_hasCallToGetArgs;
  std::vector<StringToExpressionPtrVecMap> m_staticVars;
  bool m_lambdaMode;
  bool m_closureGenerator;

  void pushComment();
  void pushComment(const std::string& s);
  std::string popComment();

  void newScope();
  void completeScope(BlockScopePtr inner);

  bool setIsGenerator();

  ExpressionPtr getDynamicVariable(ExpressionPtr exp, bool encap);
  ExpressionPtr createDynamicVariable(ExpressionPtr exp);

  bool hasType(Token &type);

  void checkAssignThis(Token &var);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_PARSER_H__
