/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/debugger/debugger_hook_handler.h"

namespace HPHP { namespace Eval {

//////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);

// See if the given name matches the function's name.
static bool matchFunctionName(std::string name, const Func* f) {
  return name == f->name()->data();
}

static void addBreakPointInUnit(BreakPointInfoPtr bp, Unit* unit) {
  OffsetRangeVec offsets;
  if (!unit->getOffsetRanges(bp->m_line1, offsets) || offsets.size() == 0) {
    bp->m_bindState = BreakPointInfo::KnownToBeInvalid;
    return;
  }
  bp->m_bindState = BreakPointInfo::KnownToBeValid;
  TRACE(3, "Add to breakpoint filter for %s:%d, unit %p:\n",
      unit->filepath()->data(), bp->m_line1, unit);
  phpAddBreakPointRange(unit, offsets);
}

void proxySetBreakPoints(DebuggerProxy* proxy) {
  std::vector<BreakPointInfoPtr> bps;
  proxy->getBreakPoints(bps);
  for (unsigned int i = 0; i < bps.size(); i++) {
    BreakPointInfoPtr bp = bps[i];
    bp->m_bindState = BreakPointInfo::Unknown;
    auto className = bp->getClass();
    if (!className.empty()) {
      auto clsName = makeStaticString(className);
      auto cls = Unit::lookupClass(clsName);
      if (cls == nullptr) continue;
      bp->m_bindState = BreakPointInfo::KnownToBeInvalid;
      size_t numFuncs = cls->numMethods();
      if (numFuncs == 0) continue;
      auto methodName = bp->getFunction();
      for (size_t i = 0; i < numFuncs; ++i) {
        auto f = cls->getMethod(i);
        if (!matchFunctionName(methodName, f)) continue;
        bp->m_bindState = BreakPointInfo::KnownToBeValid;
        phpAddBreakPointFuncEntry(f);
        break;
      }
      // TODO(#2527229): what about superclass methods accessed via the derived
      // class?
      continue;
    }
    auto funcName = bp->getFuncName();
    if (!funcName.empty()) {
      auto fName = makeStaticString(funcName);
      Func* f = Unit::lookupFunc(fName);
      if (f == nullptr) continue;
      bp->m_bindState = BreakPointInfo::KnownToBeValid;
      phpAddBreakPointFuncEntry(f);
      continue;
    }
    auto fileName = bp->m_file;
    if (!fileName.empty()) {
      for (auto& kv : g_context->m_evaledFiles) {
        auto const unit = kv.second;
        if (!BreakPointInfo::MatchFile(fileName,
                            unit->filepath()->toCppString())) {
          continue;
        }
        addBreakPointInUnit(bp, unit);
        break;
      }
      continue;
    }
    auto exceptionClassName = bp->getExceptionClass();
    if (exceptionClassName == "@") {
      bp->m_bindState = BreakPointInfo::KnownToBeValid;
      continue;
    } else if (!exceptionClassName.empty()) {
      auto expClsName = makeStaticString(exceptionClassName);
      auto cls = Unit::lookupClass(expClsName);
      if (cls != nullptr) {
        auto baseClsName = makeStaticString("Exception");
        auto baseCls = Unit::lookupClass(baseClsName);
        if (baseCls != nullptr) {
          if (cls->classof(baseCls)) {
            bp->m_bindState = BreakPointInfo::KnownToBeValid;
          } else {
            bp->m_bindState = BreakPointInfo::KnownToBeInvalid;
          }
        }
      }
      continue;
    } else {
      continue;
    }
    // If we get here, the break point is of a type that does
    // not need to be explicitly enabled in the VM. For example
    // a break point that get's triggered when the server starts
    // to process a page request.
    bp->m_bindState = BreakPointInfo::KnownToBeValid;
  }
}

DebuggerHook* HphpdHook::GetInstance() {
  static DebuggerHook* instance = new HphpdHook();
  return instance;
}

void HphpdHook::onFileLoad(Unit* unit) {
  DebuggerProxy* proxy = Debugger::GetProxy().get();
  if (proxy == nullptr) return;

  // Look up the proxy's breakpoints and add needed breakpoints to the passed
  // unit
  std::vector<BreakPointInfoPtr> bps;
  proxy->getBreakPoints(bps);
  for (unsigned int i = 0; i < bps.size(); i++) {
    BreakPointInfoPtr bp = bps[i];
    if (BreakPointInfo::MatchFile(bp->m_file,
                                        unit->filepath()->toCppString())) {
      addBreakPointInUnit(bp, unit);
    }
  }
}

void HphpdHook::onDefClass(const Class* cls) {
  // Make sure we have a proxy
  DebuggerProxy* proxy = Debugger::GetProxy().get();
  if (proxy == nullptr) return;

  // If the proxy has enabled breakpoints that match entry into methods of
  // the given class, arrange for the VM to stop execution and notify the
  // debugger whenever execution enters one of these matched method.
  // This function is called once, when a class is first loaded, so it is not
  // performance critical.
  size_t numFuncs = cls->numMethods();
  if (numFuncs == 0) return;
  auto clsName = cls->name();
  std::vector<BreakPointInfoPtr> bps;
  proxy->getBreakPoints(bps);
  for (unsigned int i = 0; i < bps.size(); i++) {
    BreakPointInfoPtr bp = bps[i];
    if (bp->m_state == BreakPointInfo::Disabled) continue;
    // TODO: check name space separately
    if (bp->getClass() != clsName->data()) continue;
    bp->m_bindState = BreakPointInfo::KnownToBeInvalid;
    for (size_t i = 0; i < numFuncs; ++i) {
      auto f = cls->getMethod(i);
      if (!matchFunctionName(bp->getFunction(), f)) continue;
      bp->m_bindState = BreakPointInfo::KnownToBeValid;
      phpAddBreakPointFuncEntry(f);
    }
  }
}

void HphpdHook::onDefFunc(const Func* f) {
  // Make sure we have a proxy
  DebuggerProxyPtr proxy = Debugger::GetProxy();
  if (proxy == nullptr) return;

  // If the proxy has an enabled breakpoint that matches entry into the given
  // function, arrange for the VM to stop execution and notify the debugger
  // whenever execution enters the given function.
  std::vector<BreakPointInfoPtr> bps;
  proxy->getBreakPoints(bps);
  for (unsigned int i = 0; i < bps.size(); i++) {
    BreakPointInfoPtr bp = bps[i];
    if (bp->m_state == BreakPointInfo::Disabled) continue;
    if (!matchFunctionName(bp->getFuncName(), f)) continue;
    bp->m_bindState = BreakPointInfo::KnownToBeValid;
    phpAddBreakPointFuncEntry(f);
    return;
  }
}

//////////////////////////////////////////////////////////////////////////
}}
