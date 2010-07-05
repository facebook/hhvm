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

#ifndef __EVAL_PARSER_H__
#define __EVAL_PARSER_H__

#include <stack>

#include <runtime/eval/parser/parser_defines.h>
#include <util/ylmm/basic_parser.hh>
#include <util/ylmm/basic_location.hh>
#include <runtime/eval/parser/scanner.h>
#include <runtime/eval/base/eval_base.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Location);
DECLARE_AST_PTR(Statement);
DECLARE_AST_PTR(ClassStatement);
DECLARE_AST_PTR(FunctionStatement);
DECLARE_AST_PTR(StaticStatement);

class Parser : public ylmm::basic_parser<Token> {
public:
  static StatementPtr
  parseString(const char *input,
              std::vector<StaticStatementPtr> &statics);
  static StatementPtr
  parseFile(const char *input,
            std::vector<StaticStatementPtr> &statics);
public:
  enum NameKind {
    StringName,
    ExprName,
    StaticClassExprName,
    StaticName
  };
  static Mutex s_lock;

  Parser(Scanner &s, const char *fileName,
         std::vector<StaticStatementPtr> &statics);
  // Gets
  StatementPtr getTree() const;
  std::string getMessage();
  void getLocation(Location &location);
  const char *file();
  int line0();
  int char0();
  int line1();
  int char1();

  // implementing basic_parser
  int scan(void *arg = NULL);

  // parser handlers
  void saveParseTree(Token &tree);
  void onName(Token &out, Token &name, NameKind kind);
  void onStaticVariable(Token &out, Token *exprs, Token &var, Token *value);
  void onSimpleVariable(Token &out, Token &var);
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
  void onConstant(Token &out, Token &constant);
  void onScalar(Token &out, int type, Token &scalar);
  void onExprListElem(Token &out, Token *exprs, Token &expr);

  void pushObject(Token &base);
  void popObject(Token &out);
  void appendMethodParams(Token &params);
  void appendProperty(Token &prop);
  void appendRefDim(Token &offset);

  void onListAssignment(Token &out, Token &vars, Token &expr);
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
  void onClassConst(Token &out, Token &className, Token &name, bool text);
  void onFunctionStart(Token &name);
  void onFunction(Token &out, Token &ref, Token &name, Token &params,
                  Token &stmt);
  void onParam(Token &out, Token *params, Token &type, Token &var,
               bool ref, Token *defValue);
  void onClassStart(int type, Token &name, Token *parent);
  void onClass(Token &out, Token &bases);
  void onInterface(Token &out, Token &bases);
  void onInterfaceName(Token &out, Token *names, Token &name);
  void onClassVariableStart(Token &mods);
  void onClassVariable(Token &name, Token *val);
  void onClassConstant(Token &name, Token &val);
  void onMethodStart(Token &name, Token &mods);
  void onMethod(Token &modifiers, Token &ref, Token &name, Token &params,
                Token &stmt);
  void onMemberModifier(Token &out, Token *modifiers, Token &modifier);
  void onStatementListStart(Token &out);
  void addStatement(Token &out, Token &stmts, Token &new_stmt);
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

  //void addHphpNote(ConstructPtr c, const std::string &note);
  void onHphpNoteExpr(Token &out, Token &note, Token &expr);
  void onHphpNoteStatement(Token &out, Token &note, Token &stmt);

  void addHphpDeclare(Token &declare);
  void addHphpSuppressError(Token &error);

  ClassStatementPtr currentClass() const;

  bool haveClass() const;
  ClassStatementPtr peekClass() const;
  bool haveFunc() const;
  FunctionStatementPtr peekFunc() const;
private:
  ylmm::basic_location m_location;
  std::ostringstream m_err;
  std::ostringstream m_msg;
  ylmm::basic_messenger<ylmm::basic_lock> m_messenger;

  Scanner &m_scanner;
  const char *m_fileName;
  std::vector<ExpressionPtr> m_objects; // for parsing object property/method calls
  std::stack<ClassStatementPtr> m_classes;
  std::stack<FunctionStatementPtr> m_funcs;
  std::vector<StaticStatementPtr> &m_staticStatements;
  int m_classVarMods;

  // parser output
  StatementPtr m_tree;

  void pushClass(ClassStatementPtr cl);
  void popClass();

  void pushFunc(FunctionStatementPtr fs);
  void popFunc();
  bool m_hasCallToGetArgs;

  ExpressionPtr getDynamicVariable(ExpressionPtr exp, bool encap);
  ExpressionPtr createDynamicVariable(ExpressionPtr exp);
  NamePtr procStaticClassName(Token &className, bool text);
};

///////////////////////////////////////////////////////////////////////////////
}
}


#endif // __HPHP_PARSER_H__
