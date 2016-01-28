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

#ifndef incl_HPHP_EVAL_DEBUGGER_CMD_FLOW_CONTROL_H_
#define incl_HPHP_EVAL_DEBUGGER_CMD_FLOW_CONTROL_H_

#include "hphp/runtime/debugger/debugger_command.h"
#include "hphp/runtime/debugger/cmd/cmd_interrupt.h"

namespace HPHP { namespace Eval {

struct InterruptSite;

/*
 * CmdFlowControl is the base class of all "flow control" cmds: Continue, Step,
 * Next, and Out. A DebuggerProxy can execute exactly one of these cmds at a
 * time.
 *
 * 1. Proxy receives a flow command (say, Next).
 * 2. Proxy calls the cmd's onSetup() with current source position.
 * 3. Cmd sets up state (see below) in the VM to control stepping, etc.
 * 4. If the cmd is complete the proxy deletes the cmd, otherwise it saves it
 *    in m_flow.
 * 5. Proxy lets the thread continue execution.
 * 6. Proxy receives an interrupt from the VM, delivers it to the cmd in m_flow.
 * 7. Cmd inspects the site and VM state, and either decides it is done, or
 *    sets up state in the VM to continue the control flow operation.
 * 8. If the cmd is complete the proxy stops and waits for more input from the
 *    client. Otherwise, rinse and repeat at step 5.
 *
 * When a cmd is setup, or receives an interrupt and determines it is not
 * complete, it will do one of three things to ensure it gets interrupted again
 * in the future:
 *
 * 1. Set m_needsVMInterrupt to true, to ensure we interpret and interrupt on
 *    each opcode executed.
 * 2. Set m_needsVMInterrupt to true, and setup a location filter. This ensures
 *    we interpret, but only interrupt when the PC misses the location filter.
 * 3. Place an "internal breakpoint", which ensures an interrupt when the
 *    breakpoint is hit without forcing us to interpret everything.
 *
 * The cmd may get interrupted for other reasons, such as an exception, reaching
 * a breakpoint, a hard break, etc. All flow cmds are designed to tollerate this
 * and remember enough state to determine if they should really transition their
 * state or not.
 */
struct CmdFlowControl : DebuggerCommand {
  explicit CmdFlowControl(Type type): DebuggerCommand(type) {}
  ~CmdFlowControl() override;

  bool onServer(DebuggerProxy&) override;
  void onClient(DebuggerClient&) override;

  // Work done to setup a new flow command, after receiving it from the client.
  virtual void onSetup(DebuggerProxy&, CmdInterrupt&) = 0;

  // Work done when a VM thread interrupts the proxy.
  virtual void onBeginInterrupt(DebuggerProxy&, CmdInterrupt&) = 0;

  // A completed flow cmd has done all its work and can be deleted.
  bool complete() const {
    return m_complete;
  }

  // Does this cmd need to force interrupts in the interpreter loop?
  bool needsVMInterrupt() const {
    return m_needsVMInterrupt;
  }

protected:
  void sendImpl(DebuggerThriftBuffer&) override;
  void recvImpl(DebuggerThriftBuffer&) override;

  int decCount() {
    assert(m_count > 0);
    return --m_count;
  }

  int getCount() const {
    assert(m_count > 0);
    return m_count;
  }

  void installLocationFilterForLine(InterruptSite*);
  void removeLocationFilter();
  void setupStepOuts();
  void cleanupStepOuts();
  bool hasStepOuts();
  bool atStepOutOffset(Unit* unit, Offset o);

  bool m_complete{false};
  bool m_needsVMInterrupt{false};

  // Support for stepping operations
  int m_stackDepth{0};
  int m_vmDepth{0};
  std::string m_loc; // last break's source location

  // Represents the destination of an internal stepping operation by
  // unit and offset. Implictly maintains the breakpoint filter.
  struct StepDestination {
    StepDestination();
    StepDestination(const HPHP::Unit* unit, Offset offset);
    StepDestination(const StepDestination& other) = delete;
    StepDestination& operator=(const StepDestination& other) = delete;
    StepDestination(StepDestination&& other);
    StepDestination& operator=(StepDestination&& other);
    ~StepDestination();

    bool valid() const { return m_unit != nullptr; }
    bool at(const Unit* unit, Offset o) const {
      return (m_unit == unit) && (m_offset == o);
    }

  private:
    const Unit* m_unit;
    Offset m_offset;
    bool m_ownsInternalBreakpoint;
  };

private:
  StepDestination m_stepOut1;
  StepDestination m_stepOut2;
  int16_t m_count{1};
  bool m_smallStep;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_EVAL_DEBUGGER_CMD_FLOW_CONTROL_H_
