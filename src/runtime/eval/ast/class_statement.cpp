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

#include <runtime/eval/ast/expression.h>
#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/runtime/eval_object_data.h>
#include <runtime/eval/ast/statement_list_statement.h>
#include <runtime/eval/ast/use_trait_statement.h>
#include <runtime/eval/ast/trait_prec_statement.h>
#include <runtime/eval/ast/trait_alias_statement.h>
#include <runtime/eval/ast/user_attribute.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/strict_mode.h>
#include <runtime/base/runtime_option.h>
#include <runtime/eval/eval.h>
#include <util/util.h>
#include <runtime/eval/ast/scalar_expression.h>
#include <runtime/eval/ast/array_expression.h>
#include <runtime/ext/ext_class.h>

namespace HPHP {
namespace Eval {

///////////////////////////////////////////////////////////////////////////////

static StaticString s___construct("__construct");
static StaticString s_protPrefix("\0*\0", 3);
static StaticString s_zero("\0", 1);

ClassVariable::ClassVariable(CONSTRUCT_ARGS, const string &name, int modifiers,
    ExpressionPtr value, const string &doc, ClassStatement *cls)
  : Construct(CONSTRUCT_PASS), m_name(StringData::GetStaticString(name)),
    m_modifiers(modifiers), m_value(value), m_docComment(doc), m_cls(cls) {
}

ClassVariable *ClassVariable::optimize(VariableEnvironment &env) {
  Eval::optimize(env, m_value);
  return NULL;
}

void ClassVariable::set(VariableEnvironment &env, EvalObjectData *self) const {
  if (!(m_modifiers & ClassStatement::Static)) {
    Variant val(m_value ? m_value->eval(env) : null_variant);
    if (m_modifiers & ClassStatement::Private) {
      self->o_setPrivate(env.currentClass(), m_name, val);
    } else if (!self->o_exists(m_name)) {
      self->o_i_set(m_name, val);
    }
  }
}
void ClassVariable::setStatic(VariableEnvironment &env, LVariableTable &st)
  const {
  if ((m_modifiers & ClassStatement::Static)) {
    Variant val;
    if (m_value) {
      val = m_value->eval(env);
    }
    st.get(String(m_name)) = val;
  }
}

void ClassVariable::dump(std::ostream &out) const {
  ClassStatement::dumpModifiers(out, m_modifiers, true);
  out << "$" << m_name->data();
  if (m_value) {
    out  << " = ";
    m_value->dump(out);
  }
  out << ";";
}

void ClassVariable::getInfo(ClassInfo::PropertyInfo &info) const {
  int attr = 0;
  if (m_modifiers & ClassStatement::Protected) attr |= ClassInfo::IsProtected;
  if (m_modifiers & ClassStatement::Private) attr |= ClassInfo::IsPrivate;
  if (attr == 0) attr |= ClassInfo::IsPublic;
  if (m_modifiers & ClassStatement::Static) attr |= ClassInfo::IsStatic;
  info.attribute = (ClassInfo::Attribute)attr;
  info.name = m_name;
  if (!m_docComment.empty()) {
    info.docComment = m_docComment.c_str();
  }
}

void ClassVariable::eval(VariableEnvironment &env, Variant &res) const {
  res = m_value ? m_value->eval(env) : null_variant;
}

ClassStatement::ClassStatement(STATEMENT_ARGS, const string &name,
                               const string &parent, const string &doc)
  : Statement(STATEMENT_PASS), m_name(StringData::GetStaticString(name)),
    m_modifiers(0), m_attributes(0), m_needCheckHoist(-1),
    m_parent(StringData::GetStaticString(parent)), m_docComment(doc) { }

void ClassStatement::finish() {
  if (findMethod("__get", false, false, false)) {
    m_attributes |= ObjectData::UseGet;
  }
  if (findMethod("__set", false, false, false)) {
    m_attributes |= ObjectData::UseSet;
  }
  if (findMethod("__isset", false, false, false)) {
    m_attributes |= ObjectData::UseIsset;
  }
  if (findMethod("__unset", false, false, false)) {
    m_attributes |= ObjectData::UseUnset;
  }
  if (findMethod("__call", false, false, false)) {
    m_attributes |= ObjectData::HasCall;
  }
  if (findMethod("__callstatic", false, false, false)) {
    m_attributes |= ObjectData::HasCallStatic;
  }
  const MethodStatement *c = findMethod("__construct", false, false, false);
  if (!c && !(m_modifiers & ClassStatement::Trait)) {
    c = findMethod(m_name->data(), false, false, false);
  }
  if (c) {
    if (c && (c->getModifiers() & ClassStatement::Static)) {
      raise_error("Constructor %s::%s() cannot be static",
                  m_name->data(), c->name().data());
    }
    const_cast<MethodStatement*>(c)->setConstructor();
  }
}

bool ClassStatement::isHoistable(std::set<StringData *> &seen) {
  // shouldn't be called twice on the same class
  assert(m_needCheckHoist == -1);
  m_needCheckHoist = 1;

  if (!m_bases.empty()) return false;
  if (!m_useTraitsVec.empty()) return false;

  if (m_parent->empty() ||
      seen.find(m_parent) != seen.end() ||
      (ClassInfo::FindSystemClass(m_parent))) {
    m_needCheckHoist = 0;
    seen.insert(m_name);
  }
  return true;
}

const ClassStatement *ClassStatement::parentStatement() const {
  if (!m_parent->empty()) {
    return RequestEvalState::findClass(m_parent, true);
  }
  return NULL;
}

void ClassStatement::loadInterfaceStatements() const {
  for (unsigned int i = 0; i < m_bases.size(); ++i) {
    RequestEvalState::findClass(m_bases[i], true);
  }
}

void ClassStatement::loadMethodTable(ClassEvalState &ce) const {
  // make sure classes don't define each other as their
  // own parents (no cycles in the inheritance tree).
  // omit this check for builtin classes (since we assume
  // that our builtin runtime is sane, plus it's impossible
  // for a builtin class to extends a user-defind class).
  std::set<const ClassStatement*> &seen = ce.getSeen();
  if (seen.find(this) != seen.end()) {
    raise_error("%s is defined as its own parent", name().c_str());
  } else {
    seen.insert(this);
  }
  ce.setAttributes(m_attributes);
  ClassEvalState::MethodTable &mtable = ce.getMethodTable();
  if (!m_parent->empty()) {
    const ClassStatement* parent_cls = parentStatement();
    if (parent_cls) {
      parent_cls->loadMethodTable(ce);
    } else {
      // Built in
      ClassInfo::MethodVec meths;
      ClassInfo::GetClassMethods(meths, m_parent, 1);
      if (const ClassInfo *info =
          ClassInfo::FindClassInterfaceOrTrait(m_parent)) {
        int attr = info->getAttribute(), o_attr = 0;
        if (attr & ClassInfo::HasCall) {
          o_attr |= ObjectData::HasCall;
        }
        if (attr & ClassInfo::HasCallStatic) {
          o_attr |= ObjectData::HasCallStatic;
        }
        ce.setAttributes(o_attr);
      }
      for (ClassInfo::MethodVec::const_iterator it = meths.begin();
           it != meths.end(); ++it) {
        int mods = 0;
        if ((*it)->attribute & ClassInfo::IsPrivate) mods |= Private;
        else if ((*it)->attribute & ClassInfo::IsProtected) mods |= Protected;
        else mods |= Public;
        MethodStatementWrapper &p = mtable[(*it)->name];
        p.m_access = mods;
      }
    }
  }
  for (vector<MethodStatementPtr>::const_iterator it = m_methodsVec.begin();
       it != m_methodsVec.end(); ++it) {
    ClassEvalState::MethodTable::iterator mit =
      mtable.find((*it)->name());
    if (mit != mtable.end()) {
      int mods = mit->second.m_access;
      const MethodStatement *mmit = mit->second.m_methodStatement;
      if (mods & Final) {
        static StringData* sd___MockClass =
          StringData::GetStaticString("__MockClass");
        if (m_userAttributes.find(sd___MockClass) == m_userAttributes.end()) {
          const Location *loc = (*it).get()->loc();
          set_line(loc->line0, loc->char0, loc->line1, loc->char1);
          throw FatalErrorException(0,
                                    "Cannot override final method %s::%s()",
                                    mmit->getClass()->name().c_str(),
                                    (*it)->name().c_str());
        }
      } else if ((mods & (Public|Protected|Private)) <
                 ((*it)->getModifiers() & (Public|Protected|Private))) {
        const char *al = "public";
        if (mods & Protected) {
          al = "protected";
        } else if (mods & Private) {
          al = "private";
        }
        const Location *loc = (*it).get()->loc();
        set_line(loc->line0, loc->char0, loc->line1, loc->char1);
        throw FatalErrorException(0,
                                  "Access level to %s must be %s or weaker "
                                  "(as in class %s)",
                                  (*it)->name().c_str(), al,
                                  mmit ?
                                  mmit->getClass()->name().c_str() :
                                  m_parent->data());
      }
      mit->second.m_methodStatement = it->get();
      mit->second.m_access = (*it)->getModifiers();
      mit->second.m_className = m_name;
    } else {
      MethodStatementWrapper &p = mtable[(*it)->name()];
      p.m_methodStatement = it->get();
      p.m_access = (*it)->getModifiers();
      p.m_className = m_name;
    }
  }

  StringIMap<MethodStatementPtr>::const_iterator it = m_methods.find(m_name);
  if (it != m_methods.end()) {
    ce.getConstructorWrapper() =
      MethodStatementWrapper(it->second.get(), 0, m_name);
  }
  it = m_methods.find("__construct");
  if (it != m_methods.end()) {
    ce.getConstructorWrapper() =
      MethodStatementWrapper(it->second.get(), 0, m_name);
  }
  addTraits(ce);
  bindTraits(ce);
}

void ClassStatement::addTrait(ClassEvalState &ce,
                              const UseTraitStatement *useTraitStmt) const {
  const std::vector<NamePtr> &traitNames = useTraitStmt->getNames();
  const std::vector<StatementPtr> &traitRules =
    useTraitStmt->getStmts()->stmts();
  for (unsigned int i = 0; i < traitNames.size(); i++) {
    String traitName = traitNames[i]->get();
    const ClassStatement *trait =
      RequestEvalState::findClass(traitName, true);
    if (!trait) {
      raise_error("Trait '%s' does not exist.", traitName.c_str());
    }
    if (!trait->isTrait()) {
      raise_error("%s cannot use %s - it is not a trait",
                  ce.getClass()->name().c_str(), traitName.c_str());
    }
    ce.implementTrait(trait);
  }
  for (unsigned int i = 0; i < traitRules.size(); i++) {
    TraitPrecStatement *traitPrec =
      dynamic_cast<TraitPrecStatement*>(traitRules[i].get());
    if (traitPrec) {
      ClassEvalState::TraitPrecedence tp(traitPrec->getTraitName()->get(),
                                         traitPrec->getMethodName()->get(),
                                         traitPrec);
      ce.getTraitPrecedences().push_back(tp);
      continue;
    }
    TraitAliasStatement *traitAlias =
      dynamic_cast<TraitAliasStatement*>(traitRules[i].get());
    if (traitAlias) {
      ClassEvalState::TraitAlias ta(traitAlias->getTraitName()->get(),
                                    traitAlias->getMethodName()->get(),
                                    traitAlias->getNewMethodName()->get(),
                                    traitAlias->getModifiers());
      ce.getTraitAliases().push_back(ta);
      continue;
    }
    assert(false);
  }
}

void ClassStatement::addTraits(ClassEvalState &ce) const {
  ce.initTraits(this);
  for (unsigned int i = 0; i < m_useTraitsVec.size(); i++) {
    addTrait(ce, m_useTraitsVec[i].get());
  }
}

void ClassStatement::initTraitStructures(ClassEvalState &ce) const {
  std::vector<ClassEvalState::TraitPrecedence> &traitPrecedences =
    ce.getTraitPrecedences();
  std::vector<ClassEvalState::TraitAlias> &traitAliases =
    ce.getTraitAliases();

  // resolve class references
  for (unsigned int i = 0 ; i < traitPrecedences.size(); i++) {
    ClassEvalState::TraitPrecedence &curPrecedence = traitPrecedences[i];
    curPrecedence.m_classEvalState =
      RequestEvalState::findClassState(curPrecedence.m_trait, true);
    const std::vector<NamePtr> &names = curPrecedence.m_prec->getNames();
    curPrecedence.m_excludeFromClasses.resize(names.size());
    for (unsigned int j = 0; j < names.size(); j++) {
      curPrecedence.m_excludeFromClasses[j] =
        RequestEvalState::findClassState(names[j]->get(), true);
    }
  }

  for (unsigned int i = 0 ; i < traitAliases.size(); i++) {
    ClassEvalState::TraitAlias &curAlias = traitAliases[i];
    curAlias.m_classEvalState =
      RequestEvalState::findClassState(curAlias.m_trait, true);
  }
}

void ClassStatement::bindMethods(ClassEvalState &ce) const {
  std::vector<const ClassStatement *> &traits = ce.getTraits();
  std::vector<ClassEvalState::MethodTable> methodTables;
  ClassEvalState::MethodTable resultTable;
  unsigned int numTraits =  traits.size();
  methodTables.resize(numTraits);

  // prepare copies of trait method tables for combination
  for (unsigned int i = 0; i < numTraits; i++) {
    std::vector<ClassEvalState::TraitPrecedence> &traitPrecedences =
      ce.getTraitPrecedences();
    std::vector<ClassEvalState::TraitAlias> &traitAlias = ce.getTraitAliases();

    StringISet excludeTable;
    if (traitPrecedences.size()) {
      ce.compileExcludeTable(excludeTable, traitPrecedences, traits[i]);
    }
    ce.copyTraitMethodTable(methodTables[i], traits[i],
                            traitAlias, excludeTable);
  }

  // now merge trait methods
  for (unsigned int i = 0; i < numTraits; i++) {
    ce.mergeTraitMethods(methodTables[i], i, numTraits, methodTables,
                         resultTable);
  }

  // Now the resultTable contains all trait methods we would have to
  // add to the class. The methods are inserted into the method table.
  // All inherited methods are overridden, methods defined in the class are
  // left untouched
  ce.mergeTraitMethodsToClass(resultTable, this);
}

bool ClassStatement::checkCompatible(
  ClassEvalState &ce, const std::vector<ClassVariable *> &variableVec,
  unsigned int currentTrait, ClassVariable *cv) const {
  ClassVariable *colliding = NULL;
  bool compatible = true;
  for (unsigned i = 0; i < variableVec.size(); i++) {
    if (!variableVec[i]->name()->same(cv->name().get())) continue;
    colliding = variableVec[i];
    if ((colliding->getModifiers() & (AccessMask | Static)) !=
        (cv->getModifiers() & (AccessMask | Static))) {
      compatible = false;
      break;
    }
    const Expression *val1 = colliding->getValue();
    const Expression *val2 = cv->getValue();
    if (!val1 && !val2) continue;
    if (!val1 || !val2) {
      compatible = false;
      break;
    }
    bool sc1 = val1->isKindOf(Expression::KindOfScalarExpression) ||
               val1->isKindOf(Expression::KindOfScalarValueExpression);
    bool sc2 = val2->isKindOf(Expression::KindOfScalarExpression) ||
               val2->isKindOf(Expression::KindOfScalarValueExpression);
    if (!sc1 || !sc2) {
      compatible = false;
      break;
    }
    Variant v1;
    Variant v2;
    DummyVariableEnvironment env;
    bool ret ATTRIBUTE_UNUSED = val1->evalStaticScalar(env, v1);
    ASSERT(ret);
    ret = val2->evalStaticScalar(env, v2);
    ASSERT(ret);
    if (!same(v1, v2) || v1.isArray() || v2.isArray()) {
      compatible = false;
      break;
    }
  }
  if (!colliding) {
    ASSERT(compatible);
    return true;
  }
  if (compatible) {
    const ClassVariable *firstDef = findFirstDef(ce, currentTrait, colliding);
    raise_strict_warning("%s and %s define the same property ($%s) in "
      "the composition of %s. This might be incompatible, to improve "
      "maintainability consider using accessor methods in traits "
      "instead. Class was composed",
      firstDef->getClass()->name().c_str(),
      cv->getClass()->name().c_str(), cv->name().c_str(),
      m_name->data());
  } else {
    const ClassVariable *firstDef = findFirstDef(ce, currentTrait, colliding);
    raise_error("%s and %s define the same property ($%s) in the "
                "composition of %s. However, the definition differs "
                "and is considered incompatible. Class was composed",
                firstDef->getClass()->name().c_str(),
                cv->getClass()->name().c_str(), cv->name().c_str(),
                m_name->data());
  }
  return compatible;
}

const ClassVariable *ClassStatement::findFirstDef(
  ClassEvalState &ce, unsigned int currentTrait,
  const ClassVariable *colliding) const {
  ASSERT(colliding);
  std::vector<const ClassStatement *> &traits = ce.getTraits();
  for (unsigned i = 0; i < currentTrait && i < traits.size(); i++) {
    const ClassVariable *cv = findVariable(colliding->name());
    if (cv) return cv;
  }
  return colliding;
}

void ClassStatement::bindProperties(ClassEvalState &ce) const {
  if (ce.getClass() != this) return;
  std::vector<const ClassStatement *> &traits = ce.getTraits();
  std::vector<ClassVariable *> traitVariables;
  StringMap<ClassVariable *> traitVariableMap;
  std::vector<ClassVariable *> variableVec;
  unsigned int numTraits =  traits.size();

  // add current class variables
  for (vector<ClassVariablePtr>::const_iterator it = m_variablesVec.begin();
       it != m_variablesVec.end(); ++it) {
    variableVec.push_back(it->get());
  }
  for (unsigned int i = 0; i < numTraits; i++) {
    const ClassStatement *trait = traits[i];
    ClassEvalState *tce = RequestEvalState::findClassState(trait->name());
    tce->semanticCheck();
    const std::vector<ClassVariable *> &tvs = tce->getTraitVariables();
    for (unsigned int j = 0; j < trait->m_variablesVec.size(); j++) {
      ClassVariable *cv = trait->m_variablesVec[j].get();
      if (checkCompatible(ce, variableVec, i, cv)) {
        traitVariables.push_back(cv);
        traitVariableMap[cv->name()] = cv;
        variableVec.push_back(cv);
      }
    }
    for (unsigned int j = 0; j < tvs.size(); j++) {
      ClassVariable *cv = tvs[j];
      if (checkCompatible(ce, variableVec, numTraits, cv)) {
        traitVariables.push_back(cv);
        traitVariableMap[cv->name()] = cv;
        variableVec.push_back(cv);
      }
    }
  }
  ce.setTraitVariables(traitVariables);
  ce.setTraitVariableMap(traitVariableMap);
}

void ClassStatement::verifyAbstractClass(ClassEvalState &ce) const {
  if (!isAbstract() && !isTrait()) {
    ClassEvalState::MethodTable &traitMethodTable = ce.getTraitMethodTable();
    ASSERT(ce.getClass() == this || traitMethodTable.empty());
    for (ClassEvalState::MethodTable::const_iterator it =
         traitMethodTable.begin(); it != traitMethodTable.end(); it++) {
      if (it->second.m_methodStatement->isAbstract()) {
        raise_error("Class %s contains abstract method %s and must therefore"
                    " be declared abstract", m_name->data(),
                    it->second.m_methodStatement->name().c_str());
      }
    }
  }
}

void ClassStatement::bindTraits(ClassEvalState &ce) const {
  std::vector<const ClassStatement *> &traits = ce.getTraits();
  if (traits.size() == 0) return;
  initTraitStructures(ce);
  bindMethods(ce);
  bindProperties(ce);
  verifyAbstractClass(ce);
}

Statement *ClassStatement::optimize(VariableEnvironment &env) {
  std::vector<ClassVariablePtr> variablesVec = m_variablesVec;
  for (unsigned int i = 0; i < m_variablesVec.size(); i++) {
    Eval::optimize(env, m_variablesVec[i]);
  }
  // shared with m_variables so cannot change
  for (unsigned int i = 0; i < variablesVec.size(); i++) {
    assert(variablesVec[i].get() == m_variablesVec[i].get());
  }
  for (StringMap<ExpressionPtr>::iterator it = m_constants.begin();
       it != m_constants.end(); it++) {
    Eval::optimize(env, it->second);
  }
  std::vector<MethodStatementPtr> methodsVec = m_methodsVec;
  for (unsigned int i = 0; i < m_methodsVec.size(); i++) {
    Eval::optimize(env, m_methodsVec[i]);
  }
  // shared with m_methods so cannot change
  for (unsigned int i = 0; i < m_methodsVec.size(); i++) {
    assert(methodsVec[i].get() == m_methodsVec[i].get());
  }
  return NULL;
}

void ClassStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  evalImpl(env);
}

bool ClassStatement::checkHoist() const {
  if (!m_parent->empty() &&
      !RequestEvalState::findClassState(m_parent, false)) {
    return false;
  }
  for (unsigned int i = 0; i < m_bases.size(); i++) {
    if (RequestEvalState::findClassState(m_bases[i], false)) return false;
  }
  for (unsigned int i = 0; i < m_useTraitsVec.size(); i++) {
    const std::vector<NamePtr> &traitNames = m_useTraitsVec[i]->getNames();
    for (unsigned int j = 0; j < traitNames.size(); j++) {
      if (RequestEvalState::findClassState(traitNames[j]->get(), false)) {
        return false;
      }
    }
  }
  return true;
}

void ClassStatement::evalImpl(VariableEnvironment &env,
                              bool fromMarker /* = false */) const {
  ENTER_STMT;
  // top level class statement must have m_needCheckHoist set to 0 or 1
  ASSERT(!m_marker || m_needCheckHoist != -1);

  if (fromMarker || !m_marker ||
      (!m_needCheckHoist || checkHoist())) {
    RequestEvalState::declareClass(this);
  }
  const ClassStatement* parent_cls;
  if (!m_marker) {
    // Not a top level, immediately do semantic check
    ClassEvalState *ce = RequestEvalState::findClassState(name());
    ASSERT(ce);
    ce->semanticCheck();
  }
  if (RuntimeOption::EnableStrict && (parent_cls = parentStatement())) {
    for (vector<MethodStatementPtr>::const_iterator it = m_methodsVec.begin();
         it != m_methodsVec.end(); ++it) {

      const MethodStatement *child_method = it->get();

      const ClassStatement* cs = this;
      while ((cs = cs->parentStatement())) {
        if (cs->findMethod(child_method->name())) {
          break;
        }
      }
      if (cs) {
        const MethodStatement*   parent_method =
          cs->findMethod(child_method->name());

        ASSERT(parent_method);

        const vector<ParameterPtr>& child_params = child_method->getParams();
        const vector<ParameterPtr>& parent_params = parent_method->getParams();

        vector<ParameterPtr>::const_iterator pchild = child_params.begin();
        vector<ParameterPtr>::const_iterator pparent = parent_params.begin();

        for (; pchild != child_params.end() && pparent != parent_params.end();
             ++pchild, ++pparent) {
          if ((*pchild)->isOptional() != (*pparent)->isOptional()) {
            throw_strict(MethodSignatureChangeException(
                           child_method->name().c_str()),
                         StrictMode::StrictBasic);
          }
        }

        // more parameters in child than parent
        for (; pchild != child_params.end(); ++pchild) {
          // TODO? do we want the child to allow optional extra parameters ?
          throw_strict(MethodSignatureChangeException(
                         child_method->name().c_str()),
                       StrictMode::StrictBasic);
        }

        // more parameters in parent than child
        if (pparent != parent_params.end()) {
          throw_strict(MethodSignatureChangeException(
                         child_method->name().c_str()),
                       StrictMode::StrictBasic);
        }
      }
    }
  }
}

Object ClassStatement::create(ClassEvalState &ce,
                              ObjectData* root /* = NULL*/) const {
  if (getModifiers() & (Abstract|Interface|Trait)) {
    throw FatalErrorException(0, "Cannot instantiate %s %s",
                              getModifiers() & Trait ? "trait" :
                              getModifiers() & Interface ?
                              "interface" : "abstract class",
                              name().c_str());
  }

  ce.initializeInstance();

  // Only need a parent for the dynamic class if the parent
  // is builtin or compiled
  const char *builtinParent = NULL;
  const ClassStatement *cls = this;
  while (!cls->parent().empty()) {
    const ClassStatement *pcls = cls->parentStatement();
    if (!pcls) {
      builtinParent = cls->parent().c_str();
      break;
    }
    cls = pcls;
  }

  return NEWOBJ(EvalObjectData)(ce, builtinParent, root);
}

void ClassStatement::initializeObject(
  ClassEvalState &ce, EvalObjectData *obj) const {
  DummyVariableEnvironment env;
  env.setCurrentClass(m_name->data());
  for (vector<ClassVariablePtr>::const_iterator it = m_variablesVec.begin();
       it != m_variablesVec.end(); ++it) {
    (*it)->set(env, obj);
  }
  const vector<ClassVariable *> &traitVariables = ce.getTraitVariables();
  for (vector<ClassVariable *>::const_iterator it = traitVariables.begin();
       it != traitVariables.end(); ++it) {
    (*it)->set(env, obj);
  }
  const ClassStatement *cls = parentStatement();
  if (cls) {
    ClassEvalState *pce = ce.getParentClassEvalState();
    cls->initializeObject(*pce, obj);
  }
}

void ClassStatement::initializeStatics(ClassEvalState &ce,
                                       LVariableTable &statics) const {
  DummyVariableEnvironment env;
  for (vector<ClassVariablePtr>::const_iterator it = m_variablesVec.begin();
       it != m_variablesVec.end(); ++it) {
    (*it)->setStatic(env, statics);
  }
  const vector<ClassVariable *> &traitVariables = ce.getTraitVariables();
  for (vector<ClassVariable *>::const_iterator it =
       traitVariables.begin(); it != traitVariables.end(); ++it) {
    (*it)->setStatic(env, statics);
  }
}

void ClassStatement::addBases(const std::vector<String> &bases) {
  for (unsigned i = 0; i < bases.size(); i++) {
    m_bases.push_back(StringData::GetStaticString(bases[i].get()));
  }
}

void ClassStatement::setUserAttributes(
  const std::vector<UserAttributePtr> &elems) {
  for (unsigned i = 0; i < elems.size(); i++) {
    const StringData* name = StringData::GetStaticString(elems[i]->getName());
    if (m_userAttributes.find(name) != m_userAttributes.end()) {
      throw FatalErrorException(0, "Redeclared attribute %s", name->data());
    }
    m_userAttributes[name] = elems[i]->getExp();
  }
}

void ClassStatement::addVariable(ClassVariablePtr v) {
  if (getModifiers() & Interface) {
    throw FatalErrorException(0, "Interface %s may not include member "
                              "variable %s", name().c_str(),
                              v->name().c_str());
  }

  ClassVariablePtr &cv = m_variables[v->name()];
  if (cv) {
    throw FatalErrorException(0, "Cannot redeclare %s::$%s",
                              name().c_str(), v->name().c_str());
  }
  cv = v;
  m_variablesVec.push_back(v);
}

void ClassStatement::addMethod(MethodStatementPtr m) {
  if (m_methods.find(m->name()) != m_methods.end()) {
    raise_error("Cannot redeclare %s::%s()",
                m_name->data(), m->name().c_str());
  }
  if (m->getModifiers() & Abstract) {
    if (m->getModifiers() & Private) {
      raise_error("Cannot declare abstract %s::%s() private",
                  m_name->data(), m->name().c_str());
    }
    if (m->getModifiers() & Final) {
      raise_error("Cannot declare abstract %s::%s() final",
                  m_name->data(), m->name().c_str());
    }
    if (m->hasBody()) {
      raise_error("Abstract %s::%s() cannot contain body",
                  m_name->data(), m->name().c_str());
    }
  }
  if (!m->hasBody()) {
    if (!(m_modifiers & Interface)) {
      if (!(m->getModifiers() & Abstract)) {
        raise_error("Non-abstract %s::%s() must contain body",
                    m_name->data(), m->name().c_str());
      }
    } else if (m->getModifiers() & ~(Public | Static)) {
      raise_error("Access type for interface method %s::%s() must be "
                  "omitted", m_name->data(), m->name().c_str());
    }
  }

  m_methods[m->name()] = m;
  m_methodsVec.push_back(m);
}
void ClassStatement::addUseTrait(UseTraitStatementPtr u) {
  m_useTraitsVec.push_back(u);
}

void ClassStatement::addConstant(const string &name, ExpressionPtr v) {
  // Array is the only one allowed by the grammer but disallowed semantically
  if (v->isKindOf(Expression::KindOfArrayExpression)) {
    raise_error("Arrays are not allowed in class constants");
  }
  m_constantNames.push_back(StringData::GetStaticString(name));
  m_constants[m_constantNames.back()] = v;
}

bool ClassStatement::instanceOf(CStrRef c) const {
  if (m_name->isame(c.get())) return true;
  for (unsigned i = 0; i < m_bases.size(); i++) {
    if (m_bases[i]->isame(c.get())) return true;
  }
  for (unsigned i = 0; i < m_bases.size(); i++) {
    const ClassStatement *iface = RequestEvalState::findClass(m_bases[i]);
    if (iface) {
      if (iface->instanceOf(c)) {
        return true;
      }
    } else {
      // Class lookup failed, but it may be due to crossing the boundary into
      // the static ClassInfo data, in which case RequestEvalState::findClass()
      // ignores interfaces.
      const ClassInfo *ci;
      if ((ci = ClassInfo::FindInterface(m_bases[i]))
          && ci->derivesFrom(c, true)) {
        return true;
      }
    }
  }
  return false;
}

bool ClassStatement::subclassOf(CStrRef c) const {
  const ClassStatement *cls = this;
  while (cls) {
    if (cls->instanceOf(c)) return true;
    cls = cls->parentStatement();
  }
  return false;
}

const MethodStatement* ClassStatement::findMethod(CStrRef name,
    bool recursive /* = false */, bool interfaces /* = false */,
    bool trait /* = true */) const {
  StringIMap<MethodStatementPtr>::const_iterator it =
    m_methods.find(name);
  if (it != m_methods.end()) {
    return it->second.get();
  } else {
    if (trait && !m_useTraitsVec.empty()) {
      ClassEvalState *ce = RequestEvalState::findClassState(m_name);
      const MethodStatementWrapper *msw = ce->getTraitMethod(name);
      if (msw) return msw->m_methodStatement;
    }
    if (recursive) {
      return findParentMethod(name, interfaces);
    }
    return NULL;
  }
}

const ClassVariable* ClassStatement::findVariable(CStrRef name,
                                                  bool recursive /* = false */)
  const {
  StringMap<ClassVariablePtr>::const_iterator it = m_variables.find(name);
  if (it != m_variables.end()) {
    return it->second.get();
  } else {
    if (recursive) {
      const ClassStatement *parent = parentStatement();
      if (parent) {
        return parent->findVariable(name, true);
      }
    }
    return NULL;
  }
}

bool ClassStatement::getConstant(Variant &res, const char *c,
                                 bool recursive /* = false */) const {
  StringMap<ExpressionPtr>::const_iterator it = m_constants.find(c);
  if (it != m_constants.end()) {
    DummyVariableEnvironment env;
    res = it->second->eval(env);
    return true;
  }
  if (recursive) {
    const ClassStatement *p = parentStatement();
    if (p && p->getConstant(res, c, true)) return true;
    for (unsigned i = 0; i < m_bases.size(); i++) {
      const ClassStatement *b =
        RequestEvalState::findClass(m_bases[i], true);
      if (b && b->getConstant(res, c, true)) return true;
    }
  }
  return false;
}

String ClassStatement::resolveSpInTrait(
  VariableEnvironment &env, Name *clsName) {
  bool sp = clsName->isSp();
  assert(sp);
  const std::string &originalText = clsName->getOriginalText();
  ASSERT(!originalText.empty());
  if (originalText == "self") {
    return FrameInjection::GetClassName(false);
  }
  if (originalText == "parent") {
    return FrameInjection::GetParentClassName(false);
  }
  assert(false);
}

void ClassStatement::dump(std::ostream &out) const {
  if (m_modifiers & Abstract) {
    out << "abstract ";
  }
  if (m_modifiers & Final) {
    out << "final ";
  }
  if (m_modifiers & Interface) {
    out << "interface ";
  } else if (m_modifiers & Trait) {
    out << "trait ";
  } else {
    out << "class ";
  }
  out << m_name->data();
  if (!m_parent->empty()) {
    out << " extends " << m_parent->data();
  }
  if (!m_bases.empty()) {
    if (m_modifiers & Interface) {
      out << " extends ";
    } else {
      out << " implements ";
    }
    for (unsigned int i = 0; i < m_bases.size(); i++) {
      if (i > 0) out << ", ";
      out << m_bases[i]->data();
    }
  }
  out << " {\n";
  if (!m_constants.empty()) {
    out << "const ";
    bool first = true;
    for (unsigned i = 0; i < m_constantNames.size(); i++) {
      StringMap<ExpressionPtr>::const_iterator iter =
        m_constants.find(m_constantNames[i]);
      if (first) {
        first = false;
      } else {
        out << ", ";
      }
      out << iter->first << " = ";
      iter->second->dump(out);
    }
    out << ";\n";
  }
  for (unsigned int i = 0; i < m_variablesVec.size(); i++) {
    m_variablesVec[i]->dump(out);
    out << "\n";
  }
  for (unsigned int i = 0; i < m_useTraitsVec.size(); i++) {
    m_useTraitsVec[i]->dump(out);
  }
  for (unsigned int i = 0; i < m_methodsVec.size(); i++) {
    m_methodsVec[i]->dump(out);
  }
  out << "}\n";
}

void ClassStatement::dumpModifiers(std::ostream &out, int m, bool variable) {
  if (m & Protected) {
    out << "protected ";
  } else if (m & Private) {
    out << "private ";
  } else {
    out << "public ";
  }
  if (m & Static)   out << "static ";
  if (m & Abstract) out << "abstract ";
  if (m & Final)    out << "final ";
}

void ClassStatement::getPropertyInfo(ClassInfoEvaled &owner)  const {
  for (vector<ClassVariablePtr>::const_iterator it = m_variablesVec.begin();
       it != m_variablesVec.end(); ++it) {
    ClassInfo::PropertyInfo *p = new ClassInfo::PropertyInfo;
    (*it)->getInfo(*p);
    p->owner = &owner;
    owner.m_properties[p->name] = p;
    owner.m_propertiesVec.push_back(p);
  }
  ClassEvalState *ce = RequestEvalState::findClassState(name());
  const vector<ClassVariable *> &traitVariables = ce->getTraitVariables();
  for (vector<ClassVariable *>::const_iterator it = traitVariables.begin();
       it != traitVariables.end(); ++it) {
    ClassInfo::PropertyInfo *p = new ClassInfo::PropertyInfo;
    (*it)->getInfo(*p);
    p->owner = &owner;
    owner.m_properties[p->name] = p;
    owner.m_propertiesVec.push_back(p);
  }
}

void ClassStatement::getInfo(ClassInfoEvaled &info) const {
  int attr = 0;
  if (m_modifiers & Interface) attr |= ClassInfo::IsInterface;
  if (m_modifiers & Abstract) attr |= ClassInfo::IsAbstract;
  if (m_modifiers & Final) attr |= ClassInfo::IsFinal;
  if (m_modifiers & Trait) attr |= ClassInfo::IsTrait;
  if (attr == 0) attr = ClassInfo::IsNothing;

  info.m_attribute = (ClassInfo::Attribute)attr;
  info.m_name = m_name;
  info.m_file = m_loc.file;
  info.m_line1 = m_loc.line0;
  info.m_line2 = m_loc.line1;
  info.m_parentClass = m_parent;
  if (!m_docComment.empty()) {
    info.m_docComment = m_docComment.c_str();
  }
  for (unsigned i = 0; i < m_bases.size(); i++) {
    info.m_interfacesVec.push_back(m_bases[i]);
    info.m_interfaces.insert(m_bases[i]);
  }
  getPropertyInfo(info);
  DummyVariableEnvironment dv;
  for (std::vector<StringData*>::const_iterator it = m_constantNames.begin();
       it != m_constantNames.end(); ++it) {
    const Expression *con = m_constants.find(*it)->second.get();
    StringData *conName = *it;
    ClassInfo::ConstantInfo *c = new ClassInfo::ConstantInfo;
    c->name = conName;
    c->setValue(con->eval(dv));
    String sv = c->getValue().toString();
    char* buf = new char[sv.size()+1];
    memcpy(buf, sv.data(), sv.size()+1);
    c->valueLen = sv.size();
    c->valueText = buf;
    info.m_constants[c->name] = c;
    info.m_constantsVec.push_back(c);
  }

  for (vector<UseTraitStatementPtr>::const_iterator it = m_useTraitsVec.begin();
       it != m_useTraitsVec.end(); ++it) {
    const std::vector<NamePtr> &traitNames = (*it)->getNames();
    for (unsigned int i = 0; i < traitNames.size(); i++) {
      String traitName = traitNames[i]->get();
      if (info.m_traits.find(traitName) == info.m_traits.end()) {
        info.m_traits.insert(traitName);
        info.m_traitsVec.push_back(traitName);
      }
    }
  }
  for (vector<MethodStatementPtr>::const_iterator it = m_methodsVec.begin();
       it != m_methodsVec.end(); ++it) {
    if (ParserBase::IsAnonFunctionName((*it)->name())) continue;
    ClassInfo::MethodInfo *m = new ClassInfo::MethodInfo;
    (*it)->getInfo(*m);
    info.m_methods[(*it)->name()] = m;
    info.m_methodsVec.push_back(m);
  }
  ClassEvalState *ce = RequestEvalState::findClassState(m_name);
  ClassEvalState::MethodTable &traitMethodTable = ce->getTraitMethodTable();
  for (ClassEvalState::MethodTable::const_iterator it =
       traitMethodTable.begin(); it != traitMethodTable.end(); it++) {
    const MethodStatement *ms = it->second.m_methodStatement;
    if (ParserBase::IsAnonFunctionName(ms->name())) continue;
    ClassInfo::MethodInfo *m = new ClassInfo::MethodInfo;
    int access = it->second.m_access;
    ms->getInfo(*m, access);
    info.m_methods[ms->name()] = m;
    info.m_methodsVec.push_back(m);
  }
  const std::vector<ClassEvalState::TraitAlias> &traitAliases =
    ce->getTraitAliases();
  for (unsigned int i = 0; i < traitAliases.size(); i++) {
    const ClassEvalState::TraitAlias &traitAlias = traitAliases[i];
    info.m_traitAliasesVec.push_back(std::pair<String, String>(
      traitAlias.m_alias, traitAlias.getFullName()));
  }
}

bool ClassStatement::checkPropExist(CStrRef prop) const {
  StringMap<ClassVariablePtr>::const_iterator it = m_variables.find(prop);
  if (it != m_variables.end()) return true;
  const ClassStatement *par = parentStatement();
  return par && par->checkPropExist(prop);
}

static void AddVariable(const ClassVariable *v, const Array *prv,
                        CStrRef cname, Array &props, Array &dyn_props) {
  if (v->getModifiers() & ClassStatement::Static) return;
  if (v->getModifiers() & ClassStatement::Private) {
    if (!prv) return;
    String name = v->name();
    CVarRef val = prv->rvalAtRef(name, AccessFlags::Key);
    if (!val.isInitialized()) return;
    props.lvalAt(concat4(s_zero, cname, s_zero, name), AccessFlags::Key).
      setWithRef(val);
    return;
  }

  String name = v->name();
  if (v->getModifiers() & ClassStatement::Protected) {
    if (prv) {
      CVarRef val = dyn_props.rvalAtRef(name, AccessFlags::Key);
      if (&val == &null_variant) return;
      props.lvalAt(s_protPrefix + name, AccessFlags::Key).
        setWithRef(val);
    }
  } else {
    CVarRef val = dyn_props.rvalAtRef(name, AccessFlags::Key);
    if (&val == &null_variant) return;
    props.lvalAt(name, AccessFlags::Key).setWithRef(val);
  }
  dyn_props.remove(name, true);
}

void ClassStatement::getArray(Array &props, Array &dyn_props,
                              ClassEvalState *ce,
                              const Array *privates) const {
  StrNR cname = StrNR(m_name);
  Array prv, *pprv = 0;
  if (privates) {
    CVarRef pr = privates->rvalAtRef(cname);
    if (pr.isArray()) {
      prv = pr.toCArrRef();
    }
    pprv = &prv;
  }

  for (size_t i = 0, s = m_variablesVec.size(); i < s; i++) {
    const ClassVariable *v = m_variablesVec[i].get();
    AddVariable(v, pprv, cname, props, dyn_props);
  }

  const vector<ClassVariable *> &traitVariables = ce->getTraitVariables();
  for (size_t i = 0, s = traitVariables.size(); i < s; i++) {
    const ClassVariable *v = traitVariables[i];
    AddVariable(v, pprv, cname, props, dyn_props);
  }

  ClassEvalState *pce = ce->getParentClassEvalState();
  if (pce) {
    pce->getClass()->getArray(props, dyn_props, pce, privates);
  }
}

bool ClassStatement::hasAccess(CStrRef context, Modifier level) const {
  ASSERT(context);
  switch (level) {
  case Public: return true;
  case Private: return context->isame(m_name);
  case Protected:
    {
      if (!*context) return false;
      if (context->isame(m_name)) return true;
      const ClassStatement *cls = RequestEvalState::findClass(context);
      return cls->subclassOf(m_name) || subclassOf(context);
    }
  default:
    ASSERT(false);
    return true;
  }
}

bool ClassStatement::attemptPropertyAccess(CStrRef prop, CStrRef context,
                                           int &mods,
                                           bool rec /* = false */) const {
  if (g_context->getDebuggerBypassCheck()) {
    return true;
  }
  StringMap<ClassVariablePtr>::const_iterator it = m_variables.find(prop);
  if (it == m_variables.end()) {
    const ClassStatement *par = parentStatement();
    if (par) {
      return par->attemptPropertyAccess(prop, context, mods, true);
    }
    // Var doesn't exist
    return true;
  }
  mods = it->second->getModifiers();
  Modifier level = Public;
  if (mods & Private) {
    level = Private;
  } else if (mods & Protected) {
    level = Protected;
  }
  // Var is private in superclass, treat an new
  if (rec && level == Private && (mods & Static) == 0) return true;
  return hasAccess(context, level);
}

void ClassStatement::failPropertyAccess(CStrRef prop, CStrRef context,
    int mods) const {
  const char *mod = "protected";
  Modifier level = Public;
  if (mods & Private) level = Private;
  else if (mods & Protected) level = Protected;
  if (level == ClassStatement::Private) mod = "private";
  throw FatalErrorException(0, "Attempt to access %s %s::%s%s%s",
      mod, m_name->data(), prop.data(),
      *context->data() ? " from " : "",
      *context->data() ? context->data() : "");
}

///////////////////////////////////////////////////////////////////////////////
// Semantic Checking
//
// This section defines the logic to check the semantics of class derivation.
// The semantic checking is segmented into a few templated functions which
// implement the core of the logic. These templated functions are designed
// to be generic enough to work with both hphpi AST trees, and the
// corresponding ClassInfo data structures for builtin functions.

/**
 * The SemanticExtractor is glue which makes the templated semantic checking
 * functions work. It allows the logic functions to implement semantic
 * checks agnostic to what kind of data structures are present. This is what
 * allows us to compare AST trees with ClassInfo data structures.
 *
 * Note: the findMethod variants are all recursive, but the findVariable
 * variants are all *not* recursive. methods() and variables() are also *not*
 * recursive
 */
struct SemanticExtractor {
private:
  template <typename T>
  inline std::vector<const T*> extractRawConstPointers(
      const std::vector< AstPtr<T> > &ptrs) const {
    std::vector<const T*> rp;
    rp.reserve(ptrs.size());
    for (typename std::vector< AstPtr<T> >::const_iterator it = ptrs.begin();
         it != ptrs.end(); ++it) {
      rp.push_back((*it).get());
    }
    return rp;
  }
public:

// ClassStatement* helpers
  inline bool isAbstract(const ClassStatement *cs) const {
    return cs->getModifiers() & ClassStatement::Abstract;
  }
  inline bool isInterface(const ClassStatement *cs) const {
    return cs->getModifiers() & ClassStatement::Interface;
  }
  inline bool isTrait(const ClassStatement *cs) const {
    return cs->getModifiers() & ClassStatement::Trait;
  }
  inline bool isFinal(const ClassStatement *cs) const {
    return cs->getModifiers() & ClassStatement::Final;
  }
  inline String name(const ClassStatement *cs) const {
    return cs->name();
  }
  inline const MethodStatement *
  findEvalMethod(const ClassStatement *cs,
                 CStrRef name,
                 bool ifaces) const {
    const ClassStatement *dc;
    return findEvalMethodWithClass(cs, name, dc, ifaces);
  }
  inline const MethodStatement *
  findEvalMethodWithClass(const ClassStatement *cs,
                          CStrRef name,
                          const ClassStatement *&definingClass,
                          bool ifaces) const {
    definingClass = NULL;
    const MethodStatement* ms =
      cs->findMethod(name, true, ifaces);
    if (ms) {
      definingClass = ms->getClass();
      return ms;
    }
    return NULL;
  }
  inline const ClassInfo::MethodInfo *
  findBuiltinMethod(const ClassStatement *cs,
                    CStrRef name,
                    bool ifaces) const {
    const ClassInfo *dc;
    return findBuiltinMethodWithClass(cs, name, dc, ifaces);
  }
  inline const ClassInfo::MethodInfo *
  findBuiltinMethodWithClass(const ClassStatement *cs,
                             CStrRef name,
                             const ClassInfo *&definingClass,
                             bool ifaces) const {
    definingClass = NULL;
    const ClassInfo::MethodInfo *result = NULL;
    const ClassInfo *pci = cs->getBuiltinParentInfo();
    if (pci) {
      ClassInfo *ci;
      result = pci->hasMethod(name, ci, ifaces);
      definingClass = ci;
    }
    if (result || !ifaces) return result;
    vector<const ClassInfo*> builtinIfaces;
    cs->collectBuiltinInterfaceInfos(builtinIfaces, true);
    for (vector<const ClassInfo*>::const_iterator it = builtinIfaces.begin();
         it != builtinIfaces.end(); ++it) {
      const ClassInfo *ici = *it;
      ClassInfo *ci;
      result = ici->hasMethod(name, ci, ifaces);
      definingClass = ci;
      if (result) return result;
    }
    return NULL;
  }
  inline std::vector<const MethodStatement*>
  methods(const ClassStatement *cs) const {
    std::vector<const MethodStatement*> methods =
      extractRawConstPointers(cs->m_methodsVec);
    if (!cs->m_useTraitsVec.empty()) {
      ClassEvalState *ce = RequestEvalState::findClassState(cs->m_name);
      ClassEvalState::MethodTable &traitMethodTable = ce->getTraitMethodTable();
      for (ClassEvalState::MethodTable::const_iterator it =
           traitMethodTable.begin(); it != traitMethodTable.end(); it++) {
        methods.push_back(it->second.m_methodStatement);
      }
    }
    return methods;
  }
  inline std::vector<const ClassVariable*>
  variables(const ClassStatement *cs) const {
    return extractRawConstPointers(cs->m_variablesVec);
  }
  inline const ClassVariable* findVariable(const ClassStatement *cs,
                                           CStrRef name) const {
    StringMap<ClassVariablePtr>::const_iterator it =
      cs->m_variables.find(name);
    return it == cs->m_variables.end() ? NULL : it->second.get();
  }

// MethodStatement* helpers
  inline bool isAbstract(const MethodStatement *ms) const {
    return ms->isAbstract();
  }
  inline bool isStatic(const MethodStatement *ms) const {
    return ms->getModifiers() & ClassStatement::Static;
  }
  inline bool isPublic(const MethodStatement *ms) const {
    return ms->getModifiers() & ClassStatement::Public;
  }
  inline bool isProtected(const MethodStatement *ms) const {
    return ms->getModifiers() & ClassStatement::Protected;
  }
  inline bool isPrivate(const MethodStatement *ms) const {
    return ms->getModifiers() & ClassStatement::Private;
  }
  inline String name(const MethodStatement *ms) const {
    return ms->name();
  }
  inline bool refReturn(const MethodStatement *ms) const {
    return ms->refReturn();
  }
  inline size_t numParams(const MethodStatement *ms) const {
    return ms->getParams().size();
  }
  inline std::vector<const Parameter*>
  getParams(const MethodStatement *ms) const {
    return extractRawConstPointers(ms->getParams());
  }

// Parameter* helpers
  inline bool isOptional(const Parameter *p) const {
    return p->isOptional();
  }
  inline bool isRef(const Parameter *p) const {
    return p->isRef();
  }
  inline std::string type(const Parameter *p) const {
    return p->type();
  }

// ClassVariable* helpers
  inline bool isPublic(const ClassVariable *cv) const {
    return cv->getModifiers() & ClassStatement::Public;
  }
  inline bool isProtected(const ClassVariable *cv) const {
    return cv->getModifiers() & ClassStatement::Protected;
  }
  inline bool isPrivate(const ClassVariable *cv) const {
    return cv->getModifiers() & ClassStatement::Private;
  }
  inline bool isStatic(const ClassVariable *cv) const {
    return cv->getModifiers() & ClassStatement::Static;
  }
  inline bool hasInitialValue(const ClassVariable *cv) const {
    return cv->hasInitialValue();
  }
  inline String name(const ClassVariable *cv) const {
    return cv->name();
  }

// ClassInfo* helpers
  inline bool isAbstract(const ClassInfo *ci) const {
    return ci->getAttribute() & ClassInfo::IsAbstract;
  }
  inline bool isInterface(const ClassInfo *ci) const {
    return ci->getAttribute() & ClassInfo::IsInterface;
  }
  inline bool isFinal(const ClassInfo *ci) const {
    return ci->getAttribute() & ClassInfo::IsFinal;
  }
  inline String name(const ClassInfo *ci) const {
    return ci->getName();
  }
  inline const std::vector<ClassInfo::MethodInfo*>&
  methods(const ClassInfo *ci) const {
    return ci->getMethodsVec();
  }
  inline const std::vector<ClassInfo::PropertyInfo*>&
  variables(const ClassInfo *ci) const {
    return ci->getPropertiesVec();
  }
  inline ClassInfo::PropertyInfo*
  findVariable(const ClassInfo *ci,
               CStrRef name) const {
    return ci->getPropertyInfo(name);
  }
  inline ClassInfo::MethodInfo*
  findEvalMethod(const ClassInfo* ci,
                 CStrRef name,
                 bool ifaces) const {
    const ClassInfo *cs;
    return findEvalMethodWithClass(ci, name, cs, ifaces);
  }
  inline ClassInfo::MethodInfo*
  findEvalMethodWithClass(const ClassInfo* ci,
                          CStrRef name,
                          const ClassInfo* &definingClass,
                          bool ifaces) const {
    ASSERT((ci->getAttribute() & ClassInfo::IsSystem) &&
      !(ci->getAttribute() & (ClassInfo::IsTrait | ClassInfo::UsesTraits)));
    ClassInfo *dci;
    ClassInfo::MethodInfo *mi = ci->hasMethod(name, dci, ifaces);
    definingClass = dci;
    return mi;
  }
  inline ClassInfo::MethodInfo*
  findBuiltinMethod(const ClassInfo* ci,
                    CStrRef name,
                    bool ifaces) const {
    return findEvalMethod(ci, name, ifaces);
  }
  inline ClassInfo::MethodInfo*
  findBuiltinMethodWithClass(const ClassInfo* ci,
                             CStrRef name,
                             const ClassInfo* &definingClass,
                             bool ifaces) const {
    return findEvalMethodWithClass(ci, name, definingClass, ifaces);
  }

// ClassInfo::MethodInfo* helpers
  inline bool isAbstract(const ClassInfo::MethodInfo *mi) const {
    return mi->attribute & ClassInfo::IsAbstract;
  }
  inline bool isStatic(const ClassInfo::MethodInfo *mi) const {
    return mi->attribute & ClassInfo::IsStatic;
  }
  inline bool isPublic(const ClassInfo::MethodInfo *mi) const {
    return mi->attribute & ClassInfo::IsPublic;
  }
  inline bool isProtected(const ClassInfo::MethodInfo *mi) const {
    return mi->attribute & ClassInfo::IsProtected;
  }
  inline bool isPrivate(const ClassInfo::MethodInfo *mi) const {
    return mi->attribute & ClassInfo::IsPrivate;
  }
  inline String name(const ClassInfo::MethodInfo *mi) const {
    return mi->name;
  }
  inline bool refReturn(const ClassInfo::MethodInfo *mi) const {
    return mi->attribute & ClassInfo::IsReference;
  }
  inline size_t numParams(const ClassInfo::MethodInfo *mi) const {
    return mi->parameters.size();
  }
  inline const std::vector<const ClassInfo::ParameterInfo*>&
  getParams(const ClassInfo::MethodInfo *mi) const {
    return mi->parameters;
  }

// ClassInfo::ParameterInfo* helpers
  inline bool isOptional(const ClassInfo::ParameterInfo *pi) const {
    return pi->value && *pi->value != '\0';
  }
  inline bool isRef(const ClassInfo::ParameterInfo *pi) const {
    return pi->attribute & ClassInfo::IsReference;
  }
  inline std::string type(const ClassInfo::ParameterInfo *pi) const {
    return pi->type;
  }

// ClassInfo::PropertyInfo* helpers
  inline bool isPublic(const ClassInfo::PropertyInfo *pi) const {
    return pi->attribute & ClassInfo::IsPublic;
  }
  inline bool isProtected(const ClassInfo::PropertyInfo *pi) const {
    return pi->attribute & ClassInfo::IsProtected;
  }
  inline bool isPrivate(const ClassInfo::PropertyInfo *pi) const {
    return pi->attribute & ClassInfo::IsPrivate;
  }
  inline bool isStatic(const ClassInfo::PropertyInfo *pi) const {
    return pi->attribute & ClassInfo::IsStatic;
  }
  inline bool hasInitialValue(const ClassInfo::PropertyInfo *pi) const {
    return false; // TODO: PropertyInfo does not contain this info
  }
  inline String name(const ClassInfo::PropertyInfo *pi) const {
    return pi->name;
  }

};

static SemanticExtractor s_semanticExtractor;

///////////////////////////////////////////////////////////////////////////////
// Semantic Checking - Template Functions
//
// This section contains templated functions which implement semantic checks
// for class derivations. This allows us to avoid duplication of this logic
// when we want to compare different data-structures (say a parent built-in
// versus an AST node)

/**
 * Returns if it an error for child to override parent?
 */
template <typename Extractor,
          typename MethodProxyP, typename ParamProxyP,
          typename MethodProxyC, typename ParamProxyC>
bool MethodLevelSemanticCheckImpl(bool extendingAbstractClass,
                                  MethodProxyP parent,
                                  MethodProxyC child,
                                  const Extractor &x) {
  bool incompatible = false;
  if (extendingAbstractClass && x.name(child)->same(s___construct.get())) {
    // When a class C extends an abstract class B, the
    // signature of C::__construct does not have to match
    // that of B::__construct
  } else if (x.numParams(child) < x.numParams(parent) ||
             x.isStatic(child) != x.isStatic(parent)) {
    incompatible = true;
  } else if (x.refReturn(child) != x.refReturn(parent)) {
    // If one signature returns by value and the other returns by
    // reference then they are not compatible
    incompatible = true;
  } else {
    const vector<ParamProxyP> &p1 = x.getParams(parent);
    const vector<ParamProxyC> &p2 = x.getParams(child);
    for (size_t i = 0; !incompatible && i < p2.size(); ++i) {
      if (i >= p1.size()) {
        if (!x.isOptional(p2[i])) {
          incompatible = true;
        }
      } else if ((x.isRef(p1[i]) != x.isRef(p2[i])) ||
                 (x.isOptional(p1[i]) && !x.isOptional(p2[i]))) {
        incompatible = true;
      }  else if (x.type(p1[i]) != x.type(p2[i])) {
        incompatible = true;
      }
    }
  }
  return incompatible;
}

/**
 * Checks to see if child overrides parent's methods correctly
 * (non-recursively)
 */
template <typename Extractor,
          typename ClassProxyP,       typename ClassProxyC,
          typename MethEvalProxyP,    typename ParamEvalProxyP,
          typename MethEvalProxyC,    typename ParamEvalProxyC,
          typename MethBuiltinProxyC, typename ParamBuiltinProxyC>
void ClassLevelMethodOverrideCheckImpl(ClassProxyP parent,
                                       ClassProxyC child,
                                       const Extractor &x) {
  ASSERT(!x.isInterface(child) &&
         !x.isAbstract(child));
  if (x.isInterface(parent) || x.isAbstract(parent)) {
    bool extendingAbstractClass = !x.isInterface(parent);
    const vector<MethEvalProxyP> &parentMethods =
      x.methods(parent);
    for (typename vector<MethEvalProxyP>::const_iterator it =
           parentMethods.begin();
         it != parentMethods.end(); ++it) {
      MethEvalProxyP parentMethod = *it;
      if (x.isAbstract(parentMethod) || x.isInterface(parent)) {
        // Interface => that the definition doesn't exist (for now)
        // with traits, this will change
        bool found = false;
        bool incompatible = false;

        MethEvalProxyC childMethod =
          // fully recursive, no ifaces
          x.findEvalMethod(child, x.name(parentMethod), false);
        if (!childMethod) {
          MethBuiltinProxyC childMethod0 =
            // fully recursive, no ifaces
            x.findBuiltinMethod(child, x.name(parentMethod), false);
          if (childMethod0 && !x.isAbstract(childMethod0)) {
            found = true;
            incompatible =
              MethodLevelSemanticCheckImpl<
                Extractor,
                MethEvalProxyP,    ParamEvalProxyP,
                MethBuiltinProxyC, ParamBuiltinProxyC >(
                    extendingAbstractClass,
                    parentMethod,
                    childMethod0,
                    x);
          }
        } else if (!x.isAbstract(childMethod)) {
          found = true;
          incompatible =
            MethodLevelSemanticCheckImpl<
              Extractor,
              MethEvalProxyP, ParamEvalProxyP,
              MethEvalProxyC, ParamEvalProxyC >(
                  extendingAbstractClass,
                  parentMethod,
                  childMethod,
                  x);
        }
        if (!found) {
          throw FatalErrorException(0,"Class %s does not implement abstract "
              "method %s::%s", x.name(child).c_str(),
              x.name(parent).c_str(), x.name(parentMethod).c_str());
        }
        if (incompatible) {
          throw FatalErrorException(0,"Declaration of %s::%s() must be "
              "compatible with that of %s::%s()",
              x.name(child).c_str(),  x.name(parentMethod).c_str(),
              x.name(parent).c_str(), x.name(parentMethod).c_str());
        }
      }
    }
  }
}

/**
 * Does child override parent's properties correctly?
 */
template <typename Extractor,
          typename ClassProxyP, typename ClassProxyC,
          typename PropProxyP,  typename PropProxyC>
void ClassLevelPropertyOverrideCheckImpl(ClassProxyP parent,
                                         ClassProxyC child,
                                         const Extractor &x) {
  // TODO: remove the calls to raise_debugging in the abstract child
  // class case once the errors go away in www
  const vector<PropProxyP> &parentVariables =
    x.variables(parent);
  for (typename vector<PropProxyP>::const_iterator it =
         parentVariables.begin();
       it != parentVariables.end(); ++it) {
    PropProxyP parentVariable = *it;
    PropProxyC childVariable = x.findVariable(child, x.name(parentVariable));
    if (childVariable) {
      int p1 =
        (x.isPublic(parentVariable)    ? ClassStatement::Public    : 0) |
        (x.isProtected(parentVariable) ? ClassStatement::Protected : 0) |
        (x.isPrivate(parentVariable)   ? ClassStatement::Private   : 0);
      int p2 =
        (x.isPublic(childVariable)     ? ClassStatement::Public    : 0) |
        (x.isProtected(childVariable)  ? ClassStatement::Protected : 0) |
        (x.isPrivate(childVariable)    ? ClassStatement::Private   : 0);
      if (!p1) p1 = ClassStatement::Public;
      if (!p2) p2 = ClassStatement::Public;
      if (p1 < p2) {
        const char *pn1;
        if (p1 == ClassStatement::Private)        pn1 = "private";
        else if (p1 == ClassStatement::Protected) pn1 = "protected";
        else                                      pn1 = "public";
        // Illegal strengthening of privacy
#define RAISE_ARGUMENTS_PRIVACY \
  "Access level to %s::$%s must be %s (as in class %s) or weaker", \
  x.name(child).c_str(), x.name(childVariable).c_str(), \
  pn1,                   x.name(parent).c_str()
        if (x.isAbstract(child)) {
          raise_debugging(RAISE_ARGUMENTS_PRIVACY);
        } else {
          raise_error(RAISE_ARGUMENTS_PRIVACY);
        }
      } else if (x.isStatic(parentVariable) &&
                 p1 == ClassStatement::Protected &&
                 p2 == ClassStatement::Public &&
                 x.hasInitialValue(childVariable)) {
        // No initial value allowed in redefinition of protected to public
        // static

        // TODO: this is a PHP 5.2-ism, 5.3 cleans this up. Remove this when
        // we can get around to it
#define RAISE_ARGUMENTS_INIT_VAL \
  "Cannot change initial value of property %s::$%s in class %s", \
  x.name(parent).c_str(), x.name(parentVariable).c_str(), \
  x.name(child).c_str()
        if (x.isAbstract(child)) {
          raise_debugging(RAISE_ARGUMENTS_INIT_VAL);
        } else {
          raise_error(RAISE_ARGUMENTS_INIT_VAL);
        }
      }
      // Staticness
      if (p1 != ClassStatement::Private) {
        if (x.isStatic(parentVariable) && !x.isStatic(childVariable)) {
#define RAISE_ARGUMENTS_STATIC_NONSTATIC \
  "Cannot redeclare static %s::$%s as non-static %s::$%s", \
  x.name(parent).c_str(), x.name(parentVariable).c_str(), \
  x.name(child).c_str(),  x.name(childVariable).c_str()
          if (x.isAbstract(child)) {
            raise_debugging(RAISE_ARGUMENTS_STATIC_NONSTATIC);
          } else {
            raise_error(RAISE_ARGUMENTS_STATIC_NONSTATIC);
          }
        } else if (!x.isStatic(parentVariable) && x.isStatic(childVariable)) {
#define RAISE_ARGUMENTS_NONSTATIC_STATIC \
  "Cannot redeclare non-static %s::$%s as static %s::$%s", \
  x.name(parent).c_str(), x.name(parentVariable).c_str(), \
  x.name(child).c_str(),  x.name(childVariable).c_str()
          if (x.isAbstract(child)) {
            raise_debugging(RAISE_ARGUMENTS_NONSTATIC_STATIC);
          } else {
            raise_error(RAISE_ARGUMENTS_NONSTATIC_STATIC);
          }
        }
      }
    }
  }
}

template <typename Extractor,
          typename ClassProxyP, typename MethProxyP,
          typename ClassProxyC, typename MethProxyC>
void MethodLevelAccessLevelCheckImpl(ClassProxyP parentClass,
                                     MethProxyP  parentMethod,
                                     ClassProxyC childClass,
                                     MethProxyC  childMethod,
                                     const Extractor &x) {
  bool iface       = x.isInterface(childClass);
  bool ifaceParent = x.isInterface(parentClass);

  if (x.isStatic(childMethod) && !x.isStatic(parentMethod)) {
    raise_error("Cannot make non static method %s::%s() static in class %s",
                x.name(parentClass).c_str(), x.name(parentMethod).c_str(),
                x.name(childClass).c_str());
  }

  if (!x.isStatic(childMethod) && x.isStatic(parentMethod)) {
    raise_error("Cannot make static method %s::%s() non static in class %s",
                x.name(parentClass).c_str(), x.name(parentMethod).c_str(),
                x.name(childClass).c_str());
  }

  if (x.isAbstract(childMethod) || iface) {
    if (!x.isAbstract(parentMethod) && x.isAbstract(childMethod)) {
      raise_error("Cannot make non abstract method %s::%s() abstract in "
                  "class %s",
                  x.name(parentClass).c_str(), x.name(parentMethod).c_str(),
                  x.name(childClass).c_str());
    }
    if (x.isAbstract(parentMethod) && !ifaceParent) {
      raise_error("Cannot re-declare abstract method %s::%s() abstract "
                  "in class %s",
                  x.name(parentClass).c_str(), x.name(parentMethod).c_str(),
                  x.name(childClass).c_str());
    }
  }

  int p1 =
    (x.isPublic(childMethod)     ? ClassStatement::Public    : 0) |
    (x.isProtected(childMethod)  ? ClassStatement::Protected : 0) |
    (x.isPrivate(childMethod)    ? ClassStatement::Private   : 0);
  int p2 =
    (x.isPublic(parentMethod)    ? ClassStatement::Public    : 0) |
    (x.isProtected(parentMethod) ? ClassStatement::Protected : 0) |
    (x.isPrivate(parentMethod)   ? ClassStatement::Private   : 0);
  if (!p1) p1 = ClassStatement::Public;
  if (!p2) p2 = ClassStatement::Public;
  if (p1 > p2) {
    const char *pn;
    if (p2 == ClassStatement::Private)        pn = "private";
    else if (p2 == ClassStatement::Protected) pn = "protected";
    else                                      pn = "public";
    // Illegal strengthening of privacy
    raise_error("Access level to %s::%s() must be %s (as in class %s) "
                "or weaker",
                x.name(childClass).c_str(), x.name(childMethod).c_str(),
                pn, x.name(parentClass).c_str());
  }
}

template <typename Extractor,
          typename ClassEvalProxyP,    typename MethEvalProxyP,
          typename ClassBuiltinProxyP, typename MethBuiltinProxyP,
          typename ClassEvalProxyC,    typename MethEvalProxyC>
void ClassLevelMethodAccessLevelCheckImpl(ClassEvalProxyP parent,
                                          ClassEvalProxyC child,
                                          const Extractor &x) {
  const vector<MethEvalProxyC> &childMethods = x.methods(child);
  for (typename vector<MethEvalProxyC>::const_iterator it =
         childMethods.begin();
       it != childMethods.end(); ++it) {
    MethEvalProxyC  childMethod = *it;
    ClassEvalProxyP definingParentClass;
    MethEvalProxyP  parentMethod =
      // fully recursive, including ifaces
      x.findEvalMethodWithClass(
        parent, x.name(childMethod), definingParentClass, true);
    if (!parentMethod) {
      ClassBuiltinProxyP definingParentClass0;
      MethBuiltinProxyP  parentMethod0 =
        // fully recursive, including ifaces
        x.findBuiltinMethodWithClass(
          parent, x.name(childMethod), definingParentClass0, true);
      if (!parentMethod0) continue;
      MethodLevelAccessLevelCheckImpl<
        Extractor,
        ClassBuiltinProxyP, MethBuiltinProxyP,
        ClassEvalProxyC,    MethEvalProxyC >(
          definingParentClass0,
          parentMethod0,
          child,
          childMethod,
          x);
    } else {
      MethodLevelAccessLevelCheckImpl<
        Extractor,
        ClassEvalProxyP, MethEvalProxyP,
        ClassEvalProxyC, MethEvalProxyC >(
          definingParentClass,
          parentMethod,
          child,
          childMethod,
          x);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Semantic Checking - ClassStatement specializations
//
// This section contains the specializations of the template functions that we
// will use, so we don't have to keep re-typing the template type parameters

template <>
void ClassStatement::ClassLevelMethodOverrideCheck
< const ClassStatement*, const ClassStatement* >
(const ClassStatement* parent,
 const ClassStatement* child) {
  ClassLevelMethodOverrideCheckImpl<
    SemanticExtractor,
    const ClassStatement*,        const ClassStatement*,
    const MethodStatement*,       const Parameter*,
    const MethodStatement*,       const Parameter*,
    const ClassInfo::MethodInfo*, const ClassInfo::ParameterInfo* >(
      parent, child, s_semanticExtractor);
}

template <>
void ClassStatement::ClassLevelMethodOverrideCheck
< const ClassInfo*, const ClassStatement* >
(const ClassInfo*      parent,
 const ClassStatement* child) {
  ClassLevelMethodOverrideCheckImpl<
    SemanticExtractor,
    const ClassInfo*,             const ClassStatement*,
    ClassInfo::MethodInfo*,       const ClassInfo::ParameterInfo*,
    const MethodStatement*,       const Parameter*,
    const ClassInfo::MethodInfo*, const ClassInfo::ParameterInfo* >(
      parent, child, s_semanticExtractor);
}

template <>
void ClassStatement::ClassLevelPropertyOverrideCheck
< const ClassStatement*, const ClassStatement* >
(const ClassStatement* parent,
 const ClassStatement* child) {
  ClassLevelPropertyOverrideCheckImpl<
    SemanticExtractor,
    const ClassStatement*, const ClassStatement*,
    const ClassVariable*,  const ClassVariable* >(
      parent, child, s_semanticExtractor);
}

template <>
void ClassStatement::ClassLevelPropertyOverrideCheck
< const ClassInfo*, const ClassStatement* >
(const ClassInfo*      parent,
 const ClassStatement* child) {
  ClassLevelPropertyOverrideCheckImpl<
    SemanticExtractor,
    const ClassInfo*,         const ClassStatement*,
    ClassInfo::PropertyInfo*, const ClassVariable* >(
      parent, child, s_semanticExtractor);
}

template <>
void ClassStatement::ClassLevelMethodAccessLevelCheck
< const ClassStatement*, const ClassStatement* >
(const ClassStatement* parent,
 const ClassStatement* child) {
  ClassLevelMethodAccessLevelCheckImpl<
    SemanticExtractor,
    const ClassStatement*, const MethodStatement*,
    const ClassInfo*,      const ClassInfo::MethodInfo*,
    const ClassStatement*, const MethodStatement* >(
      parent, child, s_semanticExtractor);
}

template <>
void ClassStatement::ClassLevelMethodAccessLevelCheck
< const ClassInfo*, const ClassStatement* >
(const ClassInfo*      parent,
 const ClassStatement* child) {
  ClassLevelMethodAccessLevelCheckImpl<
    SemanticExtractor,
    const ClassInfo*,      const ClassInfo::MethodInfo*,
    const ClassInfo*,      const ClassInfo::MethodInfo*,
    const ClassStatement*, const MethodStatement* >(
      parent, child, s_semanticExtractor);
}

///////////////////////////////////////////////////////////////////////////////
// Semantic Checking - Entry Points
//
// This section contains the core entry points into semantic checking

/**
 * Child wants to derive from a builtin parent-
 * check the semantics of that derivation
 */
void ClassStatement::BuiltinSemanticCheck(const ClassInfo      *parent,
                                          const ClassStatement *child) {
  ASSERT(parent);
  ASSERT(child);

  ASSERT(!s_semanticExtractor.isInterface(child));

  if (!s_semanticExtractor.isAbstract(child)) {
    // method inheritance check, for non-abstract classes
    ClassLevelMethodOverrideCheck(parent, child);
  }

  // property level check
  ClassLevelPropertyOverrideCheck(parent, child);

  // recurse up the inheritance tree
  const ClassInfo *pp = parent->getParentClassInfo();
  if (pp) BuiltinSemanticCheck(pp, child);
  const ClassInfo::InterfaceVec &ifaces = parent->getInterfacesVec();
  for (ClassInfo::InterfaceVec::const_iterator it = ifaces.begin();
       it != ifaces.end(); ++it) {
    const ClassInfo *iface = ClassInfo::FindInterface(*it);
    if (iface) BuiltinSemanticCheck(iface, child);
  }
}

/**
 * Main entry point into semantic checking
 *
 * If the cls param is NULL, then this means start the semantic check, assuming
 * this ClassStatement is the class at the bottom of the class hierarchy.
 * Otherwise, it means that the cls param wants to derive from this
 * ClassStatement, and we need to check to see if that's ok
 */
void ClassStatement::semanticCheck(const ClassStatement *cls)
const {
  loadInterfaceStatements();
  const ClassStatement *parent = parentStatement();
  const ClassInfo *builtinParent = NULL;
  if (!parent && !m_parent->empty()) {
    builtinParent = ClassInfo::FindClass(m_parent);
    if (!builtinParent) {
      builtinParent = ClassInfo::FindInterface(m_parent);
    }
  }
  if (cls) {
    if (!s_semanticExtractor.isAbstract(cls)) {
      // method inheritance check
      ClassLevelMethodOverrideCheck(this, cls);
    }

    // property level check
    ClassLevelPropertyOverrideCheck(this, cls);
  } else {
    // make sure non-(abstract classes/ifaces) don't declare
    // abstract methods
    if (!s_semanticExtractor.isInterface(this) &&
        !s_semanticExtractor.isAbstract(this) &&
        !s_semanticExtractor.isTrait(this)) {
      for (vector<MethodStatementPtr>::const_iterator it =
             m_methodsVec.begin();
           it != m_methodsVec.end(); ++it) {
        if ((*it)->isAbstract()) {
          raise_error("Class %s contains abstract method %s and must therefore"
                      " be declared abstract", name().c_str(),
                      (*it)->name().c_str());
        }
      }
    }

    // make sure no properties are declared abstract
    for (vector<ClassVariablePtr>::const_iterator it =
           m_variablesVec.begin();
         it != m_variablesVec.end(); ++it) {
      if ((*it)->getModifiers() & Abstract) {
        raise_error("Properties cannot be declared abstract");
      }
    }

    // make sure a final class is not being extended
    static const StringData* sd___MockClass =
      StringData::GetStaticString("__MockClass");
    if (parent && s_semanticExtractor.isFinal(parent)) {
      if (m_userAttributes.find(sd___MockClass) == m_userAttributes.end()) {
        throw FatalErrorException(0,"Class %s may not inherit from final class "
                                  "(%s)",
                                  name().c_str(),
                                  parent->name().c_str());
      }
    }
    if (builtinParent && s_semanticExtractor.isFinal(builtinParent)) {
      if (m_userAttributes.find(sd___MockClass) == m_userAttributes.end()) {
        throw FatalErrorException(0,"Class %s may not inherit from final class "
                                  "(%s)",
                                  name().c_str(),
                                  builtinParent->getName().c_str());
      }
    }

    // make sure classes don't loosen the access level of
    // inherited methods
    if (parent) {
      ClassLevelMethodAccessLevelCheck(parent, this);
    } else if (builtinParent) {
      ClassLevelMethodAccessLevelCheck(builtinParent, this);
    }

    // do the same check as above for all the ifaces
    for (size_t i = 0; i < m_bases.size(); i++) {
      const ClassStatement *iface = RequestEvalState::findClass(m_bases[i]);
      const ClassInfo *builtinIface =
        !iface ? ClassInfo::FindInterface(m_bases[i]) : NULL;
      if (iface) {
        ClassLevelMethodAccessLevelCheck(iface, this);
      } else if (builtinIface) {
        ClassLevelMethodAccessLevelCheck(builtinIface, this);
      }
    }

    // Check for multiple abstract function declarations
    if (!m_parent->empty() || !m_bases.empty()) {
      StringIMap<String> abstracts;
      abstractMethodCheck(abstracts, true);
    }
    cls = this;
  }

  bool doSemanticCheck = !s_semanticExtractor.isInterface(cls);

  // make sure the parent class exists and is not an iface. recursively
  // apply semantic checks to the parent class
  if (parent) {
    if (s_semanticExtractor.isInterface(parent)) {
      raise_error("%s cannot extend %s - it is an interface",
                  name().c_str(), parent->name().c_str());
    }
    if (doSemanticCheck) {
      parent->semanticCheck(cls);
    }
  } else if (builtinParent) {
    if (s_semanticExtractor.isInterface(builtinParent)) {
      raise_error("%s cannot extend %s - it is an interface",
                  name().c_str(), builtinParent->getName().c_str());
    }
    if (doSemanticCheck) {
      BuiltinSemanticCheck(builtinParent, cls);
    }
  } else if (!m_parent->empty()) {
    raise_error("Class '%s' does not exist.", m_parent->data());
  }

  // make sure the ifaces exist and are not classes. recursively
  // apply semantic checks to the ifaces
  for (size_t i = 0; i < m_bases.size(); i++) {
    const ClassStatement *iface = RequestEvalState::findClass(m_bases[i]);
    const ClassInfo *builtinIface = NULL;
    if (!iface) {
      // try to find it as a class first (so we can report error)
      builtinIface = ClassInfo::FindClass(m_bases[i]);
      if (LIKELY(!builtinIface)) {
        // ok, now try to find it as an interface
        builtinIface = ClassInfo::FindInterface(m_bases[i]);
      }
    }
    if (iface) {
      if (!s_semanticExtractor.isInterface(iface)) {
        raise_error("%s cannot implement %s - it is not an interface",
                    name().c_str(), iface->name().c_str());
      }
      if (doSemanticCheck) {
        iface->semanticCheck(cls);
      }
    } else if (builtinIface) {
      if (!s_semanticExtractor.isInterface(builtinIface)) {
        raise_error("%s cannot implement %s - it is not an interface",
                    name().c_str(), builtinIface->getName().c_str());
      }
      if (doSemanticCheck) {
        BuiltinSemanticCheck(builtinIface, cls);
      }
    } else {
      raise_error("Interface '%s' does not exist.", m_bases[i]->data());
    }
  }
}

const MethodStatement* ClassStatement::findParentMethod(CStrRef name,
    bool interface) const {
  const ClassStatement *parent = parentStatement();
  if (parent) {
    const MethodStatement *m = parent->findMethod(name, true, interface);
    if (m) return m;
  }
  if (interface) {
    for (unsigned i = 0; i < m_bases.size(); i++) {
      const ClassStatement *iface = RequestEvalState::findClass(m_bases[i]);
      if (iface) {
        const MethodStatement *m = iface->findMethod(name, true, true);
        if (m) return m;
      }
    }
  }
  return NULL;
}

void ClassStatement::abstractMethodCheck(
  StringIMap<String> &abstracts, bool ifaces) const {
  bool iface = getModifiers() & Interface;
  if (!iface && getModifiers() & Abstract)  {
    for (vector<MethodStatementPtr>::const_iterator it = m_methodsVec.begin();
        it != m_methodsVec.end(); ++it) {
      if ((*it)->getModifiers() & Abstract) {
        StringIMap<String>::const_iterator ait = abstracts.find((*it)->name());
        if (ait != abstracts.end() && ait->second != name()) {
          raise_error("Can't inherit abstract function %s::%s (previously "
              "declared abstract in %s)", name().c_str(), ait->first.c_str(),
              ait->second.c_str());
        }
        abstracts[(*it)->name()] = name();
      }
    }
  }
  const ClassStatement *parent = parentStatement();
  if (parent && parent->getModifiers() & Abstract) {
    // No builtin abstract classes
    // Only recurse into abstract parents since other parents don't
    // contribute abstract methods
    parent->abstractMethodCheck(abstracts, false);
  }
  if (ifaces) {
    for (unsigned i = 0; i < m_bases.size(); i++) {
      const ClassStatement *iface = RequestEvalState::findClass(m_bases[i]);
      if (iface) {
        iface->abstractMethodCheck(abstracts, false);
      } else {
        // may be built in
        const ClassInfo *ici = ClassInfo::FindInterface(m_bases[i]);
        if (ici) {
          const ClassInfo::MethodVec &meths = ici->getMethodsVec();
          for (ClassInfo::MethodVec::const_iterator mit = meths.begin();
              mit != meths.end(); ++mit) {
            StringIMap<String>::const_iterator ait =
              abstracts.find((*mit)->name);
            if (ait != abstracts.end() && ait->second != ici->getName()) {
              raise_error("Can't inherit abstract function %s::%s (previously "
                          "declared abstract in %s)",
                          m_bases[i]->data(), (*mit)->name.c_str(),
                          ait->second.c_str());
            }
            abstracts[(*mit)->name] = ici->getName();
          }
        }
      }
    }
  }
}
const ClassInfo *ClassStatement::getBuiltinParentInfo() const {
  const ClassStatement *parent = parentStatement();
  if (parent) return parent->getBuiltinParentInfo();
  if (!m_parent->empty()) return ClassInfo::FindClass(m_parent);
  return NULL;
}

/**
 * Recursively gather all the builtin interfaces which this
 * class implements. Do not recurse into builtins
 */
void ClassStatement::collectBuiltinInterfaceInfos(
    std::vector<const ClassInfo*>& infos,
    bool excludeParent) const {
  const ClassStatement *parent = excludeParent ? NULL : parentStatement();
  if (parent) parent->collectBuiltinInterfaceInfos(infos, false);
  for (size_t i = 0; i < m_bases.size(); i++) {
    const ClassStatement *iface =
      RequestEvalState::findClass(m_bases[i]);
    if (iface) {
      iface->collectBuiltinInterfaceInfos(infos, excludeParent);
    } else {
      const ClassInfo *ci = ClassInfo::FindInterface(m_bases[i]);
      if (ci) infos.push_back(ci);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

ClassStatementMarkerPtr ClassStatement::getMarker() {
  ASSERT(!m_marker);
  m_marker = new ClassStatementMarker(this, loc());
  return m_marker;
}

ClassStatementMarker::ClassStatementMarker(ClassStatement *cls,
                                           const Location *loc)
  : Statement(loc), m_class(cls) {
}

void ClassStatementMarker::eval(VariableEnvironment &env) const {
  ClassEvalState *ce = RequestEvalState::findClassState(m_class->name());
  if (!ce || ce->getClass() != m_class) {
    // Delayed due to volatility
    m_class->evalImpl(env, true);
    ce = RequestEvalState::findClassState(m_class->name());
  }
  ASSERT(ce);
  ce->semanticCheck();
}

void ClassStatementMarker::dump(std::ostream &out) const {
}

void optimize(VariableEnvironment &env, ClassVariablePtr &before) {
  if (before) {
    ClassVariable *optCV = before->optimize(env);
    if (optCV) {
      ClassVariablePtr after = dynamic_cast<ClassVariable *>(optCV);
      if (after) {
        before = after;
      } else {
        assert(false);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
}
