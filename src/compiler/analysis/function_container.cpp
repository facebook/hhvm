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

#include <compiler/analysis/function_container.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/code_error.h>
#include <compiler/statement/statement_list.h>
#include <compiler/option.h>
#include <util/util.h>
#include <util/hash.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

FunctionContainer::FunctionContainer() {
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

bool FunctionContainer::addFunction(AnalysisResultPtr ar,
                                    FunctionScopePtr funcScope) {
  if (ar->declareFunction(funcScope)) {
    m_functions[funcScope->getName()].push_back(funcScope);
    return true;
  }
  m_ignoredFunctions.push_back(funcScope);
  return false;
}

void FunctionContainer::countReturnTypes(std::map<std::string, int> &counts) {
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    BOOST_FOREACH(FunctionScopePtr f, iter->second) {
      TypePtr type = f->getReturnType();
      if (type) {
        type->count(counts);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void FunctionContainer::outputCPPJumpTableDecl(CodeGenerator &cg,
                                               AnalysisResultPtr ar) {
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    if (iter->second[0]->isRedeclaring()) {
      BOOST_FOREACH(FunctionScopePtr func, iter->second) {
        cg_printf("Variant %s%s(void *extra, CArrRef params);\n",
                  Option::InvokePrefix, func->getId(cg).c_str());
        cg_printf("Variant %s%s(void *extra, int count, "
            "INVOKE_FEW_ARGS_IMPL_ARGS);\n", Option::InvokeFewArgsPrefix,
            func->getId(cg).c_str());
      }
    }
  }
}

class FunctionIterator {
public:
  FunctionIterator(const StringToFunctionScopePtrVecMap &map,
      bool &hasRedec) : m_map(map),
  m_iter(map.begin()), m_hasRedec(hasRedec) {
    if (ready()) {
      m_innerIter = m_iter->second.begin();
      head();
    }
  }
  bool ready() const { return m_iter != m_map.end(); }
  bool firstInner() const { return m_innerIter == m_iter->second.begin(); }
  void next() {
    ++m_innerIter;
    if (m_innerIter == m_iter->second.end()) {
      ++m_iter;
      if (ready()) {
        m_innerIter = m_iter->second.begin();
        head();
      }
    }
  }
  const string &name() const { return m_iter->first; }
  FunctionScopePtr get() const {
    return *m_innerIter;
  }

private:
  const StringToFunctionScopePtrVecMap &m_map;
  StringToFunctionScopePtrVecMap::const_iterator m_iter;
  FunctionScopePtrVec::const_iterator m_innerIter;
  bool &m_hasRedec;
  void head() {
    if (m_iter->second[0]->isRedeclaring()) {
      m_hasRedec = true;
    }
  }
};

void FunctionContainer::outputCPPJumpTableSupport
(CodeGenerator &cg, AnalysisResultPtr ar, bool &hasRedeclared,
 vector<const char *> *funcs /* = NULL */) {
  bool systemcpp = cg.getOutput() == CodeGenerator::SystemCPP;
  bool profile = systemcpp;
  const char *funcPrefix = Option::FunctionPrefix;
  // output invoke support methods
  for (FunctionIterator fit(m_functions, hasRedeclared); fit.ready();
      fit.next()) {
    FunctionScopePtr func = fit.get();
    if (func->inPseudoMain() || !(systemcpp || func->isDynamic())) continue;
    string name = func->getId(cg);
    const char *cname = name.c_str();
    if (funcs && fit.firstInner()) {
      funcs->push_back(fit.name().c_str());
    }

    cg_indentBegin("Variant %s%s(void *extra, CArrRef params) {\n",
        Option::InvokePrefix, cname);
    if (profile) {
      cg_printf("FUNCTION_INJECTION(%s);\n", cname);
    }
    FunctionScope::OutputCPPDynamicInvokeCount(cg);
    func->outputCPPDynamicInvoke(cg, ar, funcPrefix, cname);
    cg_indentEnd("}\n");

    cg_indentBegin("Variant %s%s(void *extra, int count, "
        "INVOKE_FEW_ARGS_IMPL_ARGS) {\n",
        Option::InvokeFewArgsPrefix, cname);
    func->outputCPPDynamicInvoke(cg, ar, funcPrefix, cname, false,
        true);
    cg_indentEnd("}\n");

    if (func->isRedeclaring()) {
      hasRedeclared = true;
      if (func->getRedeclaringId() == 0) {
        cg_indentBegin("Variant %s%s(void *extra, CArrRef params) {\n",
            Option::InvokePrefix, func->getOriginalName().c_str());
        cg_printf("DECLARE_GLOBAL_VARIABLES(g);\n");
        cg_printf("return g->%s%s(extra, params);\n",
            Option::InvokePrefix, func->getName().c_str());
        cg_indentEnd("}\n");
      }
    }
  }
}

void FunctionContainer::outputCPPCallInfoTableSupport
(CodeGenerator &cg, AnalysisResultPtr ar, bool &hasRedeclared,
 vector<const char *> *funcs /* = NULL */) {
  bool systemcpp = cg.getOutput() == CodeGenerator::SystemCPP;
  for (FunctionIterator fit(m_functions, hasRedeclared); fit.ready();
      fit.next()) {
    FunctionScopePtr func = fit.get();
    if (func->inPseudoMain() || !(systemcpp || func->isDynamic())) continue;
    if (funcs && fit.firstInner()) {
      funcs->push_back(fit.name().c_str());
    }
    func->outputCPPCallInfo(cg, ar);
  }
}
void FunctionContainer::outputCPPJumpTableEvalSupport
(CodeGenerator &cg, AnalysisResultPtr ar, bool &hasRedeclared,
 vector<const char *> *funcs /* = NULL */) {
  bool systemcpp = cg.getOutput() == CodeGenerator::SystemCPP;
  const char *funcPrefix = Option::FunctionPrefix;
  if (systemcpp) funcPrefix = Option::BuiltinFunctionPrefix;
  // output invoke support methods
  for (FunctionIterator fit(m_functions, hasRedeclared); fit.ready();
      fit.next()) {
    FunctionScopePtr func = fit.get();
    if (func->inPseudoMain() ||
        (!systemcpp && (!func->isUserFunction() ||
                        !(func->isSepExtension() || func->isDynamic())))) {
      continue;
    }
    if (funcs && fit.firstInner()) {
      funcs->push_back(fit.name().c_str());
    }
    string sname = func->getId(cg);

    cg_indentBegin("Variant %s%s(Eval::VariableEnvironment &env, "
        "const Eval::FunctionCallExpression *caller) {\n",
        Option::EvalInvokePrefix, func->getId(cg).c_str());
    func->outputCPPEvalInvoke(cg, ar, funcPrefix, func->getId(cg).c_str());
    cg_indentEnd("}\n");

    if (func->isRedeclaring()) {
      hasRedeclared = true;
    }
  }
}

void FunctionContainer::outputCPPJumpTable(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;

  vector<const char *> funcs;
  bool needGlobals = false;
  outputCPPJumpTableSupport(cg, ar, needGlobals, &funcs);

  // output invoke()
  cg_indentBegin("Variant invoke_old%s"
                 "(const char *s, CArrRef params, int64 hash, bool fatal) {\n",
                 system ? "_builtin" : "");
  if (needGlobals) cg.printDeclareGlobals();

  for (JumpTable fit(cg, funcs, true, true, false); fit.ready(); fit.next()) {
    const char *name = fit.key();
    StringToFunctionScopePtrVecMap::const_iterator iterFuncs =
      m_functions.find(name);
    ASSERT(iterFuncs != m_functions.end());
    FunctionScopePtr func = iterFuncs->second[0];
    if (func->isRedeclaring()) {
      cg_printf("HASH_INVOKE_REDECLARED(0x%016llXLL, %s);\n",
                hash_string_i(name), cg.formatLabel(name).c_str());
    } else {
      cg_printf("HASH_INVOKE(0x%016llXLL, %s);\n",
                hash_string_i(name), func->getId(cg).c_str());
    }
  }

  cg_printf("return invoke_failed(s, params, hash, fatal);\n");
  cg_indentEnd("}\n");
}

void FunctionContainer::outputCPPCodeInfoTable(CodeGenerator &cg,
    AnalysisResultPtr ar, bool support,
    const StringToFunctionScopePtrVecMap *functions /* = NULL */) {
  if (!functions) functions = &m_functions;
  bool needGlobals = false;
  vector<const char *> funcs;
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  if (support) {
    outputCPPCallInfoTableSupport(cg, ar, needGlobals, NULL);
  }
  for (FunctionIterator fit(*functions, needGlobals); fit.ready();
      fit.next()) {
    FunctionScopePtr func = fit.get();
    if (!func->inPseudoMain() && (system || func->isDynamic()) &&
        fit.firstInner()) {
      funcs.push_back(fit.name().c_str());
      if (!support) {
        cg_printf("extern CallInfo %s%s;\n", Option::CallInfoPrefix,
        func->getId(cg).c_str());
      }
    }
  }
  cg.indentBegin("bool get_call_info%s(const CallInfo *&ci, void *&extra, "
      "const char *s, int64 hash)"
      " {\n",
      system ? "_builtin" : "");
  if (needGlobals) cg.printDeclareGlobals();
  cg_printf("extra = NULL;\n");

  if (!system && (!Option::DynamicInvokeFunctions.empty() ||
        Option::EnableEval == Option::FullEval)) {
    cg_printf("const char *ss = get_renamed_function(s);\n");
    cg_printf("if (ss != s) { s = ss; hash = -1;};\n");
  }
  if (!system && Option::EnableEval == Option::FullEval) {
    cg_printf("if (eval_get_call_info_hook(ci, extra, s, hash)) "
        "return true;\n");
  }

  for (JumpTable fit(cg, funcs, true, true, false); fit.ready(); fit.next()) {
    const char *name = fit.key();
    StringToFunctionScopePtrVecMap::const_iterator iterFuncs =
      functions->find(name);
    ASSERT(iterFuncs != functions->end());
    cg.indentBegin("HASH_GUARD(0x%016llXLL, %s) {\n",
                hash_string_i(name), name);
    if (iterFuncs->second[0]->isRedeclaring()) {
      string lname(cg.formatLabel(name));
      cg_printf("ci = g->%s%s;\n", Option::CallInfoPrefix, lname.c_str());
    } else {
      cg_printf("ci = &%s%s;\n", Option::CallInfoPrefix,
          iterFuncs->second[0]->getId(cg).c_str());
    }
    cg_printf("return true;\n");
    cg.indentEnd("}\n");
  }
  if (system) {
    cg_printf("return false;\n");
  } else {
    cg_printf("return get_call_info_builtin(ci, extra, s, hash);\n");
  }
  cg.indentEnd("}\n");
}

void FunctionContainer::outputCPPEvalInvokeTable(CodeGenerator &cg,
                                                 AnalysisResultPtr ar) {
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;
  bool generate = Option::EnableEval >= Option::LimitedEval || system;
  vector<const char *> funcs;
  bool needGlobals = false;
  if (generate) outputCPPJumpTableEvalSupport(cg, ar, needGlobals, &funcs);
  // output invoke()
  cg_indentBegin("Variant Eval::invoke_from_eval%s"
                 "(const char *s, Eval::VariableEnvironment &env, "
                 "const Eval::FunctionCallExpression *caller, int64 hash, "
                 "bool fatal) {\n",
                 system ? "_builtin" : "");
  if (generate) {
    if (needGlobals) cg.printDeclareGlobals();

    // builtin functions (not methods)
    for (JumpTable fit(cg, funcs, true, true, false); fit.ready();
         fit.next()) {
      const char *name = fit.key();
      StringToFunctionScopePtrVecMap::const_iterator iterFuncs =
        m_functions.find(name);
      ASSERT(iterFuncs != m_functions.end());
      if (iterFuncs->second[0]->isRedeclaring()) {
        cg_printf("HASH_INVOKE_REDECLARED_FROM_EVAL(0x%016llXLL, %s);\n",
                  hash_string_i(name), cg.formatLabel(name).c_str());
      } else {
        cg_printf("HASH_INVOKE_FROM_EVAL(0x%016llXLL, %s);\n",
                  hash_string_i(name), cg.formatLabel(name).c_str());
      }
    }
  }
  if (system) {
    cg_printf("return invoke_failed(s, eval_get_params(env, caller),"
              " -1, fatal);\n");
  } else {
    cg_printf("return invoke_from_eval_builtin(s, env, caller, hash, "
              "fatal);\n");
  }
  cg_indentEnd("}\n");
}
