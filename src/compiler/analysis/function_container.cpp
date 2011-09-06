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
#include <compiler/analysis/file_scope.h>
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

void
FunctionContainer::getFunctionsFlattened(FunctionScopePtrVec &funcs,
                                         bool excludePseudoMains /* = false */)
const {
  for (StringToFunctionScopePtrVecMap::const_iterator it = m_functions.begin();
       it != m_functions.end(); ++it) {
    BOOST_FOREACH(FunctionScopePtr func, it->second) {
      if (!excludePseudoMains || !func->inPseudoMain()) {
        funcs.push_back(func);
      }
    }
  }
}

void FunctionContainer::outputCPPJumpTableDecl(CodeGenerator &cg,
                                               AnalysisResultPtr ar) {
  for (StringToFunctionScopePtrVecMap::const_iterator iter =
         m_functions.begin(); iter != m_functions.end(); ++iter) {
    if (iter->second[0]->isRedeclaring()) {
      BOOST_FOREACH(FunctionScopePtr func, iter->second) {
        cg_printf("Variant %s%s(void *extra, CArrRef params);\n",
                  Option::InvokePrefix, func->getId().c_str());
        cg_printf("Variant %s%s(void *extra, int count, "
                  "INVOKE_FEW_ARGS_IMPL_ARGS);\n",
                  Option::InvokeFewArgsPrefix, func->getId().c_str());
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
  string name = func->getId();
  const char *cname = name.c_str();

  string origName = !func->inPseudoMain() ? func->getOriginalName() :
                    ("run_init::" + func->getContainingFile()->getName());
  cg_printf("Variant");
  if (Option::FunctionSections.find(origName) !=
      Option::FunctionSections.end()) {
    string funcSection = Option::FunctionSections[origName];
    if (!funcSection.empty()) {
      cg_printf(" __attribute__ ((section (\".text.%s\")))",
          funcSection.c_str());
    }
  }
  cg_indentBegin(" %s%s(void *extra, int count, "
                 "INVOKE_FEW_ARGS_IMPL_ARGS) {\n",
                 Option::InvokeFewArgsPrefix, cname);
  func->outputCPPDynamicInvoke(cg, ar, funcPrefix, cname, false, true);
  cg_indentEnd("}\n");

  cg_indentBegin("Variant %s%s(void *extra, CArrRef params) {\n",
                 Option::InvokePrefix, cname);
  if (func->getMaxParamCount() <= Option::InvokeFewArgsCount &&
      !func->isVariableArgument()) {
    if (Option::InvokeWithSpecificArgs && !func->getMaxParamCount() &&
        !ar->isSystem() && !ar->isSepExtension()) {
      // For functions with no parameter, we can combine the i_ wrapper and
      // the ifa_ wrapper.
      cg_printf("return ((CallInfo::FuncInvoker0Args)&%s%s)(extra, 0);\n",
                Option::InvokeFewArgsPrefix, cname);
    } else {
      cg_printf("return invoke_func_few_handler(extra, params, &%s%s);\n",
                Option::InvokeFewArgsPrefix, cname);
    }
  } else {
    FunctionScope::OutputCPPDynamicInvokeCount(cg);
    func->outputCPPDynamicInvoke(cg, ar, funcPrefix, cname);
  }
  cg_indentEnd("}\n");
}

void
FunctionContainer::outputCPPHelperClassAllocSupport(CodeGenerator &cg,
                                                    AnalysisResultPtr ar) {
  bool hasRedeclared;
  for (FunctionIterator fit(m_functions, hasRedeclared); fit.ready();
      fit.next()) {
    FunctionScopePtr func = fit.get();
    func->outputCPPHelperClassAlloc(cg, ar);
  }
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
    string sname = func->getId();

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
                hash_string_i(name), CodeGenerator::FormatLabel(name).c_str());
    } else {
      cg_printf("HASH_INVOKE(0x%016llXLL, %s);\n",
                hash_string_i(name), func->getId().c_str());
    }
  }

  cg_printf("return invoke_failed(s, params, hash, fatal);\n");
  cg_indentEnd("}\n");
}

void FunctionContainer::outputGetCallInfoHeader(CodeGenerator &cg,
                                                const char *suffix,
                                                bool needGlobals) {
  cg_indentBegin("bool get_call_info%s(const CallInfo *&ci, void *&extra, "
      "const char *s, int64 hash) {\n", suffix ? suffix : "");

  if (needGlobals) cg.printDeclareGlobals();
  cg_printf("extra = NULL;\n");

  if (!suffix && (!Option::DynamicInvokeFunctions.empty() ||
                  Option::EnableEval == Option::FullEval)) {
    cg_printf("const char *ss = get_renamed_function(s);\n");
    cg_printf("if (ss != s) { s = ss; hash = -1;};\n");
  }
  if (!suffix && Option::EnableEval == Option::FullEval) {
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
  CodeGenerator &cg, bool system, bool noEval,
  const StringToFunctionScopePtrVecMap *functions,
  const vector<const char *> &funcs) {
  ASSERT(cg.getCurrentIndentation() == 0);
  const char text1[] =
    "\n"
    "struct hashNodeFunc {\n"
    "  int64 hash;\n"
    "  bool offset;\n"
    "  bool end;\n"
    "  const char *name;\n"
    "  const void *data;\n"
    "};\n"
    "static const hashNodeFunc funcBuckets[] = {\n";

  const char text3[] =
    "static inline const hashNodeFunc *"
    "findFunc(const char *name, int64 hash) {\n"
    "  const hashNodeFunc *p = funcMapTable[hash & %d];\n"
    "  if (UNLIKELY(!p)) return NULL;\n"
    "  do {\n"
    "    if (LIKELY(p->hash == hash) && (LIKELY(p->name==name)||"
    "LIKELY(!strcasecmp(p->name, name)))) return p;\n"
    "  } while (!p++->end);\n"
    "  return NULL;\n"
    "}\n"
    "\n";

  const char text4[] =
    "  if (hash < 0) hash = hash_string(s);\n"
    "  const hashNodeFunc *p = findFunc(s, hash);\n"
    "  if (LIKELY(p!=0)) {\n"
    "    if (UNLIKELY(p->offset)) {\n"
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
    "  if (LIKELY(p!=0)) {\n"
    "    ci = (const CallInfo *)p->data;\n"
    "    return true;\n"
    "  }\n";

  int numEntries = funcs.size();
  if (!noEval && numEntries > 0) {
    JumpTable jt(cg, funcs, true, true, true, true);
    cg_printf(text1);

    vector<int> offsets;
    int prev = -1;
    for (int n = 0; jt.ready(); ++n, jt.next()) {
      int cur = jt.current();
      if (prev != cur) {
        while (++prev != cur) {
          offsets.push_back(-1);
        }
        offsets.push_back(n);
      }
      const char *name = jt.key();

      StringToFunctionScopePtrVecMap::const_iterator iterFuncs =
        functions->find(name);iterFuncs =
      functions->find(name);
      ASSERT(iterFuncs != functions->end());
      // We have assumptions that function names do not contain ".."
      // (e.g., call_user_func0 ~ call_user_func6)
      if (strstr(name, "..")) assert(false);
      FunctionScopePtr func = iterFuncs->second[0];
      cg_printf(" {0x%016llXLL,%d,%d,\"%s\",",
                hash_string_i(name),
                (int)func->isRedeclaring(), (int)jt.last(),
                CodeGenerator::EscapeLabel(name).c_str());

      if (func->isRedeclaring()) {
        assert(!system);
        string lname(CodeGenerator::FormatLabel(name));
        cg_printf("(const void *)(offsetof(GlobalVariables, GCI(%s)))",
                  lname.c_str());
      } else {
        cg_printf("&%s%s",
                  Option::CallInfoPrefix, func->getId().c_str());
      }
      cg_printf("},\n");
    }
    cg_printf("};\n");
    cg_indentBegin("static const hashNodeFunc *funcMapTable[] = {\n");
    for (int i = 0, e = jt.size(), s = offsets.size(); i < e; i++) {
      int o = i < s ? offsets[i] : -1;
      if (o < 0) {
        cg_printf("0,");
      } else {
        cg_printf("funcBuckets+%d,", o);
      }
      if ((i & 7) == 7) cg_printf("\n");
    }
    cg_printf("\n");
    cg_indentEnd("};\n");
    cg_printf(text3, jt.size() - 1);
  }
  outputGetCallInfoHeader(cg, system ? "_builtin" : noEval ? "_no_eval" : 0,
                          !system);
  cg_indentEnd("");
  if (numEntries > 0) {
    cg_printf(system ? text4s : text4);
  }
  cg_indentBegin("  ");
  outputGetCallInfoTail(cg, system);
}

void FunctionContainer::outputCPPCodeInfoTable(
  CodeGenerator &cg, AnalysisResultPtr ar, bool support,
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
                  func->getId().c_str());
      }
    }
  }
  if (!system) {
    outputCPPHashTableGetCallInfo(cg, system, false, functions, funcs);
    if (!system) {
      outputCPPHashTableGetCallInfo(cg, system, true, functions, funcs);
    }
    return;
  }
  if (!system) cg_printf("static ");
  outputGetCallInfoHeader(cg, system ? "_builtin" : "_impl", needGlobals);

  for (JumpTable fit(cg, funcs, true, true, false); fit.ready(); fit.next()) {
    const char *name = fit.key();
    StringToFunctionScopePtrVecMap::const_iterator iterFuncs =
      functions->find(name);
    ASSERT(iterFuncs != functions->end());
    cg_indentBegin("HASH_GUARD(0x%016llXLL, %s) {\n",
                   hash_string_i(name),
                   CodeGenerator::EscapeLabel(name).c_str());
    if (iterFuncs->second[0]->isRedeclaring()) {
      string lname(CodeGenerator::FormatLabel(name));
      cg_printf("ci = g->GCI(%s);\n", lname.c_str());
    } else {
      cg_printf("ci = &%s%s;\n", Option::CallInfoPrefix,
                iterFuncs->second[0]->getId().c_str());
    }
    cg_printf("return true;\n");
    cg_indentEnd("}\n");
  }
  outputGetCallInfoTail(cg, system);

  if (system) return;

  outputGetCallInfoHeader(cg, 0, false);
  cg_printf("return get_call_info_impl(ci, extra, s, hash);\n");
  cg_indentEnd("}\n");

  outputGetCallInfoHeader(cg, "_no_eval", false);
  cg_printf("return get_call_info_impl(ci, extra, s, hash);\n");
  cg_indentEnd("}\n");
}

