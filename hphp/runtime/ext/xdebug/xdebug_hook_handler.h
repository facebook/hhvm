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

#ifndef incl_HPHP_XDEBUG_HOOK_HANDLER_H_
#define incl_HPHP_XDEBUG_HOOK_HANDLER_H_

#include "hphp/runtime/ext/xdebug/ext_xdebug.h"
#include "hphp/runtime/ext/xdebug/xdebug_server.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_var.h"

#include "hphp/runtime/vm/debugger-hook.h"

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////
// XDebug Breakpoint Type

// This struct contains all the runtime information about a given breakpoint.
// Each breakpoint is tagged with its type.
struct XDebugBreakpoint {
  enum class Type : int8_t {
    LINE,
    CALL,
    RETURN,
    EXCEPTION
  };

  // The hit value will be compared against the hit count using one of these
  // operators
  enum class HitCondition : int8_t {
    GREATER_OR_EQUAL,
    EQUAL,
    MULTIPLE
  };

  Type type; // The breakpoint type. This must be provided.
  bool enabled = true; // Whether or not this breakpoint is enabled
  bool temporary = false; // If true, removed after being hit
  HitCondition hitCondition = HitCondition::GREATER_OR_EQUAL;
  int hitValue = 0; // Value to compare hitCount against
  int hitCount = 0; // # of times this breakpoint has been hit

  // A line breakpoint requires a filename and a line number. It has an
  // optional condition represented by the given unit where the pseud-main is
  // called when over the given file/line. It should returns true if we should
  // break, false otherwise.  Note that php5 xdebug doesn't distinguish
  // internally between this and a conditional breakpoint (which is in the spec)
  // The encoded condition is kept around for display purposes. The unit is
  // updated to the matched unit once the breakpoint has been matched.
  String fileName;
  int line = -1;
  Unit* conditionUnit = nullptr;
  String condition;
  const Unit* unit = nullptr;

  // A call or return breakpoint occurs when the function in the given class
  // with the given name is called/returns. The class name and function name
  // are kept around for listing breakpoint info. The function id is added
  // once the breakpoint has been matched with a function
  Variant className = init_null(); // optional
  String funcName;
  String fullFuncName;
  int funcId = -1;

  // An exception breakpoint occurs when an exception with a given name is
  // thrown.
  String exceptionName;
};

////////////////////////////////////////////////////////////////////////////////
// Thread Local Storage for Breakpoints

struct XDebugThreadBreakpoints {
  // Adding a breakpoint. Returns a unique id for the breakpoint. Throws an
  // XDebugServer::ErrorCode on failure.
  int addBreakpoint(XDebugBreakpoint& bp);

  // Removing a breakpoint with the given id. If the id does not exit, does
  // nothing
  void removeBreakpoint(int id);

  // Gets the breakpoint with the given id. Returns nullptr if the breakpoint
  // does not exist
  inline const XDebugBreakpoint* getBreakpoint(int id) {
    auto iter = m_breakMap.find(id);
    return iter != m_breakMap.end() ? &iter->second : nullptr;
  }

  // Breakpoint update methods. Some of these modify internal state, so it's
  // better to use these to update breakpoints. There could be more update
  // methods, but these are the only ones needed by the xdebug commmands right
  // now. These return true on success, false on failure.
  bool updateBreakpointLine(int id, int newLine);
  bool updateBreakpointState(int id, bool enabled);
  bool updateBreakpointHitCondition(int id, XDebugBreakpoint::HitCondition con);
  bool updateBreakpointHitValue(int id, int hitValue);

  // Map from id => breakpoint. This where breakpoints are stored
  hphp_hash_map<int, XDebugBreakpoint> m_breakMap;

  // Map from func id => breakpoint id for enter/exit breakpoints
  hphp_hash_map<int, int> m_funcEntryMap;
  hphp_hash_map<int, int> m_funcExitMap;

  // Map of filename => map of line # => breakpoint ids for the line
  // php5 xdebug allows multiple line breakpoints per line
  hphp_string_map<std::multimap<int, int> > m_lineMap;

  // Map from exception name => breakpoint id
  hphp_string_map<int> m_exceptionMap;

  // Set of breakpoint ids that don't yet have valid location. php5
  // xdebug does no validity checking on breakpoint input and just assumes if
  // function or file/line does not exist, it will in the future.
  hphp_hash_set<int> m_unmatched;

  // The next breakpoint id to assign
  int m_nextBreakpointId = 0;
};

extern DECLARE_THREAD_LOCAL(XDebugThreadBreakpoints, s_xdebug_breakpoints);

// Breakpoint entrypoints for other files
#define XDEBUG_ADD_BREAKPOINT(bp) \
  (s_xdebug_breakpoints->addBreakpoint(bp));
#define XDEBUG_REMOVE_BREAKPOINT(id) \
  (s_xdebug_breakpoints->removeBreakpoint(id))
#define XDEBUG_GET_BREAKPOINT(id) \
  (s_xdebug_breakpoints->getBreakpoint(id))
#define XDEBUG_BREAKPOINTS \
  (s_xdebug_breakpoints->m_breakMap)

////////////////////////////////////////////////////////////////////////////////
// XDebugHookHandler

struct XDebugHookHandler : DebugHookHandler {
  static DebugHookHandler* GetInstance();
  // Starting the server on request init
  void onRequestInit() override {
    if (XDebugServer::isNeeded()) {
      XDebugServer::attach(XDebugServer::Mode::REQ);
    }
  }

  // Stopping the server on request shutdown
  void onRequestShutdown() override {
    if (XDebugServer::isAttached()) {
      XDebugServer::detach();
    }
  }

  void onOpcode(PC pc) override;

  // Information possibly passed during a breakpoint
  union BreakInfo {
    // Function enter/exit breakpoint
    const Func* func;
    // Line breakpoint
    struct {
      const Unit* unit;
      int line;
    };
    // Exception breakpoint (for both errors and exceptions)
    struct {
      const StringData* name;
      const StringData* message;
    };
  };

  // Generic breakpoint handling routine. Most of the break handling
  // code is the same across the different breakpoint types.
  template<XDebugBreakpoint::Type type>
  void onBreak(const BreakInfo& bi);

  void onFuncEntryBreak(const Func* func) override {
    BreakInfo bi;
    bi.func = func;
    onBreak<XDebugBreakpoint::Type::CALL>(bi);
  }

  void onFuncExitBreak(const Func* func) override {
    BreakInfo bi;
    bi.func = func;
    onBreak<XDebugBreakpoint::Type::RETURN>(bi);
  }

  void onLineBreak(const Unit* unit, int line) override {
    BreakInfo bi;
    bi.unit = unit;
    bi.line = line;
    onBreak<XDebugBreakpoint::Type::LINE>(bi);
  }

  // Helper for errors and exceptions that potentially starts the debug server
  // if needed
  void onExceptionBreak(const StringData* name, const StringData* msg) {
    // Potentially start the debug server if it hasn't been started
    if (XDEBUG_GLOBAL(RemoteEnable) &&
        !XDebugServer::isAttached() &&
        XDEBUG_GLOBAL(RemoteMode) == "jit") {
      XDebugServer::attach(XDebugServer::Mode::JIT);
    }

    // Handle the exception
    BreakInfo bi;
    bi.name = name;
    bi.message = msg;
    onBreak<XDebugBreakpoint::Type::EXCEPTION>(bi);
  }

  void onExceptionThrown(ObjectData* exception) override;
  void onError(const ExtendedException& ee,
               int errnum,
               const std::string& message) override {
    auto const name = xdebug_error_type(errnum);
    auto const msg = String(message);
    onExceptionBreak(name.get(), msg.get());
  }

  // Flow control. Each break type just calls the generic onFlowBreak
  void onFlowBreak(const Unit* unit, int line);
  void onStepInBreak(const Unit* unit, int line) override {
    onFlowBreak(unit, line);
  }
  void onStepOutBreak(const Unit* unit, int line) override {
    onFlowBreak(unit, line);
  }
  void onNextBreak(const Unit* unit, int line) override {
    onFlowBreak(unit, line);
  }

  // Handle loading unmatched breakpoints
  void onFileLoad(Unit* efile) override;
  void onDefClass(const Class* cls) override;
  void onDefFunc(const Func* func) override;
 private:
  XDebugHookHandler() {}
  ~XDebugHookHandler() override {}
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_XDEBUG_HOOK_HANDLER_H_
