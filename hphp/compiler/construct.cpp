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

#include "hphp/compiler/construct.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/util/util.h"

#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/ast_walker.h"

#include "hphp/compiler/statement/function_statement.h"

#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/expression/simple_variable.h"
#include "hphp/compiler/expression/closure_expression.h"
#include <iomanip>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

Construct::Construct(BlockScopePtr scope, LocationPtr loc)
    : m_blockScope(scope), m_flagsVal(0), m_loc(loc),
      m_containedEffects(0), m_effectsTag(0) {
}

void Construct::resetScope(BlockScopeRawPtr scope, bool resetOrigScope) {
  setBlockScope(scope);
  if (resetOrigScope) {
    ExpressionPtr expr =
      dynamic_pointer_cast<Expression>(shared_from_this());
    if (expr) {
      expr->setOriginalScope(scope);
    }
  }
  for (int i = 0, n = getKidCount(); i < n; i++) {
    if (ConstructPtr kid = getNthKid(i)) {
      if (FunctionWalker::SkipRecurse(kid)) continue;
      kid->resetScope(scope, resetOrigScope);
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
      if (FunctionWalker::SkipRecurse(child)) continue;
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
  return Expression::MakeConstant(ar, getScope(), getLocation(), value);
}

ExpressionPtr Construct::makeScalarExpression(AnalysisResultConstPtr ar,
                                               const Variant &value) const {
  return Expression::MakeScalarExpression(ar, getScope(), getLocation(), value);
}

std::string Construct::getText(bool useCache /* = false */,
                               bool translate /* = false */,
                               AnalysisResultPtr ar
                               /* = AnalysisResultPtr() */) {
  std::string &text = m_text;
  if (useCache && !text.empty()) return text;
  std::ostringstream o;
  CodeGenerator cg(&o, CodeGenerator::PickledPHP);
  cg.translatePredefined(translate);
  outputPHP(cg, ar);
  text = o.str();
  return text;
}

void Construct::serialize(JSON::CodeError::OutputStream &out) const {
  JSON::CodeError::ListStream ls(out);
  ls << m_loc->file << m_loc->line0 << m_loc->char0 <<
                       m_loc->line1 << m_loc->char1;
  ls.done();
}

void Construct::addUserFunction(AnalysisResultPtr ar,
                                const std::string &name) {
  if (!name.empty()) {
    if (ar->getPhase() == AnalysisResult::AnalyzeAll ||
        ar->getPhase() == AnalysisResult::AnalyzeFinal) {
      FunctionScopePtr func = getFunctionScope();
      getFileScope()->addFunctionDependency(ar, name, func &&
                                            func->isInlined());
    }
  }
}

void Construct::addUserClass(AnalysisResultPtr ar,
                             const std::string &name) {
  if (!name.empty()) {
    if (ar->getPhase() == AnalysisResult::AnalyzeAll ||
        ar->getPhase() == AnalysisResult::AnalyzeFinal) {
      getFileScope()->addClassDependency(ar, name);
    }
  }
}

void Construct::printSource(CodeGenerator &cg) {
  if (m_loc) {
    cg_printf("/* SRC: %s line %d */\n", m_loc->file, m_loc->line0);
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
  unsigned id = 0;
  ExpressionPtr idPtr = ExpressionPtr();
  ExpressionPtr idCsePtr = ExpressionPtr();
  int ef = 0;

  if (Statement *s = dynamic_cast<Statement*>(this)) {
    Statement::KindOf stype = s->getKindOf();
    name = Statement::Names[stype];
    value = s->getName();
    type = (int)stype;
  } else if (Expression *e = dynamic_cast<Expression*>(this)) {
    id = e->getCanonID();
    idPtr = e->getCanonLVal();
    idCsePtr = e->getCanonCsePtr();

    ef = e->getLocalEffects();

    Expression::KindOf etype = e->getKindOf();
    name = Expression::Names[etype];
    switch (etype) {
      case Expression::KindOfSimpleFunctionCall:
        value = static_cast<SimpleFunctionCall*>(e)->getName();
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
    if (c & Expression::NoRefWrapper) {
      scontext += "|NoRefWrapper";
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

    if (e->getActualType()) {
      type_info = e->getActualType()->toString();
      if (e->getExpectedType()) {
        type_info += ":" + e->getExpectedType()->toString();
      } else {
        type_info += ":";
      }
      if (e->getImplementedType()) {
        type_info += ";" + e->getImplementedType()->toString();
      } else {
        type_info += ";";
      }
      if (e->getAssertedType()) {
        type_info += "!" + e->getAssertedType()->toString();
      } else {
        type_info += "!";
      }
      type_info = "{" + type_info + "} ";
    }
  } else {
    assert(FALSE);
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
  if (id) {
    std::cout << "id=" << id << " ";
  }
  if (idPtr) {
    std::cout << "idp=0x" <<
      std::hex << std::setfill('0') << std::setw(10) <<
      (int64_t)idPtr.get() << " ";
  }
  if (idCsePtr) {
    std::cout << "idcsep=0x" <<
      std::hex << std::setfill('0') << std::setw(10) <<
      (int64_t)idCsePtr.get() << " ";
  }

  if (value != "") {
    std::cout << "[" << value << "] ";
  }

  string sef;
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

  string localtered;
  if (Expression *e = dynamic_cast<Expression*>(this)) {
    localtered = e->isLocalExprAltered() ? "LocalAltered" : "NotLocalAltered";
  }
  if (localtered != "") {
    localtered = " (" + localtered + ")";
  }

  string refstr;
  if (dynamic_cast<SimpleVariable*>(this) != nullptr) {
    if (isReferencedValid()) {
      if (isReferenced()) {
        refstr += ",Referenced";
      } else {
        refstr += ",NotReferenced";
      }
    }
    if (!maybeRefCounted()) refstr += ",NotRefCounted";
    if (!maybeInited()) refstr += ",NotInited";
    if (isNonNull()) refstr += ",NotNull";
    if (refstr.empty()) refstr = ",NoRefInfo";
  }
  if (refstr != "") refstr = " (" + refstr.substr(1) + ")";

  string objstr;
  if (dynamic_cast<SimpleVariable*>(this) != nullptr) {
    if (isNeededValid()) {
      if (isNeeded()) {
        objstr += "Object";
      } else {
        objstr += "NotObject";
      }
    } else {
      objstr = "NoObjInfo";
    }
  }
  if (objstr != "") objstr = " (" + objstr + ")";

  string noremoved;
  if (isNoRemove()) {
    noremoved = " (NoRemove)";
  }

  std::cout << type_info << nkid << scontext << sef
    << localtered << refstr << objstr << noremoved;
  if (m_loc) {
    std::cout << " " << m_loc->file << ":" <<
      m_loc->line1 << "@" << m_loc->char1;
  }
  std::cout << "\n";
}

class ConstructDumper : public FunctionWalker {
public:
  ConstructDumper(int spc, AnalysisResultConstPtr ar,
                  bool functionOnly = false) :
      m_spc(spc), m_ar(ar), m_functionOnly(functionOnly), m_showEnds(true) {}

  void walk(AstWalkerStateVec state,
            ConstructRawPtr endBefore, ConstructRawPtr endAfter) {
    AstWalker::walk(*this, state, endBefore, endAfter);
  }
  int before(ConstructRawPtr cp) {
    int ret = m_functionOnly ? FunctionWalker::before(cp) : WalkContinue;
    cp->dumpNode(m_spc, m_ar);
    m_spc += 2;
    m_showEnds = false;
    return ret;
  }
  int after(ConstructRawPtr cp) {
    if (m_showEnds) {
      int s = m_spc;
      while (s > 0) {
        int n = s > 10 ? 10 : s;
        std::cout << ("          "+10-n);
        s -= n;
      }
      std::cout << "<<";
      cp->dumpNode(0, m_ar);
    }
    m_spc -= 2;
    // HACK: dump the closure function as a "child" of the
    // closure expression
    ClosureExpressionPtr c =
      dynamic_pointer_cast<ClosureExpression>(cp);
    if (c) {
      c->getClosureFunction()->dump(m_spc, m_ar);
    }
    return WalkContinue;
  }
private:
  int m_spc;
  AnalysisResultConstPtr m_ar;
  bool m_functionOnly;
  bool m_showEnds;
};

void Construct::dump(int spc, AnalysisResultConstPtr ar) {
  ConstructDumper cd(spc, ar);
  cd.walk(AstWalkerStateVec(ConstructRawPtr(this)),
    ConstructRawPtr(), ConstructRawPtr());
}

void Construct::dump(int spc, AnalysisResultConstPtr ar, bool functionOnly,
                     const AstWalkerStateVec &state,
                     ConstructPtr endBefore, ConstructPtr endAfter) {
  ConstructDumper cd(spc, ar, functionOnly);
  cd.walk(state, endBefore, endAfter);
}

void Construct::parseTimeFatal(Compiler::ErrorType err, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  va_end(ap);

  if (err != Compiler::NoError) Compiler::Error(err, shared_from_this());
  throw ParseTimeFatalException(m_loc->file, m_loc->line0, "%s", msg.c_str());
}
