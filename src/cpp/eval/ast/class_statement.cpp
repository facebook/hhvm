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

#include <cpp/eval/ast/class_statement.h>
#include <cpp/eval/ast/method_statement.h>
#include <cpp/eval/ast/expression.h>
#include <cpp/eval/runtime/eval_object_data.h>
#include <cpp/eval/ast/statement_list_statement.h>
#include <cpp/eval/runtime/eval_state.h>
#include <cpp/eval/runtime/variable_environment.h>
#include <cpp/eval/strict_mode.h>
#include <cpp/base/runtime_option.h>
#include <util/util.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

ClassVariable::ClassVariable(CONSTRUCT_ARGS, const string &name, int modifiers,
                             ExpressionPtr value, const string &doc)
  : Construct(CONSTRUCT_PASS), m_name(name),
    m_hash(hash_string_i(name.c_str(), name.size())),
    m_modifiers(modifiers), m_value(value), m_docComment(doc) {}

void ClassVariable::set(VariableEnvironment &env, EvalObjectData *self) const {
  if (!(m_modifiers & ClassStatement::Static) &&
      !self->o_exists(m_name.c_str(), m_hash)) {
    self->o_set(m_name.c_str(), m_hash, m_value ? m_value->eval(env) : Variant(),
                true);
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

ClassStatement::ClassStatement(STATEMENT_ARGS, const string &name,
                               const string &parent, const string &doc)
  : Statement(STATEMENT_PASS), m_name(name),
    m_lname(Util::toLower(m_name)),
    m_modifiers(0), m_parent(parent), m_docComment(doc) {}

void ClassStatement::finish() {
}

const ClassStatement *ClassStatement::parentStatement() const {
  return RequestEvalState::findClass(m_parent.c_str());
}


void ClassStatement::
loadMethodTable(hphp_const_char_imap<const MethodStatement*> &mtable) const {
  if (!m_parent.empty()) {
    const ClassStatement* parent_cls = parentStatement();
    if (parent_cls) {
      parent_cls->loadMethodTable(mtable);
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
    mtable[(*it)->name().c_str()] = it->get();
  }
  MethodStatement *constructor = NULL;
  bool nameMethod = false;
  bool constructMethod = false;
  hphp_const_char_imap<MethodStatementPtr>::const_iterator it =
    m_methods.find(m_name.c_str());
  if (it != m_methods.end()) {
    constructor = it->second.get();
    nameMethod = true;
  }
  it = m_methods.find("__construct");
  if (it != m_methods.end()) {
    constructor = it->second.get();
    constructMethod = true;
  }
  if (constructMethod && !nameMethod) {
    mtable[m_name.c_str()] = constructor;
  } else if (!constructMethod && nameMethod) {
    mtable["__construct"] = constructor;
  }
}

void ClassStatement::eval(VariableEnvironment &env) const {
  ENTER_STMT;
  RequestEvalState::declareClass(this);

  const ClassStatement* parent_cls = parentStatement();
  if (parent_cls && parent_cls->getModifiers() & Final) {
    // Extended a final class
    throw FatalErrorException("Class %s may not inherit from final class (%s)",
                              name().c_str(), parent_cls->name().c_str());
  }
  if (RuntimeOption::EnableStrict && parent_cls) {
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
  ce.initializeMethods();

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
  m_methods[m->lname().c_str()] = m;
  m_methodsVec.push_back(m);
}
void ClassStatement::addConstant(const string &name, ExpressionPtr v) {
  m_constants[name] = v;
}

bool ClassStatement::instanceOf(const char *c) const {
  if (strcasecmp(m_name.c_str(), c) == 0 ||
      m_bases.find(c) != m_bases.end()) return true;
  for (vector<std::string>::const_iterator it = m_basesVec.begin();
       it != m_basesVec.end(); ++it) {
    const ClassStatement *iface = RequestEvalState::findClass(it->c_str());
    if (iface && iface->instanceOf(c)) return true;
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

const MethodStatement* ClassStatement::findMethod(const char* name) const {
  hphp_const_char_imap<MethodStatementPtr>::const_iterator it =
    m_methods.find(name);
  if (it != m_methods.end()) {
    return it->second.get();
  } else {
    return NULL;
  }
}

bool ClassStatement::getConstant(Variant &res, const char *c) const {
  map<string, ExpressionPtr>::const_iterator it = m_constants.find(c);
  if (it != m_constants.end()) {
    DummyVariableEnvironment env;
    res = it->second->eval(env);
    return true;
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

bool ClassStatement::attemptPropertyAccess(CStrRef prop) const {
  CStrRef context = FrameInjection::getClassName(false);
  hphp_const_char_imap<ClassVariablePtr>::const_iterator it =
    m_variables.find(prop);
  if (it == m_variables.end()) {
    return false;
  }
  int mods = it->second->getModifiers();
  Modifier level = Public;
  if (mods & Private) level = Private;
  else if (mods & Protected) level = Protected;
  if (!hasAccess(context, level)) {
    string msg("Attempt to access ");
    if (level == Private) msg += "private ";
    else msg += string("protected ");
    msg += m_name + "::" + prop.data();
    if (!context.empty()) msg += string(" from ") + context.c_str();
    throw_fatal(msg.c_str());
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
}
