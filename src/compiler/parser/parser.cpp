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
#include <util/parser/hphp.tab.hpp>
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

#include <compiler/analysis/function_scope.h>

#include <compiler/analysis/code_error.h>
#include <compiler/analysis/analysis_result.h>

#include <util/preprocess.h>

#ifdef FACEBOOK
#include <../facebook/src/compiler/fb_compiler_hooks.h>
#define RealSimpleFunctionCall FBSimpleFunctionCall
#else
#define RealSimpleFunctionCall SimpleFunctionCall
#endif

using namespace std;
using namespace boost;

#define NEW_EXP0(cls)                                           \
  cls##Ptr(new cls(BlockScopePtr(), getLocation(), Expression::KindOf##cls))
#define NEW_EXP(cls, e...)                                      \
  cls##Ptr(new cls(BlockScopePtr(), getLocation(), \
                   Expression::KindOf##cls, ##e))
#define NEW_STMT0(cls)                                          \
  cls##Ptr(new cls(BlockScopePtr(), getLocation(), Statement::KindOf##cls))
#define NEW_STMT(cls, e...)                                     \
  cls##Ptr(new cls(BlockScopePtr(), getLocation(), Statement::KindOf##cls, ##e))

namespace HPHP {

SimpleFunctionCallPtr NewSimpleFunctionCall(
  EXPRESSION_CONSTRUCTOR_PARAMETERS,
  const std::string &name, ExpressionListPtr params,
  ExpressionPtr cls) {
  return SimpleFunctionCallPtr(
    new RealSimpleFunctionCall(EXPRESSION_CONSTRUCTOR_PARAMETER_VALUES,
                               name, params, cls));
}

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////
// statics

StatementListPtr Parser::ParseString(const char *input, AnalysisResultPtr ar,
                                     const char *fileName /* = NULL */) {
  ASSERT(input);
  if (!fileName || !*fileName) fileName = "string";

  int len = strlen(input);
  Scanner scanner(input, len, Option::ScannerType, fileName);
  Parser parser(scanner, fileName, ar, len);
  if (parser.parse()) {
    return parser.getTree();
  }
  printf("Error parsing %s: %s\n%s\n", fileName,
         parser.getMessage().c_str(), input);
  return StatementListPtr();
}

///////////////////////////////////////////////////////////////////////////////

Parser::Parser(Scanner &scanner, const char *fileName,
               AnalysisResultPtr ar, int fileSize /* = 0 */)
    : ParserBase(scanner, fileName), m_ar(ar) {
  m_file = FileScopePtr(new FileScope(m_fileName, fileSize));
  m_ar->addFileScope(m_file);
  newScope();
}

void Parser::pushComment() {
  m_comments.push_back(m_scanner.detachDocComment());
}

std::string Parser::popComment() {
  std::string ret = m_comments.back();
  m_comments.pop_back();
  return ret;
}

void Parser::newScope() {
  m_scopes.push_back(BlockScopePtrVec());
}

void Parser::completeScope(BlockScopePtr inner) {
  assert(inner);
  BlockScopePtrVec &sv = m_scopes.back();
  for (int i = 0, n = sv.size(); i < n; i++) {
    BlockScopePtr scope = sv[i];
    scope->setOuterScope(inner);
  }
  inner->getStmt()->resetScope(inner);
  m_scopes.pop_back();
  if (m_scopes.size()) {
    m_scopes.back().push_back(inner);
  }
}

///////////////////////////////////////////////////////////////////////////////
// variables

void Parser::onName(Token &out, Token &name, NameKind kind) {
  switch (kind) {
    case StringName:
    case StaticName:
      onScalar(out, T_STRING, name);
      break;
    case StaticClassExprName:
    case ExprName:
    case VarName:
      out = name;
      break;
  }
}

void Parser::onStaticVariable(Token &out, Token *exprs, Token &var,
                              Token *value) {
  onVariable(out, exprs, var, value);
}

void Parser::onClassVariable(Token &out, Token *exprs, Token &var,
                             Token *value) {
  onVariable(out, exprs, var, value);
}

void Parser::onClassConstant(Token &out, Token *exprs, Token &var,
                             Token &value) {
  onVariable(out, exprs, var, &value, true);
}

void Parser::onVariable(Token &out, Token *exprs, Token &var, Token *value,
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
  expList->addElement(exp);
  out->exp = expList;
}

void Parser::onSimpleVariable(Token &out, Token &var) {
  out->exp = NEW_EXP(SimpleVariable, var->text());
}

void Parser::onDynamicVariable(Token &out, Token &expr, bool encap) {
  out->exp = getDynamicVariable(expr->exp, encap);
}

void Parser::onIndirectRef(Token &out, Token &refCount, Token &var) {
  out->exp = var->exp;
  for (int i = 0; i < refCount->num(); i++) {
    out->exp = createDynamicVariable(out->exp);
  }
}

void Parser::onStaticMember(Token &out, Token &cls, Token &name) {
  if (name->exp->is(Expression::KindOfArrayElementExpression) &&
      dynamic_pointer_cast<ArrayElementExpression>(name->exp)->
      appendClass(cls->exp)) {
    out->exp = name->exp;
  } else {
    out->exp = NEW_EXP(StaticMemberExpression, cls->exp, name->exp);
  }
}

void Parser::onRefDim(Token &out, Token &var, Token &offset) {
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
  m_file->setAttribute(FileScope::ContainsDynamicVariable);
  return NEW_EXP(DynamicVariable, exp);
}

void Parser::onCallParam(Token &out, Token *params, Token &expr, bool ref) {
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

void Parser::onCall(Token &out, bool dynamic, Token &name, Token &params,
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
    SimpleFunctionCallPtr call
      (new RealSimpleFunctionCall
       (BlockScopePtr(), getLocation(),
        Expression::KindOfSimpleFunctionCall, name->text(),
        dynamic_pointer_cast<ExpressionList>(params->exp), clsExp));
    out->exp = call;
    call->onParse(m_ar, m_file);
  }
}

///////////////////////////////////////////////////////////////////////////////
// object property and method calls

void Parser::pushObject(Token &base) {
  m_objects.push_back(base->exp);
}

void Parser::popObject(Token &out) {
  out->exp = m_objects.back();
  m_objects.pop_back();
}

void Parser::appendMethodParams(Token &params) {
  ExpressionListPtr paramsExp;
  if (params->exp) {
    paramsExp = dynamic_pointer_cast<ExpressionList>(params->exp);
  } else if (params->num() == 1) {
    paramsExp = NEW_EXP0(ExpressionList);
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

void Parser::appendProperty(Token &prop) {
  if (!prop->exp) {
    prop->exp = NEW_EXP(ScalarExpression, T_STRING, prop->text());
  }
  m_objects.back() = NEW_EXP(ObjectPropertyExpression, m_objects.back(),
                             prop->exp);
}

void Parser::appendRefDim(Token &offset) {
  m_objects.back() = NEW_EXP(ArrayElementExpression, m_objects.back(),
                             offset->exp);
}

///////////////////////////////////////////////////////////////////////////////
// encapsed expressions

void Parser::onEncapsList(Token &out, int type, Token &list) {
  out->exp = NEW_EXP(EncapsListExpression, type,
                     dynamic_pointer_cast<ExpressionList>(list->exp));
}

void Parser::addEncap(Token &out, Token &list, Token &expr, int type) {
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
    exp = NEW_EXP(ScalarExpression, T_ENCAPSED_AND_WHITESPACE,
                  expr->text(), true);
  }
  expList->addElement(exp);
  out->exp = expList;
}

void Parser::encapRefDim(Token &out, Token &var, Token &offset) {
  ExpressionPtr dim;
  switch (offset->num()) {
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

void Parser::encapObjProp(Token &out, Token &var, Token &name) {
  ExpressionPtr obj = NEW_EXP(SimpleVariable, var->text());

  ExpressionPtr prop = NEW_EXP(ScalarExpression, T_STRING, name->text());
  out->exp = NEW_EXP(ObjectPropertyExpression, obj, prop);
}

void Parser::encapArray(Token &out, Token &var, Token &expr) {
  ExpressionPtr arr = NEW_EXP(SimpleVariable, var->text());
  out->exp = NEW_EXP(ArrayElementExpression, arr, expr->exp);
}

///////////////////////////////////////////////////////////////////////////////
// expressions

void Parser::onConstant(Token &out, Token &constant) {
  out->exp = NEW_EXP(ConstantExpression, constant->text());
}

void Parser::onScalar(Token &out, int type, Token &scalar) {
  if (type == T_FILE) {
    onUnaryOpExp(out, scalar, T_FILE, true);
    return;
  }

  ScalarExpressionPtr exp;
  switch (type) {
  case T_STRING:
  case T_LNUMBER:
  case T_DNUMBER:
  case T_LINE:
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
  out->exp = exp;
}

void Parser::onExprListElem(Token &out, Token *exprs, Token &expr) {
  ExpressionPtr expList;
  if (exprs && exprs->exp) {
    expList = exprs->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(expr->exp);
  out->exp = expList;
}

void Parser::onListAssignment(Token &out, Token &vars, Token *expr) {
  out->exp = NEW_EXP(ListAssignment,
                     dynamic_pointer_cast<ExpressionList>(vars->exp),
                     expr ? expr->exp : ExpressionPtr());
}

void Parser::onAListVar(Token &out, Token *list, Token *var) {
  Token empty_list, empty_var;
  if (!var) {
    empty_var.exp = ExpressionPtr();
    var = &empty_var;
  }
  if (!list) {
    empty_list.exp = NEW_EXP0(ExpressionList);
    list = &empty_list;
  }
  onExprListElem(out, list, *var);
}

void Parser::onAListSub(Token &out, Token *list, Token &sublist) {
  onListAssignment(out, sublist, NULL);
  onExprListElem(out, list, out);
}

void Parser::onAssign(Token &out, Token &var, Token &expr, bool ref) {
  out->exp = NEW_EXP(AssignmentExpression, var->exp, expr->exp, ref);
}

void Parser::onAssignNew(Token &out, Token &var, Token &name, Token &args) {
  ExpressionPtr exp =
    NEW_EXP(NewObjectExpression, name->exp,
            dynamic_pointer_cast<ExpressionList>(args->exp));
  out->exp = NEW_EXP(AssignmentExpression, var->exp, exp, false);
}

void Parser::onNewObject(Token &out, Token &name, Token &args) {
  out->exp = NEW_EXP(NewObjectExpression, name->exp,
                     dynamic_pointer_cast<ExpressionList>(args->exp));
}

void Parser::onUnaryOpExp(Token &out, Token &operand, int op, bool front) {
  switch (op) {
  case T_INCLUDE:
  case T_INCLUDE_ONCE:
  case T_REQUIRE:
  case T_REQUIRE_ONCE:
    {
      IncludeExpressionPtr exp = NEW_EXP(IncludeExpression, operand->exp, op);
      out->exp = exp;
      exp->onParse(m_ar, m_file);
    }
    break;
  default:
    {
      UnaryOpExpressionPtr exp = NEW_EXP(UnaryOpExpression, operand->exp, op,
                                         front);
      out->exp = exp;
      exp->onParse(m_ar, m_file);
    }
    break;
  }
}

void Parser::onBinaryOpExp(Token &out, Token &operand1, Token &operand2,
                           int op) {
  out->exp = NEW_EXP(BinaryOpExpression, operand1->exp, operand2->exp, op);
}

void Parser::onQOp(Token &out, Token &exprCond, Token &expYes, Token &expNo) {
  out->exp = NEW_EXP(QOpExpression, exprCond->exp, expYes->exp, expNo->exp);
}

void Parser::onArray(Token &out, Token &pairs) {
  onUnaryOpExp(out, pairs, T_ARRAY, true);
}

void Parser::onArrayPair(Token &out, Token *pairs, Token *name, Token &value,
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

void Parser::onClassConst(Token &out, Token &cls, Token &name, bool text) {
  if (!cls->exp) {
    cls->exp = NEW_EXP(ScalarExpression, T_STRING, cls->text());
  }
  out->exp = NEW_EXP(ClassConstantExpression, cls->exp, name->text());
}

///////////////////////////////////////////////////////////////////////////////
// function/method declaration

void Parser::onFunctionStart(Token &name) {
  m_file->pushAttribute();
  pushComment();
  newScope();
}

void Parser::onMethodStart(Token &name, Token &mods) {
  onFunctionStart(name);
}

void Parser::onFunction(Token &out, Token &ref, Token &name, Token &params,
                        Token &stmt) {
  if (!stmt->stmt) {
    stmt->stmt = NEW_STMT0(StatementList);
  }
  FunctionStatementPtr func = NEW_STMT
    (FunctionStatement, ref->num(), name->text(),
     dynamic_pointer_cast<ExpressionList>(params->exp),
     dynamic_pointer_cast<StatementList>(stmt->stmt),
     m_file->popAttribute(),
     popComment());
  out->stmt = func;
  func->onParse(m_ar, m_file);
  completeScope(func->getFunctionScope());
  if (func->ignored()) {
    out->stmt = NEW_STMT0(StatementList);
  }
}

void Parser::onParam(Token &out, Token *params, Token &type, Token &var,
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

void Parser::onClassStart(int type, Token &name, Token *parent) {
  pushComment();
  newScope();
}

void Parser::onClass(Token &out, Token &type, Token &name, Token &base,
                     Token &baseInterface, Token &stmt) {
  StatementListPtr stmtList;
  if (stmt->stmt) {
    stmtList = dynamic_pointer_cast<StatementList>(stmt->stmt);
  }

  ClassStatementPtr cls = NEW_STMT
    (ClassStatement, type->num(), name->text(), base->text(),
     dynamic_pointer_cast<ExpressionList>(baseInterface->exp),
     popComment(), stmtList);
  out->stmt = cls;
  cls->onParse(m_ar, m_file);
  completeScope(cls->getClassScope());
  if (cls->ignored()) {
    out->stmt = NEW_STMT0(StatementList);
  }
}

void Parser::onInterface(Token &out, Token &name, Token &base, Token &stmt) {
  StatementListPtr stmtList;
  if (stmt->stmt) {
    stmtList = dynamic_pointer_cast<StatementList>(stmt->stmt);
  }

  InterfaceStatementPtr intf = NEW_STMT
    (InterfaceStatement, name->text(),
     dynamic_pointer_cast<ExpressionList>(base->exp), popComment(), stmtList);
  out->stmt = intf;
  intf->onParse(m_ar, m_file);
  completeScope(intf->getClassScope());
}

void Parser::onInterfaceName(Token &out, Token *names, Token &name) {
  ExpressionPtr expList;
  if (names) {
    expList = names->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(NEW_EXP(ScalarExpression, T_STRING, name->text()));
  out->exp = expList;
}

void Parser::onClassVariableStart(Token &out, Token *modifiers, Token &decl) {
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

void Parser::onMethod(Token &out, Token &modifiers, Token &ref, Token &name,
                      Token &params, Token &stmt) {
  ModifierExpressionPtr exp = modifiers->exp ?
    dynamic_pointer_cast<ModifierExpression>(modifiers->exp)
    : NEW_EXP0(ModifierExpression);

  StatementListPtr stmts;
  if (!stmt->stmt && stmt->num() == 1) {
    stmts = NEW_STMT0(StatementList);
  } else {
    stmts = dynamic_pointer_cast<StatementList>(stmt->stmt);
  }

  MethodStatementPtr mth = NEW_STMT
    (MethodStatement, exp, ref->num(), name->text(),
     dynamic_pointer_cast<ExpressionList>(params->exp), stmts,
     m_file->popAttribute(),
     popComment());

  completeScope(mth->onInitialParse(m_ar, m_file, true));
  out->stmt = mth;
}

void Parser::onMemberModifier(Token &out, Token *modifiers, Token &modifier) {
  ModifierExpressionPtr expList;
  if (modifiers) {
    expList = dynamic_pointer_cast<ModifierExpression>(modifiers->exp);
  } else {
    expList = NEW_EXP0(ModifierExpression);
  }
  expList->add(modifier->num());
  out->exp = expList;
}

///////////////////////////////////////////////////////////////////////////////
// statements

void Parser::saveParseTree(Token &tree) {
  if (tree->stmt) {
    m_tree = dynamic_pointer_cast<StatementList>(tree->stmt);
  } else {
    m_tree = NEW_STMT0(StatementList);
  }
  FunctionScopePtr pseudoMain = m_file->setTree(m_ar, m_tree);
  completeScope(pseudoMain);
  pseudoMain->setOuterScope(m_file);
  m_file->setOuterScope(m_ar);
}

void Parser::onStatementListStart(Token &out) {
  out.reset();
}

void Parser::addStatement(Token &out, Token &stmts, Token &new_stmt) {
  if (!stmts->stmt) {
    out->stmt = NEW_STMT0(StatementList);
  } else {
    out->stmt = stmts->stmt;
  }
  if (new_stmt->stmt) {
    out->stmt->addElement(new_stmt->stmt);
  }
}

void Parser::finishStatement(Token &out, Token &stmts) {
  if (!stmts->stmt) {
    out->stmt = NEW_STMT0(StatementList);
  } else {
    out->stmt = stmts->stmt;
  }
}

void Parser::onBlock(Token &out, Token &stmts) {
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

void Parser::onIf(Token &out, Token &cond, Token &stmt, Token &elseifs,
                  Token &elseStmt) {
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

void Parser::onElseIf(Token &out, Token &elseifs, Token &cond, Token &stmt) {
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

void Parser::onWhile(Token &out, Token &cond, Token &stmt) {
  if (stmt->stmt && stmt->stmt->is(Statement::KindOfStatementList)) {
    stmt->stmt = NEW_STMT(BlockStatement,
                          dynamic_pointer_cast<StatementList>(stmt->stmt));
  }
  out->stmt = NEW_STMT(WhileStatement, cond->exp, stmt->stmt);
}

void Parser::onDo(Token &out, Token &stmt, Token &cond) {
  out->stmt = NEW_STMT(DoStatement, stmt->stmt, cond->exp);
}

void Parser::onFor(Token &out, Token &expr1, Token &expr2, Token &expr3,
                   Token &stmt) {
  if (stmt->stmt && stmt->stmt->is(Statement::KindOfStatementList)) {
    stmt->stmt = NEW_STMT(BlockStatement,
                          dynamic_pointer_cast<StatementList>(stmt->stmt));
  }
  out->stmt = NEW_STMT(ForStatement, expr1->exp, expr2->exp, expr3->exp,
                       stmt->stmt);
}

void Parser::onSwitch(Token &out, Token &expr, Token &cases) {
  out->stmt = NEW_STMT(SwitchStatement, expr->exp,
                       dynamic_pointer_cast<StatementList>(cases->stmt));
}

void Parser::onCase(Token &out, Token &cases, Token *cond, Token &stmt) {
  if (!cases->stmt) {
    out->stmt = NEW_STMT0(StatementList);
  } else {
    out->stmt = cases->stmt;
  }
  out->stmt->addElement(NEW_STMT(CaseStatement,
                                 cond ? cond->exp : ExpressionPtr(),
                                 stmt->stmt));
}

void Parser::onBreak(Token &out, Token *expr) {
  out->stmt = NEW_STMT(BreakStatement, expr ? expr->exp : ExpressionPtr());
}

void Parser::onContinue(Token &out, Token *expr) {
  out->stmt = NEW_STMT(ContinueStatement, expr ? expr->exp : ExpressionPtr());
}

void Parser::onReturn(Token &out, Token *expr) {
  out->stmt = NEW_STMT(ReturnStatement, expr ? expr->exp : ExpressionPtr());
}

void Parser::onGlobal(Token &out, Token &expr) {
  out->stmt = NEW_STMT(GlobalStatement,
                       dynamic_pointer_cast<ExpressionList>(expr->exp));
}

void Parser::onGlobalVar(Token &out, Token *exprs, Token &expr) {
  ExpressionPtr expList;
  if (exprs && exprs->exp) {
    expList = exprs->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  switch (expr->num()) {
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

void Parser::onStatic(Token &out, Token &expr) {
  out->stmt = NEW_STMT(StaticStatement,
                       dynamic_pointer_cast<ExpressionList>(expr->exp));
}

void Parser::onEcho(Token &out, Token &expr, bool html) {
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

void Parser::onUnset(Token &out, Token &expr) {
  out->stmt = NEW_STMT(UnsetStatement,
                       dynamic_pointer_cast<ExpressionList>(expr->exp));
  m_file->setAttribute(FileScope::ContainsUnset);
}

void Parser::onExpStatement(Token &out, Token &expr) {
  out->stmt = NEW_STMT(ExpStatement, expr->exp);
}

void Parser::onForEach(Token &out, Token &arr, Token &name, Token &value,
                       Token &stmt) {
  if (stmt->stmt && stmt->stmt->is(Statement::KindOfStatementList)) {
    stmt->stmt = NEW_STMT(BlockStatement,
                          dynamic_pointer_cast<StatementList>(stmt->stmt));
  }
  out->stmt = NEW_STMT(ForEachStatement, arr->exp, name->exp, name->num() == 1,
                       value->exp, value->num() == 1, stmt->stmt);
}

void Parser::onTry(Token &out, Token &tryStmt, Token &className, Token &var,
                   Token &catchStmt, Token &catches) {
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

void Parser::onCatch(Token &out, Token &catches, Token &className, Token &var,
                     Token &stmt) {
  StatementPtr stmtList;
  if (catches->stmt) {
    stmtList = catches->stmt;
  } else {
    stmtList = NEW_STMT0(StatementList);
  }
  stmtList->addElement(NEW_STMT(CatchStatement, className->text(),
                                var->text(), stmt->stmt));
  out->stmt = stmtList;
}

void Parser::onThrow(Token &out, Token &expr) {
  out->stmt = NEW_STMT(ThrowStatement, expr->exp);
}

void Parser::onHphpNoteExpr(Token &out, Token &note, Token &expr) {
  out->exp = expr->exp;
}
void Parser::onHphpNoteStatement(Token &out, Token &note, Token &stmt) {
  out->stmt = stmt->stmt;
}

void Parser::addHphpDeclare(Token &declare) {
}

void Parser::addHphpSuppressError(Token &error) {
}

///////////////////////////////////////////////////////////////////////////////
}}
