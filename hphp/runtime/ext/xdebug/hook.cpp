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

#include "hphp/runtime/ext/xdebug/hook.h"

#include "hphp/runtime/base/file.h"

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_xml.h"
#include "hphp/runtime/ext/xdebug/xdebug_command.h"

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/util/match.h"

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

// These don't need to be exposed, but make dealing the breakpoints less verbose.
#define BREAKPOINT_MAP (s_xdebug_breakpoints->m_breakMap)
#define FUNC_ENTRY (s_xdebug_breakpoints->m_funcEntryMap)
#define FUNC_EXIT (s_xdebug_breakpoints->m_funcExitMap)
#define LINE_MAP (s_xdebug_breakpoints->m_lineMap)
#define EXCEPTION_MAP (s_xdebug_breakpoints->m_exceptionMap)
#define UNMATCHED (s_xdebug_breakpoints->m_unmatched)

using BreakType = XDebugBreakpoint::Type;

////////////////////////////////////////////////////////////////////////////////

namespace {

// Helper that adds the given function breakpoint corresponding to the given
// function and id as a breakpoint. If a duplicate breakpoint already exists,
// it is overwritten.
void add_func_breakpoint(int id, XDebugBreakpoint& bp, const Func* func) {
  // Function id is added to the breakpoint once matched
  auto const func_id = func->getFuncId();
  bp.funcId = func_id;

  // Add the appropriate function map
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

// Helper that adds the given line breakpoint that has been matched to the given
// unit as a breakpoint. The line number is assumed to be valid in the unit.
void add_line_breakpoint(int id, XDebugBreakpoint& bp, const Unit* unit) {
  auto filepath = unit->filepath()->toCppString();
  LINE_MAP[filepath].emplace(bp.line, id);
  bp.unit = unit;
  bp.resolved = true;
}

// Helper used when looping through the unmatched breakpoints. Checks if the
// given function breakpoint matches the given function. If it does, it is
// removed from the unmatched set using the given iterator. The passed iterator
// reference is then modified to be the correct "next" iterator. Returns true
// if there was a match, false otherwise.
bool check_func_match(XDebugBreakpoint& bp,
                      const Func* func,
                      hphp_hash_set<int>::iterator& iter) {
  if (bp.fullFuncName == func->fullName()->toCppString()) {
    add_func_breakpoint(*iter, bp, func);
    iter = UNMATCHED.erase(iter);
    return true;
  }
  return false;
}

// Given a filename, finds the corresponding unit if it exists. Returns nullptr
// otherwise.
const Unit* find_unit(String filename) {
  // Search the given filename in the list of evaled files. We translate each
  // unit's filename to a canonical format, which is slow, but necessary.
  for (auto& kv : g_context->m_evaledFiles) {
    auto const unit = kv.second.unit;
    String ufilename(const_cast<StringData*>(unit->filepath()));
    if (filename == File::TranslatePath(ufilename)) {
      return unit;
    }
  }
  return nullptr;
}

bool calibrateBreakpointLine(const Unit* unit, int& line) {
  auto const size = [] (const SourceLoc& loc) -> size_t {
    return loc.line1 - loc.line0 + 1;
  };

  auto const rank = [&] (const SourceLoc& loc) {
    auto distance = loc.line1 - line;
    // loc is completely above input line, ignore it.
    if (distance < 0) {
      return 0.0;
    }
    // Favor SourceLoc whose ending line is close to input line.
    return 10.0 / (distance + 1) + 1.0 / size(loc);
  };

  SourceLoc best;
  best.line0 = -1;
  best.line1 = -1;
  auto bestRank = rank(best);

  for (auto const& ent : getSourceLocTable(unit)) {
    auto curRank = rank(ent.val());
    if (curRank > bestRank) {
      best = ent.val();
      bestRank = curRank;

      // If the Sourceloc neatly fits on a single line (hopefully the common
      // case), then bail out early.
      if (best.line0 == line && size(best) == 1) break;
    }
  }
  line = best.line1;

  return true;
}

}

////////////////////////////////////////////////////////////////////////////////
// XDebugThreadBreakpoints Implementation

// Adding a breakpoint. Returns a unique id for the breakpoint.
int XDebugThreadBreakpoints::addBreakpoint(XDebugBreakpoint& bp) {
  auto const id = s_xdebug_breakpoints->m_nextBreakpointId;

  // php5 xdebug only accepts multiple breakpoints of the same type for
  // line breakpoints. A useful addition might be to allow multiple of all
  // types, but for now, multiple function or exception breakpoints results
  // in the most recent silently being used (matching php5 xdebug).
  switch (bp.type) {
    case BreakType::EXCEPTION: {
      // Remove duplicates then insert the name
      auto exceptionName = bp.exceptionName;
      auto iter = EXCEPTION_MAP.find(exceptionName);
      if (iter != EXCEPTION_MAP.end()) {
        XDEBUG_REMOVE_BREAKPOINT(iter->second);
      }
      EXCEPTION_MAP[exceptionName] = id;
      bp.resolved = true;
      break;
    }
    // Attempt to find the unit/line combo
    case BreakType::LINE: {
      const Unit* unit = find_unit(bp.fileName);
      if (unit == nullptr) {
        UNMATCHED.insert(id);
        break;
      }
      // Ignore calibration result because we rely on
      // phpAddBreakPointLine() below to bail out.
      calibrateBreakpointLine(unit, bp.line);

      // If the file/line combo is invalid, throw an error
      if (!phpAddBreakPointLine(unit, bp.line)) {
        throw_exn(XDebugError::BreakpointInvalid);
      }
      add_line_breakpoint(id, bp, unit);
      break;
    }
    // Try to find the breakpoint's function
    case BreakType::CALL:
    case BreakType::RETURN: {
      const Class* cls = nullptr;
      const Func* func = nullptr;
      const String funcName(bp.funcName);
      if (!bp.className.hasValue()) {
        func = Unit::lookupFunc(funcName.get());
      } else {
        const String className(bp.className.value());
        cls = Unit::lookupClass(className.get());
        if (cls != nullptr) {
          func = cls->lookupMethod(funcName.get());
        }
      }

      // Either add the breakpoint or store it as unmatched. If the class
      // exists, we can verify that the method is valid.
      if (func != nullptr) {
        add_func_breakpoint(id, bp, func);
        bp.resolved = true;
      } else if (bp.className.hasValue() && cls != nullptr) {
        throw_exn(XDebugError::BreakpointInvalid);
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
  // Try to grab the breakpoint
  auto bp_iter = BREAKPOINT_MAP.find(id);
  if (bp_iter == BREAKPOINT_MAP.end()) {
    return;
  }
  auto const& bp = bp_iter->second;

  // Remove the breakpoint from the unmatched set. This will return 0
  // if the breakpoint is not in the set and so the breakpoint is not unmatched
  if (UNMATCHED.erase(id) == 0) {
    switch (bp.type) {
      case BreakType::CALL:
        FUNC_ENTRY.erase(bp.funcId);
        phpRemoveBreakPointFuncEntry(Func::fromFuncId(bp.funcId));
        break;
      case BreakType::RETURN:
        FUNC_EXIT.erase(bp.funcId);
        phpRemoveBreakPointFuncExit(Func::fromFuncId(bp.funcId));
        break;
      case BreakType::EXCEPTION:
        EXCEPTION_MAP.erase(bp.exceptionName);
        break;
      case BreakType::LINE: {
        auto filepath = bp.unit->filepath()->toCppString();
        auto& unit_map = LINE_MAP[filepath];

        // Need to ensure we don't delete breakpoints on the same line
        auto range = LINE_MAP[filepath].equal_range(bp.line);
        for (auto iter = range.first; iter != range.second; ++iter) {
          if (iter->second != id) {
            continue;
          }

          // If this is the only breakpoint on this line, unregister the line
          if (std::distance(range.first, range.second) == 1) {
            phpRemoveBreakPointLine(bp.unit, bp.line);
          }
          unit_map.erase(iter);
          break;
        }
        break;
      }
    }
  }

  // This deletes the breakpoint
  BREAKPOINT_MAP.erase(id);
}

bool XDebugThreadBreakpoints::updateBreakpointLine(int id, int newLine) {
  auto iter = BREAKPOINT_MAP.find(id);
  if (iter == BREAKPOINT_MAP.end()) {
    return false;
  }
  auto& bp = iter->second;

  // Determine if we need to unregister the line.
  auto filepath = bp.unit->filepath()->toCppString();
  if (LINE_MAP[filepath].count(bp.line) == 1) {
    phpRemoveBreakPointLine(bp.unit, bp.line);
  }

  if (!calibrateBreakpointLine(bp.unit, newLine)) {
    return false;
  }

  // Register the new line.
  assertx(newLine != -1);
  bp.line = newLine;

  LINE_MAP[filepath].emplace(bp.line, id);
  phpAddBreakPointLine(bp.unit, bp.line);
  return true;
}

bool XDebugThreadBreakpoints::updateBreakpointState(int id, bool enabled) {
  auto iter = BREAKPOINT_MAP.find(id);
  if (iter == BREAKPOINT_MAP.end()) {
    return false;
  }

  XDebugBreakpoint& bp = iter->second;
  bp.enabled = enabled;
  return true;
}

bool XDebugThreadBreakpoints::updateBreakpointHitCondition(
  int id,
  XDebugBreakpoint::HitCondition con
) {
  auto iter = BREAKPOINT_MAP.find(id);
  if (iter == BREAKPOINT_MAP.end()) {
    return false;
  }

  auto& bp = iter->second;
  bp.hitCondition = con;
  return true;
}

bool XDebugThreadBreakpoints::updateBreakpointHitValue(int id, int hitValue) {
  auto iter = BREAKPOINT_MAP.find(id);
  if (iter == BREAKPOINT_MAP.end()) {
    return false;
  }

  auto& bp = iter->second;
  bp.hitValue = hitValue;
  return true;
}

IMPLEMENT_THREAD_LOCAL_NO_CHECK(XDebugThreadBreakpoints, s_xdebug_breakpoints);

////////////////////////////////////////////////////////////////////////////////
// Debug Hook Handling

// Helper that grabs the breakpoint ids for the given breakpoint type using the
// given breakpoint info.  Pushes the ids onto the passed vector.
static void get_breakpoint_ids(const BreakInfo& bi, std::vector<int>& ids) {
  match<void>(
    bi,
    [&] (FuncBreak fb) {
      auto& map = fb.entry ? FUNC_ENTRY : FUNC_EXIT;
      ids.push_back(map.at(fb.func->getFuncId()));
    },
    [&] (LineBreak lb) {
      // Look for the breakpoint's unit.
      auto unit_iter = LINE_MAP.find(lb.unit->filepath()->toCppString());
      if (unit_iter == LINE_MAP.end()) {
        return;
      }

      // Add all the ids for this line.
      auto range = unit_iter->second.equal_range(lb.line);
      for (auto iter = range.first; iter != range.second; ++iter) {
        ids.push_back(iter->second);
      }
    },
    [&] (ExnBreak eb) {
      // Check for the wildcard exception breakpoint
      auto iter = EXCEPTION_MAP.find("*");
      if (iter != EXCEPTION_MAP.end()) {
        ids.push_back(iter->second);
        return;
      }

      // Check if breakpoint's exception is registered.
      iter = EXCEPTION_MAP.find(eb.name->toCppString());
      if (iter != EXCEPTION_MAP.end()) {
        ids.push_back(iter->second);
      }
    }
  );
}

// Helper that checks if the given breakpoint has been "hit". That is, if
// all hit conditions have been met
static bool is_breakpoint_hit(XDebugBreakpoint& bp) {
  if (!bp.enabled) {
    return false;
  }

  // Check the condition on line breakpoints. We disable then enable the
  // breakpoints before/after the evaluation in order to prevent
  // a breakpoint from being hit within this check
  if (bp.type == BreakType::LINE && bp.conditionUnit != nullptr) {
    auto const prev_disabled = g_context->m_dbgNoBreak;
    g_context->m_dbgNoBreak = true;

    auto const result = g_context->evalPHPDebugger(bp.conditionUnit, 0);
    g_context->m_dbgNoBreak = prev_disabled;
    if (result.failed) {
      // If the condition evaluation fails, err on the side of breaking so
      // that the bp is not missed, and the user can remove or update the hit
      // condition.
      //
      // Note: in this case, the evaluation failure has already been printed to
      // the error output stream and/or log.
      return true;
    } else if (!result.result.toBoolean()) {
      // Expression evaluated successfully, but returned false.
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

static Variant get_breakpoint_message(const BreakInfo& bi) {
  if (auto eb = boost::get<ExnBreak>(&bi)) return VarNR(eb->message);
  return init_null();
}

DebuggerHook* XDebugHook::GetInstance() {
  static DebuggerHook* instance = new XDebugHook();
  return instance;
}

void XDebugHook::onBreak(const BreakInfo& bi) {
  // Have to have a server to break.
  if (XDEBUG_GLOBAL(Server) == nullptr) {
    return;
  }

  // Grab the breakpoints matching the passed info
  std::vector<int> ids;
  get_breakpoint_ids(bi, ids);

  // Iterate.  Note that we only tell the server to break once.
  bool have_broken = false;
  for (auto const id : ids) {
    // Look up the breakpoint, ensure it's hittable.
    auto& bp = BREAKPOINT_MAP.at(id);
    if (!is_breakpoint_hit(bp)) {
      continue;
    }

    // We only break once per location.
    auto const temporary = bp.temporary; // breakpoint could be deleted
    if (!have_broken) {
      have_broken = true;

      // Grab the breakpoint message and do the break.
      auto const msg = get_breakpoint_message(bi);
      if (!XDEBUG_GLOBAL(Server)->breakpoint(bp, msg)) {
        // Kill the server if there's an error.
        XDebugServer::detach();
        return;
      }
    }

    // Remove the breakpoint if it was temporary.
    if (temporary) {
      XDEBUG_REMOVE_BREAKPOINT(id);
    }
  }
}

// Exception::getMessage method name.
const StaticString s_GET_MESSAGE("getMessage");

void XDebugHook::onOpcode(PC /*pc*/) {
  auto server = XDEBUG_GLOBAL(Server);
  if (server == nullptr) {
    return;
  }
  // Got the interrupt callback reset it.
  RID().setDebuggerIntr(false);
  server->processAsyncCommandQueue();
}

void XDebugHook::onExceptionThrown(ObjectData* exception) {
  // Grab the exception name and message
  const StringData* name = exception->getVMClass()->name();
  const Variant msg = exception->o_invoke(s_GET_MESSAGE, init_null(), false);
  const String msg_str = msg.isNull() ? empty_string() : msg.toString();
  onExceptionBreak(name, msg.isNull() ? nullptr : msg_str.get());
}

void XDebugHook::onFlowBreak(const Unit* unit, int line) {
  if (XDEBUG_GLOBAL(Server) != nullptr) {
    // Translate the unit filepath and then break
    auto const filepath = String(const_cast<StringData*>(unit->filepath()));
    auto const transpath = File::TranslatePath(filepath);
    XDEBUG_GLOBAL(Server)->breakpoint(transpath, init_null(),
                                      init_null(), line);
  }
}

void sendBreakpointResolvedNotify(int id, const XDebugBreakpoint& bp) {
  auto server = XDEBUG_GLOBAL(Server);
  if (server == nullptr || !server->m_supportsNotify) {
    return;
  }
  auto notify = xdebug_xml_node_init("notify");
  SCOPE_EXIT { xdebug_xml_node_dtor(notify); };

  xdebug_xml_add_attribute(notify, "name", "breakpoint_resolved");
  xdebug_xml_add_child(notify, breakpoint_xml_node(id, bp));
  server->sendMessage(*notify);
}

void XDebugHook::onFileLoad(Unit* unit) {
  // Translate the unit filename to match xdebug's internal format
  String unit_path(const_cast<StringData*>(unit->filepath()));
  auto const filename = File::TranslatePath(unit_path);

  // Loop over all unmatched breakpoints
  for (auto iter = UNMATCHED.begin(); iter != UNMATCHED.end();) {
    auto id = *iter;
    auto& bp = BREAKPOINT_MAP.at(id);
    if (bp.type != BreakType::LINE || bp.fileName != filename.toCppString()) {
      ++iter;
      continue;
    }
    // Ignore calibration result because we rely on phpAddBreakPointLine()
    // return value below to set/erase breakpoint.
    calibrateBreakpointLine(unit, bp.line);

    // If the line is invalid there's not much we can do to inform the client
    // in the dbgp protocol at this point. php5 xdebug doesn't do anything,
    // so we just cleanup
    if (phpAddBreakPointLine(unit, bp.line)) {
      add_line_breakpoint(id, bp, unit);
      bp.resolved = true;
      sendBreakpointResolvedNotify(id, bp);
      iter = UNMATCHED.erase(iter);
    } else {
      BREAKPOINT_MAP.erase(id);
      iter = UNMATCHED.erase(iter);
    }
  }
}

void XDebugHook::onDefClass(const Class* cls) {
  // If the class has no methods, no need to loop through breakpoints
  auto const num_methods = cls->numMethods();
  if (num_methods == 0) {
    return;
  }

  // Loop through the unmatched breakpoints
  const StringData* className = cls->name();
  for (auto iter = UNMATCHED.begin(); iter != UNMATCHED.end();) {
    // Grab the breakpoint, ignore it if it is the wrong type or the classname
    // doesn't match. Note that a classname does not have to exist as the user
    // can specify method bar on class Foo with "Foo::bar"
    auto& bp = BREAKPOINT_MAP.at(*iter);
    if ((bp.type != BreakType::CALL && bp.type != BreakType::RETURN) ||
        (bp.className.hasValue() &&
         className->toCppString() != bp.className.value())) {
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

void XDebugHook::onDefFunc(const Func* func) {
  // Loop through unmatched function breakpoints
  for (auto iter = UNMATCHED.begin(); iter != UNMATCHED.end();) {
    auto& bp = BREAKPOINT_MAP.at(*iter);
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
