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

#include <runtime/eval/parser/parser.h>
#include <util/parser/hphp.tab.hpp>

#include <runtime/eval/ast/array_element_expression.h>
#include <runtime/eval/ast/array_expression.h>
#include <runtime/eval/ast/assignment_op_expression.h>
#include <runtime/eval/ast/assignment_ref_expression.h>
#include <runtime/eval/ast/binary_op_expression.h>
#include <runtime/eval/ast/class_constant_expression.h>
#include <runtime/eval/ast/constant_expression.h>
#include <runtime/eval/ast/encaps_list_expression.h>
#include <runtime/eval/ast/inc_op_expression.h>
#include <runtime/eval/ast/include_expression.h>
#include <runtime/eval/ast/instanceof_expression.h>
#include <runtime/eval/ast/isset_expression.h>
#include <runtime/eval/ast/list_assignment_expression.h>
#include <runtime/eval/ast/new_object_expression.h>
#include <runtime/eval/ast/object_method_expression.h>
#include <runtime/eval/ast/object_property_expression.h>
#include <runtime/eval/ast/qop_expression.h>
#include <runtime/eval/ast/ref_param_expression.h>
#include <runtime/eval/ast/scalar_expression.h>
#include <runtime/eval/ast/simple_function_call_expression.h>
#include <runtime/eval/ast/static_method_expression.h>
#include <runtime/eval/ast/variable_expression.h>
#include <runtime/eval/ast/static_member_expression.h>
#include <runtime/eval/ast/this_expression.h>
#include <runtime/eval/ast/unary_op_expression.h>

#include <runtime/eval/ast/break_statement.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/ast/do_while_statement.h>
#include <runtime/eval/ast/echo_statement.h>
#include <runtime/eval/ast/expr_statement.h>
#include <runtime/eval/ast/for_statement.h>
#include <runtime/eval/ast/foreach_statement.h>
#include <runtime/eval/ast/function_statement.h>
#include <runtime/eval/ast/global_statement.h>
#include <runtime/eval/ast/if_statement.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/return_statement.h>
#include <runtime/eval/ast/statement_list_statement.h>
#include <runtime/eval/ast/static_statement.h>
#include <runtime/eval/ast/strong_foreach_statement.h>
#include <runtime/eval/ast/switch_statement.h>
#include <runtime/eval/ast/throw_statement.h>
#include <runtime/eval/ast/try_statement.h>
#include <runtime/eval/ast/unset_statement.h>
#include <runtime/eval/ast/while_statement.h>
#include <runtime/eval/ast/name.h>

#include <util/preprocess.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/util/string_buffer.h>

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

void Token::reset() {
  m_exp.reset();
  m_stmt.reset();
  m_name.reset();
  m_mode = None;
  if (m_data) {
    m_data->dec();
    m_data = NULL;
  }
  ScannerToken::reset();
}

StatementListStatementPtr Token::getStmtList() const {
  return getStmt<StatementListStatement>();
}

ExpressionPtr &Token::exp() {
  if (m_mode == None) {
    m_mode = SingleExpression;
  }
  ASSERT(m_mode == SingleExpression);
  return m_exp;
}

StatementPtr &Token::stmt() {
  if (m_mode == None) {
    m_mode = SingleStatement;
  }
  ASSERT(m_mode == SingleStatement);
  return m_stmt;
}

NamePtr &Token::name() {
  if (m_mode == None) {
    m_mode = SingleName;
  }
  ASSERT(m_mode == SingleName);
  return m_name;
}

#define GETTER(name, type, data)                        \
  std::vector<type> &Token::data() {                    \
    if (m_mode == None) {                               \
      ASSERT(m_data == NULL);                           \
      m_mode = name;                                    \
      m_data = new CountableVector<type>();             \
    }                                                   \
    ASSERT(m_mode == name);                             \
    return ((CountableVector<type>*)m_data)->m_vec;     \
  }

GETTER(IfBranch,       IfBranchPtr,       ifBranches )
GETTER(Expression,     ExpressionPtr,     exprs      )
GETTER(CaseStatement,  CaseStatementPtr,  cases      )
GETTER(ListElement,    ListElementPtr,    listElems  )
GETTER(ArrayPair,      ArrayPairPtr,      arrayPairs )
GETTER(CatchBlock,     CatchBlockPtr,     catches    )
GETTER(Parameter,      ParameterPtr,      params     )
GETTER(Name,           NamePtr,           names      )
GETTER(StaticVariable, StaticVariablePtr, staticVars )
GETTER(Strings,        String,            strings    )

void Token::operator=(Token &other) {
  ASSERT(&other != this);

  ScannerToken::operator=(other);
  m_exp  = other.m_exp;
  m_stmt = other.m_stmt;
  m_name = other.m_name;
  if (m_data) {
    m_data->dec();
  }
  m_mode = other.m_mode;
  m_data = other.m_data;
  if (m_data) {
    m_data->inc();
  }
}

///////////////////////////////////////////////////////////////////////////////
// statics

StatementPtr Parser::ParseString(const char *input,
                                 vector<StaticStatementPtr> &statics) {
  ASSERT(input);
  int len = strlen(input);
  Scanner scanner(input, len, RuntimeOption::ScannerType);
  Parser parser(scanner, "eval()'d code", statics);
  if (parser.parse()) {
    return parser.getTree();
  }
  raise_error("Error parsing %s: %s", input, parser.getMessage().c_str());
  return StatementPtr();
}

StatementPtr Parser::ParseFile(const char *fileName,
                               vector<StaticStatementPtr> &statics) {
  ASSERT(fileName);
  try {
    Scanner scanner(fileName, RuntimeOption::ScannerType);
    Parser parser(scanner, fileName, statics);
    if (parser.parse()) {
      return parser.getTree();
    }
    raise_error("Error parsing %s: %s", fileName, parser.getMessage().c_str());
  } catch (FileOpenException &e) {
    // ignore
  }
  return StatementPtr();
}

///////////////////////////////////////////////////////////////////////////////

Parser::Parser(Scanner &scanner, const char *fileName,
               vector<StaticStatementPtr> &statics)
    : ParserBase(scanner, fileName), m_staticStatements(statics) {
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

void Parser::pushClass(ClassStatementPtr cl) { m_classes.push(cl); }
bool Parser::haveClass() const { return !m_classes.empty(); }
ClassStatementPtr Parser::peekClass() const { return m_classes.top(); }
void Parser::popClass() { m_classes.pop(); }

void Parser::pushFunc(FunctionStatementPtr fs) { m_funcs.push(fs); }
bool Parser::haveFunc() const { return !m_funcs.empty(); }
FunctionStatementPtr Parser::peekFunc() const { return m_funcs.top(); }
void Parser::popFunc() { m_funcs.pop(); }

///////////////////////////////////////////////////////////////////////////////
// names

void Parser::onName(Token &out, Token &name, Parser::NameKind kind) {
  out.reset();
  if (kind == StringName || kind == VarName) {
    out->name() = Name::fromString(this, name.text());
  } else if (kind == ExprName) {
    out->name() = Name::fromExp(this, name->exp());
  } else if (kind == StaticClassExprName) {
    out->name() = Name::fromStaticClassExp(this, name->exp());
  } else if (kind == StaticName) {
    out->name() = Name::LateStatic(this);
  }
}

///////////////////////////////////////////////////////////////////////////////
// variables

void Parser::onStaticVariable(Token &out, Token *exprs, Token &var,
                              Token *value) {
  if (exprs) {
    out = *exprs;
  } else {
    out.reset();
  }
  ExpressionPtr exp;
  if (value) {
    exp = (*value)->exp();
  }
  out->staticVars().
    push_back(StaticVariablePtr(new StaticVariable(this, var.text(), exp)));
}

void Parser::onSimpleVariable(Token &out, Token &var) {
  out.reset();
  if (var.text() == "this" && haveClass()) {
    out->exp() = NEW_EXP0(This);
  } else {
    int idx = -1;
    if (haveFunc()) {
      idx = peekFunc()->declareVariable(var.text());
    }
    out->exp() = NEW_EXP(Variable, Name::fromString(this, var.text()), idx);
  }
}

void Parser::onDynamicVariable(Token &out, Token &expr, bool encap) {
  out.reset();
  out->exp() = getDynamicVariable(expr->exp(), encap);
}

void Parser::onIndirectRef(Token &out, Token &refCount, Token &var) {
  out.reset();
  out->exp() = var->exp();
  for (int i = 0; i < refCount.num(); i++) {
    out->exp() = createDynamicVariable(out->exp());
  }
}

void Parser::onStaticMember(Token &out, Token &className, Token &name) {
  out.reset();
  ArrayElementExpressionPtr arr = name->getExp<ArrayElementExpression>();
  NamePtr cn = procStaticClassName(className, false);
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
    //var->exp() = NEW_EXP(ConstantExpression, var.text());
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
  if (params) {
    out = *params;
  } else {
    out.reset();
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
    const string &s = name.text();
    n = Name::fromString(this, s);
    if((s == "func_num_args") ||
       (s == "func_get_args") ||
       (s == "func_get_arg")) {
      m_hasCallToGetArgs = true;
    }
  }
  if (className) {
    NamePtr cn = procStaticClassName(*className, false);
    out->exp() = NEW_EXP(StaticMethod, cn, n, params->exprs());
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
  if (params.num() == 1) {
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
  if (prop->getMode() == Token::SingleName) {
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
  out = list;
  ExpressionPtr exp;
  if (type == -1) {
    exp = expr->exp();
  } else {
    ScalarExpressionPtr scalar = NEW_EXP(Scalar, T_STRING, expr.text());
    exp = scalar;
  }
  out->exprs().push_back(exp);
}

void Parser::encapRefDim(Token &out, Token &var, Token &offset) {
  out.reset();
  ExpressionPtr dim;
  switch (offset.num()) {
  case T_STRING:
    dim = NEW_EXP(Scalar, T_STRING, offset.text());
    break;
  case T_NUM_STRING:
    dim = NEW_EXP(Scalar, T_NUM_STRING, offset.text());
    break;
  case T_VARIABLE:
    dim = NEW_EXP(Variable, Name::fromString(this, offset.text()));
    break;
  default:
    ASSERT(false);
  }

  LvalExpressionPtr arr =
    NEW_EXP(Variable, Name::fromString(this, var.text()));
  out->exp() = NEW_EXP(ArrayElement, arr, dim);
}

void Parser::encapObjProp(Token &out, Token &var, Token &name) {
  out.reset();
  ExpressionPtr obj;
  if (var.text() == "this") {
    obj = NEW_EXP0(This);
  } else {
    obj = NEW_EXP(Variable, Name::fromString(this, var.text()));
  }
  out->exp() = NEW_EXP(ObjectProperty, obj,
                       Name::fromString(this, name.text()));
}

void Parser::encapArray(Token &out, Token &var, Token &expr) {
  out.reset();
  LvalExpressionPtr arr =
    NEW_EXP(Variable, Name::fromString(this, var.text()));
  out->exp() = NEW_EXP(ArrayElement, arr, expr->exp());
}

///////////////////////////////////////////////////////////////////////////////
// expressions

void Parser::onConstant(Token &out, Token &constant) {
  out.reset();
  string lower = Util::toLower(constant.text());
  if (lower == "true") {
    out->exp() = NEW_EXP(Scalar, true);
  } else if (lower == "false") {
    out->exp() = NEW_EXP(Scalar, false);
  } else if (lower == "null") {
    out->exp() = NEW_EXP0(Scalar);
  } else {
    out->exp() = NEW_EXP(Constant, constant.text());
  }
}

void Parser::onScalar(Token &out, int type, Token &scalar) {
  out.reset();
  ScalarExpressionPtr exp;
  string stext = scalar.text();
  int subtype = 0;
  switch (type) {
  case T_CLASS_C:
    subtype = type;
    type = T_STRING;
    stext = haveClass() ? peekClass()->name() : "";
    break;
  case T_METHOD_C:
    subtype = type;
    type = T_STRING;
    if (haveClass() && haveFunc()) {
      stext = peekClass()->name() + "::" + peekFunc()->name();
      break;
    }
    // Fall through
  case T_FUNC_C:
    if (type == T_FUNC_C) subtype = T_FUNC_C;
    type = T_STRING;
    stext = haveFunc() ? peekFunc()->name() : "";
  }
  switch (type) {
  case T_STRING:
  case T_LNUMBER:
  case T_DNUMBER:
    exp = NEW_EXP(Scalar, type, stext, subtype);
    break;
  case T_CONSTANT_ENCAPSED_STRING:
    exp = NEW_EXP(Scalar, T_STRING, stext);
    break;
  case T_LINE: {
    exp = NEW_EXP(Scalar, T_LNUMBER,
                  lexical_cast<string>(line1()), T_LINE);
    break;
  }
  case T_FILE: {
    exp = NEW_EXP(Scalar, T_STRING, file(), T_FILE);
    break;
  }
  default:
    ASSERT(false);
  }
  out->exp() = exp;
}

void Parser::onExprListElem(Token &out, Token *exprs, Token &expr) {
  if (exprs) {
    out = *exprs;
  } else {
    out.reset();
  }
  out->exprs().push_back(expr->exp());
}

void Parser::onListAssignment(Token &out, Token &vars, Token *expr) {
  out.reset();
  ListElementPtr le(new SubListElement(this, vars->listElems()));
  out->exp() = NEW_EXP(ListAssignment, le, (*expr)->exp());
}

void Parser::onAListVar(Token &out, Token *list, Token *var) {
  if (list) {
    out = *list;
  } else {
    out.reset();
  }
  LvalExpressionPtr lv;
  if (var) {
    lv = (*var)->getExp<LvalExpression>();
  }
  ListElementPtr le(new LvalListElement(this, lv));
  out->listElems().push_back(le);
}
void Parser::onAListSub(Token &out, Token *list, Token &sublist) {
  if (list) {
    out = *list;
  } else {
    out.reset();
  }
  ListElementPtr le(new SubListElement(this, sublist->listElems()));
  out->listElems().push_back(le);
}

class InvalidLvalException : public FatalErrorException {
public:
  InvalidLvalException()
      : FatalErrorException("Can't use function return/method value "
                            "in write context") {}
};

void Parser::onAssign(Token &out, Token &var, Token &expr, bool ref) {
  out.reset();
  LvalExpressionPtr lv = var->getExp<LvalExpression>();
  if (!lv) {
    throw InvalidLvalException();
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
    throw InvalidLvalException();
  }
  ExpressionPtr exp;
  exp = NEW_EXP(NewObject, name->name(), args->exprs());
  out->exp() = NEW_EXP(AssignmentOp, '=', lv, exp);
}

void Parser::onNewObject(Token &out, Token &name, Token &args) {
  out.reset();
  NamePtr n = procStaticClassName(name, false);
  out->exp() = NEW_EXP(NewObject, n, args->exprs());
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
        throw InvalidLvalException();
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
        throw InvalidLvalException();
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
  if (pairs) {
    out = *pairs;
  } else {
    out.reset();
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

void Parser::onClassConst(Token &out, Token &className, Token &name,
    bool text) {
  out.reset();
  NamePtr cn = procStaticClassName(className, text);
  out->exp() = NEW_EXP(ClassConstant, cn, name.text());
}

///////////////////////////////////////////////////////////////////////////////
// function/method declaration

void Parser::onFunctionStart(Token &name) {
  FunctionStatementPtr func = NEW_STMT(Function, name.text(),
                                       m_scanner.detachDocComment());
  m_hasCallToGetArgs = false;
  pushFunc(func);
}

void Parser::onFunction(Token &out, Token &ref, Token &name, Token &params,
                        Token &stmt) {
  out.reset();
  FunctionStatementPtr func = peekFunc();
  ASSERT(func);
  StatementListStatementPtr body = stmt->getStmtList();
  func->init(this, ref.num(), params->params(), body, m_hasCallToGetArgs);
  out->stmt() = func;
  popFunc();
}

void Parser::onParam(Token &out, Token *params, Token &type, Token &var,
                     bool ref, Token *defValue) {
  if (params) {
    out = *params;
  } else {
    out.reset();
  }
  int idx = -1;
  if (haveFunc()) {
    FunctionStatementPtr func = peekFunc();
    idx = func->declareVariable(var.text());
  }
  ParameterPtr p(new Parameter(this, type.text(), var.text(),
                               idx, ref, defValue ? (*defValue)->exp()
                               : ExpressionPtr(), out->params().size() + 1));
  out->params().push_back(p);
}

void Parser::onClassStart(int type, Token &name, Token *parent) {
  ClassStatementPtr cs = NEW_STMT(Class, name.text(),
      parent ? parent->text() : "", m_scanner.detachDocComment());
  pushClass(cs);
  int mod = 0;
  if (type == T_ABSTRACT) mod = ClassStatement::Abstract;
  else if (type == T_FINAL) mod = ClassStatement::Final;
  else if (type == T_INTERFACE) mod = ClassStatement::Interface;
  cs->setModifiers(mod);
}

void Parser::onClass(Token &out, Token &type, Token &name, Token &base,
                     Token &baseInterface, Token &stmt) {
  out.reset();
  ClassStatementPtr cs = peekClass();
  popClass();
  std::vector<String> &interfaceNames = baseInterface->strings();
  cs->addBases(interfaceNames);
  cs->finish();
  out->stmt() = cs;
}

void Parser::onInterface(Token &out, Token &name, Token &base, Token &stmt) {
  out.reset();
  ClassStatementPtr cs = peekClass();
  popClass();
  std::vector<String> &interfaceNames = base->strings();
  cs->addBases(interfaceNames);
  out->stmt() = cs;
}

void Parser::onInterfaceName(Token &out, Token *names, Token &name) {
  if (names) {
    out = *names;
  } else {
    out.reset();
  }
  out->strings().push_back(name.text());
}

void Parser::onClassVariableModifer(Token &mod) {
  m_classVarMods = mod.num();
}

void Parser::onClassVariable(Token &out, Token *exprs, Token &name,
                             Token *val) {
  ClassStatementPtr cs = peekClass();
  ExpressionPtr v;
  if (val) {
    v = (*val)->exp();
  }
  cs->addVariable(ClassVariablePtr(
    new ClassVariable(this, name.text(), m_classVarMods, v,
                      m_scanner.detachDocComment(), cs.get())));
}

void Parser::onClassConstant(Token &out, Token *exprs, Token &name,
                             Token &val) {
  ClassStatementPtr cs = peekClass();
  cs->addConstant(name.text(), val->exp());
}

void Parser::onMethodStart(Token &name, Token &modifiers) {
  ClassStatementPtr cs = peekClass();
  FunctionStatementPtr func = NEW_STMT(Method, name.text(), cs.get(),
                                       modifiers.num(),
                                       m_scanner.detachDocComment());
  m_hasCallToGetArgs = false;
  pushFunc(func);
}

void Parser::onMethod(Token &out, Token &modifiers, Token &ref, Token &name,
                      Token &params, Token &stmt) {
  ClassStatementPtr cs = peekClass();
  MethodStatementPtr ms = peekFunc()->cast<MethodStatement>();
  ASSERT(ms);
  popFunc();
  StatementListStatementPtr stmts = stmt->getStmtList();
  ms->resetLoc(this);
  if (stmts) stmts->resetLoc(this);
  ms->init(this, ref.num(), params->params(), stmts, m_hasCallToGetArgs);
  cs->addMethod(ms);
}

void Parser::onMemberModifier(Token &out, Token *modifiers, Token &modifier) {
  out.reset();
  if (modifiers) {
    out.setNum(modifiers->num());
  }

  int mod = 0;
  switch (modifier.num()) {
  case T_PUBLIC:    mod = ClassStatement::Public;    break;
  case T_PROTECTED: mod = ClassStatement::Protected; break;
  case T_PRIVATE:   mod = ClassStatement::Private;   break;
  case T_STATIC:    mod = ClassStatement::Static;    break;
  case T_ABSTRACT:  mod = ClassStatement::Abstract;  break;
  case T_FINAL:     mod = ClassStatement::Final;     break;
  }

  if ((out.num() & ClassStatement::AccessMask) &&
      (mod & ClassStatement::AccessMask)) {
    raise_error("Multiple access type modifiers are not allowed "
                "in %s on line %d", file(), line1());
  }
  if ((out.num() & ClassStatement::Static) && (mod & ClassStatement::Static)) {
    raise_error("Multiple static modifiers are not allowed "
                "in %s on line %d", file(), line1());
  }
  if ((out.num() & ClassStatement::Abstract) &&
      (mod & ClassStatement::Abstract)) {
    raise_error("Multiple abstract modifiers are not allowed "
                "in %s on line %d", file(), line1());
  }
  if ((out.num() & ClassStatement::Final) && (mod & ClassStatement::Final)) {
    raise_error("Multiple final modifiers are not allowed "
                "in %s on line %d", file(), line1());
  }
  if (((out.num()|mod) & (ClassStatement::Abstract|ClassStatement::Final)) ==
      (ClassStatement::Abstract|ClassStatement::Final)) {
    raise_error("Cannot use the final modifier on an abstract class member "
                "in %s on line %d", file(), line1());
  }

  out.setNum(out.num() | mod);
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
      ClassStatementPtr cs = (*it)->cast<ClassStatement>();
      if (cs || (*it)->cast<FunctionStatement>()) {
        scopes.push_back(*it);
      } else {
        rest.push_back(*it);
      }
      if (cs) {
        rest.push_back(cs->getMarker());
      }
    }
    rest.insert(rest.begin(), scopes.begin(), scopes.end());
    m_tree = StatementPtr(new StatementListStatement(this, rest));
    // Classes that have a parent declared after them must be evaluated at
    // the marker position. I don't know why.
    hphp_const_char_map<ClassStatementPtr> seen;
    for (int i = 0; i < (int)svec.size(); ++i) {
      ClassStatementPtr cs = svec[i]->cast<ClassStatement>();
      if (cs) {
        seen[cs->name().c_str()] = cs;
        if (!cs->parent().empty()) {
          hphp_const_char_map<ClassStatementPtr>::iterator iter =
            seen.find(cs->parent().c_str());
          if (iter == seen.end() || iter->second->isDelayedDeclaration()) {
            cs->delayDeclaration();
          }
        }
      }
    }
  } else {
    m_tree = tree->stmt();
  }
  if (!m_tree) {
    m_tree = NEW_STMT0(StatementList);
  }
}

void Parser::onStatementListStart(Token &out) {
  out.reset();
  out->stmt() = NEW_STMT0(StatementList);
}

void Parser::addStatement(Token &out, Token &stmts, Token &new_stmt) {
  out.reset();
  ASSERT(stmts->stmt());
  out->stmt() = stmts->stmt();
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
  if (exprs) {
    out = *exprs;
  } else {
    out.reset();
  }
  std::vector<NamePtr> &names = out->names();
  switch (expr.num()) {
  case 0:
    names.push_back(Name::fromString(this, expr.text()));
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
    if (line1() == 2 && char1() == 0 && expr.text().data()[0] == '#') {
      // skipping linux interpreter declaration
      out->stmt() = NEW_STMT0(StatementList);
    } else {
      ExpressionPtr exp = NEW_EXP(Scalar, T_STRING, expr.text());
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
    if (value.num() == 0) {
      out->stmt() = NEW_STMT(ForEach, arr->exp(),
                             name->getExp<LvalExpression>(),
                             value->getExp<LvalExpression>(), stmt->stmt());
    } else {
      out->stmt() = NEW_STMT(StrongForEach, arr->exp(),
                             name->getExp<LvalExpression>(),
                             value->getExp<LvalExpression>(), stmt->stmt());
    }
  } else {
    if (name.num() == 0) {
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
            CatchBlockPtr(new CatchBlock(this, className.text(),
                                         var.text(), catchStmt->stmt())));
  out->stmt() = NEW_STMT(Try, tryStmt->stmt(), cs);
}

void Parser::onCatch(Token &out, Token &catches, Token &className, Token &var,
                     Token &stmt) {
  out = catches;
  out->catches().push_back(CatchBlockPtr(new CatchBlock(this,
                                                        className.text(),
                                                        var.text(),
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
  //addHphpNote(expr->exp(), note.text());
  out->exp() = expr->exp();
}
void Parser::onHphpNoteStatement(Token &out, Token &note, Token &stmt) {
  out.reset();
  //addHphpNote(stmt->stmt(), note.text());
  out->stmt() = stmt->stmt();
}

void Parser::addHphpDeclare(Token &declare) {

}

void Parser::addHphpSuppressError(Token &error) {

}

NamePtr Parser::procStaticClassName(Token &className, bool text) {
  NamePtr cname;
  if (text) {
    if (className.text() == "self") {
      cname = Name::fromString(this, peekClass()->name(), true);
    } else if (className.text() == "parent") {
      cname = Name::fromString(this, peekClass()->parent(), true);
    } else {
      cname = Name::fromString(this, className.text());
    }

  } else {
    cname = className->name();
    if (haveClass() && cname->getStatic()) {
      if (cname->getStatic() == "self") {
        cname = Name::fromString(this, peekClass()->name(), true);
      } else if (cname->getStatic() == "parent") {
        cname = Name::fromString(this, peekClass()->parent(), true);
      }
    }
  }
  return cname;
}
