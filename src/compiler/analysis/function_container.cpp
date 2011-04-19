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

bool FunctionContainer::addFunction(AnalysisResultConstPtr ar,
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

void FunctionContainer::outputCPPJumpTableSupportMethod
(CodeGenerator &cg, AnalysisResultPtr ar, FunctionScopePtr func,
 const char *funcPrefix) {
  string name = func->getId(cg);
  const char *cname = name.c_str();
  cg_indentBegin("Variant %s%s(void *extra, CArrRef params) {\n",
      Option::InvokePrefix, cname);
  FunctionScope::OutputCPPDynamicInvokeCount(cg);
  func->outputCPPDynamicInvoke(cg, ar, funcPrefix, cname);
  cg_indentEnd("}\n");

  cg_indentBegin("Variant %s%s(void *extra, int count, "
      "INVOKE_FEW_ARGS_IMPL_ARGS) {\n",
      Option::InvokeFewArgsPrefix, cname);
  func->outputCPPDynamicInvoke(cg, ar, funcPrefix, cname, false,
      true);
  cg_indentEnd("}\n");
}

void FunctionContainer::outputCPPJumpTableSupport
(CodeGenerator &cg, AnalysisResultPtr ar, bool &hasRedeclared,
 vector<const char *> *funcs /* = NULL */) {
  bool systemcpp = cg.getOutput() == CodeGenerator::SystemCPP;
  const char *funcPrefix = Option::FunctionPrefix;
  if (systemcpp) funcPrefix = Option::BuiltinFunctionPrefix;
  // output invoke support methods
  for (FunctionIterator fit(m_functions, hasRedeclared); fit.ready();
      fit.next()) {
    FunctionScopePtr func = fit.get();
    if (func->inPseudoMain() || !(systemcpp || func->isDynamic())) continue;
    if (funcs && fit.firstInner()) {
      funcs->push_back(fit.name().c_str());
    }

    outputCPPJumpTableSupportMethod(cg, ar, func, funcPrefix);

    if (func->isRedeclaring()) hasRedeclared = true;
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
 bool implementation /* = true */,
 const StringToFunctionScopePtrVecMap *in /* = NULL */,
 vector<const char *> *funcs /* = NULL */) {
  if (!in) in = &m_functions;
  bool systemcpp = cg.getOutput() == CodeGenerator::SystemCPP;
  // output invoke support methods
  for (FunctionIterator fit(*in, hasRedeclared); fit.ready();
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
    if (!implementation && func->isRedeclaring()) continue;
    string sname = func->getId(cg);

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

void FunctionContainer::outputGetCallInfoHeader(CodeGenerator &cg,
                                                bool system,
                                                bool needGlobals) {
  cg_indentBegin("bool get_call_info%s(const CallInfo *&ci, void *&extra, "
      "const char *s, int64 hash) {\n", system ? "_builtin" : "");
  if (needGlobals) cg.printDeclareGlobals();
  cg_printf("extra = NULL;\n");

  if (!system && (!Option::DynamicInvokeFunctions.empty() ||
        Option::EnableEval == Option::FullEval)) {
    cg_printf("const char *ss = get_renamed_function(s);\n");
    cg_printf("if (ss != s) { s = ss; hash = -1;};\n");
  }
  if (!system) {
    cg_printf("std::string name; const char *id;\n"
              "if (s[0] == '0' && (id = strchr(s, ':'))) {\n"
              "  name = string(s, id - s); s = name.c_str(); hash = -1;\n"
              "  extra = (void*)String(id + 1).toInt64();\n"
              "}\n"
             );
  }
  if (!system && Option::EnableEval == Option::FullEval) {
    cg_printf("if (eval_get_call_info_hook(ci, extra, s, hash)) "
              "return true;\n");
  }
}

void FunctionContainer::outputGetCallInfoTail(CodeGenerator &cg,
                                              bool system) {
  if (system) {
    cg_printf("return false;\n");
  } else {
    cg_printf("return get_call_info_builtin(ci, extra, s, hash);\n");
  }
  cg_indentEnd("}\n");
}

void FunctionContainer::outputCPPHashTableGetCallInfo(
  CodeGenerator &cg, bool system,
  const StringToFunctionScopePtrVecMap *functions,
  const vector<const char *> &funcs) {
  ASSERT(cg.getCurrentIndentation() == 0);
  const char text1[] =
    "\n"
    "class hashNodeFunc {\n"
    "public:\n"
    "  hashNodeFunc() {}\n"
    "  hashNodeFunc(int64 h, const char *n, bool off, const void *d) :\n"
    "    hash(h), name(n), offset(off), data(d), next(NULL) {}\n"
    "  int64 hash;\n"
    "  const char *name;\n"
    "  bool offset;\n"
    "  const void *data;\n"
    "  hashNodeFunc *next;\n"
    "};\n";
  const char text1s[] =
    "\n"
    "class hashNodeFunc {\n"
    "public:\n"
    "  hashNodeFunc() {}\n"
    "  hashNodeFunc(int64 h, const char *n, const void *d) :\n"
    "    hash(h), name(n), data(d), next(NULL) {}\n"
    "  int64 hash;\n"
    "  const char *name;\n"
    "  const void *data;\n"
    "  hashNodeFunc *next;\n"
    "};\n";
  const char text2[] =
    "static hashNodeFunc *funcMapTable[%d];\n"
    "static hashNodeFunc funcBuckets[%d];\n"
    "\n"
    "static class %sFuncTableInitializer {\n"
    "  public: %sFuncTableInitializer() {\n"
    "    const char *funcMapData[] = {\n";

  const char text3[] =
    "    hashNodeFunc *b = funcBuckets;\n"
    "    for (const char **s = funcMapData; *s; s++, b++) {\n"
    "      const char *name = *s++;\n"
    "%s"
    "      const void *data = *s;\n"
    "      int64 hash = hash_string(name, strlen(name));\n"
    "      hashNodeFunc *node = new(b) hashNodeFunc(hash, name%s, data);\n"
    "      int h = hash & %d;\n"
    "      if (funcMapTable[h]) node->next = funcMapTable[h];\n"
    "      funcMapTable[h] = node;\n"
    "    }\n"
    "  }\n"
    "} func_table_initializer;\n"
    "\n"
    "static inline const hashNodeFunc *"
    "findFunc(const char *name, int64 hash) {\n"
    "  for (const hashNodeFunc *p = funcMapTable[hash & %d]; p; "
    "p = p->next) {\n"
    "    if (p->hash == hash && !strcasecmp(p->name, name)) return p;\n"
    "  }\n"
    "  return NULL;\n"
    "}\n"
    "\n";

  const char text4[] =
    "  if (hash < 0) hash = hash_string(s);\n"
    "  const hashNodeFunc *p = findFunc(s, hash);\n"
    "  if (p) {\n"
    "    if (p->offset) {\n"
    "      const char *addr = (const char *)g + (int64)p->data;\n"
    "      ci = *(const CallInfo **)addr;\n"
    "      return ci != 0;\n"
    "    } else {\n"
    "      ci = (const CallInfo *)p->data;\n"
    "      return true;\n"
    "    }\n"
    "  }\n";

  const char text4s[] =
    "  if (hash < 0) hash = hash_string(s);\n"
    "  const hashNodeFunc *p = findFunc(s, hash);\n"
    "  if (p) {\n"
    "    ci = (const CallInfo *)p->data;\n"
    "    return true;\n"
    "  }\n";

  int numEntries = funcs.size();
  if (numEntries > 0) {
    int tableSize = Util::roundUpToPowerOfTwo(numEntries * 2);
    cg_printf(system ? text1s : text1);
    cg_printf(text2, tableSize, numEntries,
              (system ? "Sys" : ""),
              (system ? "Sys" : ""));
    for (int i = 0; i < numEntries; i++) {
      const char *name = funcs[i];
      StringToFunctionScopePtrVecMap::const_iterator iterFuncs =
        functions->find(name);iterFuncs =
      functions->find(name);
      ASSERT(iterFuncs != functions->end());
      cg_printf("      (const char *)\"%s\", ", cg.escapeLabel(name).c_str());
      if (!system) {
        cg_printf("(const char *)%d, ",
                  iterFuncs->second[0]->isRedeclaring() ? 1 : 0);
      }
      FunctionScopePtr func = iterFuncs->second[0];
      if (func->isRedeclaring()) {
        assert(!system);
        string lname(cg.formatLabel(name));
        cg_printf("(const char *)(offsetof(GlobalVariables, GCI(%s))),\n",
                  lname.c_str());
      } else {
        cg_printf("(const char *)&%s%s,\n",
                  Option::CallInfoPrefix, func->getId(cg).c_str());
      }
    }
    cg_printf("      NULL, NULL, %s\n    };\n", system ? "" : "NULL, ");
    cg_printf(text3, (system ? "" : "      bool offset = *s++;\n"),
              (system ? "" : ", offset"),
              tableSize - 1, tableSize - 1);
  }
  outputGetCallInfoHeader(cg, system, !system);
  cg_indentEnd("");
  if (numEntries > 0) {
    cg_printf(system ? text4s : text4);
  }
  cg_indentBegin("  ");
  outputGetCallInfoTail(cg, system);
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
    if (!func->inPseudoMain() &&
        (system || func->isDynamic() || func->isSepExtension()) &&
        fit.firstInner()) {
      funcs.push_back(fit.name().c_str());
      if (!support && !func->isRedeclaring() && !func->isSepExtension()) {
        cg_printf("extern CallInfo %s%s;\n", Option::CallInfoPrefix,
                  func->getId(cg).c_str());
      }
    }
  }
  if (Option::GenHashTableInvokeFunc && !system) {
    outputCPPHashTableGetCallInfo(cg, system, functions, funcs);
    return;
  }
  outputGetCallInfoHeader(cg, system, needGlobals);

  for (JumpTable fit(cg, funcs, true, true, false); fit.ready(); fit.next()) {
    const char *name = fit.key();
    StringToFunctionScopePtrVecMap::const_iterator iterFuncs =
      functions->find(name);
    ASSERT(iterFuncs != functions->end());
    cg_indentBegin("HASH_GUARD(0x%016llXLL, %s) {\n",
                   hash_string_i(name), cg.escapeLabel(name).c_str());
    if (iterFuncs->second[0]->isRedeclaring()) {
      string lname(cg.formatLabel(name));
      cg_printf("ci = g->GCI(%s);\n", lname.c_str());
    } else {
      cg_printf("ci = &%s%s;\n", Option::CallInfoPrefix,
                iterFuncs->second[0]->getId(cg).c_str());
    }
    cg_printf("return true;\n");
    cg_indentEnd("}\n");
  }
  outputGetCallInfoTail(cg, system);
}

