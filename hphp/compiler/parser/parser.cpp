/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/compiler/parser/parser.h"

#include "hphp/compiler/type_annotation.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/compiler/analysis/file_scope.h"

#include "hphp/compiler/expression/expression_list.h"
#include "hphp/compiler/expression/assignment_expression.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/dynamic_variable.h"
#include "hphp/compiler/expression/static_member_expression.h"
#include "hphp/compiler/expression/array_element_expression.h"
#include "hphp/compiler/expression/dynamic_function_call.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/scalar_expression.h"
#include "hphp/compiler/expression/object_property_expression.h"
#include "hphp/compiler/expression/object_method_expression.h"
#include "hphp/compiler/expression/list_assignment.h"
#include "hphp/compiler/expression/new_object_expression.h"
#include "hphp/compiler/expression/include_expression.h"
#include "hphp/compiler/expression/unary_op_expression.h"
#include "hphp/compiler/expression/binary_op_expression.h"
#include "hphp/compiler/expression/qop_expression.h"
#include "hphp/compiler/expression/array_pair_expression.h"
#include "hphp/compiler/expression/class_constant_expression.h"
#include "hphp/compiler/expression/parameter_expression.h"
#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/expression/constant_expression.h"
#include "hphp/compiler/expression/encaps_list_expression.h"
#include "hphp/compiler/expression/closure_expression.h"
#include "hphp/compiler/expression/yield_expression.h"
#include "hphp/compiler/expression/await_expression.h"
#include "hphp/compiler/expression/user_attribute.h"
#include "hphp/compiler/expression/query_expression.h"
#include "hphp/compiler/expression/simple_query_clause.h"
#include "hphp/compiler/expression/join_clause.h"
#include "hphp/compiler/expression/group_clause.h"
#include "hphp/compiler/expression/ordering.h"

#include "hphp/compiler/statement/function_statement.h"
#include "hphp/compiler/statement/class_statement.h"
#include "hphp/compiler/statement/interface_statement.h"
#include "hphp/compiler/statement/class_variable.h"
#include "hphp/compiler/statement/class_constant.h"
#include "hphp/compiler/statement/method_statement.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/statement/block_statement.h"
#include "hphp/compiler/statement/if_branch_statement.h"
#include "hphp/compiler/statement/if_statement.h"
#include "hphp/compiler/statement/while_statement.h"
#include "hphp/compiler/statement/do_statement.h"
#include "hphp/compiler/statement/for_statement.h"
#include "hphp/compiler/statement/switch_statement.h"
#include "hphp/compiler/statement/case_statement.h"
#include "hphp/compiler/statement/break_statement.h"
#include "hphp/compiler/statement/continue_statement.h"
#include "hphp/compiler/statement/return_statement.h"
#include "hphp/compiler/statement/global_statement.h"
#include "hphp/compiler/statement/static_statement.h"
#include "hphp/compiler/statement/echo_statement.h"
#include "hphp/compiler/statement/unset_statement.h"
#include "hphp/compiler/statement/exp_statement.h"
#include "hphp/compiler/statement/foreach_statement.h"
#include "hphp/compiler/statement/catch_statement.h"
#include "hphp/compiler/statement/try_statement.h"
#include "hphp/compiler/statement/finally_statement.h"
#include "hphp/compiler/statement/throw_statement.h"
#include "hphp/compiler/statement/goto_statement.h"
#include "hphp/compiler/statement/label_statement.h"
#include "hphp/compiler/statement/use_trait_statement.h"
#include "hphp/compiler/statement/trait_require_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/typedef_statement.h"

#include "hphp/compiler/analysis/function_scope.h"

#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/analysis_result.h"

#include "hphp/util/lock.h"
#include "hphp/util/logger.h"

#include "hphp/runtime/base/file-repository.h"

#ifdef FACEBOOK
#include "hphp/facebook/src/compiler/fb_compiler_hooks.h"
#define RealSimpleFunctionCall FBSimpleFunctionCall
#else
#define RealSimpleFunctionCall SimpleFunctionCall
#endif

#define NEW_EXP0(cls)                                           \
  cls##Ptr(new cls(BlockScopePtr(),                             \
                   getLocation()))
#define NEW_EXP(cls, e...)                                      \
  cls##Ptr(new cls(BlockScopePtr(),                             \
                   getLocation(), ##e))
#define NEW_STMT0(cls)                                          \
  cls##Ptr(new cls(BlockScopePtr(), getLabelScope(),            \
                   getLocation()))
#define NEW_STMT(cls, e...)                                     \
  cls##Ptr(new cls(BlockScopePtr(), getLabelScope(),            \
                   getLocation(), ##e))

#define PARSE_ERROR(fmt, args...)  HPHP_PARSER_ERROR(fmt, this, ##args)

using namespace HPHP::Compiler;

namespace HPHP {

SimpleFunctionCallPtr NewSimpleFunctionCall(
  EXPRESSION_CONSTRUCTOR_PARAMETERS,
  const std::string &name, bool hadBackslash, ExpressionListPtr params,
  ExpressionPtr cls) {
  return SimpleFunctionCallPtr(
    new RealSimpleFunctionCall(
      EXPRESSION_CONSTRUCTOR_DERIVED_PARAMETER_VALUES,
      name, hadBackslash, params, cls));
}

namespace Compiler {
///////////////////////////////////////////////////////////////////////////////
// statics

StatementListPtr Parser::ParseString(const String& input, AnalysisResultPtr ar,
                                     const char *fileName /* = NULL */,
                                     bool lambdaMode /* = false */) {
  assert(!input.empty());
  if (!fileName || !*fileName) fileName = "string";

  int len = input.size();
  Scanner scanner(input.data(), len, Option::GetScannerType(), fileName, true);
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
      m_closureGenerator(false), m_nsState(SeenNothing),
      m_aliasTable(getAutoAliasedClasses(), [&] { return isAutoAliasOn(); }) {
  string md5str = Eval::FileRepository::unitMd5(scanner.getMd5());
  MD5 md5 = MD5(md5str.c_str());

  m_file = FileScopePtr(new FileScope(m_fileName, fileSize, md5));

  newScope();
  m_staticVars.push_back(StringToExpressionPtrVecMap());
  m_inTrait = false;

  Lock lock(m_ar->getMutex());
  m_ar->addFileScope(m_file);
}

bool Parser::parse() {
  try {
    if (!parseImpl()) {
      throw ParseTimeFatalException(m_fileName, line1(),
                                    "Parse error: %s",
                                    errString().c_str());
    }
    return true;
  } catch (const ParseTimeFatalException& e) {
    m_file->cleanupForError(m_ar);
    if (e.m_parseFatal) {
      m_file->makeParseFatal(m_ar, e.getMessage(), e.m_line);
    } else {
      m_file->makeFatal(m_ar, e.getMessage(), e.m_line);
    }
    return false;
  }
}

void Parser::error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  fatal(&m_loc, msg.c_str());
}

void Parser::parseFatal(const Location* loc, const char* msg) {
  // we can't use loc->file, as the bison parser doesn't track that in YYLTYPE
  auto file = m_file->getName().c_str();
  auto exn = ParseTimeFatalException(file, loc->line0, "%s", msg);
  exn.setParseFatal();
  throw exn;
}

void Parser::fatal(const Location* loc, const char* msg) {
  throw ParseTimeFatalException(loc->file, loc->line0, "%s", msg);
}

string Parser::errString() {
  return m_error.empty() ? getMessage() : m_error;
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

LabelScopePtr Parser::getLabelScope() const {
  assert(!m_labelScopes.empty());
  assert(!m_labelScopes.back().empty());
  assert(m_labelScopes.back().back() != nullptr);
  return m_labelScopes.back().back();
}

void Parser::onNewLabelScope(bool fresh) {
  if (fresh) {
    m_labelScopes.push_back(LabelScopePtrVec());
  }
  assert(!m_labelScopes.empty());
  LabelScopePtr labelScope(new LabelScope());
  m_labelScopes.back().push_back(labelScope);
}

void Parser::onScopeLabel(const Token& stmt, const Token& label) {
  assert(!m_labelScopes.empty());
  assert(!m_labelScopes.back().empty());
  for (auto& scope : m_labelScopes.back()) {
    scope->addLabel(stmt.stmt, label.text());
  }
}

void Parser::onCompleteLabelScope(bool fresh) {
  assert(!m_labelScopes.empty());
  assert(!m_labelScopes.back().empty());
  m_labelScopes.back().pop_back();
  if (fresh) {
    m_labelScopes.pop_back();
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
    exp = NEW_EXP(ConstantExpression, var->text(), false, docComment);
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
    var->exp = NEW_EXP(ConstantExpression, var->text(), var->num() & 2);
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
    const string &s = name.text();
    // strip out namespaces for func_get_args and friends check
    size_t lastBackslash = s.find_last_of(NAMESPACE_SEP);
    const string stripped = lastBackslash == string::npos
                            ? s
                            : s.substr(lastBackslash+1);
    if (stripped == "func_num_args" ||
        stripped == "func_get_args" ||
        stripped == "func_get_arg") {
      if (m_hasCallToGetArgs.size() > 0) {
        m_hasCallToGetArgs.back() = true;
      }
    }

    SimpleFunctionCallPtr call
      (new RealSimpleFunctionCall
       (BlockScopePtr(), getLocation(), name->text(), name->num() & 2,
        dynamic_pointer_cast<ExpressionList>(params->exp), clsExp));
    if (m_scanner.isHHSyntaxEnabled() && !(name->num() & 2)) {
      // If the function name is without any backslashes or
      // namespace qualification then we treat this as a candidate
      // for optimization via bytecode promotion.
      // "idx" is the only function in that class for now but it's
      // cheaper to set the bit that to check for the function name
      call->setOptimizable();
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
    assert(false);
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
  ConstantExpressionPtr con = NEW_EXP(ConstantExpression, constant->text(),
      constant->num() & 2);
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
    case T_COMPILER_HALT_OFFSET:
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
      assert(false);
  }
  if (type == T_COMPILER_HALT_OFFSET) {
    // Keep track of this expression for later backpatching
    // If it doesn't get backpatched (because there was no HALT_COMPILER
    // then the constant will return (int)"__COMPILER_HALT_OFFSET__" (zero)
    m_compilerHaltOffsetVec.push_back(exp);
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

void Parser::onListAssignment(Token &out, Token &vars, Token *expr,
                              bool rhsFirst /* = false */) {
  ExpressionListPtr el(dynamic_pointer_cast<ExpressionList>(vars->exp));
  for (int i = 0; i < el->getCount(); i++) {
    if (dynamic_pointer_cast<FunctionCall>((*el)[i])) {
      PARSE_ERROR("Can't use return value in write context");
    }
  }
  out->exp = NEW_EXP(ListAssignment,
                     dynamic_pointer_cast<ExpressionList>(vars->exp),
                     expr ? expr->exp : ExpressionPtr(), rhsFirst);
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
  onListAssignment(out, sublist, nullptr);
  onExprListElem(out, list, out);
}

void Parser::checkAssignThis(Token &var) {
  if (SimpleVariablePtr simp = dynamic_pointer_cast<SimpleVariable>(var.exp)) {
    if (simp->getName() == "this") {
      PARSE_ERROR("Cannot re-assign $this");
    }
  }
}

void Parser::onAssign(Token &out, Token &var, Token &expr, bool ref,
                      bool rhsFirst /* = false */) {
  if (dynamic_pointer_cast<FunctionCall>(var->exp)) {
    PARSE_ERROR("Can't use return value in write context");
  }
  checkAssignThis(var);
  out->exp = NEW_EXP(AssignmentExpression, var->exp, expr->exp, ref, rhsFirst);
}

void Parser::onAssignNew(Token &out, Token &var, Token &name, Token &args) {
  checkAssignThis(var);
  ExpressionPtr exp =
    NEW_EXP(NewObjectExpression, name->exp,
            dynamic_pointer_cast<ExpressionList>(args->exp));
  out->exp = NEW_EXP(AssignmentExpression, var->exp, exp, true);
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

  // If the operands are simple enough we can fold this expression right
  // here and keep the parse tree smaller.
  if (ExpressionPtr optExp = bop->foldConst(m_ar)) out->exp = optExp;
}

void Parser::onQOp(Token &out, Token &exprCond, Token *expYes, Token &expNo) {
  out->exp = NEW_EXP(QOpExpression, exprCond->exp,
                     expYes ? expYes->exp : ExpressionPtr(), expNo->exp);
}

void Parser::onArray(Token &out, Token &pairs, int op /* = T_ARRAY */) {
  if (op != T_ARRAY && !m_scanner.isHHSyntaxEnabled()) {
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

void Parser::checkFunctionContext(string funcName,
                                  FunctionContext& funcContext,
                                  ModifierExpressionPtr modifiers,
                                  int returnsRef) {
  funcContext.checkFinalAssertions();

  // let async modifier be mandatory
  if (funcContext.isAsync && !modifiers->isAsync()) {
    PARSE_ERROR("Function '%s' contains 'await' but is not declared as async.",
                funcName.c_str());
  }

  if (modifiers->isAsync() && returnsRef) {
    PARSE_ERROR("Asynchronous function '%s' cannot return reference.",
                funcName.c_str());
  }

  if (modifiers->isAsync() && funcContext.isGenerator) {
    PARSE_ERROR("'yield' is not allowed in async functions.");
  }
}

void Parser::prepareConstructorParameters(StatementListPtr stmts,
                                          ExpressionListPtr params,
                                          bool isAbstract) {
  for (int i = 0, count = params->getCount(); i < count; i++) {
    ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*params)[i]);
    TokenID mod = param->getModifier();
    if (mod == 0) continue;

    if (isAbstract) {
       param->parseTimeFatal(Compiler::InvalidAttribute,
                             "parameter modifiers not allowed on "
                             "abstract __construct");
    }
    if (!stmts) {
       param->parseTimeFatal(Compiler::InvalidAttribute,
                             "parameter modifiers not allowed on "
                             "__construct without a body");
    }
    if (param->annotation()) {
      std::vector<std::string> typeNames;
      param->annotation()->getAllSimpleNames(typeNames);
      for (auto& typeName : typeNames) {
        if (isTypeVarInImmediateScope(typeName)) {
          param->parseTimeFatal(Compiler::InvalidAttribute,
                                "parameter modifiers not supported with "
                                "type variable annotation");
        }
      }
    }
    std::string name = param->getName();
    SimpleVariablePtr value = NEW_EXP(SimpleVariable, name);
    ScalarExpressionPtr prop = NEW_EXP(ScalarExpression, T_STRING, name);
    SimpleVariablePtr self = NEW_EXP(SimpleVariable, "this");
    ObjectPropertyExpressionPtr objProp =
        NEW_EXP(ObjectPropertyExpression, self, prop);
    AssignmentExpressionPtr assign =
        NEW_EXP(AssignmentExpression, objProp, value, false);
    ExpStatementPtr stmt = NEW_STMT(ExpStatement, assign);
    stmts->insertElement(stmt);
  }
}

string Parser::getFunctionName(FunctionType type, Token* name) {
  switch (type) {
    case FunctionType::Closure:
      return newClosureName(m_clsName, m_containingFuncName);
    case FunctionType::Function:
      assert(name);
      if (!m_lambdaMode) {
        return name->text();
      } else {
        return name->text() + "{lambda}";
      }
    case FunctionType::Method:
      assert(name);
      return name->text();
  }
  not_reached();
}

StatementPtr Parser::onFunctionHelper(FunctionType type,
                              Token *modifiers, Token &ret,
                              Token &ref, Token *name, Token &params,
                              Token &stmt, Token *attr, bool reloc) {
  // prepare and validate function modifiers
  ModifierExpressionPtr modifiersExp = modifiers && modifiers->exp ?
    dynamic_pointer_cast<ModifierExpression>(modifiers->exp)
    : NEW_EXP0(ModifierExpression);
  modifiersExp->setHasPrivacy(type == FunctionType::Method);
  if (type == FunctionType::Closure && !modifiersExp->validForClosure()) {
    PARSE_ERROR("Invalid modifier on closure funciton.");
  }
  if (type == FunctionType::Function && !modifiersExp->validForFunction()) {
    PARSE_ERROR("Invalid modifier on function %s.", name->text().c_str());
  }

  StatementListPtr stmts = stmt->stmt || stmt->num() != 1 ?
    dynamic_pointer_cast<StatementList>(stmt->stmt)
    : NEW_STMT0(StatementList);

  ExpressionListPtr old_params =
    dynamic_pointer_cast<ExpressionList>(params->exp);

  string funcName = getFunctionName(type, name);

  if (type == FunctionType::Method && old_params &&
      funcName == "__construct") {
    prepareConstructorParameters(stmts, old_params,
                                 modifiersExp->isAbstract());
  }

  fixStaticVars();

  int attribute = m_file->popAttribute();
  string comment = popComment();

  ExpressionListPtr attrList;
  if (attr && attr->exp) {
    attrList = dynamic_pointer_cast<ExpressionList>(attr->exp);
  }

  // create function/method statement
  FunctionStatementPtr func;
  MethodStatementPtr mth;
  if (type == FunctionType::Method) {
    mth = NEW_STMT(MethodStatement, modifiersExp,
                   ref->num(), funcName, old_params,
                   ret.typeAnnotation, stmts,
                   attribute, comment, attrList);
    completeScope(mth->onInitialParse(m_ar, m_file));
  } else {
    func = NEW_STMT(FunctionStatement, modifiersExp,
                   ref->num(), funcName, old_params,
                   ret.typeAnnotation, stmts,
                   attribute, comment, attrList);

    func->onParse(m_ar, m_file);
    completeScope(func->getFunctionScope());
    if (func->ignored()) {
      return NEW_STMT0(StatementList);
    }
    mth = func;
  }

  // check and set generator/async flags
  FunctionContext funcContext = m_funcContexts.back();
  checkFunctionContext(funcName, funcContext, modifiersExp, ref->num());
  mth->getFunctionScope()->setGenerator(funcContext.isGenerator);
  mth->getFunctionScope()->setAsync(modifiersExp->isAsync());
  m_funcContexts.pop_back();

  mth->setHasCallToGetArgs(m_hasCallToGetArgs.back());
  m_hasCallToGetArgs.pop_back();

  LocationPtr loc = popFuncLocation();
  if (reloc) {
    mth->getLocation()->line0 = loc->line0;
    mth->getLocation()->char0 = loc->char0;
  }

  return mth;
}

void Parser::onFunction(Token &out, Token *modifiers, Token &ret, Token &ref,
                        Token &name, Token &params, Token &stmt, Token *attr) {
  out->stmt = onFunctionHelper(FunctionType::Function,
                modifiers, ret, ref, &name, params, stmt, attr, true);
}

void Parser::onMethod(Token &out, Token &modifiers, Token &ret, Token &ref,
                      Token &name, Token &params, Token &stmt,
                      Token *attr, bool reloc /* = true */) {
  out->stmt = onFunctionHelper(FunctionType::Method,
                &modifiers, ret, ref, &name, params, stmt, attr, reloc);
}

void Parser::onParam(Token &out, Token *params, Token &type, Token &var,
                     bool ref, Token *defValue, Token *attr, Token *modifier) {
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

  TypeAnnotationPtr typeAnnotation = type.typeAnnotation;
  expList->addElement(NEW_EXP(ParameterExpression, typeAnnotation,
                              m_scanner.isHHSyntaxEnabled(), var->text(),
                              ref, (modifier) ? modifier->num() : 0,
                              defValue ? defValue->exp : ExpressionPtr(),
                              attrList));
  out->exp = expList;
}

void Parser::onClassStart(int type, Token &name) {
  const Type::TypePtrMap& typeHintTypes =
    Type::GetTypeHintTypes(m_scanner.isHHSyntaxEnabled());
  if (0 == strcasecmp("self", name.text().c_str()) ||
      0 == strcasecmp("parent", name.text().c_str()) ||
      typeHintTypes.find(name.text()) != typeHintTypes.end()) {
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

  // look for argument promotion in ctor
  ExpressionListPtr promote = NEW_EXP(ExpressionList);
  cls->checkArgumentsToPromote(promote, type);
  for (int i = 0, count = promote->getCount(); i < count; i++) {
    auto param =
        dynamic_pointer_cast<ParameterExpression>((*promote)[i]);
    TokenID mod = param->getModifier();
    std::string name = param->getName();
    std::string type = param->hasUserType() ?
                                  param->getUserTypeHint() : "";

    // create the class variable and change the location to
    // point to the paramenter location for error reporting
    LocationPtr location = param->getLocation();
    ModifierExpressionPtr modifier = NEW_EXP0(ModifierExpression);
    modifier->add(mod);
    modifier->setLocation(location);
    SimpleVariablePtr svar = NEW_EXP(SimpleVariable, name);
    svar->setLocation(location);
    ExpressionListPtr expList = NEW_EXP0(ExpressionList);
    expList->addElement(svar);
    expList->setLocation(location);
    ClassVariablePtr var = NEW_STMT(ClassVariable, modifier, type, expList);
    var->setLocation(location);
    cls->getStmts()->addElement(var);
  }

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
  registerAlias(name.text());
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

void Parser::onTraitRequire(Token &out, Token &name, bool isExtends) {
  out->stmt = NEW_STMT(TraitRequireStatement, name->text(), isExtends);
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
  assert(newStmt->stmt);
  out->stmt->addElement(newStmt->stmt);
}

void Parser::onTraitPrecRule(Token &out, Token &traitName, Token &methodName,
                             Token &otherTraits) {
  assert(otherTraits->exp);
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

  assert(ruleStmt);

  if (!newMethodName->text().empty()) {
    ScalarExpressionPtr expNewMethodName =
      NEW_EXP(ScalarExpression, T_STRING, newMethodName->text());
    ruleStmt->setNewMethodName(expNewMethodName);
  }

  if (accessModifiers->exp) {
    auto modifiersExp =
      dynamic_pointer_cast<ModifierExpression>(accessModifiers->exp);
    if (!modifiersExp->validForTraitAliasRule()) {
      PARSE_ERROR("Only access and visibility modifiers are allowed"
                  " in trait alias rule");
    }
    ruleStmt->setModifiers(modifiersExp);
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
      (ClassVariable, exp,
       (type) ? type->typeAnnotationName() : "",
       dynamic_pointer_cast<ExpressionList>(decl->exp));
  } else {
    out->stmt =
      NEW_STMT(ClassConstant,
        (type) ? type->typeAnnotationName() : "",
        dynamic_pointer_cast<ExpressionList>(decl->exp));
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

void Parser::initParseTree() {
  m_tree = NEW_STMT0(StatementList);
}

void Parser::finiParseTree() {
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

void Parser::onHaltCompiler() {
  if (m_nsState == InsideNamespace && !m_nsFileScope) {
    error("__HALT_COMPILER() can only be used from the outermost scope");
    return;
  }
  // Backpatch instances of __COMPILER_HALT_OFFSET__
  for(auto &cho : m_compilerHaltOffsetVec) {
     cho->setCompilerHaltOffset(m_scanner.getLocation()->cursor);
  }
}

void Parser::onStatementListStart(Token &out) {
  out.reset();
}

void Parser::addTopStatement(Token &new_stmt) {
  addStatement(m_tree, new_stmt->stmt);
}

void Parser::addStatement(Token &out, Token &stmts, Token &new_stmt) {
  if (!stmts->stmt) {
    out->stmt = NEW_STMT0(StatementList);
  } else {
    out->stmt = stmts->stmt;
  }
  addStatement(out->stmt, new_stmt->stmt);
}

void Parser::addStatement(StatementPtr stmt, StatementPtr new_stmt) {
  if (new_stmt) {
    stmt->addElement(new_stmt);
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

void Parser::onBreakContinue(Token &out, bool isBreak, Token* expr) {
  uint64_t depth = 1;

  if (expr) {
    Variant v;
    if (!expr->exp->getScalarValue(v)) {
      PARSE_ERROR("'%s' operator with non-constant operand is no longer "
                  "supported", (isBreak ? "break" : "continue"));
    }
    if (!v.isInteger() || v.toInt64() <= 0) {
      PARSE_ERROR("'%s' operator accepts only positive numbers",
                  (isBreak ? "break" : "continue"));
    }
    depth = static_cast<uint64_t>(v.toInt64());
  }

  if (isBreak) {
    out->stmt = NEW_STMT(BreakStatement, depth);
  } else {
    out->stmt = NEW_STMT(ContinueStatement, depth);
  }
}

void Parser::onReturn(Token &out, Token *expr) {
  out->stmt = NEW_STMT(ReturnStatement, expr ? expr->exp : ExpressionPtr());
  if (!m_funcContexts.empty()) {
    FunctionContext& fc = m_funcContexts.back();
    if (fc.isGenerator) {
      Compiler::Error(InvalidYield, out->stmt);
      PARSE_ERROR("Cannot mix 'return' and 'yield' in the same function");
      return;
    }
    fc.hasReturn = true;
  }
}

void Parser::invalidYield() {
  ExpressionPtr exp(new SimpleFunctionCall(BlockScopePtr(),
                                           getLocation(),
                                           "yield",
                                           false,
                                           ExpressionListPtr(),
                                           ExpressionPtr()));
  Compiler::Error(Compiler::InvalidYield, exp);
}

bool Parser::canBeAsyncOrGenerator(string funcName, string clsName) {
  if (clsName.empty()) {
    return true;
  }
  if (strcasecmp(funcName.c_str(), clsName.c_str()) == 0) {
    return false;
  }
  if (strncmp(funcName.c_str(), "__", 2) == 0) {
    const char *fname = funcName.c_str() + 2;
    if (!strcasecmp(fname, "construct") ||
        !strcasecmp(fname, "destruct") ||
        !strcasecmp(fname, "get") ||
        !strcasecmp(fname, "set") ||
        !strcasecmp(fname, "isset") ||
        !strcasecmp(fname, "unset") ||
        !strcasecmp(fname, "call") ||
        !strcasecmp(fname, "callstatic") ||
        !strcasecmp(fname, "invoke")) {
      return false;
    }
  }
  return true;
}

bool Parser::setIsGenerator() {
  if (m_funcContexts.empty()) {
    invalidYield();
    PARSE_ERROR("Yield can only be used inside a function");
    return false;
  }

  FunctionContext& fc = m_funcContexts.back();
  if (fc.hasReturn) {
    invalidYield();
    PARSE_ERROR("Cannot mix 'return' and 'yield' in the same function");
    return false;
  }
  if (fc.isAsync) {
    invalidYield();
    PARSE_ERROR("'yield' is not allowed in async functions.");
    return false;
  }
  fc.isGenerator = true;

  if (!canBeAsyncOrGenerator(m_funcName, m_clsName)) {
    invalidYield();
    PARSE_ERROR("'yield' is not allowed in constructor, destructor, or "
                "magic methods");
    return false;
  }

  return true;
}

void Parser::onYield(Token &out, Token &expr) {
  if (setIsGenerator()) {
    out->exp = NEW_EXP(YieldExpression, ExpressionPtr(), expr->exp);
  }
}

void Parser::onYieldPair(Token &out, Token &key, Token &val) {
  if (setIsGenerator()) {
    out->exp = NEW_EXP(YieldExpression, key->exp, val->exp);
  }
}

void Parser::onYieldBreak(Token &out) {
  if (setIsGenerator()) {
    out->stmt = NEW_STMT(ReturnStatement, ExpressionPtr());
  }
}

void Parser::invalidAwait() {
  ExpressionPtr exp(new SimpleFunctionCall(BlockScopePtr(),
                                           getLocation(),
                                           "async",
                                           false,
                                           ExpressionListPtr(),
                                           ExpressionPtr()));
  Compiler::Error(Compiler::InvalidAwait, exp);
}

bool Parser::setIsAsync() {
  if (m_funcContexts.empty()) {
    invalidAwait();
    PARSE_ERROR("'await' can only be used inside a function");
    return false;
  }

  FunctionContext& fc = m_funcContexts.back();
  if (fc.isGenerator) {
    invalidAwait();
    PARSE_ERROR("'await' is not allowed in generators.");
    return false;
  }
  fc.isAsync = true;

  if (!canBeAsyncOrGenerator(m_funcName, m_clsName)) {
    invalidAwait();
    PARSE_ERROR("'await' is not allowed in constructors, destructors, or "
                    "magic methods.");
  }

  return true;
}


void Parser::onAwait(Token &out, Token &expr) {
  if (setIsAsync()) {
    out->exp = NEW_EXP(AwaitExpression, expr->exp);
  }
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
    assert(false);
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

void Parser::onForEach(Token &out, Token &arr, Token &name, Token &value,
                       Token &stmt) {
  if (dynamic_pointer_cast<FunctionCall>(name->exp) ||
      dynamic_pointer_cast<FunctionCall>(value->exp)) {
    PARSE_ERROR("Can't use return value in write context");
  }
  if (value->exp && name->num()) {
    PARSE_ERROR("Key element cannot be a reference");
    return;
  }
  checkAssignThis(name);
  checkAssignThis(value);
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
  if (tryStmt->stmt) {
    out->stmt->setLabelScope(stmtList->getLabelScope());
  }
}

void Parser::onTry(Token &out, Token &tryStmt, Token &finallyStmt) {
  out->stmt = NEW_STMT(TryStatement, tryStmt->stmt,
                       dynamic_pointer_cast<StatementList>(NEW_STMT0(StatementList)),
                       finallyStmt->stmt);
  if (tryStmt->stmt) {
    out->stmt->setLabelScope(tryStmt->stmt->getLabelScope());
  }
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
  // TODO (#3271396) This can be greatly improved. In particular
  // even when a finally block exists inside a function it is often
  // the case that the unnamed locals state & ret are not needed.
  // See task description for further details.
  m_file->setAttribute(FileScope::NeedsFinallyLocals);
}

void Parser::onThrow(Token &out, Token &expr) {
  out->stmt = NEW_STMT(ThrowStatement, expr->exp);
}

void Parser::onClosureStart(Token &name) {
  if (!m_funcName.empty()) {
    m_containingFuncName = m_funcName;
  } else {
    // pseudoMain
  }
  onFunctionStart(name, true);
}

Token Parser::onClosure(ClosureType type,
                        Token* modifiers,
                        Token& ref,
                        Token& params,
                        Token& cparams,
                        Token& stmts) {
  Token out;
  Token name;

  Token retIgnore;
  auto stmt = onFunctionHelper(
    FunctionType::Closure,
    modifiers,
    retIgnore,
    ref,
    nullptr,
    params,
    stmts,
    nullptr,
    true
  );

  ExpressionListPtr vars = dynamic_pointer_cast<ExpressionList>(cparams->exp);
  if (vars) {
    for (int i = vars->getCount() - 1; i >= 0; i--) {
      ParameterExpressionPtr param(
        dynamic_pointer_cast<ParameterExpression>((*vars)[i]));
      if (param->getName() == "this") {
        PARSE_ERROR("Cannot use $this as lexical variable");
      }
    }
  }

  ClosureExpressionPtr closure = NEW_EXP(
    ClosureExpression,
    type,
    dynamic_pointer_cast<FunctionStatement>(stmt),
    vars
  );
  closure->getClosureFunction()->setContainingClosure(closure);
  out->exp = closure;

  return out;
}

Token Parser::onExprForLambda(const Token& expr) {
  auto stmt = NEW_STMT(ReturnStatement, expr.exp);
  Token ret;
  ret.stmt = NEW_STMT0(StatementList);
  ret.stmt->addElement(stmt);
  return ret;
}

void Parser::onClosureParam(Token &out, Token *params, Token &param,
                            bool ref) {
  ExpressionPtr expList;
  if (params) {
    expList = params->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(NEW_EXP(ParameterExpression, TypeAnnotationPtr(),
                              m_scanner.isHHSyntaxEnabled(), param->text(),
                              ref, 0, ExpressionPtr(), ExpressionPtr()));
  out->exp = expList;
}

void Parser::onLabel(Token &out, Token &label) {
  out->stmt = NEW_STMT(LabelStatement, label.text());
}

void Parser::onGoto(Token &out, Token &label, bool limited) {
  out->stmt = NEW_STMT(GotoStatement, label.text());
}

void Parser::onTypedef(Token& out, const Token& name, const Token& type) {
  // Note: we don't always get TypeAnnotations (e.g. for shape types
  // currently).
  auto const annot = type.typeAnnotation
    ? type.typeAnnotation
    : std::make_shared<TypeAnnotation>(type.text(), TypeAnnotationPtr());

  auto td_stmt = NEW_STMT(TypedefStatement, name.text(), annot);
  td_stmt->onParse(m_ar, m_file);
  out->stmt = td_stmt;
}

void Parser::onTypeAnnotation(Token& out, const Token& name,
                                          const Token& typeArgs) {
  out.set(name.num(), name.text());
  out.typeAnnotation = TypeAnnotationPtr(
    new TypeAnnotation(name.text(), typeArgs.typeAnnotation));
  if (isTypeVar(name.text())) {
    out.typeAnnotation->setTypeVar();
  }
}

void Parser::onTypeList(Token& type1, const Token& type2) {
  if (!type1.typeAnnotation) {
    PARSE_ERROR("Missing type in type list");
  }
  if (type2.num() != 0 && !type1.typeAnnotation) {
    PARSE_ERROR("Missing type in type list");
  }
  if (type2.typeAnnotation) {
    type1.typeAnnotation->appendToTypeList(type2.typeAnnotation);
  }
}

void Parser::onTypeSpecialization(Token& type, char specialization) {
  if (type.typeAnnotation) {
    switch (specialization) {
    case '?':
      type.typeAnnotation->setNullable();
      break;
    case '@':
      type.typeAnnotation->setSoft();
      break;
    case 't':
      type.typeAnnotation->setTuple();
      break;
    case 'f':
      type.typeAnnotation->setFunction();
      break;
    case 'x':
      type.typeAnnotation->setXHP();
      break;
    }
  }
}

void Parser::onQuery(Token &out, Token &head, Token &body) {
  out->exp = NEW_EXP(QueryExpression, head.exp, body.exp);
}

void appendList(ExpressionListPtr expList, Token *exps) {
  if (exps != nullptr) {
    assert(exps->exp->is(Expression::KindOfExpressionList));
    ExpressionListPtr el(static_pointer_cast<ExpressionList>(exps->exp));
    for (unsigned int i = 0; i < el->getCount(); i++) {
      if ((*el)[i]) expList->addElement((*el)[i]);
    }
  }
}

void Parser::onQueryBody(
  Token &out, Token *clauses, Token &select, Token *cont) {
  ExpressionListPtr expList(NEW_EXP0(ExpressionList));
  appendList(expList, clauses);
  expList->addElement(select.exp);
  if (cont != nullptr) expList->addElement(cont->exp);
  out->exp = expList;
}

void Parser::onQueryBodyClause(Token &out, Token *clauses, Token &clause) {
  ExpressionPtr expList;
  if (clauses && clauses->exp) {
    expList = clauses->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(clause->exp);
  out->exp = expList;
}

void Parser::onFromClause(Token &out, Token &var, Token &coll) {
  out->exp = NEW_EXP(FromClause, var.text(), coll.exp);
}

void Parser::onLetClause(Token &out, Token &var, Token &expr) {
  out->exp = NEW_EXP(LetClause, var.text(), expr.exp);
}

void Parser::onWhereClause(Token &out, Token &expr) {
  out->exp = NEW_EXP(WhereClause, expr.exp);
}

void Parser::onJoinClause(Token &out, Token &var, Token &coll,
  Token &left, Token &right) {
  out->exp = NEW_EXP(JoinClause, var.text(), coll.exp,
                     left.exp, right.exp, "");
}

void Parser::onJoinIntoClause(Token &out, Token &var, Token &coll,
  Token &left, Token &right, Token &group) {
  out->exp = NEW_EXP(JoinClause, var.text(), coll.exp,
                     left.exp, right.exp, group.text());
}

void Parser::onOrderbyClause(Token &out, Token &orderings) {
  out->exp = NEW_EXP(OrderbyClause, orderings.exp);
}

void Parser::onOrdering(Token &out, Token *orderings, Token &ordering) {
  ExpressionPtr expList;
  if (orderings && orderings->exp) {
    expList = orderings->exp;
  } else {
    expList = NEW_EXP0(ExpressionList);
  }
  expList->addElement(ordering->exp);
  out->exp = expList;
}

void Parser::onOrderingExpr(Token &out, Token &expr, Token *direction) {
  out->exp = NEW_EXP(Ordering, expr.exp, (direction) ? direction->num() : 0);
}

void Parser::onSelectClause(Token &out, Token &expr) {
  out->exp = NEW_EXP(SelectClause, expr.exp);
}

void Parser::onGroupClause(Token &out, Token &coll, Token &key) {
  out->exp = NEW_EXP(GroupClause, coll.exp, key.exp);
}

void Parser::onIntoClause(Token &out, Token &var, Token &query) {
  out->exp = NEW_EXP(IntoClause, var.text(), query.exp);
}

///////////////////////////////////////////////////////////////////////////////
// namespace support

//////////////////// AliasTable /////////////////////

Parser::AliasTable::AliasTable(const std::vector<AliasEntry>& autoAliases,
                               std::function<bool ()> autoOracle) :
  m_autoAliases(autoAliases), m_autoOracle(autoOracle),
  m_alreadyImported(false) {

  if (!m_autoOracle) {
    setFalseOracle();
  }
}

void Parser::AliasTable::setFalseOracle() {
  m_autoOracle = [] () { return false; };
}

/*
 * Add the auto-imports to the map.
 * This is called by all the other operations, so the auto-import occurs
 * on-demand.
 */
void Parser::AliasTable::addAutoImports() {
  if (!m_alreadyImported && m_autoOracle()) {
    m_alreadyImported = true;

    for (auto entry : m_autoAliases) {
      m_aliases[entry.alias] = (NameEntry){entry.name, AliasType::AUTO};
    }
  }
}

std::string Parser::AliasTable::getName(std::string alias) {
  addAutoImports();
  auto it = m_aliases.find(alias);
  return (m_aliases.end() == it) ? "" : it->second.name;
}

bool Parser::AliasTable::isAliased(std::string alias) {
  addAutoImports();
  return m_aliases.find(alias) != m_aliases.end();
}

bool Parser::AliasTable::isAutoImported(std::string alias) {
  addAutoImports();
  auto it = m_aliases.find(alias);
  return it != m_aliases.end() && (it->second.type == AliasType::AUTO);
}

bool Parser::AliasTable::isUseType(std::string alias) {
  auto it = m_aliases.find(alias);
  return it != m_aliases.end() && (it->second.type == AliasType::USE);
}

void Parser::AliasTable::map(std::string alias,
                             std::string name,
                             AliasType type) {
  addAutoImports();
  m_aliases[alias] = (NameEntry){name, type};
}
/*
 * To be called when we enter a fresh namespace.
 */
void Parser::AliasTable::clear() {
  m_aliases.clear();
  m_alreadyImported = false;
}

//////////////////////////////////////////////////////


/*
 * We auto-alias classes on HH mode and only if we're in the global namespace.
 */
bool Parser::isAutoAliasOn() {
  return m_scanner.isHHSyntaxEnabled() && ("" == m_namespace);
}

std::vector<Parser::AliasTable::AliasEntry> Parser::getAutoAliasedClasses() {
  typedef AliasTable::AliasEntry AliasEntry;

  std::vector<AliasEntry> aliases {
    (AliasEntry){"Traversable", "HH\\Traversable"},
    (AliasEntry){"Iterator", "HH\\Iterator"},
    (AliasEntry){"Collection", "HH\\Collection"},
    (AliasEntry){"Vector", "HH\\Vector"},
    (AliasEntry){"Set", "HH\\Set"},
    (AliasEntry){"FrozenVector", "HH\\FrozenVector"},
    (AliasEntry){"FrozenSet", "HH\\FrozenSet"}
  };

  return aliases;
}

void Parser::nns(int token) {
  if (m_nsState == SeenNamespaceStatement && token != ';') {
    error("No code may exist outside of namespace {}: %s",
          getMessage().c_str());
    return;
  }
  if (m_nsState == SeenNothing && token != T_DECLARE && token != ';') {
    m_nsState = SeenNonNamespaceStatement;
  }
}

void Parser::onNamespaceStart(const std::string &ns,
                              bool file_scope /* =false */) {
  if (m_nsState == SeenNonNamespaceStatement) {
    error("Namespace declaration statement has to be the very first "
          "statement in the script: %s", getMessage().c_str());
    return;
  }
  if (m_nsState != SeenNothing && file_scope != m_nsFileScope) {
    error("Cannot mix bracketed namespace declarations with unbracketed "
          "namespace declarations");
  }

  m_nsState = InsideNamespace;
  m_nsFileScope = file_scope;
  pushComment();
  m_namespace = ns;
  m_aliasTable.clear();
}

void Parser::onNamespaceEnd() {
  m_nsState = SeenNamespaceStatement;
}

void Parser::onUse(const std::string &ns, const std::string &as) {
  string key = as;
  if (key.empty()) {
    size_t pos = ns.rfind(NAMESPACE_SEP);
    if (pos == string::npos) {
      key = ns;
    } else {
      key = ns.substr(pos + 1);
    }
  }

  // It's not an error if the alias already exists but is auto-imported.
  // In that case, it gets replaced. It prompts an error if it is not
  // auto-imported and 'use' statement is trying to replace it.
  if (m_aliasTable.isAliased(key) && !m_aliasTable.isAutoImported(key) &&
      (m_aliasTable.isUseType(key) || m_aliasTable.getName(key) != ns)) {
    error("Cannot use %s as %s because the name is already in use: %s",
          ns.c_str(), key.c_str(), getMessage().c_str());
    return;
  }

  m_aliasTable.map(key, ns, AliasTable::AliasType::USE);
}

std::string Parser::nsDecl(const std::string &name) {
  if (m_namespace.empty()) {
    return name;
  }
  return m_namespace + NAMESPACE_SEP + name;
}

std::string Parser::resolve(const std::string &ns, bool cls) {
  string alias = ns;
  size_t pos = ns.find(NAMESPACE_SEP);
  if (pos != string::npos) {
    alias = ns.substr(0, pos);
  }

  if (m_aliasTable.isAliased(alias)) {
    auto name = m_aliasTable.getName(alias);

    // Was it a namespace alias?
    if (pos != string::npos) {
      return name + ns.substr(pos);
    }
    // Only classes can appear directly in "use" statements
    if (cls) {
      return name;
    }
  }

  // Classes don't fallback to the global namespace.
  if (cls) {
    if (!strcasecmp("self", ns.c_str()) ||
        !strcasecmp("parent", ns.c_str())) {
      return ns;
    }
    return nsDecl(ns);
  }

  // if qualified name, prepend current namespace
  if (pos != string::npos) {
    return nsDecl(ns);
  }

  // unqualified name in global namespace
  if (m_namespace.empty()) {
    return ns;
  }

  if (!strcasecmp("true", ns.c_str()) ||
      !strcasecmp("false", ns.c_str()) ||
      !strcasecmp("null", ns.c_str())) {
    return ns;
  }
  return nsDecl(ns);
}

void Parser::invalidateGoto(TStatementPtr stmt, GotoError error) {
  GotoStatement *gs = (GotoStatement*) stmt;
  assert(gs);
  gs->invalidate(error);
}

void Parser::invalidateLabel(TStatementPtr stmt) {
  LabelStatement *ls = (LabelStatement*) stmt;
  assert(ls);
  ls->invalidate();
}

TStatementPtr Parser::extractStatement(ScannerToken *stmt) {
  Token *t = (Token*) stmt;
  return t->stmt.get();
}

///////////////////////////////////////////////////////////////////////////////

bool Parser::hasType(Token &type) {
  if (!type.text().empty()) {
    if (!m_scanner.isHHSyntaxEnabled()) {
      PARSE_ERROR("Type hint is not enabled");
      return false;
    }
    return true;
  }
  return false;
}

void Parser::registerAlias(std::string name) {
  size_t pos = name.rfind(NAMESPACE_SEP);
  if (pos != string::npos) {
    string key = name.substr(pos + 1);
    m_aliasTable.map(key, name, AliasTable::AliasType::CURRENT_NS);
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
