/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include <stack>
#include <string>
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
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/util/logger.h"
#include "hphp/parser/parse-time-fatal-exception.h"

#ifdef HPHP_PARSER_NS
#undef HPHP_PARSER_NS
#endif
#define HPHP_PARSER_NS Compiler

#ifdef HPHP_PARSER_ERROR
#undef HPHP_PARSER_ERROR
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(LabelScope);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(BlockScope);
DECLARE_BOOST_TYPES(TypeAnnotation);

namespace Compiler {

enum class ThisContextError {
  Assign,
  NullSafeBase
};

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
  virtual bool parseImpl5();
  virtual bool parseImpl7();
  bool parse();
  virtual void error(ATTRIBUTE_PRINTF_STRING const char* fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
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
                            Token *type, bool abstract = false,
                            bool typeconst = false,
                            const TypeAnnotationPtr& typeAnnot = nullptr);
  void onClassVariable(Token &out, Token *exprs, Token &var, Token *value);
  void onClassConstant(Token &out, Token *exprs, Token &var, Token &value);
  void onClassAbstractConstant(Token &out, Token *exprs, Token &var);
  void onClassTypeConstant(Token &out, Token &var, Token &value);
  void onSimpleVariable(Token &out, Token &var);
  void onPipeVariable(Token &out);
  void onSynthesizedVariable(Token &out, Token &var) {
    onSimpleVariable(out, var);
  }
  void onDynamicVariable(Token &out, Token &expr, bool encap);
  void onIndirectRef(Token &out, Token &refCount, Token &var);
  void onStaticMember(Token &out, Token &cls, Token &name);
  void onRefDim(Token &out, Token &var, Token &offset);
  void onCallParam(Token &out, Token *params, Token &expr,
                   bool ref, bool unpack);
  void onCall(Token &out, bool dynamic, Token &name, Token &params, Token *cls);
  void onEncapsList(Token &out, int type, Token &list);
  void addEncap(Token &out, Token *list, Token &expr, int type);
  void encapRefDim(Token &out, Token &var, Token &offset);
  void encapObjProp(Token &out, Token &var,
                    PropAccessType propAccessType, Token &name);
  void encapArray(Token &out, Token &var, Token &expr);
  void onConst(Token &out, Token &name, Token &value);
  void onConstantValue(Token &out, Token &constant);
  void onScalar(Token &out, int type, Token &scalar);
  void onExprListElem(Token &out, Token *exprs, Token &expr);

  void onObjectProperty(Token &out, Token &base,
                        PropAccessType propAccessType, Token &prop);
  void onObjectMethodCall(Token &out, Token &base, bool nullsafe, Token &prop,
                          Token &params);

  void checkClassDeclName(const std::string& name);
  void checkAllowedInWriteContext(ExpressionPtr e);

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
  void onNullCoalesce(Token &out, Token &expFirst, Token &expSecond);
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
  void onVariadicParam(Token &out, Token *params,
                       Token &type, Token &var,
                       bool ref, Token *attr, Token *modifier);
  void onParam(Token &out, Token *params, Token &type, Token &var,
               bool ref, Token *defValue, Token *attr, Token *modifier);
  void onClassStart(int type, Token &name);
  void onClass(Token &out, int type, Token &name, Token &base,
               Token &baseInterface, Token &stmt, Token *attr,
               Token *enumTy);
  void onClassExpressionStart();
  void onClassExpression(Token &out, Token &args, Token &base,
                         Token &baseInterface, Token &stmt);
  void onInterface(Token &out, Token &name, Token &base, Token &stmt,
                   Token *attr);
  void onEnum(Token &out, Token &name, Token &baseTy,
              Token &stmt, Token *attr);
  void onInterfaceName(Token &out, Token *names, Token &name);
  void onClassRequire(Token &out, Token &name, bool isClass);
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
  void onYield(Token &out, Token *expr);
  void onYieldFrom(Token &out, Token *expr);
  void onYieldPair(Token &out, Token *key, Token *val);
  void onYieldBreak(Token &out);
  void onAwait(Token &out, Token &expr);
  void onGlobal(Token &out, Token &expr);
  void onGlobalVar(Token &out, Token *exprs, Token &expr);
  void onStatic(Token &out, Token &expr);
  void onHashBang(Token &out, Token &text);
  void onEcho(Token &out, Token &expr, bool html);
  void onUnset(Token &out, Token &expr);
  void onExpStatement(Token &out, Token &expr);
  void onForEachStart();
  void onForEach(Token &out, Token &arr, Token &name, Token &value,
                 Token &stmt, bool awaitAs);
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
                  Token& stmts,
                  Token& ret1,
                  Token* ret2 = nullptr);
  Token onExprForLambda(const Token& expr);
  void onClosureParam(Token &out, Token *params, Token &param, bool ref);

  void onLabel(Token &out, Token &label);
  void onGoto(Token &out, Token &label, bool limited);
  void setTypeVars(Token &out, const Token &name);
  void onTypedef(Token& out, const Token& name, const Token& type,
                 const Token* attr = nullptr);

  void onTypeAnnotation(Token& out, const Token& name, const Token& typeArgs);
  void onTypeList(Token& type1, const Token& type2);
  void onTypeSpecialization(Token& type, char specialization);
  void onClsCnsShapeField(Token& out, const Token& cls, const Token& cns,
    const Token& value);
  void onShape(Token& out, const Token& shapeMemberList);

  // for namespace support
  void onNamespaceStart(const std::string &ns, bool file_scope = false);
  void onNamespaceEnd();
  void nns(int token = 0, const std::string& text = std::string());
  std::string nsClassDecl(const std::string &name);
  std::string nsDecl(const std::string &name);
  std::string resolve(const std::string &ns, bool cls);

  typedef void (Parser::*UseDeclarationConsumer)
    (const std::string&, const std::string&);

  void onUseDeclaration(Token& out, const std::string &ns,
                                    const std::string &as);
  void onMixedUseDeclaration(Token &out,
                             Token &use, UseDeclarationConsumer f);

  /*
   * The consumer parameter here vs the mixed_consumer in the
   * UseDeclarationStatementFragment is annoying. The gist is that the following
   * are both valid:
   *
   *   use function Foo\{Bar, Baz};
   *   use Foo\{function Bar, function Baz};
   *
   * This means that we need to track the type of the use with each individual
   * element in the latter "mixed" case, but also need to be able to specify it
   * for the whole clause in the former case. Thankfully, you can't mix the two,
   * so we can have some assertions around this.
   *
   * In the former case, the mixed_consumer member will always be nullptr, and
   * the consumer parameter to these two following functions must be specified.
   * In the latter case, the mixed_consumer member will always be specified, and
   * the consumer parameter to these two following function must be nullptr.
   *
   * This means exactly one will be non-null, and we can assert that the other
   * is null.
   */
  void onUse(const Token &tok, UseDeclarationConsumer f);
  void onGroupUse(const std::string &prefix, const Token &tok,
                  UseDeclarationConsumer f);

  void useClass(const std::string &fn, const std::string &as);
  void useFunction(const std::string &fn, const std::string &as);
  void useConst(const std::string &cnst, const std::string &as);

  void onDeclare(Token& out, Token& block);
  void onDeclareList(Token& out, Token& ident, Token& value);

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
      : hasCallToGetArgs(false)
      , hasNonEmptyReturn(false)
      , isGenerator(false)
      , isAsync(false)
      , mayCallSetFrameMetadata(false)
    {}

    // Function contains a call to func_num_args, func_get_args or func_get_arg.
    bool hasCallToGetArgs;

    // Function contains a non-empty return statement.
    bool hasNonEmptyReturn;

    // Function determined to be a generator.
    bool isGenerator;

    // Function determined to be async.
    bool isAsync;

    // Function may contain a call to set_frame_metadata.
    bool mayCallSetFrameMetadata;
  };

  enum class FunctionType {
    Function,
    Method,
    Closure,
  };

  struct ClassContext {
    ClassContext(int type, std::string name)
      : type(type), name(name) {}

    int type;
    std::string name;
  };

  AnalysisResultPtr m_ar;
  FileScopePtr m_file;
  std::vector<std::string> m_comments; // for docComment stack
  std::vector<std::vector<BlockScopePtr>> m_scopes;
  std::vector<std::vector<LabelScopePtr>> m_labelScopes;
  std::vector<FunctionContext> m_funcContexts;
  std::vector<ScalarExpressionPtr> m_compilerHaltOffsetVec;
  std::stack<ClassContext> m_clsContexts;
  std::string m_funcName;
  std::string m_containingFuncName;

  // parser output
  StatementListPtr m_tree;
  std::string m_error;

  std::vector<StringToExpressionPtrVecMap> m_staticVars;
  bool m_lambdaMode;
  bool m_closureGenerator;

  void pushComment();
  void pushComment(const std::string& s);
  std::string popComment();

  void newScope();
  void completeScope(BlockScopePtr inner);

  const std::string& clsName() const;
  bool inTrait() const;

  void setHasNonEmptyReturn(ConstructPtr blame);

  void invalidYield();
  void setIsGenerator();

  void invalidAwait();
  void setIsAsync();

  static bool canBeAsyncOrGenerator(const std::string& funcName,
                                    const std::string& clsName);
  void checkFunctionContext(const std::string& funcName,
                            FunctionContext& funcContext,
                            ModifierExpressionPtr modifiers,
                            int returnsRef);

  std::string getFunctionName(FunctionType type, Token* name);
  void prepareConstructorParameters(StatementListPtr stmts,
                                    ExpressionListPtr params,
                                    bool isAbstract);
  StatementPtr onFunctionHelper(FunctionType type,
                        Token *modifiers, Token &ret,
                        Token &ref, Token *name, Token &params,
                        Token &stmt, Token *attr, bool reloc);
  StatementPtr onClassHelper(int type, const std::string &name, Token &base,
                 Token &baseInterface, Token &stmt, Token *attr,
                 Token *enumTy);

  ExpressionPtr getDynamicVariable(ExpressionPtr exp, bool encap);
  ExpressionPtr createDynamicVariable(ExpressionPtr exp);

  void checkThisContext(const std::string& var, ThisContextError error);
  void checkThisContext(Token &var, ThisContextError error);
  void checkThisContext(ExpressionPtr e, ThisContextError error);
  void checkThisContext(ExpressionListPtr params, ThisContextError error);

  void addStatement(StatementPtr stmt, StatementPtr new_stmt);

  // for namespace support
  enum NamespaceState {
    SeenNothing,
    SeenNonNamespaceStatement,
    SeenNamespaceStatement,
    InsideNamespace,
  };

public:
  /*
   * An AliasTable maps aliases to names.
   * We use it instead a regular map because it lazily imports a bunch of
   * names into the current namespace when appropriate.
   */
  struct AliasTable {
    enum class AliasFlags {
      None = 0,
      HH = 0x1,
      PHP7_ScalarTypes = 0x2,
      PHP7_EngineExceptions = 0x4,
    };

    struct AutoAlias {
      std::string name;
      AliasFlags flags;
    };

    enum class AliasType {
      NONE,
      USE,
      AUTO_USE,
      DEF
    };

    using AutoAliasMap = std::unordered_map<
      std::string,
      AutoAlias,
      string_hashi,
      string_eqstri>;

    AliasTable(const AutoAliasMap& aliases,
               std::function<AliasFlags ()> autoOracle);

    std::string getName(const std::string& alias, int line_no);
    std::string getNameRaw(const std::string& alias);
    AliasType getType(const std::string& alias);
    int getLine(const std::string& alias);
    bool isAliased(const std::string& alias);
    void set(const std::string& alias,
             const std::string& name,
             AliasType type,
             int line_no);
    void clear();

  private:
    struct NameEntry {
      std::string name;
      AliasType type;
      int line_no;
    };

    hphp_string_imap<NameEntry> m_aliases;
    const AutoAliasMap& m_autoAliases;
    // Returns true if stuff should be auto-imported.
    std::function<AliasFlags ()> m_autoOracle;
    void setFalseOracle();
    const AutoAliasMap& getAutoAliases();
  };

  using AutoAlias = AliasTable::AutoAlias;
  using AliasType = AliasTable::AliasType;
  using AliasFlags = AliasTable::AliasFlags;
  using AutoAliasMap = AliasTable::AutoAliasMap;

private:
  NamespaceState m_nsState;
  bool m_nsFileScope;
  std::string m_namespace; // current namespace
  AliasTable m_nsAliasTable;
  std::vector<uint32_t> m_nsStack;

  // Function aliases
  hphp_string_iset m_fnTable;
  hphp_string_imap<std::string> m_fnAliasTable;

  // Constant aliases
  hphp_string_set m_cnstTable;
  hphp_string_map<std::string> m_cnstAliasTable;

  void registerAlias(std::string name);
  AliasFlags getAliasFlags();
  const AutoAliasMap& getAutoAliasedClasses();
};

inline Parser::AliasFlags operator|(const Parser::AliasFlags& lhs,
                                    const Parser::AliasFlags& rhs) {
  return (Parser::AliasFlags)((unsigned)lhs | (unsigned)rhs);
}

inline unsigned operator&(const Parser::AliasFlags& lhs,
                          const Parser::AliasFlags& rhs) {
  return (unsigned)lhs & (unsigned)rhs;
}

template<typename... Args>
inline void HPHP_PARSER_ERROR(const char* fmt,
                       Parser* p,
                       Args&&... args) {
  throw ParseTimeFatalException(p->file(), p->line1(), fmt, args...);
}

inline void HPHP_PARSER_ERROR(const char* msg, Parser* p) {
  throw ParseTimeFatalException(p->file(), p->line1(), "%s", msg);
}

///////////////////////////////////////////////////////////////////////////////
}}

#endif
