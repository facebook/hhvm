/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/compiler/analysis/function_scope.h"
#include "hphp/compiler/analysis/class_scope.h"
#include "hphp/compiler/expression/modifier_expression.h"
#include "hphp/compiler/expression/simple_function_call.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/analysis/file_scope.h"
#include "hphp/compiler/analysis/variable_table.h"
#include "hphp/compiler/analysis/constant_table.h"
#include "hphp/parser/hphp.tab.hpp"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/util/logger.h"
#include <vector>

using namespace HPHP;

#define BF_COLUMN_COUNT  3
#define BF_COLUMN_NAME   0
#define BF_COLUMN_RETURN 1
#define BF_COLUMN_PARAMS 2

#define CLASS_TYPE 999

///////////////////////////////////////////////////////////////////////////////

bool BuiltinSymbols::Loaded = false;
AnalysisResultPtr BuiltinSymbols::s_systemAr;

const char *const BuiltinSymbols::GlobalNames[] = {
  "HTTP_RAW_POST_DATA",
  "_COOKIE",
  "_ENV",
  "_FILES",
  "_GET",
  "_POST",
  "_REQUEST",
  "_SERVER",
  "_SESSION",
  "argc",
  "argv",
};

hphp_string_set BuiltinSymbols::s_superGlobals;

///////////////////////////////////////////////////////////////////////////////

int BuiltinSymbols::NumGlobalNames() {
  return sizeof(BuiltinSymbols::GlobalNames) /
    sizeof(BuiltinSymbols::GlobalNames[0]);
}

void BuiltinSymbols::ImportNativeConstants(AnalysisResultPtr ar,
                                           ConstantTablePtr dest) {
  for (auto cnsPair : Native::getConstants()) {
    ExpressionPtr e(Expression::MakeScalarExpression(
                      ar, ar, Location::Range(), tvAsVariant(&cnsPair.second)));

    dest->add(cnsPair.first->data(), e, ar, e);

    if ((cnsPair.second.m_type == KindOfUninit) &&
         cnsPair.second.m_data.pref) {
      // Callback based constant
      dest->setDynamic(ar, cnsPair.first->data());
    }
  }
}

bool BuiltinSymbols::Load(AnalysisResultPtr ar) {
  if (Loaded) return true;
  Loaded = true;

  if (g_context.isNull()) hphp_thread_init();

  ConstantTablePtr cns = ar->getConstants();
  // load extension constants, classes and dynamics
  ImportNativeConstants(ar, cns);

  for (int i = 0, n = NumGlobalNames(); i < n; ++i) {
    ar->getVariables()->add(GlobalNames[i], false, ar,
                            ConstructPtr(), ModifierExpressionPtr());
  }

  cns->setDynamic(ar, "PHP_BINARY");
  cns->setDynamic(ar, "PHP_BINDIR");
  cns->setDynamic(ar, "PHP_OS");
  cns->setDynamic(ar, "PHP_SAPI");
  cns->setDynamic(ar, "SID");

  for (auto sym : cns->getSymbols()) {
    sym->setSystem();
  }

  // Systemlib files were all parsed by hphp_process_init

  const auto& files = ar->getAllFiles();
  for (const auto& file : files) {
    file.second->setSystem();

    const auto& classes = file.second->getClasses();
    for (const auto& clsVec : classes) {
      assert(clsVec.second.size() == 1);
      auto cls = clsVec.second[0];
      if (auto nativeConsts = Native::getClassConstants(
            String(cls->getScopeName()).get())) {
        for (auto cnsMap : *nativeConsts) {
          auto tv = cnsMap.second;
          auto e = Expression::MakeScalarExpression(ar, ar, Location::Range(),
                                                    tvAsVariant(&tv));
          cls->getConstants()->add(cnsMap.first->data(), e, ar, e);
        }
      }
      cls->setSystem();
      ar->addSystemClass(cls);
    }

    const auto& functions = file.second->getFunctions();
    for (const auto& func : functions) {
      func.second->setSystem();
      ar->addSystemFunction(func.second);
    }
  }

  return true;
}

void BuiltinSymbols::LoadSuperGlobals() {
  if (s_superGlobals.empty()) {
    s_superGlobals.insert("_SERVER");
    s_superGlobals.insert("_GET");
    s_superGlobals.insert("_POST");
    s_superGlobals.insert("_COOKIE");
    s_superGlobals.insert("_FILES");
    s_superGlobals.insert("_ENV");
    s_superGlobals.insert("_REQUEST");
    s_superGlobals.insert("_SESSION");
  }
}

bool BuiltinSymbols::IsSuperGlobal(const std::string &name) {
  return s_superGlobals.count(name);
}
