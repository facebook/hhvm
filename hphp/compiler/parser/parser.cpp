/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <vector>

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
#include "hphp/compiler/statement/class_require_statement.h"
#include "hphp/compiler/statement/trait_prec_statement.h"
#include "hphp/compiler/statement/trait_alias_statement.h"
#include "hphp/compiler/statement/typedef_statement.h"

#include "hphp/compiler/analysis/function_scope.h"

#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/analysis_result.h"

#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/text-util.h"
#include "hphp/util/string-vsnprintf.h"

#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/annot-type.h"

#define NEW_EXP0(cls)                                           \
  std::make_shared<cls>(BlockScopePtr(),                        \
                        getRange())
#define NEW_EXP(cls, ...)                                      \
  std::make_shared<cls>(BlockScopePtr(),                        \
                        getRange(), ##__VA_ARGS__)
#define NEW_STMT0(cls)                                          \
  std::make_shared<cls>(BlockScopePtr(), getLabelScope(),       \
                        getRange())
#define NEW_STMT(cls, ...)                                     \
  std::make_shared<cls>(BlockScopePtr(), getLabelScope(),       \
                        getRange(), ##__VA_ARGS__)

#define PARSE_ERROR(fmt, ...)  HPHP_PARSER_ERROR(fmt, this, ##__VA_ARGS__)

using namespace HPHP::Compiler;

namespace HPHP {

SimpleFunctionCallPtr NewSimpleFunctionCall(
  EXPRESSION_CONSTRUCTOR_PARAMETERS,
  const std::string &name, bool hadBackslash, ExpressionListPtr params,
  ExpressionPtr cls) {
  return
    std::make_shared<SimpleFunctionCall>(
      EXPRESSION_CONSTRUCTOR_DERIVED_PARAMETER_VALUES,
      name, hadBackslash, params, cls);
}

static std::string fully_qualified_name_as_alias_key(const std::string &fqn,
                                                     const std::string &as) {
  string key = as;
  if (key.empty()) {
    size_t pos = fqn.rfind(NAMESPACE_SEP);
    if (pos == string::npos) {
      key = fqn;
    } else {
      key = fqn.substr(pos + 1);
    }
  }

  return key;
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
                parser.getMessage(false,true).c_str(), input.data());
  return StatementListPtr();
}

///////////////////////////////////////////////////////////////////////////////

Parser::Parser(Scanner &scanner, const char *fileName,
               AnalysisResultPtr ar, int fileSize /* = 0 */)
    : ParserBase(scanner, fileName), m_ar(ar), m_lambdaMode(false),
      m_closureGenerator(false), m_nsState(SeenNothing),
      m_nsAliasTable(getAutoAliasedClasses(), [&] { return isAutoAliasOn(); }) {
  auto const md5str = mangleUnitMd5(scanner.getMd5());
  MD5 md5 = MD5(md5str.c_str());

  m_file = std::make_shared<FileScope>(m_fileName, fileSize, md5);

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
    if (scanner().isHHFile()) {
      m_file->setHHFile();
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
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  fatal(&m_loc, msg.c_str());
}

void Parser::parseFatal(const Location* loc, const char* msg) {
  // If the parser has a message, prepend it to the given message. Otherwise
  // just use the given message.
  std::string str = getMessage();
  std::string strInput;
  if (!str.empty()) {
    strInput = str;
    strInput += "\n";
  }
  strInput += msg;

  auto exn = ParseTimeFatalException(m_file->getName(), loc->r.line0,
                                     "%s", strInput.c_str());

  exn.setParseFatal();
  throw exn;
}

void Parser::fatal(const Location* loc, const char* msg) {
  throw ParseTimeFatalException(m_file->getName(), loc->r.line0, "%s", msg);
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
  auto labelScope = std::make_shared<LabelScope>();
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

void Parser::onClassAbstractConstant(Token &out, Token *exprs, Token &var) {
  onVariable(out, exprs, var, nullptr, true, m_scanner.detachDocComment());
}

void Parser::onClassTypeConstant(Token &out, Token &var, Token &value) {
  Token typeConst;
  bool isAbstract = value.typeAnnotationName() == "";

  if (isAbstract) {
    onClassAbstractConstant(typeConst, nullptr, var);
  } else {
    value.setText(value.typeAnnotationName());
    Token typeConstValue;
    onScalar(typeConstValue, T_STRING, value);

    onClassConstant(typeConst, nullptr, var, typeConstValue);
  }

  onClassVariableStart(out, nullptr, typeConst, nullptr, isAbstract, true);
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

void Parser::onCallParam(Token &out, Token *params, Token &expr,
                         bool ref, bool unpack) {
  if (!params) {
    out->exp = NEW_EXP0(ExpressionList);
  } else {
    out->exp = params->exp;
  }
  if (ref) {
#ifdef FACEBOOK
    // TODO t#6485898 - Remove uses of call time pass by reference
    expr->exp->setContext(Expression::RefParameter);
    expr->exp->setContext(Expression::RefValue);
#else
    PARSE_ERROR("Call-time pass-by-reference has been removed");
#endif
  }
  if (unpack) {
    (dynamic_pointer_cast<ExpressionList>(out->exp))->setContainsUnpack();
    expr->exp->setIsUnpack();
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
    auto call = NEW_EXP(DynamicFunctionCall, name->exp,
                        dynamic_pointer_cast<ExpressionList>(params->exp),
                        clsExp);
    call->onParse(m_ar, m_file);
    out->exp = call;
  } else {
    string funcName = name.text();
    // strip out namespaces for func_get_args and friends check
    size_t lastBackslash = funcName.find_last_of(NAMESPACE_SEP);
    string stripped = lastBackslash == string::npos
                      ? funcName
                      : funcName.substr(lastBackslash+1);
    bool hadBackslash = name->num() & 2;

    if (stripped == "set_frame_metadata" && m_funcContexts.size() > 0) {
      m_funcContexts.back().mayCallSetFrameMetadata = true;
    }

    if (!cls && !hadBackslash) {
      if (stripped == "func_num_args" ||
          stripped == "func_get_args" ||
          stripped == "func_get_arg") {
        funcName = stripped;
        if (m_funcContexts.size() > 0) {
          m_funcContexts.back().hasCallToGetArgs = true;
        }
      }
      // Auto import a few functions from the HH namespace
      // TODO(#4245628): merge those into m_fnAliasTable
      if (isAutoAliasOn() &&
          (stripped == "fun" ||
           stripped == "meth_caller" ||
           stripped == "class_meth" ||
           stripped == "inst_meth" ||
           stripped == "invariant_callback_register" ||
           stripped == "invariant" ||
           stripped == "invariant_violation" ||
           stripped == "idx" ||
           stripped == "asio_get_current_context_idx" ||
           stripped == "asio_get_running_in_context" ||
           stripped == "asio_get_running" ||
           stripped == "xenon_get_data" ||
           stripped == "objprof_get_strings" ||
           stripped == "objprof_get_data" ||
           stripped == "objprof_start" ||
           stripped == "server_warmup_status"
          )) {
        funcName = "HH\\" + stripped;
      }

      auto alias = m_fnAliasTable.find(stripped);
      if (alias != m_fnAliasTable.end()) {
        funcName = alias->second;
      }
    }

    auto call = NEW_EXP(SimpleFunctionCall,
                        funcName, hadBackslash,
                        dynamic_pointer_cast<ExpressionList>(params->exp),
                        clsExp);
    out->exp = call;

    call->onParse(m_ar, m_file);
  }
}

///////////////////////////////////////////////////////////////////////////////
// object property and method calls

void Parser::onObjectProperty(Token &out, Token &base,
                              PropAccessType propAccessType, Token &prop) {
    if (prop.num() == ObjPropXhpAttr) {
    // Handle "$obj->:xhp-attr" transform
    ExpressionListPtr paramsExp = NEW_EXP0(ExpressionList);
    ScalarExpressionPtr name =
      NEW_EXP(ScalarExpression, T_CONSTANT_ENCAPSED_STRING,
              prop->text(), true);
    paramsExp->addElement(name);
    ScalarExpressionPtr getAttributeMethodName =
      NEW_EXP(ScalarExpression, T_STRING, std::string("getAttribute"));
    auto om = NEW_EXP(ObjectMethodExpression, base->exp,
                      getAttributeMethodName, paramsExp,
                      propAccessType == PropAccessType::NullSafe);
    om->onParse(m_ar, m_file);
    om->setIsXhpGetAttr();
    out->exp = om;
    return;
  }
  if (!prop->exp) {
    prop->exp = NEW_EXP(ScalarExpression, T_STRING, prop->text());
  }

  if (propAccessType == PropAccessType::NullSafe) {
    // $this?->foo is disallowed.
    checkThisContext(base.exp, ThisContextError::NullSafeBase);

    if (prop->exp->getKindOf() != Expression::KindOfScalarExpression) {
      PARSE_ERROR("?-> can only be used with scalar property names");
    }
  }

  out->exp = NEW_EXP(
    ObjectPropertyExpression,
    base->exp,
    prop->exp,
    propAccessType
  );
}

void Parser::onObjectMethodCall(Token &out, Token &base, bool nullsafe,
                                Token &prop, Token &params) {
  if (!prop->exp) {
    prop->exp = NEW_EXP(ScalarExpression, T_STRING, prop->text());
  }
  ExpressionListPtr paramsExp;
  if (params->exp) {
    paramsExp = dynamic_pointer_cast<ExpressionList>(params->exp);
  } else {
    paramsExp = NEW_EXP0(ExpressionList);
  }
  auto mcall = NEW_EXP(ObjectMethodExpression, base->exp, prop->exp, paramsExp,
                       nullsafe);
  mcall->onParse(m_ar, m_file);
  out->exp = mcall;
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

void Parser::encapObjProp(Token &out, Token &var,
                          PropAccessType propAccessType, Token &name) {
  ExpressionPtr obj = NEW_EXP(SimpleVariable, var->text());

  ExpressionPtr prop = NEW_EXP(ScalarExpression, T_STRING, name->text());
  out->exp = NEW_EXP(
    ObjectPropertyExpression, obj, prop, propAccessType
  );
}

void Parser::encapArray(Token &out, Token &var, Token &expr) {
  ExpressionPtr arr = NEW_EXP(SimpleVariable, var->text());
  out->exp = NEW_EXP(ArrayElementExpression, arr, expr->exp);
}

///////////////////////////////////////////////////////////////////////////////
// expressions

void Parser::onConstantValue(Token &out, Token &constant) {
  const auto& alias = m_cnstAliasTable.find(constant.text());
  if (alias != m_cnstAliasTable.end()) {
    constant.setText(alias->second);
  }

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
    case T_CLASS_C:
      if (m_inTrait) {
        // Inside traits we already did the magic for static::class so lets
        // reuse that
        out->exp = NEW_EXP(SimpleFunctionCall, "get_class", true,
                           ExpressionListPtr(), ExpressionPtr());
        return;
      }
      // fallthrough
    case T_STRING:
    case T_LNUMBER:
    case T_DNUMBER:
    case T_ONUMBER:
    case T_LINE:
    case T_COMPILER_HALT_OFFSET:
    case T_FUNC_C:
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

void Parser::checkAllowedInWriteContext(ExpressionPtr e) {
  if (e == nullptr) {
    return;
  }
  if (dynamic_pointer_cast<FunctionCall>(e)) {
    if (e->is(Expression::KindOfObjectMethodExpression)) {
      ObjectMethodExpressionPtr om =
        static_pointer_cast<ObjectMethodExpression>(e);
      if (om->isXhpGetAttr()) {
        PARSE_ERROR("Using ->: syntax in write context is not supported");
      }
    }
    PARSE_ERROR("Can't use return value in write context");
  } if (e->is(Expression::KindOfObjectPropertyExpression)) {
    ObjectPropertyExpressionPtr op(
      static_pointer_cast<ObjectPropertyExpression>(e)
    );
    if (op->isNullSafe()) {
      PARSE_ERROR(Strings::NULLSAFE_PROP_WRITE_ERROR);
    }
  }
}

void Parser::onListAssignment(Token &out, Token &vars, Token *expr,
                              bool rhsFirst /* = false */) {
  ExpressionListPtr el(dynamic_pointer_cast<ExpressionList>(vars->exp));
  for (int i = 0; i < el->getCount(); i++) {
    checkAllowedInWriteContext((*el)[i]);
    checkThisContext((*el)[i], ThisContextError::Assign);
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

void Parser::checkThisContext(string var, ThisContextError error) {
  if (var != "this") {
    return;
  }

  switch (error) {
    case ThisContextError::Assign:
      PARSE_ERROR(Strings::ASSIGN_THIS_ERROR);
      break;
    case ThisContextError::NullSafeBase:
      PARSE_ERROR(Strings::NULLSAFE_THIS_BASE_ERROR);
      break;
  }
}

void Parser::checkThisContext(Token &var, ThisContextError error) {
  if (SimpleVariablePtr simp = dynamic_pointer_cast<SimpleVariable>(var.exp)) {
    checkThisContext(simp->getName(), error);
  }
}

void Parser::checkThisContext(ExpressionPtr e, ThisContextError error) {
  if (SimpleVariablePtr simp = dynamic_pointer_cast<SimpleVariable>(e)) {
    checkThisContext(simp->getName(), error);
  }
}

void Parser::checkThisContext(ExpressionListPtr params,
                              ThisContextError error) {
  for (int i = 0, count = params->getCount(); i < count; i++) {
    ParameterExpressionPtr param =
        dynamic_pointer_cast<ParameterExpression>((*params)[i]);
    checkThisContext(param->getName(), error);
  }
}

void Parser::onAssign(Token &out, Token &var, Token &expr, bool ref,
                      bool rhsFirst /* = false */) {
  checkAllowedInWriteContext(var->exp);
  checkThisContext(var, ThisContextError::Assign);
  out->exp = NEW_EXP(AssignmentExpression, var->exp, expr->exp, ref, rhsFirst);
}

void Parser::onAssignNew(Token &out, Token &var, Token &name, Token &args) {
  checkAllowedInWriteContext(var->exp);
  checkThisContext(var, ThisContextError::Assign);
  auto exp = NEW_EXP(NewObjectExpression, name->exp,
                     dynamic_pointer_cast<ExpressionList>(args->exp));
  exp->onParse(m_ar, m_file);
  out->exp = NEW_EXP(AssignmentExpression, var->exp, exp, true);
}

void Parser::onNewObject(Token &out, Token &name, Token &args) {
  auto new_obj = NEW_EXP(NewObjectExpression, name->exp,
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
    checkAllowedInWriteContext(operand->exp);
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

  if (bop->isAssignmentOp()) {
    checkAllowedInWriteContext(operand1->exp);
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

void Parser::onConst(Token &out, Token &name, Token &value) {
  // Convert to a define call
  Token sname;   onScalar(sname, T_CONSTANT_ENCAPSED_STRING, name);

  Token fname;   fname.setText("define");
  Token params1; onCallParam(params1, nullptr, sname, false, false);
  Token params2; onCallParam(params2, &params1, value, false, false);
  Token call;    onCall(call, false, fname, params2, nullptr);
  Token expr;    onExpStatement(expr, call);

  addTopStatement(expr);

  m_cnstTable.insert(name.text());
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

void Parser::onClassClass(Token &out, Token &cls, Token &name,
                          bool inStaticContext) {
  if (inStaticContext) {
    if (cls->same("parent") || cls->same("static")) {
      PARSE_ERROR(
        "%s::class cannot be used for compile-time class name resolution",
        cls->text().c_str()
      );
    }
  }
  if (cls->exp && !cls->exp->is(Expression::KindOfScalarExpression)) {
    PARSE_ERROR("::class can only be used on scalars");
  }
  if (cls->same("self") || cls->same("parent") || cls->same("static")) {
    if (cls->same("self") && m_inTrait) {
      // Sooo... self:: works dynamically for everything in a trait except
      // for self::CLASS where it returns the trait name. Great...
      onScalar(out, T_TRAIT_C, cls);
    } else {
      onClassConst(out, cls, name, inStaticContext);
    }
  } else {
    onScalar(out, T_STRING, cls);
  }
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
    invalidAwait();
    PARSE_ERROR("Function '%s' contains 'await' but is not declared as async.",
                funcName.c_str());
  }

  if (modifiers->isAsync() && returnsRef) {
    PARSE_ERROR("Asynchronous function '%s' cannot return reference.",
                funcName.c_str());
  }

  if (modifiers->isAsync() && !canBeAsyncOrGenerator(funcName, m_clsName)) {
    PARSE_ERROR("cannot declare constructors, destructors, and "
                    "magic methods such as '%s' as async",
                funcName.c_str());
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
      param->parseTimeFatal(getFileScope(),
                            Compiler::InvalidAttribute,
                            "parameter modifiers not allowed on "
                            "abstract __construct");
    }
    if (!stmts) {
      param->parseTimeFatal(getFileScope(),
                            Compiler::InvalidAttribute,
                            "parameter modifiers not allowed on "
                            "__construct without a body");
    }
    if (param->annotation()) {
      std::vector<std::string> typeNames;
      param->annotation()->getAllSimpleNames(typeNames);
      for (auto& typeName : typeNames) {
        if (isTypeVarInImmediateScope(typeName)) {
          param->parseTimeFatal(getFileScope(),
                                Compiler::InvalidAttribute,
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
        NEW_EXP(ObjectPropertyExpression, self, prop, PropAccessType::Normal);
    AssignmentExpressionPtr assign =
        NEW_EXP(AssignmentExpression, objProp, value, false);
    ExpStatementPtr stmt = NEW_STMT(ExpStatement, assign);
    stmts->insertElement(stmt);
  }
}

string Parser::getFunctionName(FunctionType type, Token* name) {
  switch (type) {
    case FunctionType::Closure:
      return newClosureName(m_namespace, m_clsName, m_containingFuncName);
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
    PARSE_ERROR("Invalid modifier on closure function.");
  }
  if (type == FunctionType::Function) {
    if (!modifiersExp->validForFunction()) {
      PARSE_ERROR("Invalid modifier on function %s.", name->text().c_str());
    }

    m_fnTable.insert(name->text());
  }

  StatementListPtr stmts = stmt->stmt || stmt->num() != 1 ?
    dynamic_pointer_cast<StatementList>(stmt->stmt)
    : NEW_STMT0(StatementList);

  ExpressionListPtr old_params =
    dynamic_pointer_cast<ExpressionList>(params->exp);

  if (type == FunctionType::Method && old_params &&
     !modifiersExp->isStatic()) {
    checkThisContext(old_params, ThisContextError::Assign);
  }

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
  mth->setHasCallToGetArgs(funcContext.hasCallToGetArgs);
  mth->setMayCallSetFrameMetadata(funcContext.mayCallSetFrameMetadata);
  mth->getFunctionScope()->setGenerator(funcContext.isGenerator);
  mth->getFunctionScope()->setAsync(modifiersExp->isAsync());
  m_funcContexts.pop_back();

  auto loc = popFuncLocation();
  if (reloc) {
    mth->setFirst(loc.line0, loc.char0);
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

void Parser::onVariadicParam(Token &out, Token *params,
                             Token &type, Token &var,
                             bool ref, Token *attr, Token *modifier) {
  if (!type.text().empty()) {
    PARSE_ERROR("Parameter $%s is variadic and has a type constraint (%s)"
                "; variadic params with type constraints are not supported",
                var.text().c_str(), type.text().c_str());
  }
  if (ref) {
    PARSE_ERROR("Parameter $%s is both variadic and by reference"
                "; this is unsupported",
                var.text().c_str());
  }

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
                              ExpressionPtr(),
                              attrList,
                              /* variadic */ true));
  out->exp = expList;
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

void Parser::checkClassDeclName(const std::string& name) {
  // Check if name conflicts with a reserved typehint. This throws an
  // error for the following cases:
  //   1) "self" or "parent" in any namespace. Namespace resolution
  //      specially recognizes "self" and "parent" and doesn't prepend
  //      a prefix, we don't have to worry about stripping prefixes.
  //   2) A Hack-specific reserved typehint while in the HH namespace.
  //   3) A Hack-specific reserved typehint while in the global namespace
  //      when HH syntax is enabled.
  // Note that "array" and "callable" are disallowed by the grammar,
  // so they never reach here.
  bool isHHNamespace = (strcasecmp(m_namespace.c_str(), "HH") == 0);
  auto const* at = nameToAnnotType(
    [&]() -> const std::string& {
      if (isHHNamespace ||
          (m_namespace.empty() && m_scanner.isHHSyntaxEnabled())) {
        auto const& autoAliases = getAutoAliasedClasses();
        // For the HH namespace, it's important to apply the Hack auto-
        // alias rules so that we catch cases involving synonyms such
        // as "class Boolean {..}".
        auto it = autoAliases.find(
          // "self" and "parent" are treated specially when namespace
          // resolution is performed, so when we're in the HH namespace
          // we can't just assume the name starts with "HH\", we need
          // to actually check.
          (isHHNamespace && boost::starts_with(name, "HH\\"))
            ? name.substr(3) : name);
        if (it != autoAliases.end()) {
          return it->second;
        }
      }
      return name;
    }()
  );
  if (at) {
    switch (*at) {
      case AnnotType::Uninit:
      case AnnotType::Null:
      case AnnotType::Bool:
      case AnnotType::Int:
      case AnnotType::Float:
      case AnnotType::String:
      case AnnotType::Resource:
      case AnnotType::Mixed:
      case AnnotType::Number:
      case AnnotType::ArrayKey:
        if (!m_scanner.isHHSyntaxEnabled() && !isHHNamespace) {
          // If HH syntax is not enabled and we're not in the HH namespace,
          // allow Hack-specific reserved names such "string" to be used
          break;
        }
        // Fall though to the call to PARSE_ERROR() below
      case AnnotType::Array:
      case AnnotType::Self:
      case AnnotType::Parent:
      case AnnotType::Callable:
        PARSE_ERROR("Cannot use '%s' as class name as it is reserved",
                    name.c_str());
        break;
      case AnnotType::Object:
        // nameToAnnotType() never returns Object
        not_reached();
    }
  }
}

void Parser::onClassStart(int type, Token &name) {
  // Check if the name conflicts with a reserved typehint.
  checkClassDeclName(name.text());

  pushComment();
  newScope();
  m_clsName = name.text();
  m_inTrait = type == T_TRAIT;
}

void Parser::onClass(Token &out, int type, Token &name, Token &base,
                     Token &baseInterface, Token &stmt, Token *attr,
                     Token *enumBase) {
  StatementListPtr stmtList;
  if (stmt->stmt) {
    stmtList = dynamic_pointer_cast<StatementList>(stmt->stmt);
  }
  ExpressionListPtr attrList;
  if (attr && attr->exp) {
    attrList = dynamic_pointer_cast<ExpressionList>(attr->exp);
  }
  TypeAnnotationPtr enumBaseTy;
  if (enumBase) {
    enumBaseTy = enumBase->typeAnnotation;
  }

  ClassStatementPtr cls = NEW_STMT
    (ClassStatement, type, name->text(), base->text(),
     dynamic_pointer_cast<ExpressionList>(baseInterface->exp),
     popComment(), stmtList, attrList, enumBaseTy);

  // look for argument promotion in ctor
  ExpressionListPtr promote = NEW_EXP(ExpressionList);
  cls->checkArgumentsToPromote(m_file, promote, type);
  auto count = promote->getCount();
  cls->setPromotedParameterCount(count);
  for (int i = 0; i < count; i++) {
    auto param =
        dynamic_pointer_cast<ParameterExpression>((*promote)[i]);
    TokenID mod = param->getModifier();
    std::string name = param->getName();
    std::string type = param->hasUserType() ?
                                  param->getUserTypeHint() : "";

    // create the class variable and change the location to
    // point to the parameter location for error reporting
    auto range = param->getRange();
    ModifierExpressionPtr modifier = std::make_shared<ModifierExpression>(
      BlockScopePtr(), range);
    modifier->add(mod);
    SimpleVariablePtr svar = std::make_shared<SimpleVariable>(
      BlockScopePtr(), range, name);
    ExpressionListPtr expList = std::make_shared<ExpressionList>(
      BlockScopePtr(), range);
    expList->addElement(svar);
    ClassVariablePtr var = std::make_shared<ClassVariable>(
      BlockScopePtr(), getLabelScope(), range, modifier, type, expList);
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

void Parser::onEnum(Token &out, Token &name, Token &baseTy,
                    Token &stmt, Token *attr) {
  Token dummyBase, dummyInterface;
  dummyBase.setText("HH\\BuiltinEnum");
  onClass(out, T_ENUM, name, dummyBase, dummyInterface, stmt, attr, &baseTy);
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

void Parser::onClassRequire(Token &out, Token &name, bool isExtends) {
  out->stmt = NEW_STMT(ClassRequireStatement, name->text(), isExtends);
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
                                  Token *type, bool abstract /* = false */,
                                  bool typeconst /* = false */) {
  if (modifiers) {
    ModifierExpressionPtr exp = modifiers->exp ?
      dynamic_pointer_cast<ModifierExpression>(modifiers->exp)
      : NEW_EXP0(ModifierExpression);

    out->stmt = NEW_STMT(
      ClassVariable, exp,
      (type) ? type->typeAnnotationName() : "",
      dynamic_pointer_cast<ExpressionList>(decl->exp));
  } else {
    out->stmt = NEW_STMT(
      ClassConstant,
      (type) ? type->typeAnnotationName() : "",
      dynamic_pointer_cast<ExpressionList>(decl->exp),
      abstract,
      typeconst);
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
  pseudoMain->getStmt()->setFirst(1, 1);
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

void Parser::setHasNonEmptyReturn(ConstructPtr blame) {
  if (m_funcContexts.empty()) {
    return;
  }

  FunctionContext& fc = m_funcContexts.back();
  if (fc.isGenerator) {
    Compiler::Error(Compiler::InvalidYield, blame);
    PARSE_ERROR("Generators cannot return values using \"return\"");
  }

  fc.hasNonEmptyReturn = true;
}

void Parser::onReturn(Token &out, Token *expr) {
  out->stmt = NEW_STMT(ReturnStatement, expr ? expr->exp : ExpressionPtr());
  if (expr) {
    setHasNonEmptyReturn(out->stmt);
  }
}

void Parser::invalidYield() {
  ExpressionPtr exp = std::make_shared<SimpleFunctionCall>(
    BlockScopePtr(), getRange(), "yield", false,
    ExpressionListPtr(), ExpressionPtr());
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

void Parser::setIsGenerator() {
  if (m_funcContexts.empty()) {
    invalidYield();
    PARSE_ERROR("Yield can only be used inside a function");
  }

  FunctionContext& fc = m_funcContexts.back();
  if (fc.hasNonEmptyReturn) {
    invalidYield();
    PARSE_ERROR("Generators cannot return values using \"return\"");
  }
  if (!canBeAsyncOrGenerator(m_funcName, m_clsName)) {
    invalidYield();
    PARSE_ERROR("'yield' is not allowed in constructor, destructor, or "
                "magic methods");
  }

  fc.isGenerator = true;
}

void Parser::onYield(Token &out, Token *expr) {
  setIsGenerator();
  // yield; == yield null;
  auto expPtr = expr ? expr->exp : NEW_EXP(ConstantExpression, "null", false);
  out->exp = NEW_EXP(YieldExpression, ExpressionPtr(), expPtr);
}

void Parser::onYieldPair(Token &out, Token *key, Token *val) {
  setIsGenerator();
  out->exp = NEW_EXP(YieldExpression, key->exp, val->exp);
}

void Parser::onYieldBreak(Token &out) {
  setIsGenerator();
  out->stmt = NEW_STMT(ReturnStatement, ExpressionPtr());
}

void Parser::invalidAwait() {
  auto exp = std::make_shared<SimpleFunctionCall>(
    BlockScopePtr(), getRange(), "async", false,
    ExpressionListPtr(), ExpressionPtr());
  Compiler::Error(Compiler::InvalidAwait, exp);
}

void Parser::setIsAsync() {
  if (m_funcContexts.empty()) {
    invalidAwait();
    PARSE_ERROR("'await' can only be used inside a function");
  }

  if (!canBeAsyncOrGenerator(m_funcName, m_clsName)) {
    invalidAwait();
    PARSE_ERROR("'await' is not allowed in constructors, destructors, or "
                    "magic methods.");
  }

  FunctionContext& fc = m_funcContexts.back();
  fc.isAsync = true;
}


void Parser::onAwait(Token &out, Token &expr) {
  setIsAsync();
  out->exp = NEW_EXP(AwaitExpression, expr->exp);
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

void Parser::onHashBang(Token &out, Token &text) {
  ExpressionPtr exp = NEW_EXP(ScalarExpression, T_STRING, text->text(),
                              true);
  ExpressionListPtr expList = NEW_EXP(ExpressionList);
  expList->addElement(exp);
  ExpressionPtr callExp = NEW_EXP(SimpleFunctionCall,
                                  "__SystemLib\\print_hashbang",
                                  true, expList, ExpressionPtr());
  ExpStatementPtr expStmt(NEW_STMT(ExpStatement, callExp));
  out->stmt = expStmt;
  expStmt->onParse(m_ar, m_file);
}

void Parser::onEcho(Token &out, Token &expr, bool html) {
  if (html) {
    auto const& loc = getRange();
    if (loc.line1 == 2 && loc.char1 == 0 && expr->text()[0] == '#') {
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
  ExpressionListPtr exps = dynamic_pointer_cast<ExpressionList>(expr->exp);
  for (int i = 0, n = exps->getCount(); i < n; i++) {
    checkAllowedInWriteContext((*exps)[i]);
  }
  out->stmt = NEW_STMT(UnsetStatement, exps);
  m_file->setAttribute(FileScope::ContainsUnset);
}

void Parser::onExpStatement(Token &out, Token &expr) {
  ExpStatementPtr exp(NEW_STMT(ExpStatement, expr->exp));
  out->stmt = exp;
  exp->onParse(m_ar, m_file);
}

void Parser::onForEach(Token &out, Token &arr, Token &name, Token &value,
                       Token &stmt, bool awaitAs) {
  checkAllowedInWriteContext(name->exp);
  checkAllowedInWriteContext(value->exp);
  if (value->exp && name->num()) {
    PARSE_ERROR("Key element cannot be a reference");
  }
  if (awaitAs) {
    if (name->num() || value->num()) {
      PARSE_ERROR("Value element cannot be a reference if await as is used");
    }
    setIsAsync();
  }
  checkThisContext(name, ThisContextError::Assign);
  checkThisContext(value, ThisContextError::Assign);
  if (stmt->stmt && stmt->stmt->is(Statement::KindOfStatementList)) {
    stmt->stmt = NEW_STMT(BlockStatement,
                          dynamic_pointer_cast<StatementList>(stmt->stmt));
  }
  out->stmt = NEW_STMT(ForEachStatement, arr->exp, name->exp, name->num() == 1,
                       value->exp, value->num() == 1, awaitAs, stmt->stmt);
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
                        Token& stmts,
                        Token& ret) {
  Token out;
  Token name;

  auto stmt = onFunctionHelper(
    FunctionType::Closure,
    modifiers,
    ret,
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
  out.typeAnnotation = std::make_shared<TypeAnnotation>(
    name.text(), typeArgs.typeAnnotation);

  // Namespaced identifiers (num & 1) can never be type variables.
  if ((name.num() & 1) && isTypeVar(name.text())) {
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
    case 'a':
      type.typeAnnotation->setTypeAccess();
      break;
    }
  }
}

void Parser::onQuery(Token &out, Token &head, Token &body) {
  auto qe = NEW_EXP(QueryExpression, head.exp, body.exp);
  qe->doRewrites(m_ar, m_file);
  out->exp = qe;
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
  out->exp = NEW_EXP(Ordering, expr.exp, (direction) ? direction->text() : "");
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

Parser::AliasTable::AliasTable(const hphp_string_imap<std::string>& autoAliases,
                               std::function<bool ()> autoOracle)
  : m_autoAliases(autoAliases), m_autoOracle(autoOracle) {
  if (!m_autoOracle) {
    setFalseOracle();
  }
}

void Parser::AliasTable::setFalseOracle() {
  m_autoOracle = [] () { return false; };
}

std::string Parser::AliasTable::getName(std::string alias, int line_no) {
  auto it = m_aliases.find(alias);
  if (it != m_aliases.end()) {
    return it->second.name;
  }
  auto autoIt = m_autoAliases.find(alias);
  if (autoIt != m_autoAliases.end()) {
    set(alias, autoIt->second, AliasType::AUTO_USE, line_no);
    return autoIt->second;
  }
  return "";
}

std::string Parser::AliasTable::getNameRaw(std::string alias) {
  auto it = m_aliases.find(alias);
  if (it != m_aliases.end()) {
    return it->second.name;
  }
  return "";
}

Parser::AliasTable::AliasType Parser::AliasTable::getType(std::string alias) {
  auto it = m_aliases.find(alias);
  return it != m_aliases.end() ? it->second.type : AliasType::NONE;
}

int Parser::AliasTable::getLine(std::string alias) {
  auto it = m_aliases.find(alias);
  return (it != m_aliases.end()) ? it->second.line_no : -1;
}

bool Parser::AliasTable::isAliased(std::string alias) {
  auto t = getType(alias);
  if (t == AliasType::USE || t == AliasType::AUTO_USE) {
    return true;
  }
  return m_autoOracle() && m_autoAliases.find(alias) != m_autoAliases.end();
}

void Parser::AliasTable::set(std::string alias,
                             std::string name,
                             AliasType type,
                             int line_no) {
  m_aliases[alias] = NameEntry{name, type, line_no};
}

/*
 * To be called when we enter a fresh namespace.
 */
void Parser::AliasTable::clear() {
  m_aliases.clear();
}

//////////////////////////////////////////////////////


/*
 * We auto-alias classes only on HH mode.
 */
bool Parser::isAutoAliasOn() {
  return m_scanner.isHHSyntaxEnabled();
}

/**
 * This is the authoritative map that drives Hack's auto-importation
 * mechanism for core types and classes defined in the HH namespace.
 * When HH syntax is enabled, auto-importation will kick in for any
 * of the keys in this map are used in a source file (unless there
 * is a conflicting definition or explicit use statement earlier in
 * the file / current namespace block).
 *
 * Note that this map serves a different purpose than the AnnotType
 * map in "runtime/base/annot-type.cpp".
 */
hphp_string_imap<std::string> Parser::getAutoAliasedClassesHelper() {
  hphp_string_imap<std::string> autoAliases;
  typedef AliasTable::AliasEntry AliasEntry;
  std::vector<AliasEntry> aliases {
    AliasEntry{"AsyncIterator", "HH\\AsyncIterator"},
    AliasEntry{"AsyncKeyedIterator", "HH\\AsyncKeyedIterator"},
    AliasEntry{"Traversable", "HH\\Traversable"},
    AliasEntry{"Container", "HH\\Container"},
    AliasEntry{"KeyedTraversable", "HH\\KeyedTraversable"},
    AliasEntry{"KeyedContainer", "HH\\KeyedContainer"},
    AliasEntry{"Iterator", "HH\\Iterator"},
    AliasEntry{"KeyedIterator", "HH\\KeyedIterator"},
    AliasEntry{"Iterable", "HH\\Iterable"},
    AliasEntry{"KeyedIterable", "HH\\KeyedIterable"},
    AliasEntry{"Collection", "HH\\Collection"},
    AliasEntry{"Vector", "HH\\Vector"},
    AliasEntry{"Map", "HH\\Map"},
    AliasEntry{"Set", "HH\\Set"},
    AliasEntry{"Pair", "HH\\Pair"},
    AliasEntry{"ImmVector", "HH\\ImmVector"},
    AliasEntry{"ImmMap", "HH\\ImmMap"},
    AliasEntry{"ImmSet", "HH\\ImmSet"},
    AliasEntry{"InvariantException", "HH\\InvariantException"},
    AliasEntry{"IMemoizeParam", "HH\\IMemoizeParam"},
    AliasEntry{"Shapes", "HH\\Shapes"},

    AliasEntry{"Awaitable", "HH\\Awaitable"},
    AliasEntry{"AsyncGenerator", "HH\\AsyncGenerator"},
    AliasEntry{"WaitHandle", "HH\\WaitHandle"},
    // Keep in sync with order in hphp/runtime/ext/asio/wait-handle.h
    AliasEntry{"StaticWaitHandle", "HH\\StaticWaitHandle"},
    AliasEntry{"WaitableWaitHandle", "HH\\WaitableWaitHandle"},
    AliasEntry{"ResumableWaitHandle", "HH\\ResumableWaitHandle"},
    AliasEntry{"AsyncFunctionWaitHandle", "HH\\AsyncFunctionWaitHandle"},
    AliasEntry{"AsyncGeneratorWaitHandle", "HH\\AsyncGeneratorWaitHandle"},
    AliasEntry{"AwaitAllWaitHandle", "HH\\AwaitAllWaitHandle"},
    AliasEntry{"GenArrayWaitHandle", "HH\\GenArrayWaitHandle"},
    AliasEntry{"GenMapWaitHandle", "HH\\GenMapWaitHandle"},
    AliasEntry{"GenVectorWaitHandle", "HH\\GenVectorWaitHandle"},
    AliasEntry{"ConditionWaitHandle", "HH\\ConditionWaitHandle"},
    AliasEntry{"RescheduleWaitHandle", "HH\\RescheduleWaitHandle"},
    AliasEntry{"SleepWaitHandle", "HH\\SleepWaitHandle"},
    AliasEntry{
      "ExternalThreadEventWaitHandle",
      "HH\\ExternalThreadEventWaitHandle"
    },

    AliasEntry{"bool", "HH\\bool"},
    AliasEntry{"int", "HH\\int"},
    AliasEntry{"float", "HH\\float"},
    AliasEntry{"num", "HH\\num"},
    AliasEntry{"arraykey", "HH\\arraykey"},
    AliasEntry{"string", "HH\\string"},
    AliasEntry{"resource", "HH\\resource"},
    AliasEntry{"mixed", "HH\\mixed"},
    AliasEntry{"noreturn", "HH\\noreturn"},
    AliasEntry{"void", "HH\\void"},
    AliasEntry{"this", "HH\\this"},
    AliasEntry{"classname", "HH\\string"}, // for ::class

    // Support a handful of synonyms for backwards compat with code written
    // against older versions of HipHop, and to be consistent with PHP5 casting
    // syntax (for example, PHP5 supports both "(bool)$x" and "(boolean)$x")
    AliasEntry{"boolean", "HH\\bool"},
    AliasEntry{"integer", "HH\\int"},
    AliasEntry{"double", "HH\\float"},
    AliasEntry{"real", "HH\\float"},
  };
  for (auto entry : aliases) {
    autoAliases[entry.alias] = entry.name;
  }
  return autoAliases;
}

const hphp_string_imap<std::string>& Parser::getAutoAliasedClasses() {
  static auto autoAliases = getAutoAliasedClassesHelper();
  return autoAliases;
}

void Parser::nns(int token, const std::string& text) {
  if (m_nsState == SeenNamespaceStatement && token != ';') {
    error("No code may exist outside of namespace {}: %s",
          getMessage(false,true).c_str());
    return;
  }

  if (m_nsState == SeenNothing && !text.empty() && token != T_DECLARE &&
      token != ';' && token != T_HASHBANG) {
    m_nsState = SeenNonNamespaceStatement;
  }
}

void Parser::onNamespaceStart(const std::string &ns,
                              bool file_scope /* =false */) {
  if (m_nsState == SeenNonNamespaceStatement) {
    error("Namespace declaration statement has to be the very first "
          "statement in the script: %s", getMessage(false,true).c_str());
    return;
  }
  if (m_nsState != SeenNothing && file_scope != m_nsFileScope) {
    error("Cannot mix bracketed namespace declarations with unbracketed "
          "namespace declarations");
  }

  m_nsState = InsideNamespace;
  m_nsFileScope = file_scope;
  pushComment();
  if (file_scope) {
    m_nsStack.clear();
    m_namespace.clear();
  }
  m_nsStack.push_back(m_namespace.size());
  if (!ns.empty()) {
    if (!m_namespace.empty()) m_namespace += NAMESPACE_SEP;
    m_namespace += ns;
  }
  m_nsAliasTable.clear();
  m_fnAliasTable.clear();
  m_cnstAliasTable.clear();
}

void Parser::onNamespaceEnd() {
  m_namespace.resize(m_nsStack.back());
  m_nsStack.pop_back();
  if (m_nsStack.empty()) {
    m_nsState = SeenNamespaceStatement;
  }
}

void Parser::onUse(const std::string &ns, const std::string &as) {
  if (ns == "strict") {
    if (m_scanner.isHHSyntaxEnabled()) {
      error("To use strict hack, place // strict after the open tag. "
            "If it's already there, remove this line. "
            "Hack is strict already.");
    }
    error("You seem to be trying to use a different language. "
          "May I recommend Hack? http://hacklang.org");
  }
  string key = fully_qualified_name_as_alias_key(ns, as);

  if (m_nsAliasTable.getType(key) == AliasType::AUTO_USE) {
    error("Cannot use %s as %s because the name was implicitly used "
          "on line %d; implicit use of names from the HH namespace can "
          "be suppressed by adding an explicit `use' statement earlier "
          "in the %s: %s",
          ns.c_str(), key.c_str(), m_nsAliasTable.getLine(key),
          (m_nsState == InsideNamespace ? "current namespace block" : "file"),
          getMessage(false,true).c_str());
    return;
  } else if (m_nsAliasTable.getType(key) == AliasType::USE) {
    if (m_scanner.isHHSyntaxEnabled()) {
      error("Cannot use %s as %s because the name was explicitly used "
            "earlier via a `use' statement on line %d: %s",
            ns.c_str(), key.c_str(), m_nsAliasTable.getLine(key),
            getMessage(false,true).c_str());
    } else {
      error("Cannot use %s as %s because the name is already in use: %s",
            ns.c_str(), key.c_str(), getMessage(false,true).c_str());
    }
    return;
  }

  if (m_nsAliasTable.getType(key) == AliasType::DEF) {
    if (strcasecmp(ns.c_str(), m_nsAliasTable.getNameRaw(key).c_str())) {
      error("Cannot use %s as %s because the name is already in use: %s",
            ns.c_str(), key.c_str(), getMessage(false,true).c_str());
      return;
    }
  }

  m_nsAliasTable.set(key, ns, AliasType::USE, line1());
}

void Parser::onUseFunction(const std::string &fn, const std::string &as) {
  string key = fully_qualified_name_as_alias_key(fn, as);

  if (m_fnTable.count(key) || m_fnAliasTable.count(key)) {
    error(
      "Cannot use function %s as %s because the name is already in use in %s",
      fn.c_str(), key.c_str(), getMessage(false,true).c_str());
  }

  m_fnAliasTable[key] = fn;
}

void Parser::onUseConst(const std::string &cnst, const std::string &as) {
  string key = fully_qualified_name_as_alias_key(cnst, as);

  if (m_cnstTable.count(key) || m_cnstAliasTable.count(key)) {
    error(
      "Cannot use const %s as %s because the name is already in use in %s",
      cnst.c_str(), key.c_str(), getMessage(false,true).c_str());
  }

  m_cnstAliasTable[key] = cnst;
}

std::string Parser::nsClassDecl(const std::string &name) {
  if (m_namespace.empty() ||
      !strcasecmp("self", name.c_str()) ||
      !strcasecmp("parent", name.c_str())) {
    return name;
  }
  return m_namespace + NAMESPACE_SEP + name;
}

std::string Parser::nsDecl(const std::string &name) {
  if (m_namespace.empty()) {
    return name;
  }
  return m_namespace + NAMESPACE_SEP + name;
}

std::string Parser::resolve(const std::string &ns, bool cls) {
  size_t pos = ns.find(NAMESPACE_SEP);
  string alias = (pos != string::npos) ? ns.substr(0, pos) : ns;

  // Don't expand type variables into the current namespace.
  if (isTypeVar(ns)) {
    return ns;
  }

  if (m_nsAliasTable.isAliased(alias)) {
    auto name = m_nsAliasTable.getName(alias, line1());
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
    return nsClassDecl(ns);
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

void Parser::registerAlias(std::string name) {
  size_t pos = name.rfind(NAMESPACE_SEP);
  string key = (pos != string::npos) ? name.substr(pos + 1) : name;
  if (m_nsAliasTable.getType(key) != AliasType::USE &&
      m_nsAliasTable.getType(key) != AliasType::AUTO_USE) {
    m_nsAliasTable.set(key, name, AliasType::DEF, line1());
    return;
  }
  if (m_nsAliasTable.getType(key) == AliasType::AUTO_USE) {
    error("Cannot declare class %s because the name was implicitly used "
          "on line %d; implicit use of names from the HH namespace can "
          "be suppressed by adding an explicit `use' statement earlier "
          "in the %s: %s",
          name.c_str(), m_nsAliasTable.getLine(key),
          (m_nsState == InsideNamespace ? "current namespace block" : "file"),
          getMessage(false,true).c_str());
  } else if (strcasecmp(name.c_str(), m_nsAliasTable.getNameRaw(key).c_str())) {
    if (m_scanner.isHHSyntaxEnabled()) {
      error("Cannot declare class %s because the name was explicitly used "
            "earlier via a `use' statement on line %d: %s",
            name.c_str(), m_nsAliasTable.getLine(key),
            getMessage(false,true).c_str());
    } else {
      error("Cannot declare class %s because the name is already in use: %s",
            name.c_str(), getMessage(false,true).c_str());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
