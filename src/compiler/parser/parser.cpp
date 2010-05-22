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

#include <compiler/parser/parser.h>
#include <compiler/parser/hphp.tab.hpp>
#include <compiler/analysis/file_scope.h>

#include <compiler/expression/expression_list.h>
#include <compiler/expression/assignment_expression.h>
#include <compiler/expression/simple_variable.h>
#include <compiler/expression/dynamic_variable.h>
#include <compiler/expression/static_member_expression.h>
#include <compiler/expression/array_element_expression.h>
#include <compiler/expression/dynamic_function_call.h>
#include <compiler/expression/simple_function_call.h>
#include <compiler/expression/scalar_expression.h>
#include <compiler/expression/object_property_expression.h>
#include <compiler/expression/object_method_expression.h>
#include <compiler/expression/list_assignment.h>
#include <compiler/expression/new_object_expression.h>
#include <compiler/expression/include_expression.h>
#include <compiler/expression/unary_op_expression.h>
#include <compiler/expression/binary_op_expression.h>
#include <compiler/expression/qop_expression.h>
#include <compiler/expression/array_pair_expression.h>
#include <compiler/expression/class_constant_expression.h>
#include <compiler/expression/parameter_expression.h>
#include <compiler/expression/modifier_expression.h>
#include <compiler/expression/constant_expression.h>
#include <compiler/expression/encaps_list_expression.h>

#include <compiler/statement/function_statement.h>
#include <compiler/statement/class_statement.h>
#include <compiler/statement/interface_statement.h>
#include <compiler/statement/class_variable.h>
#include <compiler/statement/class_constant.h>
#include <compiler/statement/method_statement.h>
#include <compiler/statement/statement_list.h>
#include <compiler/statement/block_statement.h>
#include <compiler/statement/if_branch_statement.h>
#include <compiler/statement/if_statement.h>
#include <compiler/statement/while_statement.h>
#include <compiler/statement/do_statement.h>
#include <compiler/statement/for_statement.h>
#include <compiler/statement/switch_statement.h>
#include <compiler/statement/case_statement.h>
#include <compiler/statement/break_statement.h>
#include <compiler/statement/continue_statement.h>
#include <compiler/statement/return_statement.h>
#include <compiler/statement/global_statement.h>
#include <compiler/statement/static_statement.h>
#include <compiler/statement/echo_statement.h>
#include <compiler/statement/unset_statement.h>
#include <compiler/statement/exp_statement.h>
#include <compiler/statement/foreach_statement.h>
#include <compiler/statement/catch_statement.h>
#include <compiler/statement/try_statement.h>
#include <compiler/statement/throw_statement.h>

#include <compiler/analysis/code_error.h>
#include <compiler/analysis/analysis_result.h>

#include <util/preprocess.h>

using namespace HPHP;
using namespace std;
using namespace boost;

#define NEW_EXP0(cls)                                           \
  cls##Ptr(new cls(getLocation(), Expression::KindOf##cls))
#define NEW_EXP(cls, e...)                                      \
  cls##Ptr(new cls(getLocation(), Expression::KindOf##cls, ##e))
#define NEW_STMT0(cls)                                          \
  cls##Ptr(new cls(getLocation(), Statement::KindOf##cls))
#define NEW_STMT(cls, e...)                                     \
  cls##Ptr(new cls(getLocation(), Statement::KindOf##cls, ##e))
#define NEW_STMT_POP_LOC(cls, e...)                             \
  cls##Ptr(new cls(popLocation(), Statement::KindOf##cls, ##e))

///////////////////////////////////////////////////////////////////////////////
// statics

StatementListPtr Parser::ParseString(const char *input, AnalysisResultPtr ar,
                                     const char *fileName /* = NULL */) {
  ASSERT(input);
  istringstream iss(input);
  stringstream ss;
  istream *is = Option::EnableXHP ? preprocessXHP(iss, ss, fileName) : &iss;

  Scanner scanner(new ylmm::basic_buffer(*is, false, true), true, false);
  if (!fileName || !*fileName) fileName = "string";
  ParserPtr parser(new Parser(scanner, fileName, strlen(input), ar));
  if (parser->parse()) {
    printf("Error parsing %s: %s\n%s\n", fileName,
           parser->getMessage().c_str(), input);
    return StatementListPtr();
  }
  return parser->getTree();
}

///////////////////////////////////////////////////////////////////////////////

Parser::Parser(Scanner &s, const char *fileName, int fileSize,
               AnalysisResultPtr ar)
  : m_scanner(s), m_ar(ar) {
  _location = &m_location;
  m_messenger.error_stream(m_err);
  m_messenger.message_stream(m_msg);
  messenger(m_messenger);
  m_fileName = fileName ? fileName : "";

  FileScopePtr fileScope(new FileScope(m_fileName, fileSize));
  m_ar->setFileScope(fileScope);
}

std::string Parser::getMessage() {
  string ret;
  ret += m_scanner.getError();
  int line = m_scanner.getLine();
  int column = m_scanner.getColumn();

  ret += " (";
  ret += string("Line: ") + lexical_cast<string>(line);
  ret += ", Char: " + lexical_cast<string>(column) + "): ";
  ret += m_err.str() + "\n";
  return ret;
}

LocationPtr Parser::getLocation() {
  LocationPtr location(new Location());
  location->file = file();
  location->line0 = line0();
  location->char0 = char0();
  location->line1 = line1();
  location->char1 = char1();
  return location;
}

void Parser::pushComment() {
  m_comments.push_back(m_scanner.getDocComment());
}

std::string Parser::popComment() {
  std::string ret = m_comments.back();
  m_comments.pop_back();
  return ret;
}

void Parser::pushLocation() {
  m_locs.push_back(getLocation());
}

LocationPtr Parser::popLocation() {
  LocationPtr ret = m_locs.back();
  m_locs.pop_back();
  return ret;
}

const char *Parser::file() {
  return m_fileName;
}

int Parser::line0() {
  return m_location.first_line();
}

int Parser::char0() {
  return m_location.first_column();
}

int Parser::line1() {
  return m_location.last_line();
}

int Parser::char1() {
  return m_location.last_column();
}

int Parser::scan(void *arg /* = NULL */) {
  return m_scanner.getNextToken(token(), where());
}

///////////////////////////////////////////////////////////////////////////////
// variables

void Parser::onVariable(Token *out, Token *exprs, Token *var, Token *value,
                        bool constant /* = false */) {
  ExpressionPtr expList;
  if (exprs) {
    expList = exprs->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  ExpressionPtr exp;
  if (constant) {
    exp = NEW_EXP(ConstantExpression, var->text());
  } else {
    exp = NEW_EXP(SimpleVariable, var->text());
  }
  if (value) {
    exp = NEW_EXP(AssignmentExpression, exp, value->exp, false);
  }
  else {
    // implicit null, #147156
    static const std::string s_null ("null");
    ExpressionPtr null_exp (NEW_EXP(ConstantExpression, s_null));
    exp = NEW_EXP(AssignmentExpression, exp, null_exp, false);
  }
  expList->addElement(exp);
  out->exp = expList;
}

void Parser::onSimpleVariable(Token *out, Token *var) {
  out->exp = NEW_EXP(SimpleVariable, var->text());
}

void Parser::onDynamicVariable(Token *out, Token *expr, bool encap) {
  out->exp = getDynamicVariable(expr->exp, encap);
}

void Parser::onIndirectRef(Token *out, Token *refCount, Token *var) {
  out->exp = var->exp;
  for (int i = 0; i < refCount->num; i++) {
    out->exp = createDynamicVariable(out->exp);
  }
}

void Parser::onStaticMember(Token *out, Token *cls, Token *name) {
  if (name->exp->is(Expression::KindOfArrayElementExpression) &&
      dynamic_pointer_cast<ArrayElementExpression>(name->exp)->
      appendClass(cls->exp)) {
    out->exp = name->exp;
  } else {
    out->exp = NEW_EXP(StaticMemberExpression, cls->exp, name->exp);
  }
}

void Parser::onRefDim(Token *out, Token *var, Token *offset) {
  if (!var->exp) {
    var->exp = NEW_EXP(ConstantExpression, var->text());
  }
  out->exp = NEW_EXP(ArrayElementExpression, var->exp, offset->exp);
}

ExpressionPtr Parser::getDynamicVariable(ExpressionPtr exp, bool encap) {
  if (encap) {
    ConstantExpressionPtr var = dynamic_pointer_cast<ConstantExpression>(exp);
    if (var) {
      return NEW_EXP(SimpleVariable, var->getName());
    }
  } else {
    ScalarExpressionPtr var = dynamic_pointer_cast<ScalarExpression>(exp);
    if (var) {
      return NEW_EXP(SimpleVariable, var->getString());
    }
  }
  return createDynamicVariable(exp);
}

ExpressionPtr Parser::createDynamicVariable(ExpressionPtr exp) {
  m_ar->getFileScope()->setAttribute(FileScope::ContainsDynamicVariable);
  return NEW_EXP(DynamicVariable, exp);
}

void Parser::onCallParam(Token *out, Token *params, Token *expr, bool ref) {
  if (!params) {
    out->exp = NEW_EXP0(ExpressionList);
  } else {
    out->exp = params->exp;
  }
  if (ref) {
    expr->exp->setContext(Expression::RefParameter);
    expr->exp->setContext(Expression::RefValue);

  }
  out->exp->addElement(expr->exp);
}

void Parser::onCall(Token *out, bool dynamic, Token *name, Token *params,
                    Token *cls) {
  ExpressionPtr clsExp;
  if (cls) {
    clsExp = cls->exp;
  }
  if (dynamic) {
    out->exp = NEW_EXP(DynamicFunctionCall, name->exp,
                       dynamic_pointer_cast<ExpressionList>(params->exp),
                       clsExp);
  } else {
    SimpleFunctionCallPtr call =
      NEW_EXP(SimpleFunctionCall, name->text(),
              dynamic_pointer_cast<ExpressionList>(params->exp), clsExp);
    out->exp = call;
    call->onParse(m_ar);
  }
}

///////////////////////////////////////////////////////////////////////////////
// object property and method calls

void Parser::pushObject(Token *base) {
  m_objects.push_back(base->exp);
}

void Parser::popObject(Token *out) {
  out->exp = m_objects.back();
  m_objects.pop_back();
}

void Parser::appendMethodParams(Token *params) {
  ExpressionListPtr paramsExp;
  if (params) {
    if (params->exp) {
      paramsExp = dynamic_pointer_cast<ExpressionList>(params->exp);
    } else if (params->num == 1) {
      paramsExp = NEW_EXP0(ExpressionList);
    }
  }
  if (paramsExp) {
    ObjectPropertyExpressionPtr prop =
      dynamic_pointer_cast<ObjectPropertyExpression>(m_objects.back());
    if (prop) {
      ObjectMethodExpressionPtr method =
        NEW_EXP(ObjectMethodExpression,
                prop->getObject(), prop->getProperty(), paramsExp);
      m_objects.back() = method;
    } else {
      m_objects.back() = NEW_EXP(DynamicFunctionCall, m_objects.back(),
                                 paramsExp, ExpressionPtr());
    }
  }
}

void Parser::appendProperty(Token *prop) {
  if (!prop->exp) {
    prop->exp = NEW_EXP(ScalarExpression, T_STRING, prop->text());
  }
  m_objects.back() = NEW_EXP(ObjectPropertyExpression, m_objects.back(),
                             prop->exp);
}

void Parser::appendRefDim(Token *offset) {
  m_objects.back() = NEW_EXP(ArrayElementExpression, m_objects.back(),
                             offset->exp);
}

///////////////////////////////////////////////////////////////////////////////
// encapsed expressions

void Parser::onEncapsList(Token *out, int type, Token *list) {
  out->exp = NEW_EXP(EncapsListExpression, type,
                     dynamic_pointer_cast<ExpressionList>(list->exp));
}

void Parser::addEncap(Token *out, Token *list, Token *expr, int type) {
  ExpressionListPtr expList;
  if (list->exp) {
    expList = dynamic_pointer_cast<ExpressionList>(list->exp);
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  ExpressionPtr exp;
  if (type == -1) {
    exp = expr->exp;
  } else {
    ScalarExpressionPtr scalar =
      NEW_EXP(ScalarExpression, T_ENCAPSED_AND_WHITESPACE, expr->text(), true);
    scalar->onParse(m_ar);
    exp = scalar;
  }
  expList->addElement(exp);
  out->exp = expList;
}

void Parser::encapRefDim(Token *out, Token *var, Token *offset) {
  ExpressionPtr dim;
  switch (offset->num) {
  case T_STRING:
    dim = NEW_EXP(ScalarExpression, T_STRING, offset->text(), true);
    break;
  case T_NUM_STRING:
    dim = NEW_EXP(ScalarExpression, T_NUM_STRING, offset->text());
    break;
  case T_VARIABLE:
    dim = NEW_EXP(SimpleVariable, offset->text());
    break;
  default:
    ASSERT(false);
  }

  ExpressionPtr arr = NEW_EXP(SimpleVariable, var->text());
  out->exp = NEW_EXP(ArrayElementExpression, arr, dim);
}

void Parser::encapObjProp(Token *out, Token *var, Token *name) {
  ExpressionPtr obj = NEW_EXP(SimpleVariable, var->text());

  ExpressionPtr prop = NEW_EXP(ScalarExpression, T_STRING, name->text());
  out->exp = NEW_EXP(ObjectPropertyExpression, obj, prop);
}

void Parser::encapArray(Token *out, Token *var, Token *expr) {
  ExpressionPtr arr = NEW_EXP(SimpleVariable, var->text());
  out->exp = NEW_EXP(ArrayElementExpression, arr, expr->exp);
}

///////////////////////////////////////////////////////////////////////////////
// expressions

void Parser::onConstant(Token *out, Token *constant) {
  out->exp = NEW_EXP(ConstantExpression, constant->text());
}

void Parser::onScalar(Token *out, int type, Token *scalar) {
  ScalarExpressionPtr exp;
  switch (type) {
  case T_STRING:
  case T_LNUMBER:
  case T_DNUMBER:
  case T_LINE:
  case T_FILE:
  case T_CLASS_C:
  case T_METHOD_C:
  case T_FUNC_C:
    exp = NEW_EXP(ScalarExpression, type, scalar->text());
    break;
  case T_CONSTANT_ENCAPSED_STRING:
    exp = NEW_EXP(ScalarExpression, type, scalar->text(), true);
    break;
  default:
    ASSERT(false);
  }
  exp->onParse(m_ar);
  out->exp = exp;
}

void Parser::onExprListElem(Token *out, Token *exprs, Token *expr) {
  ExpressionPtr expList;
  if (exprs && exprs->exp) {
    expList = exprs->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(expr->exp);
  out->exp = expList;
}

void Parser::onListAssignment(Token *out, Token *vars, Token *expr) {
  out->exp = NEW_EXP(ListAssignment,
                     dynamic_pointer_cast<ExpressionList>(vars->exp),
                     expr ? expr->exp : ExpressionPtr());
}

void Parser::onAssign(Token *out, Token *var, Token *expr, bool ref) {
  out->exp = NEW_EXP(AssignmentExpression, var->exp, expr->exp, ref);
}

void Parser::onAssignNew(Token *out, Token *var, Token *name, Token *args) {
  ExpressionPtr exp =
    NEW_EXP(NewObjectExpression, name->exp,
            dynamic_pointer_cast<ExpressionList>(args->exp));
  out->exp = NEW_EXP(AssignmentExpression, var->exp, exp, true);
}

void Parser::onNewObject(Token *out, Token *name, Token *args) {
  out->exp = NEW_EXP(NewObjectExpression, name->exp,
                     dynamic_pointer_cast<ExpressionList>(args->exp));
}

void Parser::onUnaryOpExp(Token *out, Token *operand, int op, bool front) {
  switch (op) {
  case T_INCLUDE:
  case T_INCLUDE_ONCE:
  case T_REQUIRE:
  case T_REQUIRE_ONCE:
    {
      IncludeExpressionPtr exp = NEW_EXP(IncludeExpression, operand->exp, op);
      out->exp = exp;
      exp->onParse(m_ar);
    }
    break;
  default:
    {
      UnaryOpExpressionPtr exp = NEW_EXP(UnaryOpExpression, operand->exp, op,
                                         front);
      out->exp = exp;
      exp->onParse(m_ar);
    }
    break;
  }
}

void Parser::onBinaryOpExp(Token *out, Token *operand1, Token *operand2,
                           int op) {
  out->exp = NEW_EXP(BinaryOpExpression, operand1->exp, operand2->exp, op);
}

void Parser::onQOp(Token *out, Token *exprCond, Token *expYes, Token *expNo) {
  out->exp = NEW_EXP(QOpExpression, exprCond->exp, expYes->exp, expNo->exp);
}

void Parser::onArrayPair(Token *out, Token *pairs, Token *name, Token *value,
                         bool ref) {
  ExpressionPtr expList;
  if (pairs) {
    expList = pairs->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  ExpressionPtr nameExp = name ? name->exp : ExpressionPtr();
  expList->addElement(NEW_EXP(ArrayPairExpression, nameExp, value->exp, ref));
  out->exp = expList;
}

void Parser::onClassConst(Token *out, Token *cls, Token *name) {
  if (!cls->exp) {
    cls->exp = NEW_EXP(ScalarExpression, T_STRING, cls->text());
  }
  out->exp = NEW_EXP(ClassConstantExpression, cls->exp, name->text());
}

///////////////////////////////////////////////////////////////////////////////
// function/method declaration

void Parser::onFunctionStart() {
  m_ar->getFileScope()->pushAttribute();
  pushLocation();
  pushComment();
}

void Parser::onFunction(Token *out, Token *ref, Token *name, Token *params,
                        Token *stmt) {
  if (!stmt->stmt) {
    stmt->stmt = NEW_STMT0(StatementList);
  }
  FunctionStatementPtr func = NEW_STMT_POP_LOC
    (FunctionStatement, ref->num, name->text(),
     dynamic_pointer_cast<ExpressionList>(params->exp),
     dynamic_pointer_cast<StatementList>(stmt->stmt),
     m_ar->getFileScope()->popAttribute(),
     popComment());
  out->stmt = func;
  func->onParse(m_ar);
}

void Parser::onParam(Token *out, Token *params, Token *type, Token *var,
                     bool ref, Token *defValue) {
  ExpressionPtr expList;
  if (params) {
    expList = params->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(NEW_EXP(ParameterExpression, type->text(), var->text(),
                              ref,
                              defValue ? defValue->exp : ExpressionPtr()));
  out->exp = expList;
}

void Parser::onClassStart() {
  pushLocation();
  pushComment();
}

void Parser::onClass(Token *out, Token *type, Token *name, Token *base,
                     Token *baseInterface, Token *stmt) {
  StatementListPtr stmtList;
  if (stmt->stmt) {
    stmtList = dynamic_pointer_cast<StatementList>(stmt->stmt);
  }

  ClassStatementPtr cls = NEW_STMT_POP_LOC
    (ClassStatement, type->num, name->text(), base->text(),
     dynamic_pointer_cast<ExpressionList>(baseInterface->exp),
     popComment(), stmtList);
  out->stmt = cls;
  cls->onParse(m_ar);
}

void Parser::onInterface(Token *out, Token *name, Token *base, Token *stmt) {
  StatementListPtr stmtList;
  if (stmt->stmt) {
    stmtList = dynamic_pointer_cast<StatementList>(stmt->stmt);
  }

  InterfaceStatementPtr intf = NEW_STMT_POP_LOC
    (InterfaceStatement, name->text(),
     dynamic_pointer_cast<ExpressionList>(base->exp), popComment(), stmtList);
  out->stmt = intf;
  intf->onParse(m_ar);
}

void Parser::onInterfaceName(Token *out, Token *names, Token *name) {
  ExpressionPtr expList;
  if (names) {
    expList = names->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(NEW_EXP(ScalarExpression, T_STRING, name->text()));
  out->exp = expList;
}

void Parser::onClassVariable(Token *out, Token *modifiers, Token *decl) {
  if (modifiers) {
    ModifierExpressionPtr exp = modifiers->exp ?
      dynamic_pointer_cast<ModifierExpression>(modifiers->exp)
      : NEW_EXP0(ModifierExpression);

    out->stmt = NEW_STMT
      (ClassVariable, exp, dynamic_pointer_cast<ExpressionList>(decl->exp));
  } else {
    out->stmt =
      NEW_STMT(ClassConstant, dynamic_pointer_cast<ExpressionList>(decl->exp));
  }
}

void Parser::onMethod(Token *out, Token *modifiers, Token *ref, Token *name,
                      Token *params, Token *stmt) {
  ModifierExpressionPtr exp = modifiers->exp ?
    dynamic_pointer_cast<ModifierExpression>(modifiers->exp)
    : NEW_EXP0(ModifierExpression);

  StatementListPtr stmts;
  if (!stmt->stmt && stmt->num == 1) {
    stmts = NEW_STMT0(StatementList);
  } else {
    stmts = dynamic_pointer_cast<StatementList>(stmt->stmt);
  }

  out->stmt = NEW_STMT_POP_LOC
    (MethodStatement, exp, ref->num, name->text(),
     dynamic_pointer_cast<ExpressionList>(params->exp), stmts,
     m_ar->getFileScope()->popAttribute(),
     popComment());
}

void Parser::onMemberModifier(Token *out, Token *modifiers, Token *modifier) {
  ModifierExpressionPtr expList;
  if (modifiers) {
    expList = dynamic_pointer_cast<ModifierExpression>(modifiers->exp);
  } else {
    expList = NEW_EXP0(ModifierExpression);
  }
  expList->add(modifier->num);
  out->exp = expList;
}

///////////////////////////////////////////////////////////////////////////////
// statements

void Parser::saveParseTree(Token *tree) {
  if (tree->stmt) {
    m_tree = dynamic_pointer_cast<StatementList>(tree->stmt);
  } else {
    m_tree = NEW_STMT0(StatementList);
  }
  FileScopePtr fileScope = m_ar->getFileScope();
  fileScope->setTree(m_tree);
  m_ar->pushScope(fileScope);
  m_tree->preOptimize(m_ar);
  m_ar->popScope();
}

void Parser::addStatement(Token *out, Token *stmts, Token *new_stmt) {
  if (!stmts || !stmts->stmt) {
    out->stmt = NEW_STMT0(StatementList);
  } else {
    out->stmt = stmts->stmt;
  }
  if (new_stmt->stmt) {
    out->stmt->addElement(new_stmt->stmt);
  }
}

void Parser::finishStatement(Token *out, Token *stmts) {
  if (!stmts->stmt) {
    out->stmt = NEW_STMT0(StatementList);
  } else {
    out->stmt = stmts->stmt;
  }
}

void Parser::onBlock(Token *out, Token *stmts) {
  if (!stmts->stmt) {
    stmts->stmt = NEW_STMT0(StatementList);
  } else if (!stmts->stmt->is(Statement::KindOfStatementList)) {
    out->stmt = NEW_STMT0(StatementList);
    out->stmt->addElement(stmts->stmt);
    stmts->stmt = out->stmt;
  }
  out->stmt = NEW_STMT(BlockStatement,
                       dynamic_pointer_cast<StatementList>(stmts->stmt));
}

void Parser::onIf(Token *out, Token *cond, Token *stmt, Token *elseifs,
                  Token *elseStmt) {
  StatementPtr stmtList;
  if (!elseifs->stmt) {
    stmtList = NEW_STMT0(StatementList);
  } else {
    stmtList = elseifs->stmt;
  }
  if (stmt->stmt && stmt->stmt->is(Statement::KindOfStatementList)) {
    stmt->stmt = NEW_STMT(BlockStatement,
                          dynamic_pointer_cast<StatementList>(stmt->stmt));
  }
  stmtList->insertElement(NEW_STMT(IfBranchStatement, cond->exp, stmt->stmt));
  if (elseStmt->stmt) {
    if (elseStmt->stmt->is(Statement::KindOfStatementList)) {
      elseStmt->stmt = NEW_STMT
        (BlockStatement, dynamic_pointer_cast<StatementList>(elseStmt->stmt));
    }
    stmtList->addElement(NEW_STMT(IfBranchStatement, ExpressionPtr(),
                                  elseStmt->stmt));
  }
  out->stmt = NEW_STMT(IfStatement,
                       dynamic_pointer_cast<StatementList>(stmtList));
}

void Parser::onElseIf(Token *out, Token *elseifs, Token *cond, Token *stmt) {
  if (!elseifs->stmt) {
    out->stmt = NEW_STMT0(StatementList);
  } else {
    out->stmt = elseifs->stmt;
  }
  if (stmt->stmt && stmt->stmt->is(Statement::KindOfStatementList)) {
    stmt->stmt = NEW_STMT(BlockStatement,
                          dynamic_pointer_cast<StatementList>(stmt->stmt));
  }
  out->stmt->addElement(NEW_STMT(IfBranchStatement, cond->exp, stmt->stmt));
}

void Parser::onWhile(Token *out, Token *cond, Token *stmt) {
  if (stmt->stmt && stmt->stmt->is(Statement::KindOfStatementList)) {
    stmt->stmt = NEW_STMT(BlockStatement,
                          dynamic_pointer_cast<StatementList>(stmt->stmt));
  }
  out->stmt = NEW_STMT(WhileStatement, cond->exp, stmt->stmt);
}

void Parser::onDo(Token *out, Token *stmt, Token *cond) {
  out->stmt = NEW_STMT(DoStatement, stmt->stmt, cond->exp);
}

void Parser::onFor(Token *out, Token *expr1, Token *expr2, Token *expr3,
                   Token *stmt) {
  if (stmt->stmt && stmt->stmt->is(Statement::KindOfStatementList)) {
    stmt->stmt = NEW_STMT(BlockStatement,
                          dynamic_pointer_cast<StatementList>(stmt->stmt));
  }
  out->stmt = NEW_STMT(ForStatement, expr1->exp, expr2->exp, expr3->exp,
                       stmt->stmt);
}

void Parser::onSwitch(Token *out, Token *expr, Token *cases) {
  out->stmt = NEW_STMT(SwitchStatement, expr->exp,
                       dynamic_pointer_cast<StatementList>(cases->stmt));
}

void Parser::onCase(Token *out, Token *cases, Token *cond, Token *stmt) {
  if (!cases->stmt) {
    out->stmt = NEW_STMT0(StatementList);
  } else {
    out->stmt = cases->stmt;
  }
  out->stmt->addElement(NEW_STMT(CaseStatement,
                                 cond ? cond->exp : ExpressionPtr(),
                                 stmt->stmt));
}

void Parser::onBreak(Token *out, Token *expr) {
  out->stmt = NEW_STMT(BreakStatement, expr ? expr->exp : ExpressionPtr());
}

void Parser::onContinue(Token *out, Token *expr) {
  out->stmt = NEW_STMT(ContinueStatement, expr ? expr->exp : ExpressionPtr());
}

void Parser::onReturn(Token *out, Token *expr) {
  out->stmt = NEW_STMT(ReturnStatement, expr ? expr->exp : ExpressionPtr());
}

void Parser::onGlobal(Token *out, Token *expr) {
  out->stmt = NEW_STMT(GlobalStatement,
                       dynamic_pointer_cast<ExpressionList>(expr->exp));
}

void Parser::onGlobalVar(Token *out, Token *exprs, Token *expr) {
  ExpressionPtr expList;
  if (exprs && exprs->exp) {
    expList = exprs->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  switch (expr->num) {
  case 0:
    expList->addElement(NEW_EXP(SimpleVariable, expr->text()));
    break;
  case 1:
    expList->addElement(createDynamicVariable(expr->exp));
    break;
  default:
    ASSERT(false);
  }
  out->exp = expList;
}

void Parser::onStatic(Token *out, Token *expr) {
  out->stmt = NEW_STMT(StaticStatement,
                       dynamic_pointer_cast<ExpressionList>(expr->exp));
}

void Parser::onEcho(Token *out, Token *expr, bool html) {
  if (html) {
    LocationPtr loc = getLocation();
    if (loc->line1 == 2 && loc->char1 == 0 && expr->text()[0] == '#') {
      // skipping linux interpreter declaration
      out->stmt = NEW_STMT0(StatementList);
    } else {
      ExpressionPtr exp = NEW_EXP(ScalarExpression, T_STRING, expr->text(),
                                  true);
      ExpressionListPtr expList = NEW_EXP(ExpressionList);
      expList->addElement(exp);
      out->stmt = NEW_STMT(EchoStatement, expList);
    }
  } else {
    out->stmt = NEW_STMT(EchoStatement,
                         dynamic_pointer_cast<ExpressionList>(expr->exp));
  }
}

void Parser::onUnset(Token *out, Token *expr) {
  out->stmt = NEW_STMT(UnsetStatement,
                       dynamic_pointer_cast<ExpressionList>(expr->exp));
  m_ar->getFileScope()->setAttribute(FileScope::ContainsUnset);
}

void Parser::onExpStatement(Token *out, Token *expr) {
  out->stmt = NEW_STMT(ExpStatement, expr->exp);
}

void Parser::onForEach(Token *out, Token *arr, Token *name, Token *value,
                       Token *stmt) {
  if (stmt->stmt && stmt->stmt->is(Statement::KindOfStatementList)) {
    stmt->stmt = NEW_STMT(BlockStatement,
                          dynamic_pointer_cast<StatementList>(stmt->stmt));
  }
  out->stmt = NEW_STMT(ForEachStatement, arr->exp, name->exp, name->num == 1,
                       value->exp, value->num == 1, stmt->stmt);
}

void Parser::onTry(Token *out, Token *tryStmt, Token *className, Token *var,
                   Token *catchStmt, Token *catches) {
  StatementPtr stmtList;
  if (catches->stmt) {
    stmtList = catches->stmt;
  } else {
    stmtList = NEW_STMT0(StatementList);
  }
  stmtList->insertElement(NEW_STMT(CatchStatement, className->text(),
                                   var->text(), catchStmt->stmt));
  out->stmt = NEW_STMT(TryStatement, tryStmt->stmt,
                       dynamic_pointer_cast<StatementList>(stmtList));
}

void Parser::onCatch(Token *out, Token *className, Token *var, Token *stmt) {
  out->stmt = NEW_STMT(CatchStatement, className->text(), var->text(),
                       stmt->stmt);
}

void Parser::onThrow(Token *out, Token *expr) {
  out->stmt = NEW_STMT(ThrowStatement, expr->exp);
}

void Parser::addHphpNote(ConstructPtr c, const std::string &note) {
  if (note[0] == '@') {
    CodeError::ErrorType e;
    if (CodeError::lookupErrorType(note.substr(1), e)) {
      c->addSuppressError(e);
    } else {
      c->addHphpNote(note);
    }
  } else {
    c->addHphpNote(note);
  }
}

void Parser::onHphpNoteExpr(Token *out, Token *note, Token *expr) {
  addHphpNote(expr->exp, note->text());
  out->exp = expr->exp;
}
void Parser::onHphpNoteStatement(Token *out, Token *note, Token *stmt) {
  addHphpNote(stmt->stmt, note->text());
  out->stmt = stmt->stmt;
}

void Parser::addHphpDeclare(Token *declare) {
  m_ar->getFileScope()->addDeclare(declare->text());
}

void Parser::addHphpSuppressError(Token *error) {
  CodeError::ErrorType e;
  if (CodeError::lookupErrorType(error->text(), e)) {
    m_ar->getFileScope()->addSuppressError(e);
  }
}
