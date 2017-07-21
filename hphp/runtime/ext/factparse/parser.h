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

/* A minimal parser for extracting "Facts" data from PHP files. In general this
 * is more forgiving than the full HHVM compile-parser so being parsable here
 * in no way means you have a valid program. However, if you do have a valid
 * program this should extract all the required data.
 */

#ifndef incl_HPHP_COMPILER_PARSER_H_
#define incl_HPHP_COMPILER_PARSER_H_

#include <set>

#include "hphp/parser/parser.h"

#include "hphp/parser/parse-time-fatal-exception.h"


#ifdef HPHP_PARSER_NS
#undef HPHP_PARSER_NS
#endif
#define HPHP_PARSER_NS Facts

#ifdef HPHP_PARSER_ERROR
#undef HPHP_PARSER_ERROR
#endif

#ifndef NAMESPACE_SEP
#define NAMESPACE_SEP '\\'
#endif

namespace HPHP {
namespace Facts {

#define FACT_KIND_GEN(x) \
  x(Class, class), \
  x(Interface, interface), \
  x(Enum, enum), \
  x(Trait, trait), \
  x(Unknown, unknown), \
  x(Mixed, mixed),

enum class FactKindOf : int {
  #define FACT_KIND_ENUM(x, y) x
  FACT_KIND_GEN(FACT_KIND_ENUM)
  #undef FACT_KIND_ENUM
};

enum class FactTypeFlags : uint64_t {
  Abstract = 1,
  Final = 2,
  // if this is set the flags are a union of features from all declarations
  MultipleDeclarations = 4,
};

struct ParseResult {
  bool error{false};

  // types = classes/interfaces/traits
  struct TypeDetails {
    std::set<std::string> baseTypes;
    FactKindOf kindOf;
    uint64_t flags{0};
    // Only valid if kindOf = FactKindOf::Trait | FactKindOf::Interface
    std::set<std::string> requireExtends;
    // Only valid if kindOf = FactKindOf::Trait
    std::set<std::string> requireImplements;
  };

  uint64_t md5sum[2];

  std::map<std::string, TypeDetails> typesMap;
  std::vector<std::string> functions;
  std::vector<std::string> constants;
  std::vector<std::string> typeAliases;
};

typedef void* TStatementPtr;

struct Token : ScannerToken {
  std::set<std::string> m_baseTypes;
  std::set<std::string> m_requireExtends;
  std::set<std::string> m_requireImplements;
  std::string m_constantName;
  int m_constArgs{0};

  Token &operator+(const char *str) {
    m_text += str;
    return *this;
  }

  Token &operator+(Token &token) {
    m_text += token.text();
    return *this;
  }

  Token *operator->() {
    return this;
  }

  void operator=(int num) {
    m_num = num;
  }

  // For stealing string text
  std::string& takeText() {
    return m_text;
  }

  // Destructively copy other token's accumulated data by stealing its contents
  void takeAccumulate(Token& other) {
    for (auto& baseType : other.m_baseTypes) {
      m_baseTypes.emplace(std::move(baseType));
    }
    for (auto& requireExtends : other.m_requireExtends) {
      m_requireExtends.emplace(std::move(requireExtends));
    }
    for (auto& requireImplements : other.m_requireImplements) {
      m_requireImplements.emplace(std::move(requireImplements));
    }
    m_constantName.swap(other.m_constantName);
    if (other.m_constArgs == -1) {
      m_constArgs = -1;
    } else {
      m_constArgs += other.m_constArgs;
    }
  }

  Token& operator=(const Token& other) {
    ScannerToken::operator=(other);
    m_baseTypes = other.m_baseTypes;
    m_requireExtends = other.m_requireExtends;
    m_requireImplements = other.m_requireImplements;
    m_constantName = other.m_constantName;
    m_constArgs = other.m_constArgs;
    return *this;
  }

  void reset() {
    ScannerToken::reset();
    m_baseTypes.clear();
    m_requireExtends.clear();
    m_requireImplements.clear();
    m_constantName.clear();
    m_constArgs = 0;
  }
};

struct Parser : ParserBase {
  Parser(
    Scanner& scanner,
    const char* file_name,
    ParseResult& result
  ) :
    ParserBase(scanner, file_name),
    m_result(result)
  { }

  // implementing ParserBase
  virtual bool parseImpl();
  virtual bool parseImpl5();
  virtual bool parseImpl7();
  void parse();
  virtual void error(ATTRIBUTE_PRINTF_STRING const char* fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
  IMPLEMENT_XHP_ATTRIBUTES;

  virtual void fatal(const Location* loc, const char* msg);
  virtual void parseFatal(const Location* loc, const char* msg);
  std::string errString();

  // parser handlers
  void initParseTree();
  void finiParseTree();
  void onHaltCompiler();
  void onName(Token &out, Token &name, NameKind kind);
  void onVariable(Token &out, Token *exprs, Token &var, Token *value,
                  bool constant = false,
                  const std::string &docComment = "");
  void onStaticVariable(Token &out, Token *exprs, Token &var, Token *value);
  void onClassVariableModifer(Token& /*mod*/) {}
  void onClassVariableStart(Token &out, Token *modifiers, Token &decl,
                            Token *type, bool abstract = false,
                            bool typeconst = false);
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
  void onDict(Token &out, Token &pairs);
  void onVec(Token& out, Token& exprs);
  void onKeyset(Token& out, Token& exprs);
  void onVArray(Token& out, Token& exprs);
  void onDArray(Token& out, Token& exprs);
  void onEmptyCollection(Token &out);
  void onUserAttribute(Token &out, Token *attrList, Token &name, Token &value);
  void onClassConst(Token &out, Token &cls, Token &name, bool text);
  void onClassClass(Token &out, Token &cls, Token &name, bool text);
  void fixStaticVars();
  void onFunctionStart(Token &name);
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
  void onClassStatement(Token &out, Token &stmts, Token &new_stmt);
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
  void setTypeVars(Token &out, Token &name);
  void onTypedef(Token& out, Token& name, const Token& type,
                 const Token* attr = nullptr);

  void onTypeAnnotation(Token& out, Token& name, const Token& typeArgs);
  void onTypeList(Token& type1, const Token& type2);
  void onTypeSpecialization(Token& type, char specialization);
  void onShapeFieldSpecialization(Token& shapeField, char specialization);
  void onClsCnsShapeField(Token& out, const Token& cls, const Token& cns,
    const Token& value);
  void onShape(
    Token& out, const Token& shapeMemberList, bool terminatedWithEllipsis);

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

  void onUse(const Token &tok, UseDeclarationConsumer f);
  void onGroupUse(const std::string &prefix, const Token &tok,
                  UseDeclarationConsumer f);

  void useClassAndNamespace(const std::string &fn, const std::string &as);
  void useClass(const std::string &fn, const std::string &as);
  void useNamespace(const std::string &fn, const std::string &as);
  void useFunction(const std::string &fn, const std::string &as);
  void useConst(const std::string &cnst, const std::string &as);

  void onDeclare(Token& out, Token& block);
  void onDeclareList(Token& out, Token& ident, Token& value);

  void onNewLabelScope(bool fresh);
  void onScopeLabel(const Token& stmt, const Token& label);
  void onCompleteLabelScope(bool fresh);

  virtual void invalidateGoto(TStatementPtr stmt, GotoError error);
  virtual void invalidateLabel(TStatementPtr stmt);

  virtual TStatementPtr extractStatement(ScannerToken *stmt);

 private:
  // If we are inside a class then anything declared will need to go via a some
  // kind of reference to the class. Hence whatever symbols appear inside a
  // class are not needed for autoloading. This may need to change one day if
  // we support inner-classes so we can build a full type hierarchy.
  bool inline isOutsideOfClass() {
    return !m_classScope;
  }

  ParseResult& m_result;
  int m_classScope{0};
  int m_functionScope{0};
  std::string m_namespace;
  std::vector<uint32_t> m_nsStack;
};

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
