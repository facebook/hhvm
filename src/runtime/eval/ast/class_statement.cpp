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

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ClassVariable::ClassVariable(CONSTRUCT_ARGS, const string &name, int modifiers,
    ExpressionPtr value, const string &doc, ClassStatement *cls)
  : Construct(CONSTRUCT_PASS), m_name(name),
    m_hash(hash_string(name.c_str(), name.size())),
    m_modifiers(modifiers), m_value(value), m_docComment(doc), m_cls(cls) {
}

void ClassVariable::set(VariableEnvironment &env, EvalObjectData *self) const {
  if (!(m_modifiers & ClassStatement::Static)) {
    Variant val(m_value ? m_value->eval(env) : null_variant);
    if (m_modifiers & ClassStatement::Private) {
      self->o_setPrivate(m_cls->name().c_str(), m_name.c_str(), m_hash, val);
    } else if (!self->o_exists(m_name.c_str(), m_hash)) {
      self->o_set(m_name.c_str(), m_hash, val, true);
    }
  }
}
void ClassVariable::setStatic(VariableEnvironment &env, LVariableTable &st)
  const {
  if ((m_modifiers & ClassStatement::Static)) {
    st.get(m_name.c_str()) = m_value ? m_value->eval(env) : Variant();
  }
}

void ClassVariable::dump() const {
  ClassStatement::printModifiers(m_modifiers);
  printf("$%s", m_name.c_str());
  if (m_value) {
    printf(" = ");
    m_value->dump();
  }
  printf(";");
}

void ClassVariable::getInfo(ClassInfo::PropertyInfo &info) const {
  int attr = 0;
  if (m_modifiers & ClassStatement::Protected) attr |= ClassInfo::IsProtected;
  if (m_modifiers & ClassStatement::Private) attr |= ClassInfo::IsPrivate;
  if (attr == 0) attr |= ClassInfo::IsPublic;
  if (m_modifiers & ClassStatement::Static) attr |= ClassInfo::IsStatic;
  info.attribute = (ClassInfo::Attribute)attr;
  info.name = m_name.c_str();
  if (!m_docComment.empty()) {
    info.docComment = m_docComment.c_str();
  }
}

void ClassVariable::eval(VariableEnvironment &env, Variant &res) const {
  res = m_value->eval(env);
}

ClassStatement::ClassStatement(STATEMENT_ARGS, const string &name,
                               const string &parent, const string &doc)
  : Statement(STATEMENT_PASS), m_name(name),
    m_lname(Util::toLower(m_name)),
    m_modifiers(0), m_parent(parent), m_docComment(doc),
    m_marker(new ClassStatementMarker(STATEMENT_PASS, this)) {}

void ClassStatement::finish() {
}

const ClassStatement *ClassStatement::parentStatement() const {
  if (!m_parent.empty()) {
    return RequestEvalState::findClass(m_parent.c_str(), true);
  }
  return NULL;
}

void ClassStatement::loadInterfaceStatements() const {
  for (unsigned int i = 0; i < m_basesVec.size(); ++i) {
    RequestEvalState::findClass(m_basesVec[i].c_str(), true);
  }
}

void ClassStatement::
loadMethodTable(ClassEvalState &ce) const {
  hphp_const_char_imap<const MethodStatement*> &mtable = ce.getMethodTable();
  if (!m_parent.empty()) {
    const ClassStatement* parent_cls = parentStatement();
    if (parent_cls) {
      parent_cls->loadMethodTable(ce);
    } else {
      // Built in
      ClassInfo::MethodVec meths;
      ClassInfo::GetClassMethods(meths, m_parent.c_str(), 1);
      for (ClassInfo::MethodVec::const_iterator it = meths.begin();
           it != meths.end(); ++it) {
        mtable[(*it)->name] = NULL;
      }
    }
  }
  for (vector<MethodStatementPtr>::const_iterator it = m_methodsVec.begin();
       it != m_methodsVec.end(); ++it) {
    hphp_const_char_imap<const MethodStatement*>::iterator mit =
      mtable.find((*it)->name().c_str());
    if (mit != mtable.end()) {
      int mods = mit->second ? mit->second->getModifiers() : Public;
      if (mods & Final) {
        throw FatalErrorException("Cannot override final method %s::%s() at "
                                  "%s:%d",
                                  mit->second->getClass()->name().c_str(),
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
        throw FatalErrorException("Access level to %s must be %s or weaker "
                                  "(as in class %s) at %s:%d",
                                  (*it)->name().c_str(), al,
                                  mit->second ?
                                  mit->second->getClass()->name().c_str() :
                                  m_parent.c_str(),(*it)->loc()->file,
                                  (*it)->loc()->line1);
      }
      mit->second = it->get();
    } else {
      mtable[(*it)->name().c_str()] = it->get();
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
  ENTER_STMT;
  RequestEvalState::declareClass(this);

  const ClassStatement* parent_cls;
  if (RuntimeOption::EnableStrict && (parent_cls = parentStatement())) {
    for (vector<MethodStatementPtr>::const_iterator it = m_methodsVec.begin();
         it != m_methodsVec.end(); ++it) {

      const MethodStatementPtr child_method = *it;

      const ClassStatement* cs = this;
      while ((cs = cs->parentStatement())) {
        if (cs->findMethod(child_method->name().c_str())) {
          break;
        }
      }
      if (cs) {
        const MethodStatement*   parent_method =
          cs->findMethod(child_method->name().c_str());

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
    throw FatalErrorException("Cannot instantiate abstract class %s",
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

  eo = NEW(EvalObjectData)(ce, builtinParent, root);

  Object o(eo);
  if (init) {
    eo->dynCreate(params, init);
  } else {
    initializeObject(eo);
  }
  return o;
}

void ClassStatement::initializeObject(EvalObjectData *obj) const {
  DummyVariableEnvironment env;
  for (vector<ClassVariablePtr>::const_iterator it =
         m_variablesVec.begin();
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
  for (vector<ClassVariablePtr>::const_iterator it =
         m_variablesVec.begin();
       it != m_variablesVec.end(); ++it) {
    (*it)->setStatic(env, statics);
  }
}

void ClassStatement::addBases(const std::vector<String> &bases) {
  for (std::vector<String>::const_iterator it = bases.begin();
       it != bases.end(); ++it) {
    string s(it->data(), it->size());
    m_basesVec.push_back(s);
    m_bases[m_basesVec.back().c_str()] = true;
  }
}

void ClassStatement::addMethod(MethodStatementPtr m) {
  if (m_methods.find(m->lname().c_str()) != m_methods.end()) {
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
  } else if (!(m_modifiers & Interface) && !m->hasBody()) {
    raise_error("Non-abstract %s::%s() must contain body in %s on line %d",
        m_name.c_str(), m->name().c_str(),
        m->loc()->file, m->loc()->line1);
  }

  m_methods[m->lname().c_str()] = m;
  m_methodsVec.push_back(m);
}
void ClassStatement::addConstant(const string &name, ExpressionPtr v) {
  // Array is the only one allowed by the grammer but disallowed semantically
  if (v->cast<ArrayExpression>()) {
    raise_error("Arrays are not allowed in class constants on %s:%d",
                v->loc()->file, v->loc()->line1);
  }
  m_constants[name] = v;
}

bool ClassStatement::instanceOf(const char *c) const {
  if (strcasecmp(m_name.c_str(), c) == 0 ||
      m_bases.find(c) != m_bases.end()) return true;
  for (vector<std::string>::const_iterator it = m_basesVec.begin();
       it != m_basesVec.end(); ++it) {
    const ClassStatement *iface = RequestEvalState::findClass(it->c_str());
    if (iface) {
      if (iface->instanceOf(c)) {
        return true;
      }
    } else {
      // Class lookup failed, but it may be due to crossing the boundary into
      // the static ClassInfo data, in which case RequestEvalState::findClass()
      // ignores interfaces.
      const ClassInfo *ci;
      if ((ci = ClassInfo::FindInterface(it->c_str()))
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
                                                  bool recursive /* = false */)
  const {
  hphp_const_char_imap<MethodStatementPtr>::const_iterator it =
    m_methods.find(name);
  if (it != m_methods.end()) {
    return it->second.get();
  } else {
    if (recursive) {
      const ClassStatement *parent = parentStatement();
      if (parent) {
        return parent->findMethod(name, true);
      }
    }
    return NULL;
  }
}

const ClassVariable* ClassStatement::findVariable(const char* name,
                                                  bool recursive /* = false */)
  const {
  hphp_const_char_imap<ClassVariablePtr>::const_iterator it =
    m_variables.find(name);
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
  map<string, ExpressionPtr>::const_iterator it = m_constants.find(c);
  if (it != m_constants.end()) {
    DummyVariableEnvironment env;
    res = it->second->eval(env);
    return true;
  }
  if (recursive) {
    const ClassStatement *p = parentStatement();
    if (p && p->getConstant(res, c, true)) return true;
    for (vector<string>::const_iterator it = m_basesVec.begin();
         it != m_basesVec.end(); ++it) {
      const ClassStatement *b =
        RequestEvalState::findClass(it->c_str(), true);
      if (b && b->getConstant(res, c, true)) return true;
    }
  }
  return false;
}

void ClassStatement::dump() const {
  printf("class %s {", m_name.c_str());
  for (hphp_const_char_imap<MethodStatementPtr>::const_iterator it =
         m_methods.begin(); it != m_methods.end(); ++it) {
    it->second->dump();
  }
  printf("}");
}

void ClassStatement::printModifiers(int m) {
  if (m & Public) printf("public ");
  if (m & Protected) printf("protected ");
  if (m & Private) printf("private ");
  if (m & Static) printf("static ");
  if (m & Abstract) printf("abstract ");
  if (m & Final) printf("final ");
  if (m & Interface) printf("interface ");
}

void ClassStatement::getPropertyInfo(ClassInfoEvaled &owner)  const {
  for (vector<ClassVariablePtr>::const_iterator it =
         m_variablesVec.begin();
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
  info.m_name = m_name.c_str();
  info.m_parentClass = m_parent.c_str();
  if (!m_docComment.empty()) {
    info.m_docComment = m_docComment.c_str();
  }
  for (vector<string>::const_iterator it = m_basesVec.begin();
       it != m_basesVec.end(); ++it) {
    info.m_interfacesVec.push_back(it->c_str());
    info.m_interfaces.insert(it->c_str());
  }
  getPropertyInfo(info);
  DummyVariableEnvironment dv;
  for (map<string, ExpressionPtr>::const_iterator it = m_constants.begin();
       it != m_constants.end(); ++it) {
    ClassInfo::ConstantInfo *c = new ClassInfo::ConstantInfo;
    c->name = it->first.c_str();
    c->value = it->second->eval(dv);
    String sv = c->value.toString();
    char* buf = new char[sv.size()+1];
    memcpy(buf, sv.data(), sv.size()+1);
    c->valueLen = sv.size();
    c->valueText = buf;
    info.m_constants[c->name] = c;
  }

  for (vector<MethodStatementPtr>::const_iterator it = m_methodsVec.begin();
       it != m_methodsVec.end(); ++it) {
    ClassInfo::MethodInfo *m = new ClassInfo::MethodInfo;
    (*it)->getInfo(*m);
    info.m_methods[(*it)->lname().c_str()] = m;
    info.m_methodsVec.push_back(m);
  }
}

bool ClassStatement::hasAccess(CStrRef context, Modifier level) const {
  ASSERT(context);
  switch (level) {
  case Public: return true;
  case Private: return strcasecmp(context.c_str(), m_name.c_str()) == 0;
  case Protected:
    {
      if (context.empty()) return false;
      if (strcasecmp(context.c_str(), m_name.c_str()) == 0) return true;
      const ClassStatement *cls = RequestEvalState::findClass(context.c_str());
      return cls->subclassOf(m_name.c_str()) || subclassOf(context.c_str());
    }
  default:
    ASSERT(false);
    return true;
  }
}

bool ClassStatement::attemptPropertyAccess(EvalObjectData *obj,
                                           CStrRef prop,
                                           CStrRef context,
                                           bool rec /* = false */) const {

  hphp_const_char_imap<ClassVariablePtr>::const_iterator it =
    m_variables.find(prop);
  if (it == m_variables.end()) {
    const ClassStatement *par = parentStatement();
    if (par) {
      par->attemptPropertyAccess(obj, prop, context, true);
    }
    return false;
  }
  int mods = it->second->getModifiers();
  Modifier level = Public;
  if (mods & Private) level = Private;
  else if (mods & Protected) level = Protected;
  if (level == Private && rec) return true;
  if (!hasAccess(context, level)) {
    // If __get() is defined, we fall back to it regardless.
    if (obj && obj->getMethodStatement("__get")) {
      return false;
    }

    const char *mod = "protected";
    if (level == ClassStatement::Private) mod = "private";
    throw FatalErrorException("Attempt to access %s %s::%s%s%s",
                              mod, m_name.c_str(), prop.data(),
                              !context.empty() ? " from " : "",
                              !context.empty() ? context.c_str() : "");
  }

  return true;
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
        Variant &p = vals.lvalAt(pname, -1);
        props.set(pname, p.isReferenced() ? ref(p) : p);
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
      for (vector<MethodStatementPtr>::const_iterator it =
        m_methodsVec.begin(); it != m_methodsVec.end(); ++it) {
        if ((*it)->isAbstract()) {
          const MethodStatement *m = cls->findMethod((*it)->name().c_str(),
              true);
          if (!m || m->isAbstract()) {
            throw FatalErrorException("Class %s does not implement abstract "
                "method %s::%s", cls->name().c_str(),
                name().c_str(), (*it)->name().c_str());
          }
          bool incompatible = false;
          if (strcmp(m->name().c_str(), "__construct") == 0) {
            // for some reason construct params aren't checked
          } else if (m->getParams().size() < (*it)->getParams().size() ||
              (m->getModifiers() & Static) !=
              ((*it)->getModifiers() & Static)) {
            incompatible = true;
          } else {
            const vector<ParameterPtr> &p1 = (*it)->getParams();
            const vector<ParameterPtr> &p2 = m->getParams();
            for (uint i = 0; i < p2.size(); ++i) {
              if (i >= p1.size()) {
                if (!p2[i]->isOptional()) {
                  incompatible = true;
                  break;
                }
              } else if (p1[i]->isRef() != p2[i]->isRef() ||
                  p1[i]->isOptional() != p2[i]->isOptional()) {
                incompatible = true;
                break;
              }
            }
          }
          if (incompatible) {
            throw FatalErrorException("Declaration of %s::%s() must be "
                "compatible with that of %s::%s()",
                cls->name().c_str(), m->name().c_str(),
                name().c_str(), (*it)->name().c_str());
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
      throw FatalErrorException("Class %s may not inherit from final class "
                                "(%s)",
                                name().c_str(), parent->name().c_str());
    }
    if (getModifiers() & (Interface|Abstract)) return;

    // checking against parent methods
    if (parent) {
      for (vector<MethodStatementPtr>::const_iterator it =
             m_methodsVec.begin(); it != m_methodsVec.end(); ++it) {
        const MethodStatement *m =
          parent->findMethod((*it)->name().c_str(), true);
        if (m == NULL) continue;

        int tmod = (*it)->getModifiers();
        int pmod = m->getModifiers();

        if ((tmod & Static) && !(pmod & Static)) {
          raise_error("Cannot make non static method %s::%s() static in "
                      "class %s in %s on line %d",
                      parent->name().c_str(), m->name().c_str(),
                      m_name.c_str(), (*it)->loc()->file, (*it)->loc()->line1);
        }

        if (!(tmod & Static) && (pmod & Static)) {
          raise_error("Cannot make static method %s::%s() non static in "
                      "class %s in %s on line %d",
                      parent->name().c_str(), m->name().c_str(),
                      m_name.c_str(), (*it)->loc()->file, (*it)->loc()->line1);
        }

        if (tmod & Abstract) {
          if (!(pmod & Abstract)) {
            raise_error("Cannot make non abstract method %s::%s() abstract in "
                        "class %s in %s on line %d",
                        parent->name().c_str(), m->name().c_str(),
                        m_name.c_str(), (*it)->loc()->file,
                        (*it)->loc()->line1);
          }

          if (pmod & Abstract) {
            raise_error("Cannot re-declare abstract method %s::%s() abstract "
                        "in class %s in %s on line %d",
                        parent->name().c_str(), m->name().c_str(),
                        m_name.c_str(), (*it)->loc()->file,
                        (*it)->loc()->line1);
          }
        }
      }
    }

    cls = this;
  }

  if (parent) {
    parent->semanticCheck(cls);
  }
  for (vector<string>::const_iterator it = m_basesVec.begin();
       it != m_basesVec.end(); ++it) {
    const ClassStatement *iface = RequestEvalState::findClass(it->c_str());
    if (iface) {
      iface->semanticCheck(cls);
    }
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
  ClassEvalState *ce = RequestEvalState::findClassState(m_class->
                                                        name().c_str());
  ASSERT(ce);
  ce->semanticCheck();
}

void ClassStatementMarker::dump() const {
}

///////////////////////////////////////////////////////////////////////////////
}
}
