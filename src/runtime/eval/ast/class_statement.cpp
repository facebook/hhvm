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
  : Construct(CONSTRUCT_PASS), m_name(StringData::GetStaticString(name)),
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
    m_modifiers(0), m_parent(StringData::GetStaticString(parent)),
    m_docComment(doc),
    m_marker(new ClassStatementMarker(STATEMENT_PASS, this)),
    m_delayDeclaration(false) { }

void ClassStatement::finish() {
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
  ClassEvalState::MethodTable &mtable = ce.getMethodTable();
  if (!m_parent->empty()) {
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
                                  "Cannot override final method %s::%s()",
                                  mmit->getClass()->name().c_str(),
                                  (*it)->name().c_str());
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
                                  "(as in class %s)",
                                  (*it)->name().c_str(), al,
                                  mmit ?
                                  mmit->getClass()->name().c_str() :
                                  m_parent->data());
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
    m_methods.find(m_name->data());
  if (it != m_methods.end()) {
    ce.getConstructor() = it->second.get();
  }
  it = m_methods.find("__construct");
  if (it != m_methods.end()) {
    ce.getConstructor() = it->second.get();
  }
}

void ClassStatement::optimize(VariableEnvironment &env) {
  for (unsigned int i = 0; i < m_methodsVec.size(); i++) {
    m_methodsVec[i]->optimize(env);
  }
}

void ClassStatement::eval(VariableEnvironment &env) const {
  if (env.isGotoing()) return;
  if (m_delayDeclaration) return;
  if (!isBaseClass() && !m_marker) {
    // Class might not be valid to declare yet. If the parent and bases
    // have not been defined then don't declare until execution hits the
    // marker.
    if (!m_parent->empty() && !f_class_exists(m_parent, false)) {
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
  if (getModifiers() & Abstract) {
    throw FatalErrorException(0, "Cannot instantiate abstract class %s",
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
    m_bases.push_back(StringData::GetStaticString(bases[i].get()));
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
  if (m_methods.find(m->name().c_str()) != m_methods.end()) {
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
  } else if (!m->hasBody()) {
    if (!(m_modifiers & Interface)) {
      raise_error("Non-abstract %s::%s() must contain body",
                  m_name->data(), m->name().c_str());
    } else if (m->getModifiers() & Protected) {
      raise_error("Access type for interface method %s::%s() must be "
                  "omitted", m_name->data(), m->name().c_str());
    }
  }

  m_methods[m->name().c_str()] = m;
  m_methodsVec.push_back(m);
}
void ClassStatement::addConstant(const string &name, ExpressionPtr v) {
  // Array is the only one allowed by the grammer but disallowed semantically
  if (v->isKindOf(Expression::KindOfArrayExpression)) {
    raise_error("Arrays are not allowed in class constants");
  }
  m_constantNames.push_back(StringData::GetStaticString(name));
  m_constants[m_constantNames.back()] = v;
}

bool ClassStatement::instanceOf(const char *c) const {
  if (strcasecmp(m_name->data(), c) == 0) return true;
  for (unsigned i = 0; i < m_bases.size(); i++) {
    if (strcasecmp(m_bases[i]->data(), c) == 0) return true;
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
  case Private: return strcasecmp(context, m_name->data()) == 0;
  case Protected:
    {
      if (!*context) return false;
      if (strcasecmp(context, m_name->data()) == 0) return true;
      const ClassStatement *cls = RequestEvalState::findClass(context);
      return cls->subclassOf(m_name->data()) || subclassOf(context);
    }
  default:
    ASSERT(false);
    return true;
  }
}

bool ClassStatement::attemptPropertyAccess(CStrRef prop, const char *context,
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

void ClassStatement::failPropertyAccess(CStrRef prop, const char *context,
    int mods) const {
  const char *mod = "protected";
  Modifier level = Public;
  if (mods & Private) level = Private;
  else if (mods & Protected) level = Protected;
  if (level == ClassStatement::Private) mod = "private";
  throw FatalErrorException(0, "Attempt to access %s %s::%s%s%s",
      mod, m_name->data(), prop.data(),
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
  inline bool isFinal(const ClassStatement *cs) const {
    return cs->getModifiers() & ClassStatement::Final;
  }
  inline std::string name(const ClassStatement *cs) const {
    return cs->name().c_str();
  }
  inline const MethodStatement *
  findEvalMethod(const ClassStatement *cs,
                 const std::string &name,
                 bool ifaces) const {
    const ClassStatement *dc;
    return findEvalMethodWithClass(cs, name, dc, ifaces);
  }
  inline const MethodStatement *
  findEvalMethodWithClass(const ClassStatement *cs,
                          const std::string &name,
                          const ClassStatement *&definingClass,
                          bool ifaces) const {
    definingClass = NULL;
    const MethodStatement* ms =
      cs->findMethod(name.c_str(), true, ifaces);
    if (ms) {
      definingClass = ms->getClass();
      return ms;
    }
    return NULL;
  }
  inline const ClassInfo::MethodInfo *
  findBuiltinMethod(const ClassStatement *cs,
                    const std::string &name,
                    bool ifaces) const {
    const ClassInfo *dc;
    return findBuiltinMethodWithClass(cs, name, dc, ifaces);
  }
  inline const ClassInfo::MethodInfo *
  findBuiltinMethodWithClass(const ClassStatement *cs,
                             const std::string &name,
                             const ClassInfo *&definingClass,
                             bool ifaces) const {
    definingClass = NULL;
    const ClassInfo::MethodInfo *result = NULL;
    const ClassInfo *pci = cs->getBuiltinParentInfo();
    if (pci) {
      ClassInfo *ci;
      result = pci->hasMethod(name.c_str(), ci, ifaces);
      definingClass = ci;
    }
    if (result || !ifaces) return result;
    vector<const ClassInfo*> builtinIfaces;
    cs->collectBuiltinInterfaceInfos(builtinIfaces, true);
    for (vector<const ClassInfo*>::const_iterator it = builtinIfaces.begin();
         it != builtinIfaces.end(); ++it) {
      const ClassInfo *ici = *it;
      ClassInfo *ci;
      result = ici->hasMethod(name.c_str(), ci, ifaces);
      definingClass = ci;
      if (result) return result;
    }
    return NULL;
  }
  inline std::vector<const MethodStatement*>
  methods(const ClassStatement *cs) const {
    return extractRawConstPointers(cs->m_methodsVec);
  }
  inline std::vector<const ClassVariable*>
  variables(const ClassStatement *cs) const {
    return extractRawConstPointers(cs->m_variablesVec);
  }
  inline const ClassVariable* findVariable(const ClassStatement *cs,
                                           const std::string &name) const {
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
  inline std::string name(const MethodStatement *ms) const {
    return ms->name().c_str();
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
  inline std::string name(const ClassVariable *cv) const {
    return cv->name().c_str();
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
  inline std::string name(const ClassInfo *ci) const {
    return ci->getName().c_str();
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
               const std::string &name) const {
    return ci->getPropertyInfo(name.c_str());
  }
  inline ClassInfo::MethodInfo*
  findEvalMethod(const ClassInfo* ci,
                 const std::string &name,
                 bool ifaces) const {
    const ClassInfo *cs;
    return findEvalMethodWithClass(ci, name, cs, ifaces);
  }
  inline ClassInfo::MethodInfo*
  findEvalMethodWithClass(const ClassInfo* ci,
                          const std::string &name,
                          const ClassInfo* &definingClass,
                          bool ifaces) const {
    ClassInfo *dci;
    ClassInfo::MethodInfo *mi = ci->hasMethod(name.c_str(), dci, ifaces);
    definingClass = dci;
    return mi;
  }
  inline ClassInfo::MethodInfo*
  findBuiltinMethod(const ClassInfo* ci,
                    const std::string &name,
                    bool ifaces) const {
    return findEvalMethod(ci, name, ifaces);
  }
  inline ClassInfo::MethodInfo*
  findBuiltinMethodWithClass(const ClassInfo* ci,
                             const std::string &name,
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
  inline std::string name(const ClassInfo::MethodInfo *mi) const {
    return mi->name.c_str();
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
  inline std::string name(const ClassInfo::PropertyInfo *pi) const {
    return pi->name.c_str();
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
  if (extendingAbstractClass &&
      strcmp(x.name(child).c_str(), "__construct") == 0) {
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
    if (x.isAbstract(parentMethod) || ifaceParent) {
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
    builtinParent = ClassInfo::FindClass(m_parent->data());
    if (!builtinParent) {
      builtinParent = ClassInfo::FindInterface(m_parent->data());
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
        !s_semanticExtractor.isAbstract(this)) {
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
    if (parent && s_semanticExtractor.isFinal(parent)) {
      throw FatalErrorException(0,"Class %s may not inherit from final class "
                                "(%s)",
                                name().c_str(),
                                parent->name().c_str());
    }
    if (builtinParent && s_semanticExtractor.isFinal(builtinParent)) {
      throw FatalErrorException(0,"Class %s may not inherit from final class "
                                "(%s)",
                                name().c_str(),
                                builtinParent->getName().c_str());
    }

    // make sure classes don't define each other as their
    // own parents (no cycles in the inheritance tree).
    // omit this check for builtin classes (since we assume
    // that our builtin runtime is sane, plus it's impossible
    // for a builtin class to extends a user-defind class).
    if (parent) {
      set<const ClassStatement *> seen;
      recursiveParentCheck(seen);
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
        !iface ? ClassInfo::FindInterface(m_bases[i]->data()) : NULL;
      if (iface) {
        ClassLevelMethodAccessLevelCheck(iface, this);
      } else if (builtinIface) {
        ClassLevelMethodAccessLevelCheck(builtinIface, this);
      }
    }

    // Check for multiple abstract function declarations
    if (!m_parent->empty() || !m_bases.empty()) {
      hphp_const_char_imap<const char*> abstracts;
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
      builtinIface = ClassInfo::FindClass(m_bases[i]->data());
      if (LIKELY(!builtinIface)) {
        // ok, now try to find it as an interface
        builtinIface = ClassInfo::FindInterface(m_bases[i]->data());
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
                          m_bases[i]->data(), (*mit)->name.c_str(),
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
  if (!m_parent->empty()) return ClassInfo::FindClass(m_parent->data());
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
      const ClassInfo *ci = ClassInfo::FindInterface(m_bases[i]->data());
      if (ci) infos.push_back(ci);
    }
  }
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

///////////////////////////////////////////////////////////////////////////////

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
