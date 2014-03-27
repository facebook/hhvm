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

#ifndef incl_HPHP_COMPILER_PARSER_H_
#define incl_HPHP_COMPILER_PARSER_H_

#include <functional>
#include <vector>

#include "hphp/runtime/base/exceptions.h"
#include "hphp/parser/parser.h"
#include "hphp/compiler/construct.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/type_annotation.h"
#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/statement/statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/util/logger.h"

#ifdef HPHP_PARSER_NS
#undef HPHP_PARSER_NS
#endif
#define HPHP_PARSER_NS Compiler

#ifdef HPHP_PARSER_ERROR
#undef HPHP_PARSER_ERROR
#endif
#define HPHP_PARSER_ERROR(fmt, p, args...)  \
  throw HPHP::ParseTimeFatalException((p)->file(), (p)->line1(), fmt, ##args)

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(LabelScope);
DECLARE_BOOST_TYPES(Location);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(BlockScope);
DECLARE_BOOST_TYPES(TypeAnnotation);

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////
// scanner

class Token : public ScannerToken {
public:
  ExpressionPtr exp;
  StatementPtr stmt;
  TypeAnnotationPtr typeAnnotation;

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

  Token& operator=(const Token& other) {
    ScannerToken::operator=(other);
    exp = other.exp;
    stmt = other.stmt;
    typeAnnotation = other.typeAnnotation;
    return *this;
  }

  void reset() {
    exp.reset();
    stmt.reset();
    typeAnnotation.reset();
    ScannerToken::reset();
  }

  const std::string typeAnnotationName() {
    return (typeAnnotation) ? typeAnnotation->fullName() : "";
  }
};

///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Parser);
class Parser : public ParserBase {
public:
  static StatementListPtr ParseString(const String& input, AnalysisResultPtr ar,
                                      const char *fileName = nullptr,
                                      bool lambdaMode = false);

public:
  Parser(Scanner &scanner, const char *fileName,
         AnalysisResultPtr ar, int fileSize = 0);

  // implementing ParserBase
  virtual bool parseImpl();
  bool parse();
  virtual void error(const char* fmt, ...) ATTRIBUTE_PRINTF(2,3);
  IMPLEMENT_XHP_ATTRIBUTES;

  virtual void fatal(const Location* loc, const char* msg);
  virtual void parseFatal(const Location* loc, const char* msg);
  std::string errString();

  // result
  StatementListPtr getTree() const { return m_tree;}

  // parser handlers
  void initParseTree();
  void finiParseTree();
  void onHaltCompiler();
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
  void onCall(Token &out, bool dynamic, Token &name, Token &params, Token *cls);
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

  void onListAssignment(Token &out, Token &vars, Token *expr,
                        bool rhsFirst = false);
  void onAListVar(Token &out, Token *list, Token *var);
  void onAListSub(Token &out, Token *list, Token &sublist);
  void onAssign(Token &out, Token &var, Token &expr, bool ref,
                bool rhsFirst = false);
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
  void onClassClass(Token &out, Token &cls, Token &name, bool text);
  void fixStaticVars();
  void onFunctionStart(Token &name, bool doPushComment = true);
  void onFunction(Token &out, Token *modifier, Token &ret, Token &ref,
                  Token &name, Token &params, Token &stmt, Token *attr);
  void onParam(Token &out, Token *params, Token &type, Token &var,
               bool ref, Token *defValue, Token *attr, Token *modifiers);
  void onClassStart(int type, Token &name);
  void onClass(Token &out, int type, Token &name, Token &base,
               Token &baseInterface, Token &stmt, Token *attr);
  void onInterface(Token &out, Token &name, Token &base, Token &stmt,
                   Token *attr);
  void onInterfaceName(Token &out, Token *names, Token &name);
  void onTraitRequire(Token &out, Token &name, bool isClass);
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
  void addTopStatement(Token &new_stmt);
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
  void onBreakContinue(Token &out, bool isBreak, Token *expr);
  void onReturn(Token &out, Token *expr);
  void onYield(Token &out, Token &expr);
  void onYieldPair(Token &out, Token &key, Token &val);
  void onYieldBreak(Token &out);
  void onAwait(Token &out, Token &expr);
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

  void onClosureStart(Token &name);
  Token onClosure(ClosureType type,
                  Token* modifiers,
                  Token& ref,
                  Token& params,
                  Token& cparams,
                  Token& stmts);
  Token onExprForLambda(const Token& expr);
  void onClosureParam(Token &out, Token *params, Token &param, bool ref);

  void onLabel(Token &out, Token &label);
  void onGoto(Token &out, Token &label, bool limited);
  void onTypedef(Token& out, const Token& name, const Token& type);

  void onTypeAnnotation(Token& out, const Token& name, const Token& typeArgs);
  void onTypeList(Token& type1, const Token& type2);
  void onTypeSpecialization(Token& type, char specialization);

  // for language integrated query expressions
  void onQuery(Token &out, Token &head, Token &body);
  void onQueryBody(Token &out, Token *clauses, Token &select, Token *cont);
  void onQueryBodyClause(Token &out, Token *clauses, Token &clause);
  void onFromClause(Token &out, Token &var, Token &coll);
  void onLetClause(Token &out, Token &var, Token &expr);
  void onWhereClause(Token &out, Token &expr);
  void onJoinClause(Token &out, Token &var, Token &coll, Token &left,
    Token &right);
  void onJoinIntoClause(Token &out, Token &var, Token &coll, Token &left,
    Token &right, Token &group);
  void onOrderbyClause(Token &out, Token &orderings);
  void onOrdering(Token &out, Token *orderings, Token &ordering);
  void onOrderingExpr(Token &out, Token &expr, Token *direction);
  void onSelectClause(Token &out, Token &expr);
  void onGroupClause(Token &out, Token &coll, Token &key);
  void onIntoClause(Token &out, Token &var, Token &query);

  // for namespace support
  void onNamespaceStart(const std::string &ns, bool file_scope = false);
  void onNamespaceEnd();
  void onUse(const std::string &ns, const std::string &as);
  void nns(int token = 0, const std::string& text = std::string());
  std::string nsDecl(const std::string &name);
  std::string resolve(const std::string &ns, bool cls);

  /*
   * Get the current label scope. A new label scope is demarcated by
   * one of the following: a loop, a switch statement, a finally block,
   * a try block or one of the constructs demarcating variables scopes
   * (i.e. functions, closures, etc.)
   * For every label scope, we keep track of the labels
   * that are available inside it. This is required for supporting
   * features such as try ... finally.
   */
  LabelScopePtr getLabelScope() const;

  /*
   * Called whenever a new label scope is entered. The fresh parameter
   * indicates whether the scope is also a variable scope (i.e. a
   * function, a closure, ...) or just a label scope (i.e. a loop body,
   * a switch statement, a finally block, ...).
   */
  void onNewLabelScope(bool fresh);

  /*
   * Called whenever a new label is encountered.
   */
  void onScopeLabel(const Token& stmt, const Token& label);

  /*
   * Called whenever a label scope ends. The fresh parameter has the
   * same meaning as for onNewLabelScope.
   */
  void onCompleteLabelScope(bool fresh);

  virtual void invalidateGoto(TStatementPtr stmt, GotoError error);
  virtual void invalidateLabel(TStatementPtr stmt);

  virtual TStatementPtr extractStatement(ScannerToken *stmt);

  FileScopePtr getFileScope() { return m_file; }

private:
  struct FunctionContext {
    FunctionContext()
      : hasReturn(false)
      , isGenerator(false)
      , isAsync(false)
    {}

    void checkFinalAssertions() {
      assert(!isGenerator || (!isAsync && !hasReturn));
    }

    bool hasReturn;       // function contains a return statement
    bool isGenerator;     // function determined to be a generator
    bool isAsync;         // function determined to be async
  };

  enum class FunctionType {
    Function,
    Method,
    Closure,
  };

  AnalysisResultPtr m_ar;
  FileScopePtr m_file;
  std::vector<std::string> m_comments; // for docComment stack
  std::vector<BlockScopePtrVec> m_scopes;
  std::vector<LabelScopePtrVec> m_labelScopes;
  std::vector<FunctionContext> m_funcContexts;
  std::vector<ScalarExpressionPtr> m_compilerHaltOffsetVec;
  std::string m_clsName; // for T_CLASS_C inside a closure
  std::string m_funcName;
  std::string m_containingFuncName;
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

  void invalidYield();
  bool setIsGenerator();

  void invalidAwait();
  bool setIsAsync();

  static bool canBeAsyncOrGenerator(string funcName, string clsName);
  void checkFunctionContext(string funcName,
                            FunctionContext& funcContext,
                            ModifierExpressionPtr modifiers,
                            int returnsRef);

  string getFunctionName(FunctionType type, Token* name);
  void prepareConstructorParameters(StatementListPtr stmts,
                                    ExpressionListPtr params,
                                    bool isAbstract);
  StatementPtr onFunctionHelper(FunctionType type,
                        Token *modifiers, Token &ret,
                        Token &ref, Token *name, Token &params,
                        Token &stmt, Token *attr, bool reloc);

  ExpressionPtr getDynamicVariable(ExpressionPtr exp, bool encap);
  ExpressionPtr createDynamicVariable(ExpressionPtr exp);

  bool hasType(Token &type);

  void checkAssignThis(Token &var);

  void addStatement(StatementPtr stmt, StatementPtr new_stmt);

  // for namespace support
  enum NamespaceState {
    SeenNothing,
    SeenNonNamespaceStatement,
    SeenNamespaceStatement,
    InsideNamespace,
  };

  /*
   * An AliasTable maps aliases to names.
   * We use it instead a regular map because it lazily imports a bunch of
   * names into the current namespace when appropriate.
   */
  class AliasTable {
  public:
    struct AliasEntry {
      std::string alias;
      std::string name;
    };

    enum class AliasType {
      USE,
      DEF
    };

    AliasTable(const hphp_string_imap<std::string>& autoAliases,
               std::function<bool ()> autoOracle);

    std::string getName(std::string alias);
    std::string getDefName(std::string alias);
    std::string getUseName(std::string alias);
    bool isAliased(std::string alias);
    bool isAutoType(std::string alias);
    bool isUseType(std::string alias);
    bool isDefType(std::string alias);
    void set(std::string alias, std::string name, AliasType type);
    void clear();

  private:
    struct NameEntry {
      std::string name;
      AliasType type;
    };

    hphp_string_imap<NameEntry> m_aliases;
    const hphp_string_imap<std::string>& m_autoAliases;
    // Returns true if stuff should be auto-imported.
    std::function<bool ()> m_autoOracle;
    void setFalseOracle();
  };

  NamespaceState m_nsState;
  bool m_nsFileScope;
  std::string m_namespace; // current namespace
  AliasTable m_aliasTable;

  void registerAlias(std::string name);
  bool isAutoAliasOn();
  const hphp_string_imap<std::string>& getAutoAliasedClasses();
  hphp_string_imap<std::string> getAutoAliasedClassesHelper();
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif
