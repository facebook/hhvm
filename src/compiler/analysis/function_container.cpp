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

void FunctionContainer::addFunction(AnalysisResultPtr ar,
                                    FunctionScopePtr funcScope) {
  if (ar->declareFunction(funcScope)) {
    m_functions[funcScope->getName()].push_back(funcScope);
  } else {
    m_ignoredFunctions.push_back(funcScope);
    funcScope->setIgnored();
  }
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
        cg.printf("Variant %s%s(CArrRef params);\n",
                  Option::InvokePrefix, func->getId().c_str());
        cg.printf("Variant %s%s_few_args(int count",
                  Option::InvokePrefix, func->getId().c_str());
        for (int i = 0; i < Option::InvokeFewArgsCount; i++) {
          cg.printf(", CVarRef a%d", i);
        }
        cg.printf(");\n");
      }
    }
  }
}

void FunctionContainer::outputCPPJumpTableSupport
(CodeGenerator &cg, AnalysisResultPtr ar, bool &hasRedeclared,
 vector<const char *> *funcs /* = NULL */) {
  bool systemcpp = cg.getOutput() == CodeGenerator::SystemCPP;
  bool profile = systemcpp;
  const char *funcPrefix = Option::FunctionPrefix;
  // output invoke support methods
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    if (!iter->second[0]->isRedeclaring()) {
      FunctionScopePtr func = iter->second[0];
      if (func->inPseudoMain() || !(systemcpp || func->isDynamic())) continue;
      const char *name = iter->first.c_str();
      if (funcs) funcs->push_back(name);

      if (!systemcpp) {
        vector<const char *> &bucket = ar->getFuncTableBucket(func);
        if (bucket.size() == 1) {
          // no conflict in the function table
          cg.indentBegin("Variant %s%s(const char *s, CArrRef params, "
                         "int64 hash, bool fatal) {\n",
                         Option::InvokePrefix, name);
          cg.indentBegin("HASH_GUARD(0x%016llXLL, %s) {\n",
                          hash_string_i(name), name);
          FunctionScope::OutputCPPDynamicInvokeCount(cg);
          func->outputCPPDynamicInvoke(cg, ar, funcPrefix, name);
          cg.indentEnd("}\n");
          cg.printf("return invoke_builtin(s, params, hash, fatal);\n");
          cg.indentEnd("}\n");
          continue;
        }
      }
      cg.indentBegin("Variant %s%s(CArrRef params) {\n",
                     Option::InvokePrefix, name);
      if (profile) {
        cg.printf("FUNCTION_INJECTION(%s);\n", name);
      }
      FunctionScope::OutputCPPDynamicInvokeCount(cg);
      func->outputCPPDynamicInvoke(cg, ar, funcPrefix, name);
      cg.indentEnd("}\n");
    } else {
      hasRedeclared = true;
      if (funcs) funcs->push_back(iter->first.c_str());
      BOOST_FOREACH(FunctionScopePtr func, iter->second) {
        string prefix = Option::InvokePrefix;
        string name = func->getId();
        cg.indentBegin("Variant %s%s(CArrRef params) {\n",
                       prefix.c_str(), name.c_str());
        FunctionScope::OutputCPPDynamicInvokeCount(cg);
        func->outputCPPDynamicInvoke(cg, ar, funcPrefix, name.c_str());
        cg.indentEnd("}\n");

        cg.indentBegin("Variant %s%s_few_args(int count",
                       prefix.c_str(), name.c_str());
        for (int i = 0; i < Option::InvokeFewArgsCount; i++) {
          cg.printf(", CVarRef a%d", i);
        }
        cg.printf(") {\n");
        func->outputCPPDynamicInvoke(cg, ar, funcPrefix, name.c_str(), false,
                                     true);
        cg.indentEnd("}\n");

        if (func->getRedeclaringId() == 0) {
          cg.indentBegin("Variant %s%s(CArrRef params) {\n",
                         prefix.c_str(), func->getName().c_str());
          cg.printf("DECLARE_GLOBAL_VARIABLES(g);\n");
          cg.printf("return g->%s%s(params);\n",
                    prefix.c_str(), func->getName().c_str());
          cg.indentEnd("}\n");
        }
      }
    }
  }
}

void FunctionContainer::outputCPPJumpTableEvalSupport
(CodeGenerator &cg, AnalysisResultPtr ar, bool &hasRedeclared,
 vector<const char *> *funcs /* = NULL */) {
  bool systemcpp = cg.getOutput() == CodeGenerator::SystemCPP;
  const char *funcPrefix = Option::FunctionPrefix;
  if (systemcpp) funcPrefix = Option::BuiltinFunctionPrefix;
  // output invoke support methods
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    if (!iter->second[0]->isRedeclaring()) {
      FunctionScopePtr func = iter->second[0];
      if (func->inPseudoMain() || !(systemcpp || func->isSepExtension() ||
            func->isDynamic())) continue;
      const char *name = iter->first.c_str();
      if (funcs) funcs->push_back(name);

      cg.indentBegin("Variant %s%s(Eval::VariableEnvironment &env, "
                     "const Eval::FunctionCallExpression *caller) {\n",
                     Option::EvalInvokePrefix, name);
      func->outputCPPEvalInvoke(cg, ar, funcPrefix, name);
      cg.indentEnd("}\n");
    } else {
      hasRedeclared = true;
      if (funcs) funcs->push_back(iter->first.c_str());
      BOOST_FOREACH(FunctionScopePtr func, iter->second) {
        string prefix = Option::EvalInvokePrefix;
        string name = func->getId();
        cg.indentBegin("Variant %s%s(CArrRef params) {\n",
                       Option::EvalInvokePrefix, name.c_str());
        func->outputCPPEvalInvoke(cg, ar, funcPrefix, name.c_str());
        cg.indentEnd("}\n");
      }
    }
  }
}

void FunctionContainer::outputCPPJumpTable(CodeGenerator &cg,
                                           AnalysisResultPtr ar) {
  string parent = "invoke_failed";
  bool system = cg.getOutput() == CodeGenerator::SystemCPP;

  vector<const char *> funcs;
  bool needGlobals = false;
  outputCPPJumpTableSupport(cg, ar, needGlobals, &funcs);

  // output invoke()
  cg.indentBegin("Variant invoke%s"
                 "(const char *s, CArrRef params, int64 hash, bool fatal) {\n",
                 system ? "_builtin" : "");
  if (needGlobals) cg.printDeclareGlobals();

  for (JumpTable fit(cg, funcs, true, true, false); fit.ready(); fit.next()) {
    const char *name = fit.key();
    StringToFunctionScopePtrVecMap::const_iterator iterFuncs =
      m_functions.find(name);
    ASSERT(iterFuncs != m_functions.end());
    if (iterFuncs->second[0]->isRedeclaring()) {
      cg.printf("HASH_INVOKE_REDECLARED(0x%016llXLL, %s);\n",
                hash_string_i(name), name);
    } else {
      cg.printf("HASH_INVOKE(0x%016llXLL, %s);\n",
                hash_string_i(name), name);
    }
  }

  cg.printf("return %s(s, params, hash, fatal);\n", parent.c_str());
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
  cg.indentBegin("Variant Eval::invoke_from_eval%s"
                 "(const char *s, Eval::VariableEnvironment &env, "
                 "const Eval::FunctionCallExpression *caller, int64 hash, "
                 "bool fatal) {\n",
                 system ? "_builtin" : "");
  if (generate) {
    if (needGlobals) cg.printDeclareGlobals();

    for (JumpTable fit(cg, funcs, true, true, false); fit.ready();
         fit.next()) {
      const char *name = fit.key();
      StringToFunctionScopePtrVecMap::const_iterator iterFuncs =
        m_functions.find(name);
      ASSERT(iterFuncs != m_functions.end());
      if (iterFuncs->second[0]->isRedeclaring()) {
        cg.printf("HASH_INVOKE_REDECLARED_FROM_EVAL(0x%016llXLL, %s);\n",
                  hash_string_i(name), name);
      } else {
        cg.printf("HASH_INVOKE_FROM_EVAL(0x%016llXLL, %s);\n",
                  hash_string_i(name), name);
      }
    }
  }
  if (system) {
    cg.printf("return invoke_failed(s, Array(), -1, fatal);\n");
  } else {
    cg.printf("return invoke_from_eval_builtin(s, env, caller, hash, "
              "fatal);\n");
  }
  cg.indentEnd("}\n");
}
