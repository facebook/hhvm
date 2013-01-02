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
#include <compiler/expression/closure_expression.h>
#include <compiler/expression/user_attribute.h>

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
#include <compiler/statement/finally_statement.h>
#include <compiler/statement/throw_statement.h>
#include <compiler/statement/goto_statement.h>
#include <compiler/statement/label_statement.h>
#include <compiler/statement/use_trait_statement.h>
#include <compiler/statement/trait_prec_statement.h>
#include <compiler/statement/trait_alias_statement.h>

#include <compiler/analysis/function_scope.h>

#include <compiler/analysis/code_error.h>
#include <compiler/analysis/analysis_result.h>

#include <util/preprocess.h>
#include <util/lock.h>
#include <util/logger.h>

#include <runtime/eval/runtime/file_repository.h>

#ifdef FACEBOOK
#include <../facebook/src/compiler/fb_compiler_hooks.h>
#define RealSimpleFunctionCall FBSimpleFunctionCall
#else
#define RealSimpleFunctionCall SimpleFunctionCall
#endif

#define NEW_EXP0(cls)                                           \
  cls##Ptr(new cls(BlockScopePtr(), getLocation()))
#define NEW_EXP(cls, e...)                                      \
  cls##Ptr(new cls(BlockScopePtr(), getLocation(), ##e))
#define NEW_STMT0(cls)                                          \
  cls##Ptr(new cls(BlockScopePtr(), getLocation()))
#define NEW_STMT(cls, e...)                                     \
  cls##Ptr(new cls(BlockScopePtr(), getLocation(), ##e))

#define PARSE_ERROR(fmt, args...)  HPHP_PARSER_ERROR(fmt, this, ##args)

using namespace HPHP::Compiler;

extern void prepare_generator(Parser *_p, Token &stmt, Token &params,
                              int count);
extern void create_generator(Parser *_p, Token &out, Token &params,
                             Token &name, const std::string &closureName,
                             const char *clsname, Token *modifiers,
                             bool getArgs, Token &origGenFunc, bool isHhvm,
                             Token *attr);
extern void transform_yield(Parser *_p, Token &stmts, int index,
                            Token *expr, bool assign);
extern void transform_yield_break(Parser *_p, Token &out);
extern void transform_foreach(Parser *_p, Token &out, Token &arr, Token &name,
                              Token &value, Token &stmt, int count,
                              bool hasValue, bool byRef);

namespace HPHP {

SimpleFunctionCallPtr NewSimpleFunctionCall(
  EXPRESSION_CONSTRUCTOR_PARAMETERS,
  const std::string &name, ExpressionListPtr params,
  ExpressionPtr cls) {
  return SimpleFunctionCallPtr(
    new RealSimpleFunctionCall(
      EXPRESSION_CONSTRUCTOR_DERIVED_PARAMETER_VALUES,
      name, params, cls));
}

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////
// statics

StatementListPtr Parser::ParseString(CStrRef input, AnalysisResultPtr ar,
                                     const char *fileName /* = NULL */,
                                     bool lambdaMode /* = false */) {
  ASSERT(!input.empty());
  if (!fileName || !*fileName) fileName = "string";

  int len = input.size();
  Scanner scanner(input.data(), len, Option::ScannerType, fileName, hhvm);
  Parser parser(scanner, fileName, ar, len);
  parser.m_lambdaMode = lambdaMode;
  if (parser.parse()) {
    return parser.m_file->getStmt();
  }
  Logger::Error("Error parsing %s: %s\n%s\n", fileName,
                parser.getMessage().c_str(), input.data());
  return StatementListPtr();
}

///////////////////////////////////////////////////////////////////////////////

Parser::Parser(Scanner &scanner, const char *fileName,
               AnalysisResultPtr ar, int fileSize /* = 0 */)
    : ParserBase(scanner, fileName), m_ar(ar), m_lambdaMode(false),
      m_closureGenerator(false) {
  MD5 md5;
  if (hhvm) {
    string md5str = Eval::FileRepository::unitMd5(scanner.getMd5());
    md5 = MD5(md5str.c_str());
  }

  m_file = FileScopePtr(new FileScope(m_fileName, fileSize, md5));

  newScope();
  m_staticVars.push_back(StringToExpressionPtrVecMap());
  m_inTrait = false;

  Lock lock(m_ar->getMutex());
  m_ar->addFileScope(m_file);

  m_prependingStatements.push_back(vector<StatementPtr>());
}

bool Parser::parse() {
  try {
    if (!parseImpl()) {
      throw ParseTimeFatalException(m_fileName, line1(),
                                    "Parse error: %s",
                                    errString().c_str());
    }
  } catch (ParseTimeFatalException &e) {
    m_file->cleanupForError(m_ar, e.m_line, e.getMessage());
  }
  return true;
}

void Parser::error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  fatal(&m_loc, msg.c_str());
}

void Parser::fatal(Location *loc, const char *msg) {
  throw ParseTimeFatalException(loc->file, loc->line0,
                                "%s", msg);
}

string Parser::errString() {
  return m_error.empty() ? getMessage() : m_error;
}

bool Parser::enableXHP() {
  return Option::EnableXHP;
}

bool Parser::enableHipHopSyntax() {
  return Option::EnableHipHopSyntax;
}

bool Parser::enableFinallyStatement() {
  return Option::EnableFinallyStatement;
}

void Parser::pushComment() {
  m_comments.push_back(m_scanner.detachDocComment());
}

void Parser::pushComment(const std::string& s) {
  m_comments.push_back(s);
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
  always_assert(inner);
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
  if (m_staticVars.size()) {
    StringToExpressionPtrVecMap &m = m_staticVars.back();
    m[var->text()].push_back(out->exp);
  }
}

void Parser::onClassVariable(Token &out, Token *exprs, Token &var,
                             Token *value) {
  onVariable(out, exprs, var, value, false, m_scanner.detachDocComment());
}

void Parser::onClassConstant(Token &out, Token *exprs, Token &var,
                             Token &value) {
  onVariable(out, exprs, var, &value, true, m_scanner.detachDocComment());
}

void Parser::onVariable(Token &out, Token *exprs, Token &var, Token *value,
                        bool constant /* = false */,
                        const std::string &docComment /* = "" */) {
  ExpressionPtr expList;
  if (exprs) {
    expList = exprs->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  ExpressionPtr exp;
  if (constant) {
    exp = NEW_EXP(ConstantExpression, var->text(), docComment);
  } else {
    exp = NEW_EXP(SimpleVariable, var->text(), docComment);
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
      appendClass(cls->exp, m_ar, m_file)) {
    out->exp = name->exp;
  } else {
    StaticMemberExpressionPtr sme = NEW_EXP(StaticMemberExpression,
                                            cls->exp, name->exp);
    sme->onParse(m_ar, m_file);
    out->exp = sme;
  }
}

void Parser::onRefDim(Token &out, Token &var, Token &offset) {
  if (!var->exp) {
    var->exp = NEW_EXP(ConstantExpression, var->text());
  }
  if (!offset->exp) {
    UnaryOpExpressionPtr uop;

    if (dynamic_pointer_cast<FunctionCall>(var->exp)) {
      PARSE_ERROR("Can't use function call result as array base"
                     " in write context");
    } else if ((uop = dynamic_pointer_cast<UnaryOpExpression>(var->exp))
               && uop->getOp() == T_ARRAY) {
      PARSE_ERROR("Can't use array() as base in write context");
    }
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
                    Token *cls, bool fromCompiler) {
  ExpressionPtr clsExp;
  if (cls) {
    clsExp = cls->exp;
  }
  if (dynamic) {
    out->exp = NEW_EXP(DynamicFunctionCall, name->exp,
                       dynamic_pointer_cast<ExpressionList>(params->exp),
                       clsExp);
    ASSERT(!fromCompiler);
  } else {
    const string &s = name.text();
    if (s == "func_num_args" || s == "func_get_args" || s == "func_get_arg") {
      if (m_hasCallToGetArgs.empty()) {
        PARSE_ERROR("%s() Called from the global scope - no function context",
                    s.c_str());
      }
      m_hasCallToGetArgs.back() = true;
    }

    SimpleFunctionCallPtr call
      (new RealSimpleFunctionCall
       (BlockScopePtr(), getLocation(), name->text(),
        dynamic_pointer_cast<ExpressionList>(params->exp), clsExp));
    if (fromCompiler) {
      call->setFromCompiler();
    }
    out->exp = call;

    call->onParse(m_ar, m_file);
  }
}

///////////////////////////////////////////////////////////////////////////////
// object property and method calls

void Parser::onObjectProperty(Token &out, Token &base, Token &prop) {
  if (!prop->exp) {
    prop->exp = NEW_EXP(ScalarExpression, T_STRING, prop->text());
  }
  out->exp = NEW_EXP(ObjectPropertyExpression, base->exp, prop->exp);
}

void Parser::onObjectMethodCall(Token &out, Token &base, Token &prop,
                                Token &params) {
  if (!prop->exp) {
    prop->exp = NEW_EXP(ScalarExpression, T_STRING, prop->text());
  }
  ExpressionListPtr paramsExp;
  if (params->exp) {
    paramsExp = dynamic_pointer_cast<ExpressionList>(params->exp);
  } else {
    paramsExp = NEW_EXP0(ExpressionList);
  }
  out->exp = NEW_EXP(ObjectMethodExpression, base->exp, prop->exp, paramsExp);
}

///////////////////////////////////////////////////////////////////////////////
// encapsed expressions

void Parser::onEncapsList(Token &out, int type, Token &list) {
  out->exp = NEW_EXP(EncapsListExpression, type,
                     dynamic_pointer_cast<ExpressionList>(list->exp));
}

void Parser::addEncap(Token &out, Token *list, Token &expr, int type) {
  ExpressionListPtr expList;
  if (list && list->exp) {
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

void Parser::onConstantValue(Token &out, Token &constant) {
  ConstantExpressionPtr con = NEW_EXP(ConstantExpression, constant->text());
  con->onParse(m_ar, m_file);
  out->exp = con;
}

void Parser::onScalar(Token &out, int type, Token &scalar) {
  if (type == T_FILE || type == T_DIR) {
    onUnaryOpExp(out, scalar, type, true);
    return;
  }

  ScalarExpressionPtr exp;
  switch (type) {
    case T_METHOD_C:
      if (m_inTrait) {
        exp = NEW_EXP(ScalarExpression, type, scalar->text(),
                      m_clsName + "::" + m_funcName);
      } else {
        exp = NEW_EXP(ScalarExpression, type, scalar->text());
      }
      break;
    case T_STRING:
    case T_LNUMBER:
    case T_DNUMBER:
    case T_LINE:
    case T_FUNC_C:
    case T_CLASS_C:
      exp = NEW_EXP(ScalarExpression, type, scalar->text());
      break;
    case T_TRAIT_C:
      exp = NEW_EXP(ScalarExpression, type, scalar->text(),
                    m_inTrait ? m_clsName : "");
      break;
    case T_NS_C:
      exp = NEW_EXP(ScalarExpression, type, m_namespace);
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
  ExpressionListPtr el(dynamic_pointer_cast<ExpressionList>(vars->exp));
  for (int i = 0; i < el->getCount(); i++) {
    if (dynamic_pointer_cast<FunctionCall>((*el)[i])) {
      PARSE_ERROR("Can't use return value in write context");
    }
  }
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

void Parser::checkAssignThis(Token &var) {
  if (SimpleVariablePtr simp = dynamic_pointer_cast<SimpleVariable>(var.exp)) {
    if (simp->getName() == "this") {
      PARSE_ERROR("Cannot re-assign $this");
    }
  }
}

void Parser::onAssign(Token &out, Token &var, Token &expr, bool ref) {
  if (dynamic_pointer_cast<FunctionCall>(var->exp)) {
    PARSE_ERROR("Can't use return value in write context");
  }
  checkAssignThis(var);
  out->exp = NEW_EXP(AssignmentExpression, var->exp, expr->exp, ref);
}

void Parser::onAssignNew(Token &out, Token &var, Token &name, Token &args) {
  checkAssignThis(var);
  ExpressionPtr exp =
    NEW_EXP(NewObjectExpression, name->exp,
            dynamic_pointer_cast<ExpressionList>(args->exp));
  out->exp = NEW_EXP(AssignmentExpression, var->exp, exp, false);
}

void Parser::onNewObject(Token &out, Token &name, Token &args) {
  NewObjectExpressionPtr new_obj =
    NEW_EXP(NewObjectExpression, name->exp,
            dynamic_pointer_cast<ExpressionList>(args->exp));
  new_obj->onParse(m_ar, m_file);
  out->exp = new_obj;
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
  case T_INC:
  case T_DEC:
  case T_ISSET:
  case T_EMPTY:
  case T_UNSET:
    if (dynamic_pointer_cast<FunctionCall>(operand->exp)) {
      PARSE_ERROR("Can't use return value in write context");
    }
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
  BinaryOpExpressionPtr bop =
    NEW_EXP(BinaryOpExpression, operand1->exp, operand2->exp, op);

  if (bop->isAssignmentOp() &&
      dynamic_pointer_cast<FunctionCall>(operand1->exp)) {
    PARSE_ERROR("Can't use return value in write context");
  }

  out->exp = bop;
}

void Parser::onQOp(Token &out, Token &exprCond, Token *expYes, Token &expNo) {
  out->exp = NEW_EXP(QOpExpression, exprCond->exp,
                     expYes ? expYes->exp : ExpressionPtr(), expNo->exp);
}

void Parser::onArray(Token &out, Token &pairs, int op /* = T_ARRAY */) {
  if (op != T_ARRAY && !Option::EnableHipHopSyntax) {
    PARSE_ERROR("Typed collection is not enabled");
    return;
  }
  onUnaryOpExp(out, pairs, T_ARRAY, true);
}

void Parser::onArrayPair(Token &out, Token *pairs, Token *name, Token &value,
                         bool ref) {
  if (!value->exp) return;

  ExpressionPtr expList;
  if (pairs && pairs->exp) {
    expList = pairs->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  ExpressionPtr nameExp = name ? name->exp : ExpressionPtr();
  expList->addElement(NEW_EXP(ArrayPairExpression, nameExp, value->exp, ref));
  out->exp = expList;
}

void Parser::onEmptyCollection(Token &out) {
  out->exp = NEW_EXP0(ExpressionList);
}

void
Parser::onCollectionPair(Token &out, Token *pairs, Token *name, Token &value) {
  if (!value->exp) return;

  ExpressionPtr expList;
  if (pairs && pairs->exp) {
    expList = pairs->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  ExpressionPtr nameExp = name ? name->exp : ExpressionPtr();
  expList->addElement(NEW_EXP(ArrayPairExpression, nameExp, value->exp, false,
                              true));
  out->exp = expList;
}

void Parser::onUserAttribute(Token &out, Token *attrList, Token &name,
                             Token &value) {
  ExpressionPtr expList;
  if (attrList && attrList->exp) {
    expList = attrList->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(NEW_EXP(UserAttribute, name->text(), value->exp));
  out->exp = expList;
}

void Parser::onClassConst(Token &out, Token &cls, Token &name, bool text) {
  if (!cls->exp) {
    cls->exp = NEW_EXP(ScalarExpression, T_STRING, cls->text());
  }
  ClassConstantExpressionPtr con =
    NEW_EXP(ClassConstantExpression, cls->exp, name->text());
  con->onParse(m_ar, m_file);
  out->exp = con;
}

///////////////////////////////////////////////////////////////////////////////
// function/method declaration

void Parser::onFunctionStart(Token &name, bool doPushComment /* = true */) {
  m_file->pushAttribute();
  if (doPushComment) {
    pushComment();
  }
  newScope();
  m_funcContexts.push_back(FunctionContext());
  m_prependingStatements.push_back(vector<StatementPtr>());
  m_funcName = name.text();
  m_hasCallToGetArgs.push_back(false);
  m_staticVars.push_back(StringToExpressionPtrVecMap());
}

void Parser::onMethodStart(Token &name, Token &mods,
                           bool doPushComment /* = true */) {
  onFunctionStart(name, doPushComment);
}

void Parser::fixStaticVars() {
  StringToExpressionPtrVecMap &m = m_staticVars.back();
  for (StringToExpressionPtrVecMap::iterator it = m.begin(), end = m.end();
       it != end; ++it) {
    const ExpressionPtrVec &v = it->second;
    if (v.size() > 1) {
      ExpressionPtr last;
      for (int i = v.size(); i--; ) {
        ExpressionListPtr el(dynamic_pointer_cast<ExpressionList>(v[i]));
        for (int j = el->getCount(); j--; ) {
          ExpressionPtr s = (*el)[j];
          SimpleVariablePtr v = dynamic_pointer_cast<SimpleVariable>(
            s->is(Expression::KindOfAssignmentExpression) ?
            static_pointer_cast<AssignmentExpression>(s)->getVariable() : s);
          if (v->getName() == it->first) {
            if (!last) {
              last = s;
            } else {
              el->removeElement(j);
              el->insertElement(last->clone(), j);
            }
          }
        }
      }
    }
  }
  m_staticVars.pop_back();
}

void Parser::onFunction(Token &out, Token &ret, Token &ref, Token &name,
                        Token &params, Token &stmt, Token *attr) {
  const string &retType = ret.text();
  if (!retType.empty() && !ret.check()) {
    PARSE_ERROR("Return type hint is not supported yet");
  }
  if (!stmt->stmt) {
    stmt->stmt = NEW_STMT0(StatementList);
  }

  ExpressionListPtr old_params =
    dynamic_pointer_cast<ExpressionList>(params->exp);
  int attribute = m_file->popAttribute();
  string comment = popComment();
  LocationPtr loc = popFuncLocation();

  FunctionContext funcContext = m_funcContexts.back();
  m_funcContexts.pop_back();
  m_prependingStatements.pop_back();

  funcContext.checkFinalAssertions();

  bool hasCallToGetArgs = m_hasCallToGetArgs.back();
  m_hasCallToGetArgs.pop_back();

  fixStaticVars();

  FunctionStatementPtr func;

  if (funcContext.isGenerator) {
    AnonFuncKind fKind = name->text().empty() ?
      ContinuationFromClosure : Continuation;
    const string &closureName = getAnonFuncName(fKind);
    Token new_params;
    prepare_generator(this, stmt, new_params, funcContext.numYields);

    func = NEW_STMT(FunctionStatement, ref->num(), closureName,
                    dynamic_pointer_cast<ExpressionList>(new_params->exp),
                    dynamic_pointer_cast<StatementList>(stmt->stmt),
                    attribute, comment, ExpressionListPtr());
    out->stmt = func;
    func->getLocation()->line0 = loc->line0;
    func->getLocation()->char0 = loc->char0;
    {
      func->onParse(m_ar, m_file);
    }
    completeScope(func->getFunctionScope());
    if (func->ignored()) {
      out->stmt = NEW_STMT0(StatementList);
    } else {
      ASSERT(!m_prependingStatements.empty());
      vector<StatementPtr> &prepending = m_prependingStatements.back();
      prepending.push_back(func);

      if (name->text().empty()) m_closureGenerator = true;
      // create_generator() expects us to push the docComment back
      // onto the comment stack so that it can make sure that the
      // the MethodStatement it's building will get the docComment
      pushComment(comment);
      Token origGenFunc;
      create_generator(this, out, params, name, closureName, NULL, NULL,
                       hasCallToGetArgs, origGenFunc,
                       hhvm && Option::OutputHHBC &&
                       (!Option::WholeProgram || !Option::ParseTimeOpts),
                       attr);
      m_closureGenerator = false;
      MethodStatementPtr origStmt =
        boost::dynamic_pointer_cast<MethodStatement>(origGenFunc->stmt);
      ASSERT(origStmt);
      func->setOrigGeneratorFunc(origStmt);
      origStmt->setGeneratorFunc(func);
    }

  } else {
    string funcName = name->text();
    if (funcName.empty()) {
      funcName = getAnonFuncName(Closure);
    } else if (m_lambdaMode) {
      string f;
      f += GetAnonPrefix(CreateFunction);
      funcName = f + "_" + funcName;
    }

    ExpressionListPtr attrList;
    if (attr && attr->exp) {
      attrList = dynamic_pointer_cast<ExpressionList>(attr->exp);
    }

    func = NEW_STMT(FunctionStatement, ref->num(), funcName,
                    old_params, dynamic_pointer_cast<StatementList>(stmt->stmt),
                    attribute, comment, attrList);
    out->stmt = func;

    {
      func->onParse(m_ar, m_file);
    }
    completeScope(func->getFunctionScope());
    if (m_closureGenerator) {
      func->getFunctionScope()->setClosureGenerator();
    }
    func->getLocation()->line0 = loc->line0;
    func->getLocation()->char0 = loc->char0;
    if (func->ignored()) {
      out->stmt = NEW_STMT0(StatementList);
    }
  }

  if (hasType(ret)) {
    // TODO
  }
}

void Parser::onParam(Token &out, Token *params, Token &type, Token &var,
                     bool ref, Token *defValue, Token *attr) {
  ExpressionPtr expList;
  if (params) {
    expList = params->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  ExpressionListPtr attrList;
  if (attr && attr->exp) {
    attrList = dynamic_pointer_cast<ExpressionList>(attr->exp);
  }
  expList->addElement(NEW_EXP(ParameterExpression, type->text(), var->text(),
                              ref, defValue ? defValue->exp : ExpressionPtr(),
                              attrList));
  out->exp = expList;
}

void Parser::onClassStart(int type, Token &name) {
  if (name.text() == "self" || name.text() == "parent" ||
      Type::GetTypeHintTypes().find(name.text()) !=
      Type::GetTypeHintTypes().end()) {
    PARSE_ERROR("Cannot use '%s' as class name as it is reserved",
                name.text().c_str());
  }

  pushComment();
  newScope();
  m_clsName = name.text();
  m_inTrait = type == T_TRAIT;
}

void Parser::onClass(Token &out, int type, Token &name, Token &base,
                     Token &baseInterface, Token &stmt, Token *attr) {
  StatementListPtr stmtList;
  if (stmt->stmt) {
    stmtList = dynamic_pointer_cast<StatementList>(stmt->stmt);
  }
  ExpressionListPtr attrList;
  if (attr && attr->exp) {
    attrList = dynamic_pointer_cast<ExpressionList>(attr->exp);
  }

  ClassStatementPtr cls = NEW_STMT
    (ClassStatement, type, name->text(), base->text(),
     dynamic_pointer_cast<ExpressionList>(baseInterface->exp),
     popComment(), stmtList, attrList);
  out->stmt = cls;
  {
    cls->onParse(m_ar, m_file);
  }
  completeScope(cls->getClassScope());
  if (cls->ignored()) {
    out->stmt = NEW_STMT0(StatementList);
  }
  m_clsName.clear();
  m_inTrait = false;
}

void Parser::onInterface(Token &out, Token &name, Token &base, Token &stmt,
                         Token *attr) {
  StatementListPtr stmtList;
  if (stmt->stmt) {
    stmtList = dynamic_pointer_cast<StatementList>(stmt->stmt);
  }
  ExpressionListPtr attrList;
  if (attr && attr->exp) {
    attrList = dynamic_pointer_cast<ExpressionList>(attr->exp);
  }

  InterfaceStatementPtr intf = NEW_STMT
    (InterfaceStatement, name->text(),
     dynamic_pointer_cast<ExpressionList>(base->exp), popComment(), stmtList,
     attrList);
  out->stmt = intf;
  {
    intf->onParse(m_ar, m_file);
  }
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

void Parser::onTraitUse(Token &out, Token &traits, Token &rules) {
  if (!rules->stmt) {
    rules->stmt = NEW_STMT0(StatementList);
  }
  out->stmt = NEW_STMT(UseTraitStatement,
                       dynamic_pointer_cast<ExpressionList>(traits->exp),
                       dynamic_pointer_cast<StatementList>(rules->stmt));
}

void Parser::onTraitName(Token &out, Token *names, Token &name) {
  ExpressionPtr expList;
  if (names) {
    expList = names->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(NEW_EXP(ScalarExpression, T_STRING, name->text()));
  out->exp = expList;
}

void Parser::onTraitRule(Token &out, Token &stmtList, Token &newStmt) {
  if (!stmtList->stmt) {
    out->stmt = NEW_STMT0(StatementList);
  } else {
    out->stmt = stmtList->stmt;
  }
  ASSERT(newStmt->stmt);
  out->stmt->addElement(newStmt->stmt);
}

void Parser::onTraitPrecRule(Token &out, Token &traitName, Token &methodName,
                             Token &otherTraits) {
  ASSERT(otherTraits->exp);
  ScalarExpressionPtr expTraitName = NEW_EXP(ScalarExpression, T_STRING,
                                             traitName->text());
  ScalarExpressionPtr expMethodName = NEW_EXP(ScalarExpression, T_STRING,
                                              methodName->text());
  out->stmt = NEW_STMT(TraitPrecStatement, expTraitName, expMethodName,
                       dynamic_pointer_cast<ExpressionList>(otherTraits->exp));
}

void Parser::onTraitAliasRuleStart(Token &out, Token &traitName,
                                   Token &methodName) {
  ScalarExpressionPtr expTraitName = NEW_EXP(ScalarExpression, T_STRING,
                                             traitName->text());
  ScalarExpressionPtr expMethodName = NEW_EXP(ScalarExpression, T_STRING,
                                              methodName->text());

  ModifierExpressionPtr expModifiers = NEW_EXP0(ModifierExpression);

  ScalarExpressionPtr expNewMethodName = NEW_EXP(ScalarExpression, T_STRING,
                                                 methodName->text());

  out->stmt = NEW_STMT(TraitAliasStatement, expTraitName, expMethodName,
                       expModifiers, expNewMethodName);
}

void Parser::onTraitAliasRuleModify(Token &out, Token &rule,
                                    Token &accessModifiers,
                                    Token &newMethodName) {
  TraitAliasStatementPtr ruleStmt=
    dynamic_pointer_cast<TraitAliasStatement>(rule->stmt);

  ASSERT(ruleStmt);

  if (!newMethodName->text().empty()) {
    ScalarExpressionPtr expNewMethodName =
      NEW_EXP(ScalarExpression, T_STRING, newMethodName->text());
    ruleStmt->setNewMethodName(expNewMethodName);
  }

  if (accessModifiers->exp) {
    ruleStmt->setModifiers(dynamic_pointer_cast<ModifierExpression>
                           (accessModifiers->exp));
  }

  out->stmt = ruleStmt;
}

void Parser::onClassVariableStart(Token &out, Token *modifiers, Token &decl,
                                  Token *type) {
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

  if (type && hasType(*type)) {
    // TODO
  }
}

void Parser::onMethod(Token &out, Token &modifiers, Token &ret, Token &ref,
                      Token &name, Token &params, Token &stmt,
                      Token *attr, bool reloc /* = true */) {
  ModifierExpressionPtr exp = modifiers->exp ?
    dynamic_pointer_cast<ModifierExpression>(modifiers->exp)
    : NEW_EXP0(ModifierExpression);

  StatementListPtr stmts;
  if (!stmt->stmt && stmt->num() == 1) {
    stmts = NEW_STMT0(StatementList);
  } else {
    stmts = dynamic_pointer_cast<StatementList>(stmt->stmt);
  }

  ExpressionListPtr old_params =
    dynamic_pointer_cast<ExpressionList>(params->exp);
  int attribute = m_file->popAttribute();
  string comment = popComment();
  LocationPtr loc = popFuncLocation();

  FunctionContext funcContext = m_funcContexts.back();
  m_funcContexts.pop_back();
  m_prependingStatements.pop_back();

  funcContext.checkFinalAssertions();

  bool hasCallToGetArgs = m_hasCallToGetArgs.back();
  m_hasCallToGetArgs.pop_back();

  fixStaticVars();

  MethodStatementPtr mth;
  if (funcContext.isGenerator) {
    const string &closureName = getAnonFuncName(ParserBase::Continuation);
    Token new_params;
    prepare_generator(this, stmt, new_params, funcContext.numYields);
    ModifierExpressionPtr exp2 = Construct::Clone(exp);
    mth = NEW_STMT(MethodStatement, exp2, ref->num(), closureName,
                   dynamic_pointer_cast<ExpressionList>(new_params->exp),
                   dynamic_pointer_cast<StatementList>(stmt->stmt),
                   attribute, comment, ExpressionListPtr());
    out->stmt = mth;
    if (reloc) {
      mth->getLocation()->line0 = loc->line0;
      mth->getLocation()->char0 = loc->char0;
    }
    {
      completeScope(mth->onInitialParse(m_ar, m_file));
    }
    // create_generator() expects us to push the docComment back
    // onto the comment stack so that it can make sure that the
    // the MethodStatement it's building will get the docComment
    pushComment(comment);
    Token origGenFunc;
    create_generator(this, out, params, name, closureName, m_clsName.c_str(),
                     &modifiers, hasCallToGetArgs, origGenFunc,
                     hhvm && Option::OutputHHBC &&
                     (!Option::WholeProgram || !Option::ParseTimeOpts),
                     attr);
    MethodStatementPtr origStmt =
      boost::dynamic_pointer_cast<MethodStatement>(origGenFunc->stmt);
    ASSERT(origStmt);
    mth->setOrigGeneratorFunc(origStmt);
    origStmt->setGeneratorFunc(mth);

  } else {
    ExpressionListPtr attrList;
    if (attr && attr->exp) {
      attrList = dynamic_pointer_cast<ExpressionList>(attr->exp);
    }
    mth = NEW_STMT(MethodStatement, exp, ref->num(), name->text(),
                   old_params, stmts, attribute, comment, attrList);
    out->stmt = mth;
    if (reloc) {
      mth->getLocation()->line0 = loc->line0;
      mth->getLocation()->char0 = loc->char0;
    }
    completeScope(mth->onInitialParse(m_ar, m_file));
  }

  if (hasType(ret)) {
    // TODO
  }
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

  if (m_staticVars.size()) fixStaticVars();
  FunctionScopePtr pseudoMain = m_file->setTree(m_ar, m_tree);
  completeScope(pseudoMain);
  pseudoMain->setOuterScope(m_file);
  m_file->setOuterScope(m_ar);
  m_ar->parseExtraCode(m_file->getName());
  LocationPtr loc = getLocation();
  loc->line0 = loc->char0 = 1;
  pseudoMain->getStmt()->setLocation(loc);
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

  ASSERT(!m_prependingStatements.empty());
  vector<StatementPtr> &prepending = m_prependingStatements.back();
  if (!prepending.empty()) {
    ASSERT(prepending.size() == 1);
    for (unsigned i = 0; i < prepending.size(); i++) {
      out->stmt->addElement(prepending[i]);
    }
    prepending.clear();
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

void Parser::onReturn(Token &out, Token *expr, bool checkYield /* = true */) {
  out->stmt = NEW_STMT(ReturnStatement, expr ? expr->exp : ExpressionPtr());
  if (checkYield && !m_funcContexts.empty()) {
    if (!m_funcContexts.back().setIsNotGenerator()) {
      if (!hhvm) Compiler::Error(InvalidYield, out->stmt);
      PARSE_ERROR("Cannot mix 'return' and 'yield' in the same function");
    }
  }
}

static void invalidYield(LocationPtr loc) {
  if (hhvm) return;

  ExpressionPtr exp(new SimpleFunctionCall(BlockScopePtr(), loc, "yield",
                                           ExpressionListPtr(),
                                           ExpressionPtr()));
  Compiler::Error(Compiler::InvalidYield, exp);
}

bool Parser::setIsGenerator() {
  if (!Option::EnableHipHopSyntax) {
    PARSE_ERROR("Yield is not enabled");
    return false;
  }

  if (m_funcContexts.empty()) {
    invalidYield(getLocation());
    PARSE_ERROR("Yield can only be used inside a function");
    return false;
  }

  if (!m_funcContexts.back().setIsGenerator()) {
    invalidYield(getLocation());
    PARSE_ERROR("Cannot mix 'return' and 'yield' in the same function");
    return false;
  }

  if (!m_clsName.empty()) {
    if (strcasecmp(m_funcName.c_str(), m_clsName.c_str()) == 0) {
      invalidYield(getLocation());
      PARSE_ERROR("'yield' is not allowed in potential constructors");
      return false;
    }

    if (m_funcName[0] == '_' && m_funcName[1] == '_') {
      const char *fname = m_funcName.c_str() + 2;
      if (!strcasecmp(fname, "construct") ||
          !strcasecmp(fname, "destruct") ||
          !strcasecmp(fname, "get") ||
          !strcasecmp(fname, "set") ||
          !strcasecmp(fname, "isset") ||
          !strcasecmp(fname, "unset") ||
          !strcasecmp(fname, "call") ||
          !strcasecmp(fname, "callstatic") ||
          !strcasecmp(fname, "invoke")) {
        invalidYield(getLocation());
        PARSE_ERROR("'yield' is not allowed in constructor, destructor, or "
                    "magic methods");
        return false;
      }
    }
  }

  return true;
}

void Parser::onYield(Token &out, Token *expr, bool assign) {
  if (!setIsGenerator()) {
    return;
  }

  FunctionContext &funcContext = m_funcContexts.back();
  std::fill(funcContext.foreachHasYield.begin(),
            funcContext.foreachHasYield.end(), true);
  int index = ++funcContext.numYields;
  transform_yield(this, out, index, expr, assign);
}

void Parser::onYieldBreak(Token &out) {
  if (!setIsGenerator()) {
    return;
  }

  transform_yield_break(this, out);
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
  ExpStatementPtr exp(NEW_STMT(ExpStatement, expr->exp));
  out->stmt = exp;
  exp->onParse(m_ar, m_file);
}

void Parser::onForEachStart() {
  if (!m_funcContexts.empty()) {
    m_funcContexts.back().foreachHasYield.push_back(false);
  }
}

void Parser::onForEach(Token &out, Token &arr, Token &name, Token &value,
                       Token &stmt) {
  if (value->exp && name->num()) {
    PARSE_ERROR("Key element cannot be a reference");
    return;
  }
  checkAssignThis(name);
  checkAssignThis(value);
  if (!m_funcContexts.empty()) {
    bool hasYield = m_funcContexts.back().foreachHasYield.back();
    m_funcContexts.back().foreachHasYield.pop_back();

    if (hasYield) {
      int cnt = ++m_funcContexts.back().numForeaches;
      transform_foreach(this, out, arr, name, value, stmt, cnt, value->exp,
                        value->exp ? value->num() == 1 : name->num() == 1);
      return;
    }
  }
  if (stmt->stmt && stmt->stmt->is(Statement::KindOfStatementList)) {
    stmt->stmt = NEW_STMT(BlockStatement,
                          dynamic_pointer_cast<StatementList>(stmt->stmt));
  }
  out->stmt = NEW_STMT(ForEachStatement, arr->exp, name->exp, name->num() == 1,
                       value->exp, value->num() == 1, stmt->stmt);
}

void Parser::onTry(Token &out, Token &tryStmt, Token &className, Token &var,
                   Token &catchStmt, Token &catches, Token &finallyStmt) {
  StatementPtr stmtList;
  if (catches->stmt) {
    stmtList = catches->stmt;
  } else {
    stmtList = NEW_STMT0(StatementList);
  }
  stmtList->insertElement(NEW_STMT(CatchStatement, className->text(),
                                   var->text(), catchStmt->stmt));
  out->stmt = NEW_STMT(TryStatement, tryStmt->stmt,
                       dynamic_pointer_cast<StatementList>(stmtList),
                       finallyStmt->stmt);
}

void Parser::onTry(Token &out, Token &tryStmt, Token &finallyStmt) {
  out->stmt = NEW_STMT(TryStatement, tryStmt->stmt,
                       dynamic_pointer_cast<StatementList>(NEW_STMT0(StatementList)),
                       finallyStmt->stmt);
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

void Parser::onFinally(Token &out, Token &stmt) {
  out->stmt = NEW_STMT(FinallyStatement, stmt->stmt);
}

void Parser::onThrow(Token &out, Token &expr) {
  out->stmt = NEW_STMT(ThrowStatement, expr->exp);
}

void Parser::onClosure(Token &out, Token &ret, Token &ref, Token &params,
                       Token &cparams, Token &stmts) {
  Token func, name;
  onFunction(func, ret, ret, name, params, stmts, 0);

  out.reset();
  out->exp = NEW_EXP(ClosureExpression,
                     dynamic_pointer_cast<FunctionStatement>(func->stmt),
                     dynamic_pointer_cast<ExpressionList>(cparams->exp));
}

void Parser::onClosureParam(Token &out, Token *params, Token &param,
                            bool ref) {
  ExpressionPtr expList;
  if (params) {
    expList = params->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(NEW_EXP(ParameterExpression, "", param->text(), ref,
                              ExpressionPtr(), ExpressionPtr()));
  out->exp = expList;
}

void Parser::onLabel(Token &out, Token &label) {
  out->stmt = NEW_STMT(LabelStatement, label.text());
}

void Parser::onGoto(Token &out, Token &label, bool limited) {
  out->stmt = NEW_STMT(GotoStatement, label.text());
}

void Parser::invalidateGoto(TStatementPtr stmt, GotoError error) {
  GotoStatement *gs = (GotoStatement*) stmt;
  ASSERT(gs);
  gs->invalidate(error);
}

void Parser::invalidateLabel(TStatementPtr stmt) {
  LabelStatement *ls = (LabelStatement*) stmt;
  ASSERT(ls);
  ls->invalidate();
}

TStatementPtr Parser::extractStatement(ScannerToken *stmt) {
  Token *t = (Token*) stmt;
  return t->stmt.get();
}

///////////////////////////////////////////////////////////////////////////////

bool Parser::hasType(Token &type) {
  if (!type.text().empty()) {
    if (!Option::EnableHipHopSyntax && !m_scanner.isStrictMode()) {
      PARSE_ERROR("Type hint is not enabled");
      return false;
    }
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
