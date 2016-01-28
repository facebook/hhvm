/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/compiler/construct.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/util/string-vsnprintf.h"

#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/exceptions.h"
#include "hphp/parser/parse-time-fatal-exception.h"

#include "hphp/compiler/statement/function_statement.h"

#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/closure_expression.h"
#include <iomanip>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

Construct::Construct(BlockScopePtr scope,
                     const Location::Range& r, KindOf kindOf)
  : m_blockScope(scope)
  , m_flagsVal(0)
  , m_r(r)
  , m_kindOf(kindOf)
  , m_containedEffects(0)
  , m_effectsTag(0)
{
}

bool Construct::skipRecurse() const {
  switch (getKindOf()) {
    case KindOfFunctionStatement:
    case KindOfMethodStatement:
    case KindOfClassStatement:
    case KindOfInterfaceStatement:
      return true;
    default:
      break;
  }
  return false;
}

void Construct::copyLocationTo(ConstructPtr other) {
  always_assert(other->getFileScope() == getFileScope());
  other->m_r = m_r;
}

void Construct::resetScope(BlockScopeRawPtr scope) {
  setBlockScope(scope);
  for (int i = 0, n = getKidCount(); i < n; i++) {
    if (ConstructPtr kid = getNthKid(i)) {
      if (kid->skipRecurse()) continue;
      kid->resetScope(scope);
    }
  }
}

void Construct::recomputeEffects() {
  BlockScopeRawPtr scope = getScope();
  if (scope) scope->incEffectsTag();
}

int Construct::getChildrenEffects() const {
  int childrenEffects = NoEffect;
  for (int i = getKidCount(); i--; ) {
    ConstructPtr child = getNthKid(i);
    if (child) {
      if (child->skipRecurse()) continue;
      childrenEffects |= child->getContainedEffects();
      if ((childrenEffects & UnknownEffect) == UnknownEffect) {
        break;
      }
    }
  }
  return childrenEffects;
}

int Construct::getContainedEffects() const {
  BlockScopeRawPtr scope = getScope();
  int curTag = scope ? scope->getEffectsTag() : m_effectsTag + 1;
  if (m_effectsTag != curTag) {
    m_effectsTag = curTag;
    m_containedEffects = getLocalEffects() | getChildrenEffects();
  }
  return m_containedEffects;
}

void LocalEffectsContainer::setLocalEffect(Construct::Effect effect) {
  if ((m_localEffects & effect) != effect) {
    effectsCallback();
    m_localEffects |= effect;
  }
}

void LocalEffectsContainer::clearLocalEffect(Construct::Effect effect) {
  if (m_localEffects & effect) {
    effectsCallback();
    m_localEffects &= ~effect;
  }
}

bool LocalEffectsContainer::hasLocalEffect(Construct::Effect effect) const {
  return m_localEffects & effect;
}

ExpressionPtr Construct::makeConstant(AnalysisResultConstPtr ar,
                                      const std::string &value) const {
  return Expression::MakeConstant(ar, getScope(), getRange(), value);
}

ExpressionPtr Construct::makeScalarExpression(AnalysisResultConstPtr ar,
                                               const Variant &value) const {
  return Expression::MakeScalarExpression(ar, getScope(),
                                          getRange(), value);
}

std::string Construct::getText(AnalysisResultPtr ar
                               /* = AnalysisResultPtr() */) {
  std::ostringstream o;
  CodeGenerator cg(&o, CodeGenerator::PickledPHP);
  cg.translatePredefined(false);
  outputPHP(cg, ar);
  return o.str();
}

std::string Construct::getText() {
  return getText(AnalysisResultPtr{});
}

void Construct::serialize(JSON::CodeError::OutputStream &out) const {
  JSON::CodeError::ListStream ls(out);
  auto scope = getFileScope();
  ls <<
    scope->getName() <<
    m_r.line0 << m_r.char0 <<
    m_r.line1 << m_r.char1;
  ls.done();
}

void Construct::printSource(CodeGenerator &cg) {
  if (auto scope = getFileScope()) {
    cg_printf("/* SRC: %s line %d */\n", scope->getName().c_str(), m_r.line0);
  }
}

void Construct::dumpNode(int spc, AnalysisResultConstPtr ar) {
  dumpNode(spc);
}

void Construct::dumpNode(int spc) const {
  // evil, but helpful!
  const_cast<Construct*>(this)->dumpNode(spc);
}

void Construct::dumpNode(int spc) {
  int nkid = getKidCount();
  const char *name = 0;
  int type = 0;
  std::string scontext = "";
  std::string value = "";
  std::string type_info = "";
  int ef = 0;

  if (isStatement()) {
    Statement *s = static_cast<Statement*>(this);
    auto stype = s->getKindOf();
    name = Statement::nameOfKind(stype);
    value = s->getName();
    type = (int)stype;
  } else {
    assert(isExpression());
    Expression *e = static_cast<Expression*>(this);

    ef = e->getLocalEffects();

    Expression::KindOf etype = e->getKindOf();
    name = Expression::nameOfKind(etype);
    switch (etype) {
      case Expression::KindOfSimpleFunctionCall:
        value = static_cast<SimpleFunctionCall*>(e)->getOriginalName();
        break;
      case Expression::KindOfSimpleVariable:
        value = static_cast<SimpleVariable*>(e)->getName();
        break;
      case Expression::KindOfConstantExpression:
        value = e->getText();
        break;
      case Expression::KindOfScalarExpression:
        value = e->getText();
        break;
      default: break;
    }

    int c = e->getContext();
    if ((c & Expression::Declaration) == Expression::Declaration) {
      scontext += "|Declaration";
    } else if (c & Expression::LValue) {
      scontext += "|LValue";
    }
    if (c & Expression::NoLValueWrapper) {
      scontext += "|NoLValueWrapper";
    }
    if (c & Expression::RefValue) {
      scontext += "|RefValue";
    }
    if (c & Expression::RefParameter) {
      scontext += "|RefParameter";
    }
    if (c & Expression::DeepReference) {
      scontext += "|DeepReference";
    }
    if (c & Expression::ObjectContext) {
      scontext += "|ObjectContext";
    }
    if (c & Expression::InParameterExpression) {
      scontext += "|InParameterExpression";
    }
    if (c & Expression::ExistContext) {
      scontext += "|ExistContext";
    }
    if (c & Expression::UnsetContext) {
      scontext += "|UnsetContext";
    }
    if (c & Expression::AssignmentLHS) {
      scontext += "|AssignmentLHS";
    }
    if (c & Expression::RefAssignmentLHS) {
      scontext += "|RefAssignmentLHS";
    }
    if (c & Expression::DeepAssignmentLHS) {
      scontext += "|DeepAssignmentLHS";
    }
    if (c & Expression::AssignmentRHS) {
      scontext += "|AssignmentRHS";
    }
    if (c & Expression::InvokeArgument) {
      scontext += "|InvokeArgument";
    }
    if (c & Expression::OprLValue) {
      scontext += "|OprLValue";
    }
    if (c & Expression::DeepOprLValue) {
      scontext += "|DeepOprLValue";
    }
    if (c & Expression::AccessContext) {
      scontext += "|AccessContext";
    }
    if (c & Expression::ReturnContext) {
      scontext += "|ReturnContext";
    }

    if (scontext != "") {
      scontext = " (" + scontext.substr(1) + ")";
    }

    type = (int)etype;
  }

  int s = spc;
  while (s > 0) {
    int n = s > 10 ? 10 : s;
    std::cout << ("          "+10-n);
    s -= n;
  }

  std::cout << "-> 0x" << std::hex << std::setfill('0')
            << std::setw(10) << (int64_t)this << std::dec;

  std::cout << " " << name << "(" << type << ") ";

  if (value != "") {
    std::cout << "[" << value << "] ";
  }

  std::string sef;
  if ((ef & UnknownEffect) == UnknownEffect) {
    sef = "|UnknownEffect";
  } else {
    if (ef & IOEffect) sef += "|IOEffect";
    if (ef & AssignEffect) sef += "|AssignEffect";
    if (ef & GlobalEffect) sef += "|GlobalEffect";
    if (ef & LocalEffect) sef += "|LocalEffect";
    if (ef & ParamEffect) sef += "|ParamEffect";
    if (ef & DeepParamEffect) sef += "|DeepParamEffect";
    if (ef & DynamicParamEffect) sef += "|DynamicParamEffect";
    if (ef & CanThrow) sef += "|CanThrow";
    if (ef & AccessorEffect) sef += "|AccessorEffect";
    if (ef & CreateEffect) sef += "|CreateEffect";
    if (ef & DiagnosticEffect) sef += "|DiagnosticEffect";
    if (ef & OtherEffect) sef += "|OtherEffect";
  }
  if (sef != "") {
    sef = " (" + sef.substr(1) + ")";
  }

  std::string objstr;
  if (dynamic_cast<SimpleVariable*>(this) != nullptr) {
    objstr = " (NoObjInfo)";
  }

  std::cout << nkid << scontext << sef << objstr;
  if (auto scope = getFileScope()) {
    std::cout << " " << scope->getName() << ":"
      << "[" << m_r.line0 << "@" << m_r.char0 << ", "
      << m_r.line1 << "@" << m_r.char1 << "]";
  }
  std::cout << "\n";
}

void Construct::dump(int spc, AnalysisResultConstPtr ar) {
  dumpNode(spc);
  for (int i = 0, n = getKidCount(); i < n; i++) {
    if (auto kid = getNthKid(i)) {
      kid->dump(spc + 2, ar);
    }
  }

  if (is(KindOfClosureExpression)) {
    static_cast<ClosureExpression*>(this)->getClosureFunction()->dump(spc, ar);
  }
}

void Construct::parseTimeFatal(FileScopeRawPtr fs,
                               Compiler::ErrorType err, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  if (err != Compiler::NoError) Compiler::Error(err, shared_from_this());
  throw ParseTimeFatalException(fs->getName(), m_r.line0,
                                "%s", msg.c_str());
}

void Construct::analysisTimeFatal(Compiler::ErrorType err,
                                  const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string msg;
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  assert(err != Compiler::NoError);
  Compiler::Error(err, shared_from_this());
  throw AnalysisTimeFatalException(getFileScope()->getName(), m_r.line0,
                                   "%s [analysis]", msg.c_str());
}
