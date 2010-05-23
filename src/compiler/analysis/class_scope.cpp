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

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/code_error.h>
#include <compiler/construct.h>
#include <compiler/analysis/variable_table.h>
#include <compiler/statement/statement_list.h>
#include <compiler/option.h>
#include <compiler/statement/interface_statement.h>
#include <util/util.h>
#include <compiler/analysis/constant_table.h>
#include <compiler/analysis/file_scope.h>
#include <runtime/base/class_info.h>
#include <compiler/parser/parser.h>
#include <compiler/statement/method_statement.h>
#include <runtime/base/zend/zend_string.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

ClassScope::ClassScope(KindOf kindOf, const std::string &name,
                       const std::string &parent,
                       const vector<string> &bases,
                       const std::string &docComment, StatementPtr stmt,
                       FileScopePtr file)
  : BlockScope(name, docComment, stmt, BlockScope::ClassScope), m_file(file),
    m_kindOf(kindOf), m_parent(parent), m_bases(bases), m_attribute(0),
    m_redeclaring(-1), m_volatile(false), m_needStaticInitializer(false),
    m_derivesFromRedeclaring(FromNormal), m_sep(false) {

  m_dynamic = Option::IsDynamicClass(m_name);

  // dynamic class is also volatile
  m_volatile = Option::AllVolatile || m_dynamic;

  ASSERT(m_parent.empty() || (!m_bases.empty() && m_bases[0] == m_parent));
}

ClassScope::ClassScope(AnalysisResultPtr ar,
                       const std::string &name, const std::string &parent,
                       const std::vector<std::string> &bases,
                       const std::vector<FunctionScopePtr> &methods)
  : BlockScope(name, "", StatementPtr(), BlockScope::ClassScope),
    m_kindOf(KindOfObjectClass), m_parent(parent), m_bases(bases),
    m_attribute(0), m_dynamic(false), m_redeclaring(-1), m_volatile(false),
    m_needStaticInitializer(false),
    m_derivesFromRedeclaring(FromNormal), m_sep(false) {
  BOOST_FOREACH(FunctionScopePtr f, methods) {
    if (f->getName() == "__construct") setAttribute(HasConstructor);
    else if (f->getName() == "__destruct") setAttribute(HasDestructor);
    else if (f->getName() == "__call") setAttribute(HasUnknownMethodHandler);
    else if (f->getName() == "__get") setAttribute(HasUnknownPropHandler);
    addFunction(ar, f);
  }
  setAttribute(Extension);
  setAttribute(System);

  ASSERT(m_parent.empty() || (!m_bases.empty() && m_bases[0] == m_parent));
}

const char *ClassScope::getOriginalName() const {
  if (m_stmt) {
    return dynamic_pointer_cast<InterfaceStatement>(m_stmt)->
      getOriginalName().c_str();
  }
  return m_name.c_str();
}

///////////////////////////////////////////////////////////////////////////////

void ClassScope::collectMethods(AnalysisResultPtr ar,
                                StringToFunctionScopePtrMap &funcs,
                                bool collectPrivate /* = true */,
                                bool forInvoke /* = false */) {
  // add all functions this class has
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    if (!collectPrivate && iter->second.back()->isPrivate()) continue;

    StringToFunctionScopePtrMap::const_iterator iterFuncs =
      funcs.find(iter->first);
    if (iterFuncs == funcs.end()) {
      funcs[iter->first] = iter->second.back();
    } else {
      iterFuncs->second->setVirtual();
      iter->second.back()->setVirtual();
    }
  }

  BOOST_FOREACH(string miss, m_missingMethods) {
    StringToFunctionScopePtrMap::const_iterator iterFuncs =
      funcs.find(miss);
    if (iterFuncs != funcs.end()) {
      iterFuncs->second->setVirtual();
    }
  }

  set<string> seen;
  seen.insert(m_name);

  // walk up
  for (int i = m_bases.size() - 1; i >= 0; i--) {
    const string &base = m_bases[i];
    if (seen.find(base) != seen.end()) {
      ar->getCodeError()->record(CodeError::InvalidDerivation, m_stmt,
                                 ConstructPtr(), base.c_str());
      m_bases.erase(m_bases.begin() + i);
      continue;
    }
    seen.insert(base);
    if (forInvoke && base != m_parent) {
      continue;
    }
    ClassScopePtr super = ar->findClass(base);
    if (super) {
      if (super->isRedeclaring() && base == m_parent) {
        if (forInvoke) continue;
        const ClassScopePtrVec &classes = ar->findClasses(m_parent);
        StringToFunctionScopePtrMap pristine(funcs);
        BOOST_FOREACH(ClassScopePtr cls, classes) {
          StringToFunctionScopePtrMap cur(pristine);
          cls->collectMethods(ar, cur, false, forInvoke);
          funcs.insert(cur.begin(), cur.end());
        }
        m_derivesFromRedeclaring = DirectFromRedeclared;
        getVariables()->forceVariants(ar);
        getVariables()->setAttribute(VariableTable::NeedGlobalPointer);
        setVolatile();
      } else {
        super->collectMethods(ar, funcs, false, forInvoke);
        if (super->derivesFromRedeclaring() && base == m_parent) {
          m_derivesFromRedeclaring = IndirectFromRedeclared;
          getVariables()->forceVariants(ar);
          setVolatile();
        }
      }
    } else {
      ar->getCodeError()->record(CodeError::UnknownBaseClass, m_stmt,
                                 ConstructPtr(), base.c_str());
      if (base == m_parent) {
        ar->findClasses(m_parent);
        m_derivesFromRedeclaring = DirectFromRedeclared;
        getVariables()->setAttribute(VariableTable::NeedGlobalPointer);
        getVariables()->forceVariants(ar);
        setVolatile();
      } else {
        m_bases.erase(m_bases.begin() + i);
      }
    }
  }
}

bool ClassScope::derivesFrom(AnalysisResultPtr ar,
                             const std::string &base) const {
  BOOST_FOREACH(std::string base_i, m_bases) {
    if (base_i == base) return true;
  }
  BOOST_FOREACH(std::string base_i, m_bases) {
    ClassScopePtr cl = ar->findClass(base_i);
    if (cl && cl->derivesFrom(ar, base)) return true;
  }
  return false;
}

FunctionScopePtr ClassScope::findFunction(AnalysisResultPtr ar,
                                          const std::string &name,
                                          bool recursive,
                                          bool exclIntfBase /*= false */) {
  ASSERT(Util::toLower(name) == name);
  StringToFunctionScopePtrVecMap::const_iterator iter;
  iter = m_functions.find(name);
  if (iter != m_functions.end()) {
    ASSERT(iter->second.back());
    return iter->second.back();
  }

  // walk up
  if (recursive && derivesFromRedeclaring() != DirectFromRedeclared) {
    for (int i = m_bases.size() - 1; i >= 0; i--) {
      const string &base = m_bases[i];
      ClassScopePtr super = ar->findClass(base);
      if (!super || (super->isInterface() && exclIntfBase)) continue;
      FunctionScopePtr func =
        super->findFunction(ar, name, true, exclIntfBase);
      if (func) return func;
    }
  }
  if (derivesFromRedeclaring() == DirectFromRedeclared) {
    setDynamic(ar, name);
  }

  return FunctionScopePtr();
}

FunctionScopePtr ClassScope::findConstructor(AnalysisResultPtr ar,
                                             bool recursive) {
  StringToFunctionScopePtrVecMap::const_iterator iter;
  string name;
  if (classNameCtor()) {
    name = getName();
  }
  else {
    name = "__construct";
  }
  iter = m_functions.find(name);
  if (iter != m_functions.end()) {
    ASSERT(iter->second.back());
    return iter->second.back();
  }

  // walk up
  if (recursive && derivesFromRedeclaring() != DirectFromRedeclared) {
    ClassScopePtr super = ar->findClass(m_parent);
    if (super) {
      FunctionScopePtr func =
        super->findConstructor(ar, true);
      if (func) return func;
    }
  }
  if (derivesFromRedeclaring() == DirectFromRedeclared) {
    setDynamic(ar, name);
  }

  return FunctionScopePtr();
}

void ClassScope::setStaticDynamic(AnalysisResultPtr ar) {
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    BOOST_FOREACH(FunctionScopePtr fs, iter->second) {
      if (fs->isStatic()) fs->setDynamic();
    }
  }
  if (!m_parent.empty()) {
    if (derivesFromRedeclaring() == DirectFromRedeclared) {
      ClassScopePtrVec parents = ar->findClasses(m_parent);
      BOOST_FOREACH(ClassScopePtr cl, parents) {
        cl->setStaticDynamic(ar);
      }
    } else {
      ClassScopePtr parent = ar->findClass(m_parent);
      if (parent) {
        parent->setStaticDynamic(ar);
      }
    }
  }
}

void ClassScope::setDynamic(AnalysisResultPtr ar, const std::string &name) {
  StringToFunctionScopePtrVecMap::const_iterator iter =
    m_functions.find(name);
  if (iter != m_functions.end()) {
    BOOST_FOREACH(FunctionScopePtr fs, iter->second) {
      fs->setDynamic();
    }
  } else if (!m_parent.empty()) {
    if (derivesFromRedeclaring() == DirectFromRedeclared) {
      ClassScopePtrVec parents = ar->findClasses(m_parent);
      BOOST_FOREACH(ClassScopePtr cl, parents) {
        cl->setDynamic(ar, name);
      }
    } else {
      ClassScopePtr parent = ar->findClass(m_parent);
      if (parent) {
        parent->setDynamic(ar, name);
      }
    }
  }
}

void ClassScope::setSystem() {
  setAttribute(ClassScope::System);
  m_volatile = m_dynamic = false;
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    iter->second[0]->setSystem();
  }
}

bool ClassScope::needLazyStaticInitializer() {
  return getVariables()->getAttribute(VariableTable::ContainsDynamicStatic) ||
    getConstants()->hasDynamic();
}

bool ClassScope::hasAttribute(ClassScope::Attribute attr,
                              AnalysisResultPtr ar) const {
  if (getAttribute(attr)) return true;

  if (!m_parent.empty()) {
    ClassScopePtr super = ar->findClass(m_parent);
    if (super) return super->hasAttribute(attr, ar);
  }

  return false;
}

void ClassScope::outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar) {
  // header
  int attribute = ClassInfo::IsNothing;
  if (!isUserClass()) attribute |= ClassInfo::IsSystem;
  if (isRedeclaring()) attribute |= ClassInfo::IsRedeclared;
  if (isVolatile()) attribute |= ClassInfo::IsVolatile;
  if (isInterface()) attribute |= ClassInfo::IsInterface | ClassInfo::IsAbstract;
  if (m_kindOf == KindOfAbstractClass) attribute |= ClassInfo::IsAbstract;
  if (m_kindOf == KindOfFinalClass) attribute |= ClassInfo::IsFinal;
  if (!m_docComment.empty()) attribute |= ClassInfo::HasDocComment;
  cg.printf("(const char *)0x%04X, \"%s\", \"%s\",\n", attribute,
            getOriginalName(), m_parent.c_str());

  if (!m_docComment.empty()) {
    char *dc = string_cplus_escape(m_docComment.c_str());
    cg.printf("\"%s\",\n", dc);
    free(dc);
  }

  // parent interfaces
  for (unsigned int i = (m_parent.empty() ? 0 : 1); i < m_bases.size(); i++) {
    cg.printf("\"%s\", ", m_bases[i].c_str());
  }
  cg.printf("NULL,\n");

  // methods
  std::map<int, FunctionScopePtrVec> sortedMethods; // by source line number
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    FunctionScopePtr func = iter->second.back();
    int index = 0;
    if (func->getStmt()) {
      LocationPtr loc = func->getStmt()->getLocation();
      if (loc) {
        index = loc->line1 * 1000 + loc->char1;
      }
    }
    sortedMethods[index].push_back(func);
  }
  for (std::map<int, FunctionScopePtrVec>::const_iterator iter =
         sortedMethods.begin(); iter != sortedMethods.end(); ++iter) {
    for (unsigned int i = 0; i < iter->second.size(); i++) {
      iter->second[i]->outputCPPClassMap(cg, ar);
    }
  }
  cg.printf("NULL,\n");

  // properties && constants
  ar->pushScope(shared_from_this());
  m_variables->outputCPPClassMap(cg, ar);
  m_constants->outputCPPClassMap(cg, ar);
  ar->popScope();
}

bool ClassScope::hasConst(const string &name) {
  return m_constants->isPresent(name);
}

TypePtr ClassScope::checkProperty(const std::string &name, TypePtr type,
                                  bool coerce, AnalysisResultPtr ar,
                                  ConstructPtr construct, int &properties) {
  return getVariables()->checkProperty(name, type, coerce,
                                       ar, construct, properties);
}

TypePtr ClassScope::checkStatic(const std::string &name, TypePtr type,
                                bool coerce, AnalysisResultPtr ar,
                                ConstructPtr construct, int &properties) {
  ar->pushScope(shared_from_this());
  TypePtr ret = getVariables()->checkVariable(name, type, coerce,
                                              ar, construct, properties);
  ar->popScope();
  return ret;
}

TypePtr ClassScope::checkConst(const std::string &name, TypePtr type,
                               bool coerce, AnalysisResultPtr ar,
                               ConstructPtr construct,
                               const std::vector<std::string> &bases,
                               BlockScope *&defScope) {
  defScope = NULL;
  TypePtr t = getConstants()->check(name, type, coerce,
                                    ar, construct, m_bases, defScope);
  if (defScope) return t;
  return t;
}

ClassScopePtr ClassScope::getParentScope(AnalysisResultPtr ar) {
  if (m_parent.empty()) return ClassScopePtr();
  return ar->findClass(m_parent);
}

void ClassScope::serialize(JSON::OutputStream &out) const {
  JSON::MapStream ms(out);
  map<string, int> propMap;
  set<string> names;
  m_variables->getNames(names);
  BOOST_FOREACH(string name, names) {
    int pm = 0;
    if (m_variables->isPublic(name)) pm |= ClassScope::Public;
    else if (m_variables->isPrivate(name)) pm |= ClassScope::Private;
    else if (m_variables->isProtected(name)) pm |= ClassScope::Protected;
    if (m_variables->isStatic(name)) pm |= ClassScope::Static;
    propMap[name] = pm;
  }
  names.clear();
  vector<string> cnames;
  m_constants->getSymbols(cnames);

  // What's a mod again?
  ms.add("attributes", m_attribute)
    .add("kind", m_kindOf)
    .add("parent", m_parent)
    .add("bases", m_bases)
    .add("properties", propMap)
    .add("functions", m_functions);

  /* Super Sad */
  out.raw() << ",";
  out << JSON::Name("consts");
  JSON::MapStream cs(out);
  BOOST_FOREACH(string cname, cnames) {
    TypePtr type =  m_constants->getType(cname, true);
    if (!type) {
      cs.add(cname, -1);
    } else if (type->isSpecificObject()) {
      cs.add(cname, type->getName());
    } else {
      cs.add(cname, type->getKindOf());
    }
  }
  cs.done();
  ms.done();
}

void ClassScope::outputCPPDynamicClassDecl(CodeGenerator &cg) {
  string clsStr = getId();
  const char *clsName = clsStr.c_str();
  cg.printf("Object %s%s(CArrRef params, bool init = true);\n",
            Option::CreateObjectPrefix, clsName);
}

void ClassScope::outputCPPDynamicClassCreateDecl(CodeGenerator &cg) {
  cg.printf("Object create_object(const char *s, CArrRef params, "
            "bool init = true, ObjectData *root);\n");
}

void ClassScope::outputCPPDynamicClassImpl(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  string clsStr = getId();
  const char *clsName = clsStr.c_str();
  cg.indentBegin("Object %s%s(CArrRef params, bool init /* = true */) {\n",
                 Option::CreateObjectPrefix, clsName);
  cg.printf("return Object((NEW(%s%s)())->dynCreate(params, init));\n",
            Option::ClassPrefix, clsName);
  cg.indentEnd("}\n");
}

void ClassScope::outputCPPClassJumpTable
(CodeGenerator &cg,
 const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes, const char* macro) {
  cg.printDeclareGlobals();
  for (JumpTable jt(cg, classes, true, false, false); jt.ready(); jt.next()) {
    const char *clsName = jt.key();
    StringToClassScopePtrVecMap::const_iterator iterClasses =
      classScopes.find(clsName);
    if (iterClasses != classScopes.end()) {
      if (iterClasses->second[0]->isRedeclaring()) {
        cg.printf("%s_REDECLARED(0x%016llXLL, %s);\n", macro,
                  hash_string_i(clsName), clsName);
      } else if (iterClasses->second[0]->isVolatile()) {
        cg.printf("%s_VOLATILE(0x%016llXLL, %s);\n", macro,
                  hash_string_i(clsName), clsName);
      } else {
        cg.printf("%s(0x%016llXLL, %s);\n", macro,
                  hash_string_i(clsName), clsName);
      }
    }
  }
}

void ClassScope::outputCPPClassVarInitImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  cg.indentBegin("Variant get%s_class_var_init(const char *s, "
                 "const char *var) {\n",
                 system ? "_builtin" : "");
  bool withEval = !system && Option::EnableEval == Option::FullEval;
  if (withEval) {
    // See if there's an eval'd version
    cg.indentBegin("{\n");
    cg.printf("Variant r;\n");
    cg.printf("if (eval_get_class_var_init_hook(r, s, var)) "
              "return r;\n");
    cg.indentEnd("}\n");
  }
  outputCPPClassJumpTable(cg, classScopes, classes, "HASH_GET_CLASS_VAR_INIT");
  if (!system) {
    cg.printf("return get_builtin_class_var_init(s, var);\n");
  } else {
    cg.printf("return throw_missing_class(s);\n");
  }
  cg.indentEnd("}\n");
}

void ClassScope::outputCPPDynamicClassCreateImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  // output create_object()
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  cg.indentBegin("Object create%s_object(const char *s, "
                 "CArrRef params, bool init /* = true */,"
                 "ObjectData* root /* = NULL*/) {\n", system ?
                 "_builtin" : "");
  bool withEval = !system && Option::EnableEval == Option::FullEval;
  if (withEval) {
    // See if there's an eval'd version
    cg.indentBegin("{\n");
    cg.printf("Variant r;\n");
    cg.printf("if (eval_create_object_hook(r, s, params, init, root)) "
              "return r;\n");
    cg.indentEnd("}\n");
  }
  outputCPPClassJumpTable(cg, classScopes, classes, "HASH_CREATE_OBJECT");
  if (!system) {
    cg.printf("return create_builtin_object(s, params, init, root);\n");
  } else {
    cg.printf("return throw_missing_class(s);\n");
  }
  cg.indentEnd("}\n");
}

void ClassScope::outputCPPInvokeStaticMethodImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  cg.indentBegin("Variant invoke%s_static_method(const char *s, "
                 "const char *method, CArrRef params, bool fatal) {\n",
                 system ? "_builtin" : "");
  if (!system && Option::EnableEval == Option::FullEval) {
    // See if there's an eval'd version
    cg.printf("bool foundClass = false;\n");
    cg.indentBegin("{\n");
    cg.printf("Variant r;\n");
    cg.printf("if (eval_invoke_static_method_hook(r, s, method, params, "
              "foundClass)) return r;\n");
    cg.indentBegin("else if (foundClass) {\n");
    cg.printf("return o_invoke_failed(s, method, fatal);\n");
    cg.indentEnd("}\n");
    cg.indentEnd("}\n");
  }
  outputCPPClassJumpTable(cg, classScopes, classes, "HASH_INVOKE_STATIC_METHOD");

  // There should be invoke_failed for static methods...
  if (!system) {
    cg.printf("return invoke_builtin_static_method(s, method, params, fatal);"
              "\n");
  } else {
    cg.indentBegin("if (fatal) {\n");
    cg.printf("return throw_missing_class(s);\n");
    cg.indentEnd("");
    cg.indentBegin("} else {\n");
    cg.printf("raise_warning(\"call_user_func to non-existent class's method"
              " %%s::%%s\", s, method);\n");
    cg.printf("return false;\n");
    cg.indentEnd("}\n");
  }
  cg.indentEnd("}\n");
}

void ClassScope::outputCPPGetStaticPropertyImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  cg.indentBegin("Variant get%s_static_property(const char *s, "
                 "const char *prop) {\n",
                 system ? "_builtin" : "");
  if (!system && Option::EnableEval == Option::FullEval) {
    // See if there's an eval'd version
    cg.indentBegin("{\n");
    cg.printf("Variant r;\n");
    cg.printf("if (eval_get_static_property_hook(r, s, prop)) "
              "return r;\n");
    cg.indentEnd("}\n");
  }
  outputCPPClassJumpTable(cg, classScopes, classes, "HASH_GET_STATIC_PROPERTY");
  if (!system) {
    cg.printf("return get_builtin_static_property(s, prop);\n");
  } else {
    cg.printf("return null;\n");
  }
  cg.indentEnd("}\n");

  cg.indentBegin("Variant *get%s_static_property_lv(const char *s, "
                 "const char *prop) {\n",
                 system ? "_builtin" : "");
  if (!system && Option::EnableEval == Option::FullEval) {
    // See if there's an eval'd version
    cg.indentBegin("{\n");
    cg.printf("Variant *r;\n");
    cg.printf("if (eval_get_static_property_lv_hook(r, s, prop)) "
              "return r;\n");
    cg.indentEnd("}\n");
  }
  outputCPPClassJumpTable(cg, classScopes, classes, "HASH_GET_STATIC_PROPERTY_LV");
  if (!system) {
    cg.printf("return get_builtin_static_property_lv(s, prop);\n");
  } else {
    cg.printf("return NULL;\n");
  }
  cg.indentEnd("}\n");
}

void ClassScope::outputCPPGetClassConstantImpl
(CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
 const vector<const char*> &classes) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  cg.indentBegin("Variant get%s_class_constant(const char *s, "
                 "const char *constant, bool fatal /* = true */) {\n",
                 system ? "_builtin" : "");
  if (!system && Option::EnableEval == Option::FullEval) {
    // See if there's an eval'd version
    cg.indentBegin("{\n");
    cg.printf("Variant r;\n");
    cg.printf("if (eval_get_class_constant_hook(r, s, constant)) "
              "return r;\n");
    cg.indentEnd("}\n");
  }
  outputCPPClassJumpTable(cg, classScopes, classes, "HASH_GET_CLASS_CONSTANT");
  if (!system) {
    cg.printf("return get_builtin_class_constant(s, constant, fatal);\n");
  } else {
    cg.indentBegin("if (fatal) {\n");
    cg.printf("raise_error(\"Couldn't find constant %%s::%%s\", s, "
              "constant);\n");
    cg.indentEnd("");
    cg.indentBegin("} else {\n");
    cg.printf("raise_warning(\"Couldn't find constant %%s::%%s\", s, "
              "constant);\n");
    cg.indentEnd("}\n");
    cg.printf("return null;\n");
  }
  cg.indentEnd("}\n");
}

bool ClassScope::hasProperty(const string &name) {
  return m_variables->isPresent(name);
}

void ClassScope::setRedeclaring(AnalysisResultPtr ar, int redecId) {
  m_redeclaring = redecId;
  setVolatile(); // redeclared class is also volatile
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    BOOST_FOREACH(FunctionScopePtr fs, iter->second) {
      fs->setDynamic();
    }
  }
  m_variables->forceVariants(ar);
}

string ClassScope::getHeaderFilename() {
  FileScopePtr file = getFileScope();
  ASSERT(file);
  string fileBase = file->outputFilebase();
  string headerFile = Option::ClassHeaderPrefix;
  headerFile += getId() + ".h";
  return headerFile;
}

void ClassScope::outputCPPHeader(AnalysisResultPtr ar,
                                 CodeGenerator::Output output) {
  string filename = getHeaderFilename();
  string root = ar->getOutputPath() + "/";
  Util::mkdir(root + filename);
  ofstream f((root + filename).c_str());
  CodeGenerator cg(&f, output);

  cg.headerBegin(filename);

  // 1. includes
  BOOST_FOREACH(string base, m_bases) {
    ClassScopePtr cls = ar->findClass(base);
    if (cls && cls->isUserClass()) {
      cg.printInclude(cls->getHeaderFilename());
    }
  }

  // 2. Declarations
  cg.namespaceBegin();
  ar->pushScope(shared_from_this());
  cg.setContext(CodeGenerator::CppDeclaration);
  getStmt()->outputCPP(cg, ar);
  ar->popScope();
  cg.namespaceEnd();

  cg.headerEnd(filename);
}

void ClassScope::outputCPPSupportMethodsImpl(CodeGenerator &cg,
                                             AnalysisResultPtr ar) {
  string clsNameStr = getId();
  const char *clsName = clsNameStr.c_str();
  bool dynamicObject = derivesFromRedeclaring() == DirectFromRedeclared;
  const char *parent = "ObjectData";
  if (!getParent().empty()) parent = getParent().c_str();

  if (Option::GenerateCPPMacros) {
    // Constant Lookup Table
    getVariables()->outputCPPPropertyTable(cg, ar, parent,
                                           derivesFromRedeclaring());
    cg.indentBegin("Variant %s%s::%sconstant(const char *s) {\n",
                   Option::ClassPrefix, clsName, Option::ObjectStaticPrefix);
    if (dynamicObject) cg.printDeclareGlobals();
    getConstants()->outputCPPJumpTable(cg, ar, !dynamicObject, false);
    // If parent is redeclared, you have to go to their class statics object.
    if (dynamicObject) {
      cg.printf("return %s->%s%s->%sconstant(s);\n", cg.getGlobals(ar),
                Option::ClassStaticsObjectPrefix, parent,
                Option::ObjectStaticPrefix);
    } else {
      cg.printf("return %s%s::%sconstant(s);\n", Option::ClassPrefix, parent,
                Option::ObjectStaticPrefix);
    }
    cg.indentEnd("}\n");

    cg.printf("IMPLEMENT_CLASS(%s)\n", clsName);
  }

  // Create method
  if (getAttribute(ClassScope::HasConstructor)
   || getAttribute(ClassScope::classNameConstructor)) {
    FunctionScopePtr func = findConstructor(ar, false);
    if (func && !func->isAbstract() && !isInterface()) {
      // abstract methods are not generated, neither should the create method
      // for an abstract constructor
      ar->pushScope(func);
      func->outputCPPCreateImpl(cg, ar);
      ar->popScope();
    }
  }

  // Destruct method
  if (getAttribute(ClassScope::HasDestructor)) {
    cg.indentBegin("void %s%s::destruct() {\n", Option::ClassPrefix, clsName);
    cg.indentBegin("if (!inCtorDtor()) {\n");
    cg.printf("incRefCount();\n");
    cg.indentBegin("try {\n");
    cg.printf("%s__destruct();\n", Option::MethodPrefix);
    cg.indentEnd("} catch (...) { handle_destructor_exception();}\n");
    cg.indentEnd("}\n");
    cg.indentEnd("}\n");
  }

  // Cloning
  cg.indentBegin("ObjectData *%s%s::cloneImpl() {\n",
                 Option::ClassPrefix, clsName);
  cg.printf("%s%s *obj = NEW(%s%s)();\n", Option::ClassPrefix, clsName,
            Option::ClassPrefix, clsName);
  cg.printf("cloneSet(obj);\n");
  cg.printf("return obj;\n");
  cg.indentEnd("}\n");
  cg.indentBegin("void %s%s::cloneSet(%s%s *clone) {\n",
                 Option::ClassPrefix, clsName, Option::ClassPrefix, clsName);
  getVariables()->outputCPPPropertyClone(cg, ar, derivesFromRedeclaring());
  if (derivesFromRedeclaring()) {
    cg.printf("clone->setParent(parent->clone());\n");
  } else if(!getParent().empty()) {
    cg.printf("%s%s::cloneSet(clone);\n", Option::ClassPrefix, parent);
  } else {
    cg.printf("ObjectData::cloneSet(clone);\n");
  }
  cg.indentEnd("}\n");

  // doCall
  if (getAttribute(ClassScope::HasUnknownMethodHandler)) {
    cg.indentBegin("Variant %s%s::doCall(Variant v_name, Variant "
                   "v_arguments, bool fatal) {\n",
                   Option::ClassPrefix, clsName);
    cg.printf("return t___call(v_name, v_arguments);\n");
    cg.indentEnd("}\n");
  }

  // doGet
  if (getAttribute(ClassScope::HasUnknownPropHandler)) {
    cg.indentBegin("Variant %s%s::doGet(Variant v_name, bool error) {\n",
                   Option::ClassPrefix, clsName);
    cg.printf("return t___get(v_name);\n");
    cg.indentEnd("}\n");
  }

  if (isRedeclaring() && !derivesFromRedeclaring()) {
    cg.indentBegin("Variant %s%s::doRootCall(Variant v_name, Variant "
                   "v_arguments, bool fatal) {\n",
                   Option::ClassPrefix, clsName);
    cg.printf("return root->doCall(v_name, v_arguments, fatal);\n");
    cg.indentEnd("}\n");
  }

  // Invoke tables
  if (Option::GenerateCPPMacros) {
    outputCPPJumpTable(cg, ar, false, dynamicObject);
    outputCPPJumpTable(cg, ar, true, dynamicObject);
    if (cg.getOutput() == CodeGenerator::SystemCPP ||
        Option::EnableEval >= Option::LimitedEval) {
      outputCPPJumpTable(cg, ar, false, dynamicObject, true);
      outputCPPJumpTable(cg, ar, true, dynamicObject, true);
    }
  }
  outputCPPGlobalTableWrappersImpl(cg, ar);
}

void ClassScope::outputCPPStaticInitializerDecl(CodeGenerator &cg) {
  if (needStaticInitializer()) {
    cg.printf("void %s%s();\n", Option::ClassStaticInitializerPrefix,
              getId().c_str());
  }
}

void ClassScope::outputCPPStaticMethodWrappers(CodeGenerator &cg,
                                               AnalysisResultPtr ar,
                                               set<string> &done,
                                               const char *cls) {
  const StringToFunctionScopePtrVecMap &fmap = getFunctions();
  for (StringToFunctionScopePtrVecMap::const_iterator it = fmap.begin();
       it != fmap.end(); ++it) {
    if (done.find(it->first) != done.end()) continue;
    MethodStatementPtr m = dynamic_pointer_cast<MethodStatement>(it->second[0]->
                                                                 getStmt());
    if (!m) continue; // system classes
    m->outputCPPStaticMethodWrapper(cg, ar, cls);
    done.insert(it->first);
  }
  if (derivesFromRedeclaring() != DirectFromRedeclared) {
    ClassScopePtr par = getParentScope(ar);
    if (par) par->outputCPPStaticMethodWrappers(cg, ar, done, cls);
  }
}

void ClassScope::outputCPPGlobalTableWrappersDecl(CodeGenerator &cg,
                                                  AnalysisResultPtr ar) {
  string id = getId();
  cg.printf("Variant %s%s$os_getInit(const char *s);\n",
            Option::ClassWrapperFunctionPrefix, id.c_str());
  cg.printf("Variant %s%s$os_get(const char *s);\n",
            Option::ClassWrapperFunctionPrefix, id.c_str());
  cg.printf("Variant &%s%s$os_lval(const char *s);\n",
            Option::ClassWrapperFunctionPrefix, id.c_str());
  cg.printf("Variant %s%s$os_constant(const char *s);\n",
            Option::ClassWrapperFunctionPrefix, id.c_str());
  cg.printf("Variant %s%s$os_invoke(const char *c, const char *s, "
            "CArrRef params, bool fatal = true);\n",
            Option::ClassWrapperFunctionPrefix, id.c_str());
}

void ClassScope::outputCPPGlobalTableWrappersImpl(CodeGenerator &cg,
                                                  AnalysisResultPtr ar) {
  string id = getId();

  cg.indentBegin("Variant %s%s$os_getInit(const char *s) {\n",
                 Option::ClassWrapperFunctionPrefix, id.c_str());
  cg.printf("return %s%s::os_getInit(s, -1);\n",
            Option::ClassPrefix, id.c_str());
  cg.indentEnd("}\n");

  cg.indentBegin("Variant %s%s$os_get(const char *s) {\n",
                 Option::ClassWrapperFunctionPrefix, id.c_str());
  cg.printf("return %s%s::os_get(s, -1);\n", Option::ClassPrefix, id.c_str());
  cg.indentEnd("}\n");

  cg.indentBegin("Variant &%s%s$os_lval(const char *s) {\n",
                Option::ClassWrapperFunctionPrefix, id.c_str());
  cg.printf("return %s%s::os_lval(s, -1);\n", Option::ClassPrefix, id.c_str());
  cg.indentEnd("}\n");

  cg.indentBegin("Variant %s%s$os_constant(const char *s) {\n",
                Option::ClassWrapperFunctionPrefix, id.c_str());
  cg.printf("return %s%s::os_constant(s);\n", Option::ClassPrefix, id.c_str());
  cg.indentEnd("}\n");

  cg.indentBegin("Variant %s%s$os_invoke(const char *c, const char *s, "
                 "CArrRef params, bool fatal /* = true */) {\n",
                 Option::ClassWrapperFunctionPrefix, id.c_str());
  cg.printf("return %s%s::os_invoke(c, s, params, -1, fatal);\n",
            Option::ClassPrefix, id.c_str());
  cg.indentEnd("}\n");
}

void ClassScope::addFunction(AnalysisResultPtr ar, FunctionScopePtr funcScope) {
  FunctionScopePtrVec &funcs = m_functions[funcScope->getName()];
  if (funcs.size() == 1) {
    funcs[0]->setRedeclaring(0);
    ar->getCodeError()->record(CodeError::DeclaredFunctionTwice,
                               funcScope->getStmt(),
                               funcs[0]->getStmt());
  }
  if (funcs.size() > 0) {
    funcScope->setRedeclaring(funcs.size());
  }
  funcs.push_back(funcScope);
}

void ClassScope::findJumpTableMethods(CodeGenerator &cg, AnalysisResultPtr ar,
                                      bool staticOnly,
                                      vector<const char *> &funcs) {
  bool systemcpp = cg.getOutput() == CodeGenerator::SystemCPP;
  // output invoke support methods
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    if (!iter->second[0]->isRedeclaring()) {
      FunctionScopePtr func = iter->second[0];
      if (func->isAbstract() ||
          (staticOnly && !func->isStatic()) ||
          !(systemcpp || func->isDynamic() || func->isVirtual())) continue;
      const char *name = iter->first.c_str();
      funcs.push_back(name);
    }
  }
}

void ClassScope::
outputCPPMethodInvokeTable(CodeGenerator &cg, AnalysisResultPtr ar,
                           const vector <const char*> &keys,
                           const StringToFunctionScopePtrVecMap &funcScopes,
                           bool fewArgs, bool staticOnly, bool forEval) {
  for (JumpTable jt(cg, keys, true, true, false); jt.ready(); jt.next()) {
    const char *name = jt.key();
    StringToFunctionScopePtrVecMap::const_iterator iterFuncs;
    iterFuncs = funcScopes.find(name);
    ASSERT(iterFuncs != funcScopes.end());
    FunctionScopePtr func = iterFuncs->second[0];
    if (fewArgs &&
        func->getMinParamCount() > Option::InvokeFewArgsCount)
      continue;
    cg.indentBegin("HASH_GUARD(0x%016llXLL, %s) {\n",
                   hash_string_i(name), name);
    const char *extra = NULL;
    const char *prefix = Option::MethodPrefix;
    if (func->isStatic()) {
      prefix = Option::MethodImplPrefix;
      if (staticOnly) {
        extra = "c";
      } else {
        extra = "o_getClassName()";
      }
    }
    if (forEval) {
      func->outputCPPEvalInvoke(cg, ar, prefix, name, extra);
    } else {
      func->outputCPPDynamicInvoke(cg, ar, prefix, name, false, fewArgs,
                                   true, extra);
    }
    cg.indentEnd("}\n");
  }
}

void ClassScope::outputCPPJumpTable(CodeGenerator &cg,
                                    AnalysisResultPtr ar,
                                    bool staticOnly,
                                    bool dynamicObject /* = false */,
                                    bool forEval /* = false */) {

  string scope;
  scope += Option::ClassPrefix;
  scope += getId();
  scope += "::";
  string parent;
  string parentName = m_parent.empty() ? string("ObjectData") : m_parent;
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  bool needGlobals = false;
  if (dynamicObject) {
    if (staticOnly) {
      needGlobals = true;
      parent = string("g->") + Option::ClassStaticsObjectPrefix +
        parentName + "->";
    } else {
      parent = string("parent->");
    }
  } else {
    parent = string(Option::ClassPrefix) + parentName + "::";
  }
  string invokeName;
  invokeName += staticOnly ? Option::ObjectStaticPrefix : Option::ObjectPrefix;
  invokeName += "invoke";

  if (forEval) {
    invokeName += "_from_eval";
  }

  parent += invokeName;
  StringToFunctionScopePtrVecMap funcScopes;
  if (Option::FlattenInvoke) {
    StringToFunctionScopePtrMap fss;
    collectMethods(ar, fss, true, true);
    for (StringToFunctionScopePtrMap::const_iterator it = fss.begin();
         it != fss.end(); ++it) {
      funcScopes[it->first].push_back(it->second);
    }
  }

  vector<const char *> funcs;
  findJumpTableMethods(cg, ar, staticOnly, funcs);

  if (Option::FlattenInvoke) {
    funcs.clear();
    for (StringToFunctionScopePtrVecMap::const_iterator iter =
           funcScopes.begin(); iter != funcScopes.end(); ++iter) {
      FunctionScopePtr func = iter->second[0];
      if (func->isAbstract() || func->inPseudoMain() ||
          (staticOnly && !func->isStatic()) ||
          !(system || func->isDynamic() || func->isVirtual())) continue;
      funcs.push_back(iter->first.c_str());
    }
  }

  if (forEval) {
    if (staticOnly) { // os_invoke
      cg.indentBegin("Variant %s%s"
                     "(const char *c, const char *s, "
                     "Eval::VariableEnvironment &env, "
                     "const Eval::FunctionCallExpression *caller, "
                     "int64 hash, bool fatal) {\n", scope.c_str(),
                     invokeName.c_str());
    } else {
      cg.indentBegin("Variant %s%s"
                     "(const char *s, "
                     "Eval::VariableEnvironment &env, "
                     "const Eval::FunctionCallExpression *caller, "
                     "int64 hash, bool fatal) {\n", scope.c_str(),
                     invokeName.c_str());
    }
  } else {
    // output invoke()
    if (staticOnly) { // os_invoke
      cg.indentBegin("Variant %s%s"
                     "(const char *c, const char *s, CArrRef params,"
                     " int64 hash, bool fatal) {\n", scope.c_str(),
                     invokeName.c_str());
    } else {
      cg.indentBegin("Variant %s%s"
                     "(const char *s, CArrRef params,"
                     " int64 hash, bool fatal) {\n", scope.c_str(),
                     invokeName.c_str());
    }
  }
  if (needGlobals) cg.printDeclareGlobals();
  outputCPPMethodInvokeTable(cg, ar, funcs, funcScopes, false, staticOnly,
                             forEval);
  if (forEval) {
    if (staticOnly) {
      cg.printf("return %s(c, s, env, caller, hash, fatal);\n", parent.c_str());
    } else {
      cg.printf("return %s(s, env, caller, hash, fatal);\n", parent.c_str());
    }
  } else {
    if (staticOnly) {
      cg.printf("return %s(c, s, params, hash, fatal);\n", parent.c_str());
    } else {
      cg.printf("return %s(s, params, hash, fatal);\n", parent.c_str());
    }
  }
  cg.indentEnd("}\n");

  if (!staticOnly && !forEval) {
    cg.indentBegin("Variant %s%s_few_args(const char *s, int64 hash, "
                   "int count",
                   scope.c_str(), invokeName.c_str());
    for (int i = 0; i < Option::InvokeFewArgsCount; i++) {
      cg.printf(", CVarRef a%d", i);
    }
    cg.printf(") {\n");
    if (needGlobals) cg.printDeclareGlobals();
    outputCPPMethodInvokeTable(cg, ar, funcs, funcScopes, true, staticOnly,
                               false);
    cg.printf("return %s_few_args(s, hash, count", parent.c_str());
    for (int i = 0; i < Option::InvokeFewArgsCount; i++) {
      cg.printf(", a%d", i);
    }
    cg.printf(");\n");
    cg.indentEnd("}\n");
  }
}

void ClassScope::outputVolatileCheckBegin(CodeGenerator &cg,
                                          AnalysisResultPtr ar,
                                          const std::string &name) {
  if (isVolatile()) {
    OutputVolatileCheckBegin(cg, ar, name);
  }
}
void ClassScope::outputVolatileCheckEnd(CodeGenerator &cg) {
  if (isVolatile()) {
    OutputVolatileCheckEnd(cg);
  }
}

void ClassScope::OutputVolatileCheckBegin(CodeGenerator &cg,
                                          AnalysisResultPtr ar,
                                          const string &origName) {
  string lwrName(Util::toLower(origName));
  cg.printf("(checkClassExists(String(\"%s\", %d, AttachLiteral), "
            "%s->CDEC(%s)), (", origName.c_str(), origName.size(),
            cg.getGlobals(ar), lwrName.c_str());
}
void ClassScope::OutputVolatileCheckEnd(CodeGenerator &cg) {
  cg.printf("))");
}
