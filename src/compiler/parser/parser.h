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

#ifndef __HPHP_PARSER_H__
#define __HPHP_PARSER_H__

#include <util/parser/parser.h>
#include <compiler/construct.h>

#ifdef HPHP_PARSER_NS
#undef HPHP_PARSER_NS
#endif
#define HPHP_PARSER_NS Compiler

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(Location);
DECLARE_BOOST_TYPES(AnalysisResult);

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////
// scanner

class Token : public ScannerToken {
public:
  ExpressionPtr exp;
  StatementPtr stmt;

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
  static StatementListPtr ParseString(const char *input, AnalysisResultPtr ar,
                                      const char *fileName = NULL);

public:
  Parser(Scanner &scanner, const char *fileName,
         AnalysisResultPtr ar, int fileSize = 0);

  // implementing ParserBase
  virtual bool parse();

  // result
  StatementListPtr getTree() const { return m_tree;}

  // parser handlers
  void saveParseTree(Token &tree);
  void onName(Token &out, Token &name, NameKind kind);
  void onVariable(Token &out, Token *exprs, Token &var, Token *value,
                  bool constant = false);
  void onStaticVariable(Token &out, Token *exprs, Token &var, Token *value);
  void onClassVariableModifer(Token &mod) {}
  void onClassVariableStart(Token &out, Token *modifiers, Token &decl);
  void onClassVariable(Token &out, Token *exprs, Token &var, Token *value);
  void onClassConstant(Token &out, Token *exprs, Token &var, Token &value);
  void onSimpleVariable(Token &out, Token &var);
  void onDynamicVariable(Token &out, Token &expr, bool encap);
  void onIndirectRef(Token &out, Token &refCount, Token &var);
  void onStaticMember(Token &out, Token &cls, Token &name);
  void onRefDim(Token &out, Token &var, Token &offset);
  void onCallParam(Token &out, Token *params, Token &expr, bool ref);
  void onCall(Token &out, bool dynamic, Token &name, Token &params,
              Token *cls);
  void onEncapsList(Token &out, int type, Token &list);
  void addEncap(Token &out, Token &list, Token &expr, int type);
  void encapRefDim(Token &out, Token &var, Token &offset);
  void encapObjProp(Token &out, Token &var, Token &name);
  void encapArray(Token &out, Token &var, Token &expr);
  void onConstant(Token &out, Token &constant);
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
  void onQOp(Token &out, Token &exprCond, Token &expYes, Token &expNo);
  void onArray(Token &out, Token &pairs);
  void onArrayPair(Token &out, Token *pairs, Token *name, Token &value,
                   bool ref);
  void onClassConst(Token &out, Token &cls, Token &name, bool text);
  void onFunctionStart(Token &name);
  void onFunction(Token &out, Token &ref, Token &name, Token &params,
                  Token &stmt);
  void onParam(Token &out, Token *params, Token &type, Token &var,
               bool ref, Token *defValue);
  void onClassStart(int type, Token &name, Token *parent);
  void onClass(Token &out, Token &type, Token &name, Token &base,
               Token &baseInterface, Token &stmt);
  void onInterface(Token &out, Token &name, Token &base, Token &stmt);
  void onInterfaceName(Token &out, Token *names, Token &name);
  void onMethodStart(Token &name, Token &mods);
  void onMethod(Token &out, Token &modifiers, Token &ref, Token &name,
                Token &params, Token &stmt);
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
  void onReturn(Token &out, Token *expr);
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

  void addHphpNote(ConstructPtr c, const std::string &note);
  void onHphpNoteExpr(Token &out, Token &note, Token &expr);
  void onHphpNoteStatement(Token &out, Token &note, Token &stmt);

  void addHphpDeclare(Token &declare);
  void addHphpSuppressError(Token &error);

private:
  AnalysisResultPtr m_ar;
  ExpressionPtrVec m_objects; // for parsing object property/method calls
  std::vector<std::string> m_comments; // for docComment stack
  // parser output
  StatementListPtr m_tree;

  void pushComment();
  std::string popComment();

  ExpressionPtr getDynamicVariable(ExpressionPtr exp, bool encap);
  ExpressionPtr createDynamicVariable(ExpressionPtr exp);
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_PARSER_H__
