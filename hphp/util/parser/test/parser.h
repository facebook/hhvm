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
#ifndef incl_HPHP_UTIL_PARSER_TEST_H_
#define incl_HPHP_UTIL_PARSER_TEST_H_

#include <stdexcept>
#include <iostream>
#include <cstdarg>
#include "folly/Format.h"
#include "folly/String.h"
#include "util/parser/parser.h"

#define HPHP_PARSER_NS Test

#define HPHP_PARSER_ERROR(fmt, p, args...)      \
  throw std::runtime_error(folly::format(       \
    "{}:{}:{}", (p)->file(), (p)->line1(),      \
      folly::stringPrintf(fmt, ##args)).str())

namespace HPHP { namespace Test {

extern bool g_verifyMode;

//////////////////////////////////////////////////////////////////////

struct Token : ScannerToken {
  Token& operator=(int num) {
    ScannerToken::m_num = num;
    return *this;
  }
  Token& operator=(Token& other) {
    ScannerToken::operator=(other);
    return *this;
  }
  Token* operator->() {
    return this;
  }
  Token& operator+(const char* str) {
    ScannerToken::m_text += str;
    return *this;
  }
  Token& operator+(const Token& token) {
    ScannerToken::m_num += token.m_num;
    ScannerToken::m_text += token.m_text;
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& out, Token& t) {
    return out << t.m_text << ":" << t.m_num;
  }
};

/*
 * Parser for testing that just dumps every callback from the grammar.
 */
struct Parser : ParserBase {
  explicit Parser(Scanner& scanner,
                  const char* filename)
    : ParserBase(scanner, filename)
  {}

  void traceOne(Token& t) {
    std::cout << "tok(" << t << ") ";
  }

  void traceOne(Token* t) {
    std::cout << "ptok(";
    if (t) {
      std::cout << *t;
    } else {
      std::cout << "nul";
    }
    std::cout << ") ";
  }

  template<class T>
  void traceOne(T t) {
    std::cout << t << ' ';
  }

  void traceArgs() {
    std::cout << '\n';
  }

  template<class Head, class... Tail>
  void traceArgs(Head head, Tail... tail) {
    traceOne(head);
    traceArgs(tail...);
  }

  template<class... Args>
  void traceCb(const char* cbName, Args... args) {
    if (g_verifyMode) return;
    std::cout << cbName << ": ";
    traceArgs(args...);
  }

#define X(...) traceCb(__FUNCTION__,## __VA_ARGS__)

  void fatal(Location* loc, const char* msg) {
    throw std::runtime_error(folly::format(
      "{}:{}: {}", m_fileName, m_loc.line0, msg).str());
  }

  void parse() {
    if (!parseImpl()) {
      error("parse failure: %s\n", getMessage().c_str());
    }
  }

  bool parseImpl();

  void error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::string msg;
    Util::string_vsnprintf(msg, fmt, ap);
    va_end(ap);

    throw std::runtime_error(folly::format(
      "{}:{}: {}", m_fileName, m_loc.line0, msg).str());
  }

  void invalidateGoto(TStatementPtr, ParserBase::GotoError) { X(); }
  void invalidateLabel(TStatementPtr) { X(); }
  void* extractStatement(ScannerToken*) { X(); return nullptr; }

  bool enableFinallyStatement() { return true; }
  bool enableHipHopSyntax() { return true; }
  bool enableXHP() { return true; }

  IMPLEMENT_XHP_ATTRIBUTES;

  void saveParseTree(Token& tree) { X(); }

  void onName(Token& out, Token& name, NameKind kind) {
    X(name, kind);
  }

  void onVariable(Token& out, Token* exprs, Token& var, Token* value,
                  bool constant = false,
                  const std::string& docComment = "") {
    X(exprs, var, value, constant, docComment);
  }

  void onStaticVariable(Token &out, Token *exprs, Token &var, Token *value) {
    X(exprs, var, value);
  }

  void onClassVariableModifer(Token &mod) {
    X(mod);
  }

  void onClassVariableStart(Token &out, Token *modifiers, Token &decl,
                            Token *type) {
    X(modifiers, decl, type);
  }

  void onClassVariable(Token &out, Token *exprs, Token &var, Token *value) {
    X(exprs, var, value);
  }

  void onClassConstant(Token &out, Token *exprs, Token &var, Token &value) {
    X(exprs, var, value);
  }

  void onSimpleVariable(Token& out, Token& var) { X(var); }
  void onSynthesizedVariable(Token& out, Token& var) { X(var); }

  void onDynamicVariable(Token &out, Token &expr, bool encap) {
    X(expr, encap);
  }

  void onIndirectRef(Token &out, Token &refCount, Token &var) {
    X(refCount, var);
  }

  void onStaticMember(Token &out, Token &cls, Token &name) {
    X(cls, name);
  }

  void onRefDim(Token &out, Token &var, Token &offset) {
    X(var, offset);
  }

  void onCallParam(Token &out, Token *params, Token &expr, bool ref) {
    X(params, expr, ref);
  }

  void onCall(Token &out, bool dynamic, Token &name, Token &params,
              Token *cls, bool fromCompiler = false) {
    X(dynamic, name, params, cls, fromCompiler);
  }

  void onEncapsList(Token &out, int type, Token &list) {
    X(type, list);
  }

  void addEncap(Token &out, Token *list, Token &expr, int type) {
    X(list, expr, type);
  }

  void encapRefDim(Token &out, Token &var, Token &offset) {
    X(var, offset);
  }

  void encapObjProp(Token &out, Token &var, Token &name) {
    X(var, name);
  }

  void encapArray(Token &out, Token &var, Token &expr) {
    X(var, expr);
  }

  void onConstantValue(Token &out, Token &constant) {
    X(constant);
  }

  void onScalar(Token &out, int type, Token &scalar) {
    X(type, scalar);
  }

  void onExprListElem(Token &out, Token *exprs, Token &expr) {
    X(exprs, expr);
  }

  void onObjectProperty(Token &out, Token &base, Token &prop) {
    X(base, prop);
  }

  void onObjectMethodCall(Token &out, Token &base, Token &prop,
                          Token &params) {
    X(base, prop, params);
  }

  void onListAssignment(Token &out, Token &vars, Token *expr) {
    X(vars, expr);
  }

  void onAListVar(Token &out, Token *list, Token *var) {
    X(list, var);
  }

  void onAListSub(Token &out, Token *list, Token &sublist) {
    X(list, sublist);
  }

  void onAssign(Token& out, Token& var, Token& expr, bool ref) {
    X(var, expr, ref);
  }

  void onAssignNew(Token &out, Token &var, Token &name, Token &args) {
    X(var, name, args);
  }

  void onNewObject(Token &out, Token &name, Token &args) {
    X(name, args);
  }

  void onUnaryOpExp(Token &out, Token &operand, int op, bool front) {
    X(operand, op, front);
  }

  void onBinaryOpExp(Token &out, Token &operand1, Token &operand2, int op) {
    X(operand1, operand2, op);
  }

  void onQOp(Token &out, Token &exprCond, Token *expYes, Token &expNo) {
    X(exprCond, expYes, expNo);
  }

  void onArray(Token &out, Token &pairs, int op = T_ARRAY) {
    X(pairs, op);
  }

  void onArrayPair(Token &out, Token *pairs, Token *name, Token &value,
                   bool ref) {
    X(pairs, name, value, ref);
  }

  void onEmptyCollection(Token &out) { X(); }

  void onCollectionPair(Token &out, Token *pairs, Token *name, Token &value) {
    X(pairs, name, value);
  }

  void onUserAttribute(Token &out, Token *attrList, Token &name,
                       Token &value) {
    X(attrList, name, value);
  }

  void onClassConst(Token &out, Token &cls, Token &name, bool text) {
    X(cls, name, text);
  }

  void fixStaticVars() { X(); }

  void onFunctionStart(Token& name, bool doPushComment = true) {
    X(name, doPushComment);
  }

  void onClosureStart(Token& name) {
    X(name);
  }

  void onFunction(Token& out, Token& ret, Token& ref, Token& name,
                  Token& params, Token& stmt, Token* attr) {
    X(ret, ref, name, params, stmt, attr);
  }

  void onParam(Token &out, Token *params, Token &type, Token &var,
               bool ref, Token *defValue, Token *attr) {
    X(params, type, var, ref, defValue, attr);
  }

  void onClassStart(int type, Token &name) { X(type, name); }
  void onClass(Token &out, int type, Token &name, Token &base,
               Token &baseInterface, Token &stmt, Token *attr) {
    X(type, name, base, baseInterface, stmt, attr);
  }

  void onInterface(Token &out, Token &name, Token &base, Token &stmt,
                   Token *attr) {
    X(name, base, stmt, attr);
  }

  void onInterfaceName(Token &out, Token *names, Token &name) {
    X(name, name);
  }

  void onTraitUse(Token &out, Token &traits, Token &rules) {
    X(traits, rules);
  }

  void onTraitName(Token &out, Token *names, Token &name) {
    X(names, name);
  }

  void onTraitRule(Token &out, Token &stmtList, Token &newStmt) {
    X(stmtList, newStmt);
  }

  void onTraitPrecRule(Token &out, Token &className, Token &methodName,
                       Token &otherClasses) {
    X(className, methodName, otherClasses);
  }

  void onTraitAliasRuleStart(Token &out, Token &className, Token &methodName) {
    X(className, methodName);
  }

  void onTraitAliasRuleModify(Token &out, Token &rule, Token &accessModifiers,
                              Token &newMethodName) {
    X(rule, accessModifiers, newMethodName);
  }

  void onMethodStart(Token &name, Token &mods, bool doPushComment = true) {
    X(name, mods, doPushComment);
  }

  void onMethod(Token &out, Token &modifiers, Token &ret, Token &ref,
                Token &name, Token &params, Token &stmt, Token *attr,
                bool reloc = true) {
    X(modifiers, ret, ref, name, params, stmt, attr, reloc);
  }

  void onMemberModifier(Token &out, Token *modifiers, Token &modifier) {
    X(modifiers, modifier);
  }

  void onStatementListStart(Token &out) { X(); }

  void addStatement(Token& out, Token& stmts, Token& new_stmt) {
    X(stmts, new_stmt);
  }

  void onClassStatement(Token &out, Token &stmts, Token &new_stmt) {
    X(stmts, new_stmt);
  }

  void finishStatement(Token& out, Token& stmts) {
    X(stmts);
  }

  void onBlock(Token& out, Token& stmts) { X(stmts); }

  void onIf(Token &out, Token &cond, Token &stmt, Token &elseifs,
            Token &elseStmt) {
    X(cond, stmt, elseifs, elseStmt);
  }

  void onElseIf(Token& out, Token& elseifs, Token& cond, Token& stmt) {
    X(elseifs, cond, stmt);
  }

  void onWhile(Token &out, Token &cond, Token &stmt) {
    X(cond, stmt);
  }

  void onDo(Token &out, Token &stmt, Token &cond) {
    X(stmt, cond);
  }

  void onFor(Token &out, Token &expr1, Token &expr2, Token &expr3,
             Token &stmt) {
    X(expr1, expr2, expr3, stmt);
  }

  void onSwitch(Token &out, Token &expr, Token &cases) {
    X(expr, cases);
  }

  void onCase(Token &out, Token &cases, Token *cond, Token &stmt) {
    X(cases, cond, stmt);
  }

  void onBreak(Token &out, Token *expr) {
    X(expr);
  }

  void onContinue(Token &out, Token *expr) {
    X(expr);
  }

  void onReturn(Token &out, Token *expr, bool checkYield = true) {
    X(expr, checkYield);
  }

  void onYield(Token &out, Token *expr, bool assign) {
    X(expr, assign);
  }

  void onYieldBreak(Token &out) { X(); }

  void onGlobal(Token &out, Token &expr) { X(expr); }
  void onGlobalVar(Token &out, Token *exprs, Token &expr) {
    X(exprs, expr);
  }

  void onStatic(Token &out, Token &expr) { X(expr); }
  void onEcho(Token &out, Token &expr, bool html) { X(expr, html); }
  void onUnset(Token &out, Token &expr) { X(expr); }

  void onExpStatement(Token& out, Token& expr) { X(expr); }

  void onForEachStart() { X(); }
  void onForEach(Token &out, Token &arr, Token &name, Token &value,
                 Token &stmt) {
    X(arr, name, value, stmt);
  }

  void onTry(Token &out, Token &tryStmt, Token &className, Token &var,
             Token &catchStmt, Token &catches, Token &finallyStmt) {
    X(tryStmt, className, var, catchStmt, catches, finallyStmt);
  }

  void onTry(Token &out, Token &tryStmt, Token &finallyStmt) {
    X(tryStmt, finallyStmt);
  }

  void onCatch(Token &out, Token &catches, Token &className, Token &var,
               Token &stmt) {
    X(catches, className, var, stmt);
  }

  void onFinally(Token &out, Token &stmt) {
    X(stmt);
  }

  void onThrow(Token &out, Token &expr) { X(expr); }

  void onClosure(Token &out, Token &ret, Token &ref, Token &params,
                 Token &cparams, Token &stmts) {
    X(ret, ref, params, cparams, stmts);
  }

  void onClosureParam(Token &out, Token *params, Token &param, bool ref) {
    X(params, param, ref);
  }

  void onLabel(Token &out, Token &label) { X(label); }
  void onGoto(Token &out, Token &label, bool limited) { X(label, limited); }
};

//////////////////////////////////////////////////////////////////////

}}

#endif
