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

#include <runtime/eval/ast/class_statement.h>
#include <runtime/eval/ast/method_statement.h>
#include <runtime/eval/ast/expression.h>
#include <runtime/eval/runtime/eval_object_data.h>
#include <runtime/eval/ast/statement_list_statement.h>
#include <runtime/eval/runtime/eval_state.h>
#include <runtime/eval/runtime/variable_environment.h>
#include <runtime/eval/strict_mode.h>
#include <runtime/base/runtime_option.h>
#include <runtime/eval/eval.h>
#include <util/util.h>
#include <runtime/eval/ast/array_expression.h>
#include <runtime/ext/ext_class.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ClassVariable::ClassVariable(CONSTRUCT_ARGS, const string &name, int modifiers,
    ExpressionPtr value, const string &doc, ClassStatement *cls)
  : Construct(CONSTRUCT_PASS), m_name(name),
    m_modifiers(modifiers), m_value(value), m_docComment(doc), m_cls(cls) {
}

void ClassVariable::set(VariableEnvironment &env, EvalObjectData *self) const {
  if (!(m_modifiers & ClassStatement::Static)) {
    Variant val(m_value ? m_value->eval(env) : null_variant);
    if (m_modifiers & ClassStatement::Private) {
      self->o_setPrivate(m_cls->name(), m_name, val);
    } else if (!self->o_exists(m_name)) {
      self->o_set(m_name, val, true);
    }
  }
}
void ClassVariable::setStatic(VariableEnvironment &env, LVariableTable &st)
  const {
  if ((m_modifiers & ClassStatement::Static)) {
    Variant val;
    if (m_value) {
      val = m_value->eval(env);
    } else if (m_modifiers & ClassStatement::Public) {
      const ClassStatement *parent = m_cls->parentStatement();
      if (parent) {
        const ClassVariable* var = parent->findVariable(m_name, true);
        if (var && (var->m_modifiers & ClassStatement::Protected) &&
            (var->m_modifiers & ClassStatement::Static)) {
          // When there is no initial value, and base class's property is
          // protected and this class's same property is public, and both are
          // static, they refer to the same property. In this case, we don't
          // set the variable in st, so it will go to parent class for it.
          return;
        }
      }
    }
    st.get(m_name) = val;
  }
}

void ClassVariable::dump(std::ostream &out) const {
  ClassStatement::dumpModifiers(out, m_modifiers, true);
  out << "$" << m_name.c_str();
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
  : Statement(STATEMENT_PASS), m_name(name),
    m_modifiers(0), m_parent(parent), m_docComment(doc),
    m_marker(new ClassStatementMarker(STATEMENT_PASS, this)),
    m_delayDeclaration(false) { }

void ClassStatement::finish() {
}

const ClassStatement *ClassStatement::parentStatement() const {
  if (!m_parent.empty()) {
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
  ClassEvalState::MethodTable &mtable = ce.getMethodTable();
  if (!m_parent.empty()) {
    const ClassStatement* parent_cls = parentStatement();
    if (parent_cls) {
      parent_cls->loadMethodTable(ce);
    } else {
      // Built in
      ClassInfo::MethodVec meths;
      ClassInfo::GetClassMethods(meths, m_parent, 1);
      for (ClassInfo::MethodVec::const_iterator it = meths.begin();
           it != meths.end(); ++it) {
        int mods = 0;
        if ((*it)->attribute & ClassInfo::IsPrivate) mods |= Private;
        else if ((*it)->attribute & ClassInfo::IsProtected) mods |= Protected;
        else mods |= Public;
        pair<const MethodStatement *, int> &p = mtable[(*it)->name];
        p.first = NULL;
        p.second = mods;
      }
    }
  }
  for (vector<MethodStatementPtr>::const_iterator it = m_methodsVec.begin();
       it != m_methodsVec.end(); ++it) {
    ClassEvalState::MethodTable::iterator mit =
      mtable.find((*it)->name().c_str());
    if (mit != mtable.end()) {
      int mods = mit->second.second;
      const MethodStatement *mmit = mit->second.first;
      if (mods & Final) {
        throw FatalErrorException(0,
                                  "Cannot override final method %s::%s() at "
                                  "%s:%d",
                                  mmit->getClass()->name().c_str(),
                                  (*it)->name().c_str(), (*it)->loc()->file,
                                  (*it)->loc()->line1);
      } else if ((mods & (Public|Protected|Private)) <
                 ((*it)->getModifiers() & (Public|Protected|Private))) {
        const char *al = "public";
        if (mods & Protected) {
          al = "protected";
        } else if (mods & Private) {
          al = "private";
        }
        throw FatalErrorException(0,
                                  "Access level to %s must be %s or weaker "
                                  "(as in class %s) at %s:%d",
                                  (*it)->name().c_str(), al,
                                  mmit ?
                                  mmit->getClass()->name().c_str() :
                                  m_parent.c_str(),(*it)->loc()->file,
                                  (*it)->loc()->line1);
      }
      mit->second.first = it->get();
      mit->second.second = (*it)->getModifiers();
    } else {
      pair<const MethodStatement *, int> &p = mtable[(*it)->name().c_str()];
      p.first = it->get();
      p.second = (*it)->getModifiers();
    }
  }

  hphp_const_char_imap<MethodStatementPtr>::const_iterator it =
    m_methods.find(m_name.c_str());
  if (it != m_methods.end()) {
    ce.getConstructor() = it->second.get();
  }
  it = m_methods.find("__construct");
  if (it != m_methods.end()) {
    ce.getConstructor() = it->second.get();
  }
}

void ClassStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  if (m_delayDeclaration) return;
  if (!isBaseClass() && !m_marker) {
    // Class might not be valid to declare yet. If the parent and bases
    // have not been defined then don't declare until execution hits the
    // marker.
    if (!m_parent.empty() && !f_class_exists(m_parent, false)) {
      return;
    }
    for (uint i = 0; i < m_bases.size(); ++i) {
      if (!f_interface_exists(m_bases[i])) return;
    }
  }
  evalImpl(env);
}

void ClassStatement::evalImpl(VariableEnvironment &env) const {
  ENTER_STMT;
  RequestEvalState::declareClass(this);

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

      const MethodStatementPtr child_method = *it;

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

Object ClassStatement::create(ClassEvalState &ce, CArrRef params,
    bool init, ObjectData* root /* = NULL*/) const {
  if (getModifiers() & Abstract) {
    throw FatalErrorException(0, "Cannot instantiate abstract class %s",
                              name().c_str());
  }

  EvalObjectData *eo;
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

  eo = NEWOBJ(EvalObjectData)(ce, builtinParent, root);

  Object o(eo);
  eo->init();
  if (init) {
    o->dynConstruct(params);
  }
  return o;
}

void ClassStatement::initializeObject(EvalObjectData *obj) const {
  DummyVariableEnvironment env;
  for (vector<ClassVariablePtr>::const_iterator it = m_variablesVec.begin();
       it != m_variablesVec.end(); ++it) {
    (*it)->set(env, obj);
  }
  const ClassStatement *cls = parentStatement();
  if (cls) {
    cls->initializeObject(obj);
  }
}

void ClassStatement::initializeStatics(LVariableTable &statics) const {
  DummyVariableEnvironment env;
  for (vector<ClassVariablePtr>::const_iterator it = m_variablesVec.begin();
       it != m_variablesVec.end(); ++it) {
    (*it)->setStatic(env, statics);
  }
}

void ClassStatement::addBases(const std::vector<String> &bases) {
  for (unsigned i = 0; i < bases.size(); i++) {
    m_bases.push_back(AtomicString(bases[i].get()));
  }
}

void ClassStatement::addVariable(ClassVariablePtr v) {
  if (getModifiers() & Interface) {
    throw FatalErrorException(0, "Interface %s may not include member "
                              "variable %s", name().c_str(),
                              v->name().c_str());
  }

  m_variables[v->name()] = v;
  m_variablesVec.push_back(v);
}

void ClassStatement::addMethod(MethodStatementPtr m) {
  if (m_methods.find(m->name().c_str()) != m_methods.end()) {
    raise_error("Cannot redeclare %s::%s() in %s on line %d",
                m_name.c_str(), m->name().c_str(),
                m->loc()->file, m->loc()->line1);
  }
  if (m->getModifiers() & Abstract) {
    if (m->getModifiers() & Private) {
      raise_error("Cannot declare abstract %s::%s() private in %s on line %d",
                  m_name.c_str(), m->name().c_str(),
                  m->loc()->file, m->loc()->line1);
    }
    if (m->getModifiers() & Final) {
      raise_error("Cannot declare abstract %s::%s() final in %s on line %d",
                  m_name.c_str(), m->name().c_str(),
                  m->loc()->file, m->loc()->line1);
    }
    if (m->hasBody()) {
      raise_error("Abstract %s::%s() cannot contain body in %s on line %d",
                  m_name.c_str(), m->name().c_str(),
                  m->loc()->file, m->loc()->line1);
    }
  } else if (!m->hasBody()) {
    if (!(m_modifiers & Interface)) {
      raise_error("Non-abstract %s::%s() must contain body in %s on line %d",
                  m_name.c_str(), m->name().c_str(),
                  m->loc()->file, m->loc()->line1);
    } else if (m->getModifiers() & Protected) {
      raise_error("Access type for interface method %s::%s() must be "
                  "omitted", m_name.c_str(), m->name().c_str());
    }
  }

  m_methods[m->name().c_str()] = m;
  m_methodsVec.push_back(m);
}
void ClassStatement::addConstant(const string &name, ExpressionPtr v) {
  // Array is the only one allowed by the grammer but disallowed semantically
  if (v->is<ArrayExpression>()) {
    raise_error("Arrays are not allowed in class constants on %s:%d",
                v->loc()->file, v->loc()->line1);
  }
  m_constantNames.push_back(AtomicString(name));
  m_constants[m_constantNames.back()] = v;
}

bool ClassStatement::instanceOf(const char *c) const {
  if (strcasecmp(m_name.c_str(), c) == 0) return true;
  for (unsigned i = 0; i < m_bases.size(); i++) {
    if (strcasecmp(m_bases[i].c_str(), c) == 0) return true;
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

bool ClassStatement::subclassOf(const char *c) const {
  const ClassStatement *cls = this;
  while (cls) {
    if (cls->instanceOf(c)) return true;
    cls = cls->parentStatement();
  }
  return false;
}

const MethodStatement* ClassStatement::findMethod(const char* name,
    bool recursive /* = false */, bool interfaces /* = false */) const {
  hphp_const_char_imap<MethodStatementPtr>::const_iterator it =
    m_methods.find(name);
  if (it != m_methods.end()) {
    return it->second.get();
  } else {
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

void ClassStatement::dump(std::ostream &out) const {
  if (m_modifiers & Abstract) {
    out << "abstract ";
  }
  if (m_modifiers & Final) {
    out << "final ";
  }
  if (m_modifiers & Interface) {
    out << "interface ";
  } else {
    out << "class ";
  }
  out << m_name.c_str();
  if (!m_parent.empty()) {
    out << " extends " << m_parent.c_str();
  }
  if (!m_bases.empty()) {
    if (m_modifiers & Interface) {
      out << " extends ";
    } else {
      out << " implements ";
    }
    for (unsigned int i = 0; i < m_bases.size(); i++) {
      if (i > 0) out << ", ";
      out << m_bases[i].c_str();
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
}

void ClassStatement::getInfo(ClassInfoEvaled &info) const {
  int attr = 0;
  if (m_modifiers & Interface) attr |= ClassInfo::IsInterface;
  if (m_modifiers & Abstract) attr |= ClassInfo::IsAbstract;
  if (m_modifiers & Final) attr |= ClassInfo::IsFinal;
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
  for (StringMap<ExpressionPtr>::const_iterator it = m_constants.begin();
       it != m_constants.end(); ++it) {
    ClassInfo::ConstantInfo *c = new ClassInfo::ConstantInfo;
    c->name = it->first;
    c->setValue(it->second->eval(dv));
    String sv = c->getValue().toString();
    char* buf = new char[sv.size()+1];
    memcpy(buf, sv.data(), sv.size()+1);
    c->valueLen = sv.size();
    c->valueText = buf;
    info.m_constants[c->name] = c;
    info.m_constantsVec.push_back(c);
  }

  for (vector<MethodStatementPtr>::const_iterator it = m_methodsVec.begin();
       it != m_methodsVec.end(); ++it) {
    ClassInfo::MethodInfo *m = new ClassInfo::MethodInfo;
    (*it)->getInfo(*m);
    info.m_methods[(*it)->name()] = m;
    info.m_methodsVec.push_back(m);
  }
}

bool ClassStatement::hasAccess(const char *context, Modifier level) const {
  ASSERT(context);
  switch (level) {
  case Public: return true;
  case Private: return strcasecmp(context, m_name.c_str()) == 0;
  case Protected:
    {
      if (!*context) return false;
      if (strcasecmp(context, m_name.c_str()) == 0) return true;
      const ClassStatement *cls = RequestEvalState::findClass(context);
      return cls->subclassOf(m_name.c_str()) || subclassOf(context);
    }
  default:
    ASSERT(false);
    return true;
  }
}

bool ClassStatement::attemptPropertyAccess(CStrRef prop, const char *context,
                                           int &mods,
                                           bool rec /* = false */) const {
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

void ClassStatement::failPropertyAccess(CStrRef prop, const char *context,
    int mods) const {
  const char *mod = "protected";
  Modifier level = Public;
  if (mods & Private) level = Private;
  else if (mods & Protected) level = Protected;
  if (level == ClassStatement::Private) mod = "private";
  throw FatalErrorException(0, "Attempt to access %s %s::%s%s%s",
      mod, m_name.c_str(), prop.data(),
      *context ? " from " : "",
      *context ? context : "");
}

void ClassStatement::toArray(Array &props, Array &vals) const {
  String zero("\0", 1, AttachLiteral);
  for (vector<ClassVariablePtr>::const_iterator it = m_variablesVec.begin();
       it != m_variablesVec.end(); ++it) {
    ClassVariable *cv = it->get();
    if ((cv->getModifiers() & Static) == 0) {
      String pname(cv->name().c_str(), cv->name().size(), AttachLiteral);
      if (cv->getModifiers() & Private) {
        String tmp(pname);
        pname = zero;
        pname += name().c_str();
        pname += zero;
        pname += tmp;
      }
      if (vals.exists(pname)) {
        Variant &p = vals.lvalAt(pname, AccessFlags::Key);
        props.lvalAt(pname, AccessFlags::Key).setWithRef(p);
      }
    }
  }
  const ClassStatement *parent = parentStatement();
  if (parent) {
    parent->toArray(props, vals);
  }
}

void ClassStatement::semanticCheck(const ClassStatement *cls)
  const {
  loadInterfaceStatements();
  const ClassStatement *parent = parentStatement();
  if (cls) {
    if (getModifiers() & (Interface|Abstract))  {
      bool extendingAbstractClass = (getModifiers() & Abstract);
      for (vector<MethodStatementPtr>::const_iterator it =
        m_methodsVec.begin(); it != m_methodsVec.end(); ++it) {
        if ((*it)->isAbstract()) {
          const MethodStatement *m = cls->findMethod((*it)->name().c_str(),
              true);
          bool found = false;
          bool incompatible = false;
          if (!m) {
            // Possibly built in
            const ClassInfo *pcls = cls->getBuiltinParentInfo();
            if (pcls) {
              ClassInfo *methCls;
              const ClassInfo::MethodInfo *meth =
                pcls->hasMethod((*it)->name().c_str(), methCls);
              if (meth) {
                found = true;
                if (extendingAbstractClass &&
                    strcmp(meth->name, "__construct") == 0) {
                  // When a class C extends an abstract class B, the
                  // signature of C::__construct does not have to match
                  // that of B::__construct
                } else if (meth->parameters.size() < (*it)->getParams().size()
                    || !(meth->attribute & ClassInfo::IsStatic) !=
                    !((*it)->getModifiers() & Static)) {
                  incompatible = true;
                } else if ((bool)(*it)->refReturn() !=
                           (bool)(meth->attribute & ClassInfo::IsReference)) {
                  // If one signature returns by value and the other returns by
                  // reference then they are not compatible
                } else {
                  const vector<ParameterPtr> &p1 = (*it)->getParams();
                  const vector<const ClassInfo::ParameterInfo *> &p2 =
                    meth->parameters;
                  for (uint i = 0; !incompatible && i < p2.size(); ++i) {
                    if (i >= p1.size()) {
                      if (!p2[i]->value) {
                        incompatible = true;
                      }
                    } else if ((!p1[i]->isRef() !=
                          !(p2[i]->attribute & ClassInfo::IsReference)) ||
                          (p1[i]->isOptional() && !(p2[i]->value))) {
                      incompatible = true;
                    }  else if (p1[i]->type() != p2[i]->type) {
                      incompatible = true;
                    }
                  }
                }
              }
            }
          } else if (!m->isAbstract()) {
            found = true;
            if (extendingAbstractClass &&
                strcmp(m->name().c_str(), "__construct") == 0) {
              // When a class C extends an abstract class B, the
              // signature of C::__construct does not have to match
              // that of B::__construct
            } else if (m->getParams().size() < (*it)->getParams().size() ||
                (m->getModifiers() & Static) !=
                ((*it)->getModifiers() & Static)) {
              incompatible = true;
            } else if (m->refReturn() != (*it)->refReturn()) {
              // If one signature returns by value and the other returns by
              // reference then they are not compatible
              incompatible = true;
            } else {
              const vector<ParameterPtr> &p1 = (*it)->getParams();
              const vector<ParameterPtr> &p2 = m->getParams();
              for (uint i = 0; !incompatible && i < p2.size(); ++i) {
                if (i >= p1.size()) {
                  if (!p2[i]->isOptional()) {
                    incompatible = true;
                  }
                } else if (p1[i]->isRef() != p2[i]->isRef() ||
                    p1[i]->isOptional() && !p2[i]->isOptional()) {
                  incompatible = true;
                } else if (p1[i]->type() != p2[i]->type()) {
                  incompatible = true;
                }
              }
            }
          }
          if (!found) {
            throw FatalErrorException(0,"Class %s does not implement abstract "
                "method %s::%s", cls->name().c_str(),
                name().c_str(), (*it)->name().c_str());
          }
          if (incompatible) {
            throw FatalErrorException(0,"Declaration of %s::%s() must be "
                "compatible with that of %s::%s()",
                cls->name().c_str(), m->name().c_str(),
                name().c_str(), (*it)->name().c_str());
          }
        }
      }
    }

    // Property check
    for (vector<ClassVariablePtr>::const_iterator it = m_variablesVec.begin();
        it != m_variablesVec.end(); ++it) {
      StringMap<ClassVariablePtr>::const_iterator vit =
        cls->m_variables.find((*it)->name());
      if (vit != m_variables.end()) {
        int m1 = (*it)->getModifiers();
        int m2 = vit->second->getModifiers();
        // Access levels
        int p1 = m1 & (Public|Protected|Private);
        if (!p1) p1 = Public;
        int p2 = m2 & (Public|Protected|Private);
        if (!p2) p2 = Public;
        if (p1 < p2) {
          const char *pn1;
          if (p1 == Private) pn1 = "private";
          else if (p1 == Protected) pn1 = "protected";
          else pn1 = "public";
          // Illegal strengthening of privacy
          raise_error("Access level to %s::%s must be %s (as in class %s) "
              "or weaker", cls->name().c_str(), (*it)->name().c_str(), pn1,
              name().c_str());
        } else if (m1 & Static && p1 == Protected && p2 == Public &&
            vit->second->hasInitialValue()) {
          // No initial value allowed in redefinition of protected to public
          // static
          raise_error("Cannot change initial value of property %s::%s in class"
          " %s", name().c_str(), (*it)->name().c_str(), cls->name().c_str());
        }
        // Staticness
        if (p1 != Private) {
          if (m1 & Static && !(m2 & Static)) {
            raise_error("Cannot redeclare static %s::%s as non-static %s::%s",
                name().c_str(), (*it)->name().c_str(), cls->name().c_str(),
                vit->first.c_str());
          } else if (!(m1 & Static) && m2 & Static) {
            raise_error("Cannot redeclare non-static %s::%s as static %s::%s",
                name().c_str(), (*it)->name().c_str(), cls->name().c_str(),
                vit->first.c_str());
          }
        }
      }
    }

  } else {
    if (!(getModifiers() & (Interface|Abstract))) {
      for (vector<MethodStatementPtr>::const_iterator it =
        m_methodsVec.begin(); it != m_methodsVec.end(); ++it) {
        if ((*it)->isAbstract()) {
          raise_error("Class %s contains abstract method %s and must therefore"
                      " be declared abstract", name().c_str(),
                      (*it)->name().c_str());
        }
      }
    }
    if (parent && parent->getModifiers() & Final) {
      // Extended a final class
      throw FatalErrorException(0,"Class %s may not inherit from final class "
                                "(%s)",
                                name().c_str(), parent->name().c_str());
    }
    if (parent) {
      set<const ClassStatement *> seen;
      recursiveParentCheck(seen);
    }
    // checking against parent methods
    if (parent || !m_bases.empty()) {
      bool iface = getModifiers() & Interface;
      for (vector<MethodStatementPtr>::const_iterator it =
          m_methodsVec.begin(); it != m_methodsVec.end(); ++it) {
        const MethodStatement *m = findParentMethod((*it)->name().c_str(),
            true);
        if (m == NULL) continue;
        const ClassStatement *mClass = m->getClass();
        bool ifaceParent = mClass->getModifiers() & Interface;

        int tmod = (*it)->getModifiers();
        int pmod = m->getModifiers();

        if ((tmod & Static) && !(pmod & Static)) {
          raise_error("Cannot make non static method %s::%s() static in "
              "class %s in %s on line %d",
              mClass->name().c_str(), m->name().c_str(),
              m_name.c_str(), (*it)->loc()->file, (*it)->loc()->line1);
        }

        if (!(tmod & Static) && (pmod & Static)) {
          raise_error("Cannot make static method %s::%s() non static in "
              "class %s in %s on line %d",
              mClass->name().c_str(), m->name().c_str(),
              m_name.c_str(), (*it)->loc()->file, (*it)->loc()->line1);
        }

        if (tmod & Abstract || iface) {
          if (!(pmod & Abstract) && (tmod & Abstract)) {
            raise_error("Cannot make non abstract method %s::%s() abstract in "
                "class %s in %s on line %d",
                mClass->name().c_str(), m->name().c_str(),
                m_name.c_str(), (*it)->loc()->file,
                (*it)->loc()->line1);
          }

          if (pmod & Abstract || ifaceParent) {
            raise_error("Cannot re-declare abstract method %s::%s() abstract "
                "in class %s in %s on line %d",
                mClass->name().c_str(), m->name().c_str(),
                m_name.c_str(), (*it)->loc()->file,
                (*it)->loc()->line1);
          }
        }
        int m1 = (*it)->getModifiers();
        int m2 = m->getModifiers();
        // Access levels
        int p1 = m1 & (Public|Protected|Private);
        if (!p1) p1 = Public;
        int p2 = m2 & (Public|Protected|Private);
        if (!p2) p2 = Public;
        if (p1 > p2) {
          const char *pn;
          if (p2 == Private) pn = "private";
          else if (p2 == Protected) pn = "protected";
          else pn = "public";
          // Illegal strengthening of privacy
          raise_error("Access level to %s::%s() must be %s (as in class %s) "
              "or weaker", name().c_str(), (*it)->name().c_str(), pn,
              mClass->name().c_str());
        }
      }
      // Check for multiple abstract function declarations
      hphp_const_char_imap<const char*> abstracts;
      abstractMethodCheck(abstracts, true);
    }
    cls = this;
  }

  if (parent) {
    if (parent->getModifiers() & Interface) {
      raise_error("%s cannot extend %s - it is an interface",
                  name().c_str(), parent->name().c_str());
    }
    if ((cls->getModifiers() & (Interface|Abstract)) == 0) {
      parent->semanticCheck(cls);
    }
  } else if (!m_parent.empty() && !f_class_exists(m_parent.c_str(), false)) {
    raise_error("Class '%s' does not exist.", m_parent.c_str());
  }
  for (unsigned i = 0; i < m_bases.size(); i++) {
    const ClassStatement *iface = RequestEvalState::findClass(m_bases[i]);
    if (iface) {
      if ((iface->getModifiers() & Interface) == 0) {
        raise_error("%s cannot implement %s - it is not an interface",
                    name().c_str(), iface->name().c_str());
      }
      if ((cls->getModifiers() & (Interface|Abstract)) == 0) {
        iface->semanticCheck(cls);
      }
    } else if (!f_interface_exists(m_bases[i], false)) {
      raise_error("Interface '%s' does not exist.", m_bases[i].c_str());
    }
  }
}

const MethodStatement* ClassStatement::findParentMethod(const char* name,
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
    hphp_const_char_imap<const char*> &abstracts, bool ifaces) const {
  bool iface = getModifiers() & Interface;
  if (iface || getModifiers() & Abstract)  {
    for (vector<MethodStatementPtr>::const_iterator it = m_methodsVec.begin();
        it != m_methodsVec.end(); ++it) {
      if (iface || (*it)->getModifiers() & Abstract) {
        hphp_const_char_imap<const char*>::const_iterator ait =
          abstracts.find((*it)->name().c_str());
        if (ait != abstracts.end() && ait->second != name().c_str()) {
          raise_error("Can't inherit abstract function %s::%s (previously "
              "declared abstract in %s)", name().c_str(), ait->first,
              ait->second);
        }
        abstracts[(*it)->name().c_str()] = name().c_str();
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
            hphp_const_char_imap<const char*>::const_iterator ait =
              abstracts.find((*mit)->name);
            if (ait != abstracts.end() && ait->second != ici->getName()) {
              raise_error("Can't inherit abstract function %s::%s (previously "
                          "declared abstract in %s)",
                          m_bases[i].c_str(), (*mit)->name.c_str(),
                          ait->second);
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
  if (!m_parent.empty()) return ClassInfo::FindClass(m_parent.c_str());
  return NULL;
}

void ClassStatement::recursiveParentCheck(
    std::set<const ClassStatement*> &seen) const {
  if (seen.find(this) != seen.end()) {
    raise_error("%s is defined as its own parent", name().c_str());
  } else {
    seen.insert(this);
  }
  const ClassStatement *parent = parentStatement();
  if (parent) {
    parent->recursiveParentCheck(seen);
  }
}

ClassStatementMarkerPtr ClassStatement::getMarker() const {
  return m_marker;
}

ClassStatementMarker::ClassStatementMarker(STATEMENT_ARGS,
                                           ClassStatement *cls)
  : Statement(STATEMENT_PASS), m_class(cls) {
}

void ClassStatementMarker::eval(VariableEnvironment &env) const {
  ClassEvalState *ce = RequestEvalState::findClassState(m_class->name());
  if (!ce || ce->getClass() != m_class) {
    // Delayed due to volatility
    m_class->evalImpl(env);
    ce = RequestEvalState::findClassState(m_class->name());
  }
  ASSERT(ce);
  ce->semanticCheck();
}

void ClassStatementMarker::dump(std::ostream &out) const {
}

///////////////////////////////////////////////////////////////////////////////
}
}
