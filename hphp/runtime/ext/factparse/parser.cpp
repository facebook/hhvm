/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/factparse/parser.h"

#include <iostream>
#include <iomanip>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/util/string-vsnprintf.h"

namespace HPHP {
namespace Facts {

bool Parser::parseImpl() {
  if (RuntimeOption::PHP7_UVS) {
    return parseImpl7();
  } else {
    return parseImpl5();
  }
}

void Parser::parse() {
  m_result.error = true;

  if (!parseImpl()) {
    throw ParseTimeFatalException(
      m_fileName,
      line1(),
      "Parse error: %s",
      getMessage().c_str());
  }
  std::stringstream(m_scanner.getMd5().substr(0, 128/8))
    >> std::hex
    >> m_result.md5sum[0];
  std::stringstream(m_scanner.getMd5().substr(128/8))
    >> std::hex
    >> m_result.md5sum[1];

  m_result.error = false;
}

void Parser::error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
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

  auto exn = ParseTimeFatalException(
    m_fileName, loc->r.line0,
    "%s",
    strInput.c_str());

  exn.setParseFatal();
  throw exn;
}

void Parser::fatal(const Location* loc, const char* msg) {
  throw ParseTimeFatalException(m_fileName, loc->r.line0, "%s", msg);
}

void Parser::onClassStart(int /*type*/, Token& /*name*/) {
  m_classScope++;
}

// There can be multilple declarations of a class with the same name e.g.:
// if (...) { class X {function a(){}} } else { class X {function b(){}}}
// We take the union of base-types/traits across both declarations as this
// is what is needed to answer questions like "which classes extend Y".
//
// We also union the flags and if two declarations differ in "kind" this is set
// to MIXED. FactTypeFlags::MultipleDeclarations is always set if there are
// multiple declarations of the same type, even if all other features match.
//
// The flag for final is set for enums, and the flag for abstract is set for
// interfaces and traits.
void Parser::onClass(Token& /*out*/, int type, Token& name, Token& base,
                     Token& baseInterface, Token& stmt, Token* /*attr*/,
                     Token* /*enumBase*/) {
  m_classScope--;
  if (!isOutsideOfClass()) {
    return;
  }

  auto& name_data = name.text();
  bool type_already_existed =
    m_result.typesMap.count(name_data) != 0;
  auto& type_details = m_result.typesMap[name_data];

  FactKindOf kind_of;
  switch (type) {
    case T_ENUM:
      kind_of = FactKindOf::Enum;
      type_details.flags |= (uint64_t)FactTypeFlags::Final;
      break;
    case T_INTERFACE:
      kind_of = FactKindOf::Interface;
      type_details.flags |= (uint64_t)FactTypeFlags::Abstract;
      break;
    case T_TRAIT:
      kind_of = FactKindOf::Trait;
      type_details.flags |= (uint64_t)FactTypeFlags::Abstract;
      break;
    case T_CLASS:
      kind_of = FactKindOf::Class;
      break;
    case T_ABSTRACT:
      kind_of = FactKindOf::Class;
      type_details.flags |= (uint64_t)FactTypeFlags::Abstract;
      break;
    case T_FINAL:
      kind_of = FactKindOf::Class;
      type_details.flags |= (uint64_t)FactTypeFlags::Final;
      break;
    case T_STATIC: // abstract + Final
      kind_of = FactKindOf::Class;
      type_details.flags |=
        (uint64_t)FactTypeFlags::Abstract | (uint64_t)FactTypeFlags::Final;
      break;
    default:
      error("Unexpected class token: %d", type);
  }
  if (type_already_existed && kind_of != type_details.kindOf) {
    kind_of = FactKindOf::Mixed;
  }
  type_details.kindOf = kind_of;

  if (type_already_existed) {
    type_details.flags |= (uint64_t)FactTypeFlags::MultipleDeclarations;
  }

  for (auto& baseType : baseInterface.m_baseTypes) {
    type_details.baseTypes.emplace(std::move(baseType));
  }

  for (auto& baseType : stmt.m_baseTypes) {
    type_details.baseTypes.emplace(std::move(baseType));
  }

  if (base.text().size()) {
    type_details.baseTypes.emplace(std::move(base.takeText()));
  }

  if (!type_already_existed && type_details.kindOf == FactKindOf::Enum) {
    type_details.baseTypes.insert("HH\\BuiltinEnum");
  }

  if (kind_of == FactKindOf::Trait || kind_of == FactKindOf::Interface) {
    for (auto& requireExtend : stmt.m_requireExtends) {
      type_details.requireExtends.emplace(std::move(requireExtend));
    }
  }

  if (kind_of == FactKindOf::Trait) {
    for (auto& requireImplement : stmt.m_requireImplements) {
      type_details.requireImplements.emplace(std::move(requireImplement));
    }
  }
}

void Parser::onEnum(Token& out, Token& name, Token& /*baseTy*/, Token& /*stmt*/,
                    Token* /*attr*/) {
  Token t;
  onClass(out, T_ENUM, name, t, t, t, nullptr, nullptr);
}

void Parser::onInterface(Token& out, Token& name, Token& base, Token& stmt,
                         Token* /*attr*/) {
  Token t;
  onClass(out, T_INTERFACE, name, t, base, stmt, nullptr, nullptr);
}

void Parser::onInterfaceName(Token &out, Token *names, Token &name) {
  if (names) {
    out = Token(std::move(*names));
  }
  out.m_baseTypes.emplace(std::move(name.takeText()));
}

void Parser::onTypedef(Token& /*out*/, Token& name, const Token& /*type*/,
                       const Token* /*attr*/) {
  if (!isOutsideOfClass()) {
    return;
  }
  m_result.typeAliases.emplace_back(std::move(name.takeText()));
}

void Parser::onConst(Token& /*out*/, Token& name, Token& /*value*/) {
  if (!isOutsideOfClass()) {
    return;
  }
  m_result.constants.emplace_back(std::move(name.takeText()));
}

void Parser::onFunctionStart(Token &name) {
  SCOPE_EXIT {
    m_functionScope++;
  };
  if (!isOutsideOfClass()) {
    return;
  }
  m_result.functions.emplace_back(std::move(name.takeText()));
}

void Parser::onFunction(Token& /*out*/, Token* /*modifiers*/, Token& /*ret*/,
                        Token& /*ref*/, Token& /*name*/, Token& /*params*/,
                        Token& /*stmt*/, Token* /*attr*/) {
  m_functionScope--;
}

std::string Parser::nsDecl(const std::string &name) {
  if (m_namespace.empty()) {
    return name;
  }
  return m_namespace + NAMESPACE_SEP + name;
}

std::string Parser::nsClassDecl(const std::string &name) {
  // Doesn't need to be implemented fully as we only deal with top-level
  return nsDecl(name);
}

void Parser::onNamespaceStart(
  const std::string &ns,
  bool file_scope /* =false */)
{
  if (file_scope) {
    m_nsStack.clear();
    m_namespace.clear();
  }
  m_nsStack.push_back(m_namespace.size());
  if (!ns.empty()) {
    if (!m_namespace.empty()) m_namespace += NAMESPACE_SEP;
    m_namespace += ns;
  }
}

void Parser::onNamespaceEnd() {
  m_namespace.resize(m_nsStack.back());
  m_nsStack.pop_back();
}

// This is needed when computing the name for onCall
std::string Parser::resolve(const std::string& ns, bool /*cls*/) {
  auto const pos = ns.find(NAMESPACE_SEP);

  // if qualified name, prepend current namespace
  if (pos != std::string::npos) {
    return nsDecl(ns);
  }

  // unqualified name in global namespace
  if (m_namespace.empty()) {
    return ns;
  }

  return nsDecl(ns);
}

// Search for define("string", value)
void Parser::onCall(Token& /*out*/, bool dynamic, Token& name, Token& params,
                    Token* cls) {
  if (
    !dynamic
    && !cls
    && !strcasecmp(name.text().c_str(), "define")
    && params.m_constArgs == 2)
  {
    m_result.constants.emplace_back(std::move(params.m_constantName));
  }
}

// Match an acceptable parameter list of the form ("string literal", <expr>)
void Parser::onCallParam(
  Token &out,
  Token *prevParams,
  Token &expr,
  bool ref,
  bool unpack)
{
  // References or var-args are never the pattern we want
  if (ref || unpack) {
    out.m_constArgs = -1;
  } else {
    if (prevParams) {
      out = Token(std::move(*prevParams));
      if (out.m_constArgs != - 1) {
        out.m_constArgs++;
      }
    } else if (expr.num() == T_CONSTANT_ENCAPSED_STRING) {
      // First argument must be a string literal
      out.m_constantName.swap(expr.takeText());
      out.m_constArgs = 1;
    } else {
      out.m_constArgs = -1;
    }
  }
}

// Needed for onCallParam
void Parser::onScalar(Token &out, int type, Token &scalar) {
  out = Token(std::move(scalar));
  out.setNum(type);
}

// Needed to preserve fully qualified names of base interfaces
void Parser::onTypeAnnotation(Token& out, Token& name,
                              const Token& /*typeArgs*/) {
  out = Token(name);
}

// Trait requirements are accumulated as base types for the enclosing type
void Parser::onClassRequire(Token &out, Token &name, bool isExtends) {
  if (isExtends) {
    out.m_requireExtends.emplace(std::move(name.takeText()));
  } else {
    out.m_requireImplements.emplace(std::move(name.takeText()));
  }
}

// Trait uses are accumulated as base types for the enclosing type
void Parser::onTraitUse(Token& out, Token& traits, Token& /*rules*/) {
  out = Token(std::move(traits));
}

void Parser::onTraitName(Token &out, Token *names, Token &name) {
  if (names) {
    out = Token(std::move(*names));
  }
  out.m_baseTypes.emplace(std::move(name.takeText()));
}

// As we go through statements in a class we accumulate base types and trait
// constraints.
// out = stmts + new_stmt
void Parser::onClassStatement(Token &out, Token &stmts, Token &new_stmt) {
  out = Token(std::move(stmts));
  out.takeAccumulate(new_stmt);
}

void Parser::setTypeVars(Token &out, Token &name) {
  out = Token(std::move(name));
}


/******************************************************************************
 * EVERYTHING below this point is empty/unused/returns a falsey value         *
 ******************************************************************************/

void Parser::onScopeLabel(const Token& /*stmt*/, const Token& /*label*/) {}

void Parser::onCompleteLabelScope(bool /*fresh*/) {}

void Parser::onName(Token& /*out*/, Token& /*name*/, NameKind /*kind*/) {}

void Parser::onStaticVariable(Token& /*out*/, Token* /*exprs*/, Token& /*var*/,
                              Token* /*value*/) {}

void Parser::onClassVariable(Token& /*out*/, Token* /*exprs*/, Token& /*var*/,
                             Token* /*value*/) {}

void Parser::onClassConstant(Token& /*out*/, Token* /*exprs*/, Token& /*var*/,
                             Token& /*value*/) {}

void Parser::onClassAbstractConstant(Token& /*out*/, Token* /*exprs*/,
                                     Token& /*var*/) {}

void Parser::onClassTypeConstant(Token& /*out*/, Token& /*var*/,
                                 Token& /*value*/) {}

void Parser::onVariable(Token& /*out*/, Token* /*exprs*/, Token& /*var*/,
                        Token* /*value*/, bool /*constant*/ /* = false */,
                        const std::string& /*docComment*/ /* = "" */) {}

void Parser::onSimpleVariable(Token& /*out*/, Token& /*var*/) {}

void Parser::onPipeVariable(Token& /*out*/) {}

void Parser::onDynamicVariable(Token& /*out*/, Token& /*expr*/,
                               bool /*encap*/) {}

void Parser::onIndirectRef(Token& /*out*/, Token& /*refCount*/,
                           Token& /*var*/) {}

void Parser::onStaticMember(Token& /*out*/, Token& /*cls*/, Token& /*name*/) {}

void Parser::onRefDim(Token& /*out*/, Token& /*var*/, Token& /*offset*/) {}

void Parser::onObjectProperty(Token& /*out*/, Token& /*base*/,
                              PropAccessType /*propAccessType*/,
                              Token& /*prop*/) {}

void Parser::onObjectMethodCall(Token& /*out*/, Token& /*base*/,
                                bool /*nullsafe*/, Token& /*prop*/,
                                Token& /*params*/) {}

void Parser::onEncapsList(Token& /*out*/, int /*type*/, Token& /*list*/) {}

void Parser::addEncap(Token& /*out*/, Token* /*list*/, Token& /*expr*/,
                      int /*type*/) {}

void Parser::encapRefDim(Token& /*out*/, Token& /*var*/, Token& /*offset*/) {}

void Parser::encapObjProp(Token& /*out*/, Token& /*var*/,
                          PropAccessType /*propAccessType*/, Token& /*name*/) {}

void Parser::encapArray(Token& /*out*/, Token& /*var*/, Token& /*expr*/) {}

void Parser::onConstantValue(Token& /*out*/, Token& /*constant*/) {}

void Parser::onExprListElem(Token& /*out*/, Token* /*exprs*/, Token& /*expr*/) {
}

void Parser::onListAssignment(Token& /*out*/, Token& /*vars*/, Token* /*expr*/,
                              bool /*rhsFirst*/ /* = false */) {}

void Parser::onAListVar(Token& /*out*/, Token* /*list*/, Token* /*var*/) {}

void Parser::onAListSub(Token& /*out*/, Token* /*list*/, Token& /*sublist*/) {}

void Parser::onAssign(Token& /*out*/, Token& /*var*/, Token& /*expr*/,
                      bool /*ref*/, bool /*rhsFirst*/ /* = false */) {}

void Parser::onAssignNew(Token& /*out*/, Token& /*var*/, Token& /*name*/,
                         Token& /*args*/) {}

void Parser::onNewObject(Token& /*out*/, Token& /*name*/, Token& /*args*/) {}

void Parser::onUnaryOpExp(Token& /*out*/, Token& /*operand*/, int /*op*/,
                          bool /*front*/) {}

void Parser::onBinaryOpExp(Token& /*out*/, Token& /*operand1*/,
                           Token& /*operand2*/, int /*op*/) {}

void Parser::onQOp(Token& /*out*/, Token& /*exprCond*/, Token* /*expYes*/,
                   Token& /*expNo*/) {}

void Parser::onNullCoalesce(Token& /*out*/, Token& /*expFirst*/,
                            Token& /*expSecond*/) {}

void Parser::onArray(Token& /*out*/, Token& /*pairs*/,
                     int /*op*/ /* = T_ARRAY */) {}

void Parser::onDict(Token& /*out*/, Token& /*pairs*/) {}

void Parser::onVec(Token& /*out*/, Token& /*exprs*/) {}

void Parser::onKeyset(Token& /*out*/, Token& /*exprs*/) {}

void Parser::onVArray(Token& /*out*/, Token& /*exprs*/) {}

void Parser::onDArray(Token& /*out*/, Token& /*exprs*/) {}

void Parser::onArrayPair(Token& /*out*/, Token* /*pairs*/, Token* /*name*/,
                         Token& /*value*/, bool /*ref*/) {}

void Parser::onEmptyCollection(Token& /*out*/) {}

void Parser::onUserAttribute(Token& /*out*/, Token* /*attrList*/,
                             Token& /*name*/, Token& /*value*/) {}

void Parser::onClassConst(Token& /*out*/, Token& /*cls*/, Token& /*name*/,
                          bool /*text*/) {}

void Parser::onClassClass(Token& /*out*/, Token& /*cls*/, Token& /*name*/,
                          bool /*inStaticContext*/) {}

void Parser::onMethodStart(Token& /*name*/, Token& /*mods*/,
                           bool /*doPushComment*/ /* = true */) {}

void Parser::onMethod(Token& /*out*/, Token& /*modifiers*/, Token& /*ret*/,
                      Token& /*ref*/, Token& /*name*/, Token& /*params*/,
                      Token& /*stmt*/, Token* /*attr*/,
                      bool /*reloc*/ /* = true */) {}

void Parser::onVariadicParam(Token& /*out*/, Token* /*params*/, Token& /*type*/,
                             Token& /*var*/, bool /*ref*/, Token* /*attr*/,
                             Token* /*modifier*/) {}

void Parser::onParam(Token& /*out*/, Token* /*params*/, Token& /*type*/,
                     Token& /*var*/, bool /*ref*/, Token* /*defValue*/,
                     Token* /*attr*/, Token* /*modifier*/) {}

void Parser::onClassExpressionStart() {
}

void Parser::onClassExpression(Token& /*out*/, Token& /*args*/, Token& /*base*/,
                               Token& /*baseInterface*/, Token& /*stmt*/) {}

void Parser::onTraitRule(Token& /*out*/, Token& /*stmtList*/,
                         Token& /*newStmt*/) {}

void Parser::onTraitPrecRule(Token& /*out*/, Token& /*traitName*/,
                             Token& /*methodName*/, Token& /*otherTraits*/) {}

void Parser::onTraitAliasRuleStart(Token& /*out*/, Token& /*traitName*/,
                                   Token& /*methodName*/) {}

void Parser::onTraitAliasRuleModify(Token& /*out*/, Token& /*rule*/,
                                    Token& /*accessModifiers*/,
                                    Token& /*newMethodName*/) {}

void Parser::onClassVariableStart(Token& /*out*/, Token* /*modifiers*/,
                                  Token& /*decl*/, Token* /*type*/,
                                  bool /*abstract*/ /* = false */,
                                  bool /*typeconst*/ /* = false */) {}

void Parser::onMemberModifier(Token& /*out*/, Token* /*modifiers*/,
                              Token& /*modifier*/) {}

void Parser::initParseTree() {
}

void Parser::finiParseTree() {
}

void Parser::onHaltCompiler() {
}

void Parser::onStatementListStart(Token& /*out*/) {}

void Parser::addTopStatement(Token& /*new_stmt*/) {}

void Parser::addStatement(Token& /*out*/, Token& /*stmts*/,
                          Token& /*new_stmt*/) {}

void Parser::finishStatement(Token& /*out*/, Token& /*stmts*/) {}

void Parser::onBlock(Token& /*out*/, Token& /*stmts*/) {}

void Parser::onIf(Token& /*out*/, Token& /*cond*/, Token& /*stmt*/,
                  Token& /*elseifs*/, Token& /*elseStmt*/) {}

void Parser::onElseIf(Token& /*out*/, Token& /*elseifs*/, Token& /*cond*/,
                      Token& /*stmt*/) {}

void Parser::onWhile(Token& /*out*/, Token& /*cond*/, Token& /*stmt*/) {}

void Parser::onDo(Token& /*out*/, Token& /*stmt*/, Token& /*cond*/) {}

void Parser::onFor(Token& /*out*/, Token& /*expr1*/, Token& /*expr2*/,
                   Token& /*expr3*/, Token& /*stmt*/) {}

void Parser::onSwitch(Token& /*out*/, Token& /*expr*/, Token& /*cases*/) {}

void Parser::onCase(Token& /*out*/, Token& /*cases*/, Token* /*cond*/,
                    Token& /*stmt*/) {}

void Parser::onBreakContinue(Token& /*out*/, bool /*isBreak*/,
                             Token* /*expr*/) {}

void Parser::onReturn(Token& /*out*/, Token* /*expr*/) {}

void Parser::onYield(Token& /*out*/, Token* /*expr*/) {}

void Parser::onYieldFrom(Token& /*out*/, Token* /*expr*/) {}

void Parser::onYieldPair(Token& /*out*/, Token* /*key*/, Token* /*val*/) {}

void Parser::onYieldBreak(Token& /*out*/) {}

void Parser::onAwait(Token& /*out*/, Token& /*expr*/) {}

void Parser::onGlobal(Token& /*out*/, Token& /*expr*/) {}

void Parser::onGlobalVar(Token& /*out*/, Token* /*exprs*/, Token& /*expr*/) {}

void Parser::onStatic(Token& /*out*/, Token& /*expr*/) {}

void Parser::onHashBang(Token& /*out*/, Token& /*text*/) {}

void Parser::onEcho(Token& /*out*/, Token& /*expr*/, bool /*html*/) {}

void Parser::onUnset(Token& /*out*/, Token& /*expr*/) {}

void Parser::onExpStatement(Token& /*out*/, Token& /*expr*/) {}

void Parser::onForEach(Token& /*out*/, Token& /*arr*/, Token& /*name*/,
                       Token& /*value*/, Token& /*stmt*/, bool /*awaitAs*/) {}

void Parser::onTry(Token& /*out*/, Token& /*tryStmt*/, Token& /*className*/,
                   Token& /*var*/, Token& /*catchStmt*/, Token& /*catches*/,
                   Token& /*finallyStmt*/) {}

void Parser::onTry(Token& /*out*/, Token& /*tryStmt*/, Token& /*finallyStmt*/) {
}

void Parser::onCatch(Token& /*out*/, Token& /*catches*/, Token& /*className*/,
                     Token& /*var*/, Token& /*stmt*/) {}

void Parser::onFinally(Token& /*out*/, Token& /*stmt*/) {}

void Parser::onThrow(Token& /*out*/, Token& /*expr*/) {}

void Parser::onClosureStart(Token& /*name*/) {}

Token Parser::onClosure(ClosureType /*type*/, Token* /*modifiers*/,
                        Token& /*ref*/, Token& /*params*/, Token& /*cparams*/,
                        Token& /*stmts*/, Token& /*ret1*/,
                        Token* /*ret2*/ /* = nullptr */) {
  return Token();
}

Token Parser::onExprForLambda(const Token& /*expr*/) {
  return Token();
}

void Parser::onClosureParam(Token& /*out*/, Token* /*params*/, Token& /*param*/,
                            bool /*ref*/) {}

void Parser::onLabel(Token& /*out*/, Token& /*label*/) {}

void Parser::onGoto(Token& /*out*/, Token& /*label*/, bool /*limited*/) {}

void Parser::onTypeList(Token& /*type1*/, const Token& /*type2*/) {}

void Parser::onClsCnsShapeField(Token& /*out*/, const Token& /*cls*/,
                                const Token& /*cns*/, const Token& /*value*/) {}

void Parser::onShapeFieldSpecialization(Token& /*shapeField*/,
                                        char /*specialization*/) {}

void Parser::onShape(Token& /*out*/, const Token& /*shapeFieldsList*/,
                     bool /*terminatedWithEllipsis*/) {}

void Parser::onTypeSpecialization(Token& /*type*/, char /*specialization*/) {}

void Parser::onUseDeclaration(Token& /*out*/, const std::string& /*ns*/,
                              const std::string& /*as*/) {}

void Parser::onMixedUseDeclaration(Token& /*out*/, Token& /*use*/,
                                   UseDeclarationConsumer /*f*/) {}

void Parser::onUse(const Token& /*tok*/, UseDeclarationConsumer /*f*/) {}

void Parser::onGroupUse(const std::string& /*prefix*/, const Token& /*tok*/,
                        UseDeclarationConsumer /*f*/) {}

void Parser::onDeclare(Token& /*out*/, Token& /*block*/) {}

void Parser::onDeclareList(Token& /*out*/, Token& /*ident*/, Token& /*exp*/) {}

void Parser::nns(int /*token*/ /* = 0 */,
                 const std::string& /*text*/ /* = std::string() */) {}

void Parser::useClassAndNamespace(const std::string& /*fn*/,
                                  const std::string& /*as*/) {}

void Parser::useClass(const std::string& /*fn*/, const std::string& /*as*/) {}

void Parser::useNamespace(const std::string& /*fn*/,
                          const std::string& /*as*/) {}

void Parser::useFunction(const std::string& /*fn*/, const std::string& /*as*/) {
}

void Parser::useConst(const std::string& /*cnst*/, const std::string& /*as*/) {}

void Parser::invalidateGoto(TStatementPtr /*stmt*/, GotoError /*error*/) {}

void Parser::invalidateLabel(TStatementPtr /*stmt*/) {}

TStatementPtr Parser::extractStatement(ScannerToken* /*stmt*/) {
  return nullptr;
}

void Parser::onNewLabelScope(bool /*fresh*/) {}

/******************************************************************************
 * ONLY put new stubs here. Put implemented methods further up with the rest. *
 ******************************************************************************/

}}
