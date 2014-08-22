/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/xdebug/xdebug_hook_handler.h"

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {

// These don't need to be exposed, but make dealing the breakpoints less verbose
#define BREAKPOINT_MAP (s_xdebug_breakpoints->m_breakMap)
#define FUNC_ENTRY (s_xdebug_breakpoints->m_funcEntryMap)
#define FUNC_EXIT (s_xdebug_breakpoints->m_funcExitMap)
#define LINE_MAP (s_xdebug_breakpoints->m_lineMap)
#define EXCEPTION_MAP (s_xdebug_breakpoints->m_exceptionMap)
#define UNMATCHED (s_xdebug_breakpoints->m_unmatched)

typedef XDebugBreakpoint::Type BreakType;
typedef XDebugHookHandler::BreakInfo BreakInfo;

////////////////////////////////////////////////////////////////////////////////
// Helpers

// Helper that removes a breakpoint during the middle of iteration through the
// unmatched set.
static inline void remove_breakpoint_iter(hphp_hash_set<int>::iterator& iter) {
  // TODO(#4489053) Implement. Need to remove the breakpoint and update the
  // iterator. For now, this just disables.
  BREAKPOINT_MAP.at(*iter).enabled = false;
  ++iter;
}

// Helper that adds the given function breakpoint corresponding to the given
// function and id as a breakpoint. If a duplicate breakpoint already exists,
// it is overwritten.
static void add_func_breakpoint(int id,
                                const XDebugBreakpoint& bp,
                                const Func* func) {
  int func_id = func->getFuncId();
  switch (bp.type) {
    case BreakType::CALL: {
      auto iter = FUNC_ENTRY.find(func_id);
      if (iter != FUNC_ENTRY.end()) {
        XDEBUG_REMOVE_BREAKPOINT(iter->second);
      }
      FUNC_ENTRY[func->getFuncId()] = id;
      phpAddBreakPointFuncEntry(func);
      break;
    }
    case BreakType::RETURN: {
      auto iter = FUNC_EXIT.find(func_id);
      if (iter != FUNC_EXIT.end()) {
        XDEBUG_REMOVE_BREAKPOINT(iter->second);
      }
      FUNC_EXIT[func->getFuncId()] = id;
      phpAddBreakPointFuncExit(func);
      break;
    }
    default:
      throw Exception("Not passed a function breakpoint");
  }
}

// Helper used when looping through the unmatched breakpoints. Checks if the
// given function breakpoint matches the given function. If it does, it is
// removed from the unmatched set using the given iterator. The passed iterator
// reference is then modified to be the correct "next" iterator. Returns true
// if there was a match, false otherwise.
static bool check_func_match(const XDebugBreakpoint& bp,
                             const Func* func,
                             hphp_hash_set<int>::iterator& iter) {
  if (func->fullName()->equal(bp.fullFuncName.get())) {
    add_func_breakpoint(*iter, bp, func);
    iter = UNMATCHED.erase(iter);
    return true;
  }
  return false;
}

// Given a filename, finds the corresponding unit if it exists. Returns nullptr
// otherwise.
static const Unit* find_unit(String filename) {
  // Search the given filename in the list of evaled files. We translate each
  // unit's filename to a canonical format, which is slow, but necessary.
  for (auto& kv : g_context->m_evaledFiles) {
    const Unit* unit = kv.second;
    String ufilename(unit->filepath()->data(), CopyString);
    if (filename == File::TranslatePath(ufilename)) {
      return unit;
    }
  }
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// XDebugThreadBreakpoints Implementation

// Adding a breakpoint. Returns a unique id for the breakpoint.
int XDebugThreadBreakpoints::addBreakpoint(const XDebugBreakpoint& bp) {
  int id = s_xdebug_breakpoints->m_nextBreakpointId;

  // php5 xdebug only accepts multiple breakpoints of the same type for
  // line breakpoints. A useful addition might be to allow multiple of all
  // types, but for now, multiple function or exception breakpoints results
  // in the most recent silently being used (matching php5 xdebug).
  switch (bp.type) {
    case BreakType::EXCEPTION: {
      // Remove duplicates then insert the name
      string exceptionName = bp.exceptionName.toCppString();
      auto iter = EXCEPTION_MAP.find(exceptionName);
      if (iter != EXCEPTION_MAP.end()) {
        XDEBUG_REMOVE_BREAKPOINT(iter->second);
      }
      EXCEPTION_MAP[exceptionName] = id;
      break;
    }
    // Attempt to find the unit/line combo
    case BreakType::LINE: {
      const Unit* unit = find_unit(bp.fileName);
      if (unit == nullptr) {
        UNMATCHED.insert(id);
        break;
      }

      // If the file/line combo is invalid, throw an error
      if (!phpAddBreakPointLine(unit, bp.line)) {
        throw XDebugServer::ERROR_BREAKPOINT_INVALID;
      }

      // Add the breakpoint to the line map
      string filepath = unit->filepath()->toCppString();
      LINE_MAP[filepath].insert(std::make_pair(bp.line, id));
      break;
    }
    // Try to find the breakpoint's function
    case BreakType::CALL:
    case BreakType::RETURN: {
      const Class* cls = nullptr;
      const Func* func = nullptr;
      if (bp.className.isNull()) {
        func = Unit::lookupFunc(bp.funcName.get());
      } else {
        cls = Unit::lookupClass(bp.className.toString().get());
        if (cls != nullptr) {
          func = cls->lookupMethod(bp.funcName.get());
        }
      }

      // Either add the breakpoint or store it as unmatched. If the class
      // exists, we can verify that the method is valid.
      if (func != nullptr) {
        add_func_breakpoint(id, bp, func);
      } else if (!bp.className.isNull() && cls != nullptr) {
        throw XDebugServer::ERROR_BREAKPOINT_INVALID;
      } else {
        UNMATCHED.insert(id);
      }
      break;
    }
  }

  // Success, store the breakpoint and increment the id.
  BREAKPOINT_MAP[id] = bp;
  s_xdebug_breakpoints->m_nextBreakpointId++;
  return id;
}

// Removing a breakpoint with the given id.
void XDebugThreadBreakpoints::removeBreakpoint(int id) {
  // TODO(#4489053) Implement. For now, this just disables
  BREAKPOINT_MAP.at(id).enabled = false;
}

IMPLEMENT_THREAD_LOCAL(XDebugThreadBreakpoints, s_xdebug_breakpoints);

////////////////////////////////////////////////////////////////////////////////
// Debug Hook Handling

// Helper that grabs the breakpoint ids for the given breakpoint type using the
// given breakpoint info. Pushes the ids onto the passed vector.
// TODO(#4489053) I know there is a way to using template magic to return a
// generic iterator range over the values of the different types of maps.
template<BreakType type>
static void get_breakpoint_ids(const BreakInfo& bi, std::vector<int>& ids) {
  switch (type) {
    case BreakType::CALL:
      ids.push_back(FUNC_ENTRY.at(bi.func->getFuncId()));
      return;
    case BreakType::RETURN:
      ids.push_back(FUNC_EXIT.at(bi.func->getFuncId()));
      return;
    case BreakType::LINE: {
      // Look for the breakpoint's unit
      auto unit_iter = LINE_MAP.find(bi.unit->filepath()->toCppString());
      if (unit_iter == LINE_MAP.end()) {
        return;
      }

      // Add all the ids for this line
      auto range = unit_iter->second.equal_range(bi.line);
      for (auto iter = range.first; iter != range.second; ++iter) {
        ids.push_back(iter->second);
      }
      return;
    }
    case BreakType::EXCEPTION: {
      // Check for the wildcard exception breakpoint
      auto iter = EXCEPTION_MAP.find("*");
      if (iter != EXCEPTION_MAP.end()) {
        ids.push_back(iter->second);
        return;
      }

      // Check if breakpoint's exception is registered.
      const StringData* name = bi.exception->getVMClass()->name();
      iter = EXCEPTION_MAP.find(name->toCppString());
      if (iter != EXCEPTION_MAP.end()) {
        ids.push_back(iter->second);
      }
      return;
    }
  }
}

// Helper that checks if the given breakpoint has been "hit". That is, if
// all hit conditions have been met
template<BreakType type>
static bool is_breakpoint_hit(XDebugBreakpoint& bp) {
  if (!bp.enabled) {
    return false;
  }

  // Check the condition on line breakpoints. We disable then enable the
  // breakpoints before/after the evaluation in order to prevent
  // a breakpoint from being hit within this check
  if (type == BreakType::LINE && bp.conditionUnit != nullptr) {
    bool prev_disabled = g_context->m_dbgNoBreak;
    g_context->m_dbgNoBreak = true;

    Variant result;
    bool failure = g_context->evalPHPDebugger((TypedValue*) &result,
                                               bp.conditionUnit, 0);
    g_context->m_dbgNoBreak = prev_disabled;
    if (failure || !result.toBoolean()) {
      return false;
    }
  }

  // No hit value means the breakpoint is always hit
  bp.hitCount++;
  if (bp.hitValue == 0) {
    return true;
  }

  // Check the hit condition
  switch (bp.hitCondition) {
    case XDebugBreakpoint::HitCondition::GREATER_OR_EQUAL:
      return bp.hitCount >= bp.hitValue;
    case XDebugBreakpoint::HitCondition::EQUAL:
      return bp.hitCount == bp.hitValue;
    case XDebugBreakpoint::HitCondition::MULTIPLE:
      return bp.hitCount % bp.hitValue == 0;
    default:
      throw Exception("Invalid hit condition");
  }
}

// getMessage method for exceptions
static const StaticString s_GET_MESSAGE("getMessage");

// Returns the message from the given breakpoint info
template<BreakType type>
const Variant get_breakpoint_message(const BreakInfo& bi) {
  // In php5 xdebug, only messages have a string. But this could be extended to
  // be more useful.
  if (type == BreakType::EXCEPTION) {
    return bi.exception->o_invoke(s_GET_MESSAGE, init_null(), false);
  }
  return init_null();
}

template<BreakType type>
void XDebugHookHandler::onBreak(const BreakInfo& bi) {
  // Have to have a server to break.
  if (XDEBUG_GLOBAL(Server) == nullptr) {
    return;
  }

  // Grab the breakpoints matching the passed info
  std::vector<int> ids;
  get_breakpoint_ids<type>(bi, ids);

  // Iterate. Note that we only tell the server to break once.
  bool have_broken = false;
  for (int id : ids) {
    // Look up the breakpoint, ensure it's hittable
    XDebugBreakpoint& bp = BREAKPOINT_MAP.at(id);
    if (!is_breakpoint_hit<type>(bp)) {
      continue;
    }

    // We only break once per location
    if (!have_broken) {
      have_broken = true;

      // Grab the breakpoint message and do the break
      const Variant msg = get_breakpoint_message<type>(bi);
      if (!XDEBUG_GLOBAL(Server)->breakpoint(bp, msg)) {
        XDebugServer::detach(); // Kill the server if there's an error
        return;
      }
    }

    // Remove the breakpoint if it was temporary
    if (bp.temporary) {
      XDEBUG_REMOVE_BREAKPOINT(id);
    }
  }
}

void XDebugHookHandler::onFlowBreak(const Unit* unit, int line) {
  if (XDEBUG_GLOBAL(Server) != nullptr) {
    // Translate the unit filepath and then break
    const String filepath = String(unit->filepath()->data(), CopyString);
    const String transpath = File::TranslatePath(filepath);
    XDEBUG_GLOBAL(Server)->breakpoint(transpath, init_null(),
                                      init_null(), line);
  }
}

void XDebugHookHandler::onFileLoad(Unit* unit) {
  // Translate the unit filename to match xdebug's internal format
  String unit_path(unit->filepath()->data(), CopyString);
  String filename = File::TranslatePath(unit_path);

  // Loop over all unmatched breakpoints
  for (auto iter = UNMATCHED.begin(); iter != UNMATCHED.end();) {
    const XDebugBreakpoint& bp = BREAKPOINT_MAP.at(*iter);
    if (bp.type != BreakType::LINE || bp.fileName != filename) {
      ++iter;
      continue;
    }

    // If the line is invalid there's not much we can do to inform the client
    // in the dbgp protocol at this point. php5 xdebug doesn't do anything,
    // so we just cleanup
    if (phpAddBreakPointLine(unit, bp.line)) {
      string filepath = unit->filepath()->toCppString();
      LINE_MAP[filepath].insert(std::make_pair(bp.line, *iter));
      iter = UNMATCHED.erase(iter);
    } else {
      remove_breakpoint_iter(iter);
    }
  }
}

void XDebugHookHandler::onDefClass(const Class* cls) {
  // If the class has no methods, no need to loop through breakpoints
  size_t num_methods = cls->numMethods();
  if (num_methods == 0) {
    return;
  }

  // Loop through the unmatched breakpoints
  const StringData* className = cls->name();
  for (auto iter = UNMATCHED.begin(); iter != UNMATCHED.end();) {
    // Grab the breakpoint, ignore it if it is the wrong type or the classname
    // doesn't match. Note that a classname does not have to exist as the user
    // can specify method bar on class Foo with "Foo::bar"
    const XDebugBreakpoint& bp = BREAKPOINT_MAP.at(*iter);
    if ((bp.type != BreakType::CALL && bp.type != BreakType::RETURN) ||
        (!bp.className.isNull() &&
         !className->equal(bp.className.toString().get()))) {
      ++iter;
      continue;
    }

    // Check each method for a match
    for (size_t i = 0; i < num_methods; i++) {
      if (check_func_match(bp, cls->getMethod(i), iter)) {
        break;
      }
    }
  }
}

void XDebugHookHandler::onDefFunc(const Func* func) {
  // Loop through unmatched function breakpoints
  for (auto iter = UNMATCHED.begin(); iter != UNMATCHED.end();) {
    const XDebugBreakpoint& bp = BREAKPOINT_MAP.at(*iter);
    if (bp.type != BreakType::CALL && bp.type != BreakType::RETURN) {
      ++iter;
      continue;
    }

    // Check if the function matches this breakpoint
    if (!check_func_match(bp, func, iter)) {
      ++iter;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
