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

#include <cpp/eval/parser/parser.h>
#include <cpp/eval/parser/hphp.tab.h>

#include <cpp/eval/ast/array_element_expression.h>
#include <cpp/eval/ast/array_expression.h>
#include <cpp/eval/ast/assignment_op_expression.h>
#include <cpp/eval/ast/assignment_ref_expression.h>
#include <cpp/eval/ast/binary_op_expression.h>
#include <cpp/eval/ast/class_constant_expression.h>
#include <cpp/eval/ast/constant_expression.h>
#include <cpp/eval/ast/encaps_list_expression.h>
#include <cpp/eval/ast/inc_op_expression.h>
#include <cpp/eval/ast/include_expression.h>
#include <cpp/eval/ast/instanceof_expression.h>
#include <cpp/eval/ast/isset_expression.h>
#include <cpp/eval/ast/list_assignment_expression.h>
#include <cpp/eval/ast/new_object_expression.h>
#include <cpp/eval/ast/object_method_expression.h>
#include <cpp/eval/ast/object_property_expression.h>
#include <cpp/eval/ast/qop_expression.h>
#include <cpp/eval/ast/ref_param_expression.h>
#include <cpp/eval/ast/scalar_expression.h>
#include <cpp/eval/ast/simple_function_call_expression.h>
#include <cpp/eval/ast/static_method_expression.h>
#include <cpp/eval/ast/variable_expression.h>
#include <cpp/eval/ast/static_member_expression.h>
#include <cpp/eval/ast/this_expression.h>
#include <cpp/eval/ast/unary_op_expression.h>

#include <cpp/eval/ast/break_statement.h>
#include <cpp/eval/ast/class_statement.h>
#include <cpp/eval/ast/do_while_statement.h>
#include <cpp/eval/ast/echo_statement.h>
#include <cpp/eval/ast/expr_statement.h>
#include <cpp/eval/ast/for_statement.h>
#include <cpp/eval/ast/foreach_statement.h>
#include <cpp/eval/ast/function_statement.h>
#include <cpp/eval/ast/global_statement.h>
#include <cpp/eval/ast/if_statement.h>
#include <cpp/eval/ast/method_statement.h>
#include <cpp/eval/ast/return_statement.h>
#include <cpp/eval/ast/statement_list_statement.h>
#include <cpp/eval/ast/static_statement.h>
#include <cpp/eval/ast/strong_foreach_statement.h>
#include <cpp/eval/ast/switch_statement.h>
#include <cpp/eval/ast/throw_statement.h>
#include <cpp/eval/ast/try_statement.h>
#include <cpp/eval/ast/unset_statement.h>
#include <cpp/eval/ast/while_statement.h>

#include <cpp/eval/ast/name.h>

#include <util/preprocess.h>
#include <cpp/base/runtime_option.h>
#include <cpp/base/util/string_buffer.h>

using namespace HPHP;
using namespace HPHP::Eval;
using namespace std;
using namespace boost;

#define NEW_EXP0(cls)                                           \
  cls##ExpressionPtr(new cls##Expression(this))
#define NEW_EXP(cls, e...)                                      \
  cls##ExpressionPtr(new cls##Expression(this, e))
#define NEW_STMT0(cls)                                          \
  cls##StatementPtr(new cls##Statement(this))
#define NEW_STMT(cls, e...)                                     \
  cls##StatementPtr(new cls##Statement(this, e))

///////////////////////////////////////////////////////////////////////////////
// statics

StatementPtr Parser::parseString(const char *input,
                                 vector<StaticStatementPtr> &statics) {
  ASSERT(input);
  istringstream iss(input);
  stringstream ss;
  istream *is = RuntimeOption::EnableXHP ? preprocessXHP(iss, ss, "") : &iss;
  Scanner scanner(new ylmm::basic_buffer(*is, false, true), true, false);
  Parser parser(scanner, NULL, statics);
  if (parser.parse()) {
    scanner.flushFlex();
    Logger::Error("Error parsing %s: %s\n", input,
                  parser.getMessage().c_str());
    return StatementPtr();
  }
  StatementPtr s = parser.getTree();
  return s;
}

StatementPtr Parser::parseFile(const char *input,
                               vector<StaticStatementPtr> &statics) {
  ASSERT(input);
  ifstream iss(input);
  StatementPtr s;
  if (!iss.good()) return s;

  stringstream ss;
  istream *is = RuntimeOption::EnableXHP ? preprocessXHP(iss, ss, input) : &iss;
  Scanner scanner(new ylmm::basic_buffer(*is, false, true),
                  true, false);
  Parser parser(scanner, input, statics);
  if (parser.parse()) {
    scanner.flushFlex();
    Logger::Error("Error parsing %s: %s\n", input,
                  parser.getMessage().c_str());
    return StatementPtr();
  }
  s = parser.getTree();
  return s;
}

///////////////////////////////////////////////////////////////////////////////
String Location::toString() const {
  StringBuffer buf;
  buf.printf("%s:%d:%d", this->file, this->line1, this->char1);
  return buf.detach();
}

///////////////////////////////////////////////////////////////////////////////

Parser::Parser(Scanner &s, const char *fileName,
               vector<StaticStatementPtr> &statics)
  : m_scanner(s), m_staticStatements(statics) {
  _location = &m_location;
  m_messenger.error_stream(m_err);
  m_messenger.message_stream(m_msg);
  messenger(m_messenger);
  m_fileName = fileName;
}

ClassStatementPtr Parser::currentClass() const {
  if (haveClass()) {
    return peekClass();
  }
  return ClassStatementPtr();
}

StatementPtr Parser::getTree() const {
  return m_tree;
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

void Parser::getLocation(Location &location) {
  location.file = file();
  location.line1 = line1();
  location.char1 = char1();
}

void Parser::pushClass(ClassStatementPtr cl) { m_classes.push(cl); }
bool Parser::haveClass() const { return !m_classes.empty(); }
ClassStatementPtr Parser::peekClass() const { return m_classes.top(); }
void Parser::popClass() { m_classes.pop(); }

void Parser::pushFunc(FunctionStatementPtr fs) { m_funcs.push(fs); }
bool Parser::haveFunc() const { return !m_funcs.empty(); }
FunctionStatementPtr Parser::peekFunc() const { return m_funcs.top(); }
void Parser::popFunc() { m_funcs.pop(); }

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
// names

void Parser::onName(Token &out, Token &name, bool string) {
  out.reset();
  if (string) {
    out->name() = Name::fromString(this, name.getText());
  } else {
    out->name() = Name::fromExp(this, name->exp());
  }
}

///////////////////////////////////////////////////////////////////////////////
// variables

void Parser::onStaticVariable(Token &out, Token *exprs, Token &var,
                              Token *value) {
  out.reset();
  if (exprs) {
    out = *exprs;
  }
  ExpressionPtr exp;
  if (value) {
    exp = (*value)->exp();
  }
  out->staticVars().
    push_back(StaticVariablePtr(new StaticVariable(this, var.getText(),
                                                   exp)));
}

void Parser::onSimpleVariable(Token &out, Token &var) {
  out.reset();
  if (var.getText() == "this") {
    out->exp() = NEW_EXP0(This);
  } else {
    int idx = -1;
    if (haveFunc()) {
      idx = peekFunc()->declareVariable(var.getText());
    }
    out->exp() = NEW_EXP(Variable,
                         Name::fromString(this, var.getText()), idx);
  }
}

void Parser::onDynamicVariable(Token &out, Token &expr, bool encap) {
  out.reset();
  out->exp() = getDynamicVariable(expr->exp(), encap);
}

void Parser::onIndirectRef(Token &out, Token &refCount, Token &var) {
  out.reset();
  out->exp() = var->exp();
  for (int i = 0; i < refCount.num; i++) {
    out->exp() = createDynamicVariable(out->exp());
  }
}

void Parser::onStaticMember(Token &out, Token &className, Token &name) {
  out.reset();
  ArrayElementExpressionPtr arr = name->getExp<ArrayElementExpression>();
  string cn = className.getText();
  if (haveClass()) {
    if (cn == "self") {
      cn = peekClass()->name();
    } else if (cn == "parent") {
      cn = peekClass()->parent();
    }
  }
  if (arr) {
    arr->sinkStaticMember(this, cn);
    out->exp() = name->exp();
  } else {
    NamePtr n;
    VariableExpressionPtr var = name->getExp<VariableExpression>();
    if (var) {
      n = var->getName();
    }
    ASSERT(n);
    out->exp() = NEW_EXP(StaticMember, cn, n);
  }
}

void Parser::onRefDim(Token &out, Token &var, Token &offset) {
  out.reset();
  if (!var->exp()) {
    ASSERT(false);
    //var->exp() = NEW_EXP(ConstantExpression, var.getText());
  }
  LvalExpressionPtr lv = var->getExp<LvalExpression>();
  ASSERT(lv);
  out->exp() = NEW_EXP(ArrayElement, lv, offset->exp());
}

ExpressionPtr Parser::getDynamicVariable(ExpressionPtr exp, bool encap) {
  NamePtr n;
  if (encap) {
    ConstantExpressionPtr var = exp->cast<ConstantExpression>();
    if (var) {
      n = Name::fromString(this, var->getName());
    }
  } else {
    n = Name::fromExp(this, exp);
  }
  return NEW_EXP(Variable, n);
}

ExpressionPtr Parser::createDynamicVariable(ExpressionPtr exp) {
  return NEW_EXP(Variable, Name::fromExp(this, exp));
}

void Parser::onCallParam(Token &out, Token *params, Token &expr, bool ref) {
  out.reset();
  if (params) {
    out = *params;
  }
  ExpressionPtr param = expr->exp();
  if (ref) {
    param = NEW_EXP(RefParam, param->cast<LvalExpression>());
  }
  out->exprs().push_back(param);
}

void Parser::onCall(Token &out, bool dynamic, Token &name, Token &params,
                    Token *className) {
  out.reset();
  NamePtr n;
  if (dynamic) {
    n = Name::fromExp(this, name->exp());
  } else {
    n = Name::fromString(this, name.getText());
    string s = name.getText();
    if((s == "func_num_args") ||
       (s == "func_get_args") ||
       (s == "func_get_arg")) {
      m_hasCallToGetArgs = true;
    }
  }

  if (className) {
    string cn = className->getText();
    if (haveClass()) {
      if (cn == "self") {
        cn = peekClass()->name();
      } else if (cn == "parent") {
        cn = peekClass()->parent();
      }
    }
    out->exp() = NEW_EXP(StaticMethod, cn, n,
                         params->exprs());
  } else {
    out->exp() = SimpleFunctionCallExpression::make(this, n,
                                                    params->exprs(), *this);
  }
}

///////////////////////////////////////////////////////////////////////////////
// object property and method calls

void Parser::pushObject(Token &base) {
  m_objects.push_back(base->exp());
}

void Parser::popObject(Token &out) {
  out->exp() = m_objects.back();
  m_objects.pop_back();
}

void Parser::appendMethodParams(Token &params) {
  if (params.num == 1) {
    ObjectPropertyExpressionPtr prop = (m_objects.back())->
      cast<ObjectPropertyExpression>();
    if (prop) {
      ObjectMethodExpressionPtr method =
        NEW_EXP(ObjectMethod,
                prop->getObject(), prop->getProperty(), params->exprs());
      m_objects.back() = method;
    } else {
      m_objects.back() = NEW_EXP(SimpleFunctionCall,
                                 Name::fromExp(this, m_objects.back()),
                                 params->exprs());
    }
  }
}

void Parser::appendProperty(Token &prop) {
  // Gross
  if (prop->getMode() == TokenPayload::SingleName) {
    m_objects.back() = NEW_EXP(ObjectProperty, m_objects.back(), prop->name());
  } else {
    m_objects.back() = NEW_EXP(ObjectProperty, m_objects.back(),
                               Name::fromExp(this, prop->exp()));
  }
}

void Parser::appendRefDim(Token &offset) {
  LvalExpressionPtr lv = m_objects.back()->cast<LvalExpression>();
  ASSERT(lv);
  m_objects.back() = NEW_EXP(ArrayElement, lv, offset->exp());
}

///////////////////////////////////////////////////////////////////////////////
// encapsed expressions

void Parser::onEncapsList(Token &out, int type, Token &list) {
  out.reset();
  out->exp() = NEW_EXP(EncapsList, list->exprs(), type == '`');
}

void Parser::addEncap(Token &out, Token &list, Token &expr, int type) {
  out.reset();
  out = list;
  ExpressionPtr exp;
  if (type == -1) {
    exp = expr->exp();
  } else {
    ScalarExpressionPtr scalar =
      NEW_EXP(Scalar, T_STRING, expr.getText());
    exp = scalar;
  }
  out->exprs().push_back(exp);
}

void Parser::encapRefDim(Token &out, Token &var, Token &offset) {
  out.reset();
  ExpressionPtr dim;
  switch (offset.num) {
  case T_STRING:
    dim = NEW_EXP(Scalar, T_STRING, offset.getText());
    break;
  case T_NUM_STRING:
    dim = NEW_EXP(Scalar, T_NUM_STRING, offset.getText());
    break;
  case T_VARIABLE:
    dim = NEW_EXP(Variable, Name::fromString(this, offset.getText()));
    break;
  default:
    ASSERT(false);
  }

  LvalExpressionPtr arr = NEW_EXP(Variable,
                                  Name::fromString(this, var.getText()));
  out->exp() = NEW_EXP(ArrayElement, arr, dim);
}

void Parser::encapObjProp(Token &out, Token &var, Token &name) {
  out.reset();
  ExpressionPtr obj;
  if (var.getText() == "this") {
    obj = NEW_EXP0(This);
  } else {
    obj = NEW_EXP(Variable, Name::fromString(this, var.getText()));
  }
  out->exp() = NEW_EXP(ObjectProperty, obj, Name::fromString(this,
                                                             name.getText()));
}

void Parser::encapArray(Token &out, Token &var, Token &expr) {
  out.reset();
  LvalExpressionPtr arr = NEW_EXP(Variable, Name::fromString(this,
                                                             var.getText()));
  ExpressionPtr exp = NEW_EXP(ArrayElement, arr, expr->exp());
  out->exp() = getDynamicVariable(exp, false);
}

///////////////////////////////////////////////////////////////////////////////
// expressions

void Parser::onConstant(Token &out, Token &constant) {
  out.reset();
  string lower = Util::toLower(constant.getText());
  if (lower == "true") {
    out->exp() = NEW_EXP(Scalar, true);
  } else if (lower == "false") {
    out->exp() = NEW_EXP(Scalar, false);
  } else if (lower == "null") {
    out->exp() = NEW_EXP0(Scalar);
  } else {
    out->exp() = NEW_EXP(Constant, constant.getText());
  }
}

void Parser::onScalar(Token &out, int type, Token &scalar) {
  out.reset();
  ScalarExpressionPtr exp;
  string stext = scalar.getText();
  switch (type) {
  case T_CLASS_C:
    type = T_STRING;
    stext = haveClass() ? peekClass()->name() : "";
    break;
  case T_METHOD_C:
    type = T_STRING;
    if (haveClass() && haveFunc()) {
      stext = peekClass()->name() + "::" + peekFunc()->name();
      break;
    }
    // Fall through
  case T_FUNC_C:
    type = T_STRING;
    stext = haveFunc() ? peekFunc()->name() : "";
  }
  switch (type) {
  case T_STRING:
  case T_LNUMBER:
  case T_DNUMBER:
    exp = NEW_EXP(Scalar, type, stext);
    break;
  case T_CONSTANT_ENCAPSED_STRING:
    exp = NEW_EXP(Scalar, T_STRING, stext);
    break;
  case T_LINE: {
    exp = NEW_EXP(Scalar, T_LNUMBER,
                  lexical_cast<string>(line1()));
    break;
  }
  case T_FILE: {
    exp = NEW_EXP(Scalar, T_STRING, file());
    break;
  }
  default:
    ASSERT(false);
  }
  out->exp() = exp;
}

void Parser::onExprListElem(Token &out, Token *exprs, Token &expr) {
  out.reset();
  if (exprs) {
    out = *exprs;
  }
  out->exprs().push_back(expr->exp());
}

void Parser::onListAssignment(Token &out, Token &vars, Token &expr) {
  out.reset();
  ListElementPtr le(new SubListElement(this, vars->listElems()));
  out->exp() = NEW_EXP(ListAssignment, le, expr->exp());
}

void Parser::onAListVar(Token &out, Token *list, Token *var) {
  out.reset();
  if (list) {
    out = *list;
  }
  LvalExpressionPtr lv;
  if (var) {
    lv = (*var)->getExp<LvalExpression>();
  }
  ListElementPtr le(new LvalListElement(this, lv));
  out->listElems().push_back(le);
}
void Parser::onAListSub(Token &out, Token *list, Token &sublist) {
  out.reset();
  if (list) {
    out = *list;
  }
  ListElementPtr le(new SubListElement(this, sublist->listElems()));
  out->listElems().push_back(le);
}


void Parser::onAssign(Token &out, Token &var, Token &expr, bool ref) {
  out.reset();
  LvalExpressionPtr lv = var->getExp<LvalExpression>();
  if (!lv) {
    throw FatalErrorException("Can't use function return/method value in write "
                              "context");
  }
  if (ref) {
    out->exp() = NEW_EXP(AssignmentRef, lv, expr->exp());
  } else {
    out->exp() = NEW_EXP(AssignmentOp, '=', lv, expr->exp());
  }
}

void Parser::onAssignNew(Token &out, Token &var, Token &name, Token &args) {
  out.reset();
  LvalExpressionPtr lv = var->getExp<LvalExpression>();
  if (!lv) {
    throw FatalErrorException("Can't use function/method return value in write "
                              "context");
  }
  ExpressionPtr exp;
  exp = NEW_EXP(NewObject, name->name(), args->exprs());
  out->exp() = NEW_EXP(AssignmentOp, '=', lv, exp);
}

void Parser::onNewObject(Token &out, Token &name, Token &args) {
  out.reset();
  out->exp() = NEW_EXP(NewObject, name->name(), args->exprs());
}

void Parser::onUnaryOpExp(Token &out, Token &operand, int op, bool front) {
  out.reset();
  switch (op) {
  case T_INCLUDE:
  case T_INCLUDE_ONCE:
  case T_REQUIRE:
  case T_REQUIRE_ONCE:
    {
      IncludeExpressionPtr exp = NEW_EXP(Include, op == T_INCLUDE ||
                                         op == T_INCLUDE_ONCE,
                                         op == T_INCLUDE_ONCE ||
                                         op == T_REQUIRE_ONCE,
                                         operand->exp());
      out->exp() = exp;
    }
    break;
  case T_INC:
  case T_DEC:
    {
      LvalExpressionPtr lv = operand->getExp<LvalExpression>();
      if (!lv) {
        throw FatalErrorException("Can't use function/method return value in write "
                                  "context");
      }
      out->exp() =  NEW_EXP(IncOp,operand->exp(), op == T_INC, front);
      break;
    }
  case T_ISSET:
    {
      out->exp() = NEW_EXP(Isset, operand->exprs());
      break;
    }
  default:
    {
      UnaryOpExpressionPtr exp = NEW_EXP(UnaryOp, operand->exp(), op,
                                         front);
      out->exp() = exp;
    }
    break;
  }
}

void Parser::onBinaryOpExp(Token &out, Token &operand1, Token &operand2,
                           int op) {
  out.reset();
  switch (op) {
  case T_PLUS_EQUAL:
  case T_MINUS_EQUAL:
  case T_MUL_EQUAL:
  case T_DIV_EQUAL:
  case T_CONCAT_EQUAL:
  case T_MOD_EQUAL:
  case T_AND_EQUAL:
  case T_OR_EQUAL:
  case T_XOR_EQUAL:
  case T_SL_EQUAL:
  case T_SR_EQUAL:
    {
      LvalExpressionPtr lv = operand1->getExp<LvalExpression>();
      if (!lv) {
        throw FatalErrorException("Can't use function/method return value in write "
                                  "context");
      }
      out->exp() = NEW_EXP(AssignmentOp, op, lv, operand2->exp());
      break;
    }
  case T_INSTANCEOF:
    {
      out->exp() = NEW_EXP(InstanceOf, operand1->exp(), operand2->name());
      break;
    }
  default:
    out->exp() = NEW_EXP(BinaryOp, operand1->exp(), op, operand2->exp());
  }
}

void Parser::onQOp(Token &out, Token &exprCond, Token &expYes, Token &expNo) {
  out.reset();
  out->exp() = NEW_EXP(QOp, exprCond->exp(), expYes->exp(), expNo->exp());
}

void Parser::onArray(Token &out, Token &pairs) {
  out.reset();
  out->exp() = NEW_EXP(Array, pairs->arrayPairs());
}

void Parser::onArrayPair(Token &out, Token *pairs, Token *name, Token &value,
                         bool ref) {
  out.reset();
  if (pairs) {
    out = *pairs;
  }
  ArrayPairPtr ap;
  if (ref) {
    // Parser guarantees lv
    LvalExpressionPtr lv = value->getExp<LvalExpression>();
    ASSERT(lv);
    if (name) {
      ap = ArrayPairPtr(new ArrayPairRef(this, (*name)->exp(), lv));
    } else {
      ap = ArrayPairPtr(new ArrayPairRef(this, lv));
    }
  } else {
    if (name) {
      ap = ArrayPairPtr(new ArrayPairVal(this, (*name)->exp(),
                                         value->exp()));
    } else {
      ap = ArrayPairPtr(new ArrayPairVal(this, value->exp()));
    }
  }

  out->arrayPairs().push_back(ap);
}

void Parser::onClassConst(Token &out, Token &className, Token &name) {
  out.reset();
  string cn = className.getText();
  if (haveClass()) {
    if (cn == "self") {
      cn = peekClass()->name();
    } else if (cn == "parent") {
      cn = peekClass()->parent();
    }
  }
  out->exp() = NEW_EXP(ClassConstant, cn, name.getText());
}

///////////////////////////////////////////////////////////////////////////////
// function/method declaration

void Parser::onFunctionStart(Token &name) {
  FunctionStatementPtr func = NEW_STMT(Function, name.getText(),
                                       m_scanner.getDocComment());
  m_hasCallToGetArgs = false;
  pushFunc(func);
}

void Parser::onFunction(Token &out, Token &ref, Token &name, Token &params,
                        Token &stmt) {
  out.reset();
  FunctionStatementPtr func = peekFunc();
  ASSERT(func);
  func->init(ref.num,
             params->params(),
             stmt->getStmtList(),
             m_hasCallToGetArgs);
  out->stmt() = func;
  popFunc();
}

void Parser::onParam(Token &out, Token *params, Token &type, Token &var,
                     bool ref, Token *defValue) {
  out.reset();
  if (params) {
    out = *params;
  }
  int idx = -1;
  if (haveFunc()) {
    FunctionStatementPtr func = peekFunc();
    idx = func->declareVariable(var.getText());
  }
  ParameterPtr p(new Parameter(this, type.getText(), var.getText(),
                               idx, ref, defValue ? (*defValue)->exp()
                               : ExpressionPtr(), out->params().size() + 1));
  out->params().push_back(p);
}

void Parser::onClassStart(Token &name, Token *parent) {
  pushClass(NEW_STMT(Class, name.getText(), parent ? parent->getText() : "",
                     m_scanner.getDocComment()));
}

void Parser::onClass(Token &out, Token &type, Token &bases) {
  out.reset();
  ClassStatementPtr cs = peekClass();
  popClass();
  int mod = 0;
  if (type.num == T_ABSTRACT) mod = ClassStatement::Abstract;
  else if (type.num == T_FINAL) mod = ClassStatement::Final;
  cs->setModifiers(mod);
  std::vector<String> &interfaceNames = bases->strings();
  cs->addBases(interfaceNames);
  cs->finish();
  out->stmt() = cs;
}

void Parser::onInterface(Token &out, Token &bases) {
  out.reset();
  ClassStatementPtr cs = peekClass();
  popClass();
  cs->setModifiers(ClassStatement::Interface);
  std::vector<String> &interfaceNames = bases->strings();
  cs->addBases(interfaceNames);
  out->stmt() = cs;
}

void Parser::onInterfaceName(Token &out, Token *names, Token &name) {
  out.reset();
  if (names) {
    out = *names;
  }
  out->strings().push_back(name.getText());
}

void Parser::onClassVariableStart(Token &mods) {
  ClassStatementPtr cs = peekClass();
  cs->setModifiers(mods.num);
}

void Parser::onClassVariable(Token &name, Token *val) {
  ClassStatementPtr cs = peekClass();
  int mod = cs->getModifiers();
  ExpressionPtr v;
  if (val) {
    v = (*val)->exp();
  }
  cs->addVariable(ClassVariablePtr(
    new ClassVariable(this, name.getText(), mod, v,
                      m_scanner.getDocComment())));
}

void Parser::onClassConstant(Token &name, Token &val) {
  ClassStatementPtr cs = peekClass();
  cs->addConstant(name.getText(), val->exp());
}

void Parser::onMethodStart(Token &name, Token &modifiers) {
  ClassStatementPtr cs = peekClass();
  FunctionStatementPtr func = NEW_STMT(Method, name.getText(), cs.get(),
                                       modifiers.num,
                                       m_scanner.getDocComment());
  m_hasCallToGetArgs = false;
  pushFunc(func);
}

void Parser::onMethod(Token &modifiers, Token &ref, Token &name,
                      Token &params, Token &stmt) {
  ClassStatementPtr cs = peekClass();
  MethodStatementPtr ms = peekFunc()->cast<MethodStatement>();
  ASSERT(ms);
  popFunc();
  StatementListStatementPtr stmts = stmt->getStmtList();
  ms->init(ref.num, params->params(), stmts, m_hasCallToGetArgs);
  cs->addMethod(ms);
}

void Parser::onMemberModifier(Token &out, Token *modifiers, Token &modifier) {
  out.reset();
  if (modifiers) {
    out.num = modifiers->num;
  }
  switch (modifier.num) {
  case T_PUBLIC:    out.num |= ClassStatement::Public;    break;
  case T_PROTECTED: out.num |= ClassStatement::Protected; break;
  case T_PRIVATE:   out.num |= ClassStatement::Private;   break;
  case T_STATIC:    out.num |= ClassStatement::Static;    break;
  case T_ABSTRACT:  out.num |= ClassStatement::Abstract;  break;
  case T_FINAL:     out.num |= ClassStatement::Final;     break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// statements

void Parser::saveParseTree(Token &tree) {
  StatementListStatementPtr s = tree->getStmtList();
  // Reorder so that classes and funcs are first.
  if (s) {
    std::vector<StatementPtr> scopes;
    std::vector<StatementPtr> rest;
    const std::vector<StatementPtr> &svec = s->stmts();
    for (std::vector<StatementPtr>::const_iterator it = svec.begin();
         it != svec.end(); ++it) {
      if ((*it)->cast<ClassStatement>() || (*it)->cast<FunctionStatement>()) {
        scopes.push_back(*it);
      } else {
        rest.push_back(*it);
      }
    }
    rest.insert(rest.begin(), scopes.begin(), scopes.end());
    m_tree = StatementPtr(new StatementListStatement(this, rest));
  } else {
    m_tree = tree->stmt();
  }
  if (!m_tree) {
    m_tree = NEW_STMT0(StatementList);
  }
}

void Parser::addStatement(Token &out, Token &stmts, Token &new_stmt) {
  out.reset();
  if (!stmts->stmt()) {
    out->stmt() = NEW_STMT0(StatementList);
  } else {
    out->stmt() = stmts->stmt();
  }
  ASSERT(out->getStmtList());
  if (new_stmt->stmt()) {
    out->getStmtList()->add(new_stmt->stmt());
  }
}

void Parser::finishStatement(Token &out, Token &stmts) {
  out.reset();
  out->stmt() = stmts->stmt();
}

void Parser::onBlock(Token &out, Token &stmts) {
  out.reset();
  out->stmt() = stmts->stmt();
}

void Parser::onIf(Token &out, Token &cond, Token &stmt, Token &elseifs,
                  Token &elseStmt) {
  out.reset();
  std::vector<IfBranchPtr> &branches = elseifs->ifBranches();
  branches.insert(branches.begin(),
                  IfBranchPtr(new IfBranch(this, cond->exp(),
                                           stmt->stmt())));
  out->stmt() = NEW_STMT(If, branches, elseStmt->stmt());
}

void Parser::onElseIf(Token &out, Token &elseifs, Token &cond, Token &stmt) {
  out.reset();
  out = elseifs;
  out->ifBranches().push_back(IfBranchPtr(new IfBranch(this,
                                                       cond->exp(),
                                                       stmt->stmt())));
}

void Parser::onWhile(Token &out, Token &cond, Token &stmt) {
  out.reset();
  out->stmt() = NEW_STMT(While, cond->exp(), stmt->stmt());
}

void Parser::onDo(Token &out, Token &stmt, Token &cond) {
  out.reset();
  out->stmt() = NEW_STMT(DoWhile, stmt->stmt(), cond->exp());
}

void Parser::onFor(Token &out, Token &expr1, Token &expr2, Token &expr3,
                   Token &stmt) {
  out.reset();
  out->stmt() = NEW_STMT(For, expr1->exprs(), expr2->exprs(),
                         expr3->exprs(), stmt->stmt());
}

void Parser::onSwitch(Token &out, Token &expr, Token &cases) {
  out.reset();
  out->stmt() = NEW_STMT(Switch, expr->exp(),
                       cases->cases());
}

void Parser::onCase(Token &out, Token &cases, Token *cond, Token &stmt) {
  out.reset();
  out = cases;
  out->cases().push_back(NEW_STMT(Case, cond ?
                                     (*cond)->exp() : ExpressionPtr(),
                                     stmt->stmt()));
}

void Parser::onBreak(Token &out, Token *expr) {
  out.reset();
  out->stmt() = NEW_STMT(Break,
                         expr ? (*expr)->exp() : ExpressionPtr(), true);
}

void Parser::onContinue(Token &out, Token *expr) {
  out.reset();
  out->stmt() = NEW_STMT(Break,
                         expr ? (*expr)->exp() : ExpressionPtr(), false);
}

void Parser::onReturn(Token &out, Token *expr) {
  out.reset();
  out->stmt() = NEW_STMT(Return, expr ?
                         (*expr)->exp() : ExpressionPtr());
}

void Parser::onGlobal(Token &out, Token &expr) {
  out.reset();
  out->stmt() = NEW_STMT(Global, expr->names());
}

void Parser::onGlobalVar(Token &out, Token *exprs, Token &expr) {
  out.reset();
  if (exprs) {
    out = *exprs;
  }
  std::vector<NamePtr> &names = out->names();
  switch (expr.num) {
  case 0:
    names.push_back(Name::fromString(this, expr.getText()));
    break;
  case 1:
    names.push_back(Name::fromExp(this, expr->exp()));
    break;
  default:
    ASSERT(false);
  }
}

void Parser::onStatic(Token &out, Token &expr) {
  out.reset();
  StaticStatementPtr st = NEW_STMT(Static, expr->staticVars());
  out->stmt() = st;
  // I really hate this static nonsense.
  if (haveFunc()) {
    peekFunc()->declareStaticStatement(st);
  } else {
    m_staticStatements.push_back(st);
  }
}

void Parser::onEcho(Token &out, Token &expr, bool html) {
  out.reset();
  if (html) {
    if (line1() == 2 && char1() == 0 && expr.getText().data()[0] == '#') {
      // skipping linux interpreter declaration
      out->stmt() = NEW_STMT0(StatementList);
    } else {
      ExpressionPtr exp = NEW_EXP(Scalar, T_STRING, expr.getText());
      std::vector<ExpressionPtr> expList;
      expList.push_back(exp);
      out->stmt() = NEW_STMT(Echo, expList);
    }
  } else {
    out->stmt() = NEW_STMT(Echo, expr->exprs());
  }
}

void Parser::onUnset(Token &out, Token &expr) {
  out.reset();
  out->stmt() = NEW_STMT(Unset, expr->exprs());
}

void Parser::onExpStatement(Token &out, Token &expr) {
  out.reset();
  out->stmt() = NEW_STMT(Expr, expr->exp());
}

void Parser::onForEach(Token &out, Token &arr, Token &name, Token &value,
                       Token &stmt) {
  out.reset();
  if (value->exp()) {
    if (value.num == 0) {
      out->stmt() = NEW_STMT(ForEach, arr->exp(),
                             name->getExp<LvalExpression>(),
                             value->getExp<LvalExpression>(), stmt->stmt());
    } else {
      out->stmt() = NEW_STMT(StrongForEach, arr->exp(),
                             name->getExp<LvalExpression>(),
                             value->getExp<LvalExpression>(), stmt->stmt());
    }
  } else {
    if (name.num == 0) {
      out->stmt() = NEW_STMT(ForEach, arr->exp(), LvalExpressionPtr(),
                           name->getExp<LvalExpression>(), stmt->stmt());
    } else {
      out->stmt() = NEW_STMT(StrongForEach, arr->exp(), LvalExpressionPtr(),
                           name->getExp<LvalExpression>(), stmt->stmt());
    }
  }
}

void Parser::onTry(Token &out, Token &tryStmt, Token &className, Token &var,
                   Token &catchStmt, Token &catches) {
  out.reset();
  std::vector<CatchBlockPtr> &cs = catches->catches();
  cs.insert(cs.begin(),
            CatchBlockPtr(new CatchBlock(this, className.getText(),
                                         var.getText(), catchStmt->stmt())));
  out->stmt() = NEW_STMT(Try, tryStmt->stmt(), cs);
}

void Parser::onCatch(Token &out, Token &catches, Token &className, Token &var,
                     Token &stmt) {
  out.reset();
  out = catches;
  out->catches().push_back(CatchBlockPtr(new CatchBlock(this,
                                                        className.getText(),
                                                        var.getText(),
                                                        stmt->stmt())));
}

void Parser::onThrow(Token &out, Token &expr) {
  out.reset();
  out->stmt() = NEW_STMT(Throw, expr->exp());
}

  /*
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
  */

void Parser::onHphpNoteExpr(Token &out, Token &note, Token &expr) {
  out.reset();
  //addHphpNote(expr->exp(), note.getText());
  out->exp() = expr->exp();
}
void Parser::onHphpNoteStatement(Token &out, Token &note, Token &stmt) {
  out.reset();
  //addHphpNote(stmt->stmt(), note.getText());
  out->stmt() = stmt->stmt();
}

void Parser::addHphpDeclare(Token &declare) {

}

void Parser::addHphpSuppressError(Token &error) {

}
