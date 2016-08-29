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

#ifndef incl_HPHP_VM_UNWIND_INL_H_
#error "unwind-inl.h should only be included by unwind.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template<class Action>
inline void exception_handler(Action action) {
  ITRACE_MOD(Trace::unwind, 1, "unwind exception_handler\n");
  Trace::Indent _i;

  try {
    action();
    vmpc() = 0;
    return;
  }

  /*
   * Unwind (repropagating from a fault funclet) is slightly different
   * from the throw cases, because we need to re-raise the exception
   * as if it came from the same offset to handle nested fault
   * handlers correctly, and we continue propagating the current Fault
   * instead of pushing a new one.
   */
  catch (const VMPrepareUnwind&) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: restoring offset {}\n", vmpc());
    unwindPhp();
    return;
  }

  catch (const Object& o) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: Object of class {}\n",
               o->getVMClass()->name()->data());
    unwindPhp(o.get());
    if (LIKELY(o.get()->hasMultipleRefs()) ||
        vmfp() != nullptr ||
        !g_context->isNested()) {
      return;
    }

    assert(!vmpc());
    // o will be destroyed at the end of the catch block
    // so we have to make sure the vm state is valid in
    // case a __destruct method needs to run.
    g_context->popVMState();
    g_context->pushVMState(vmStack().top());
    // fall through
  }

  catch (VMSwitchMode&) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: VMSwitchMode\n");
    return;
  }

  catch (VMSwitchModeBuiltin&) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: VMSwitchModeBuiltin from {}\n",
               vmfp()->m_func->fullName()->data());
    unwindBuiltinFrame();
    return;
  }

  catch (VMStackOverflow&) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: VMStackOverflow\n");
    auto const fp = vmfp();
    auto const reenter = fp == vmFirstAR();
    if (!reenter) {
      /*
       * vmfp() is actually a pre-live ActRec, so rejig things so that
       * it looks like the exception was thrown when we were just
       * about to do the call. See handleStackOverflow for more
       * details.
       */
      auto const outer = fp->m_sfp;
      auto const off = outer->func()->base() + fp->m_soff;
      auto const fe = outer->func()->findPrecedingFPI(off);
      vmpc() = outer->func()->unit()->at(fe->m_fcallOff);
      assertx(isFCallStar(peek_op(vmpc())));
      vmfp() = outer;
      assert(vmsp() == reinterpret_cast<Cell*>(fp) - fp->numArgs());
    } else {
      vmsp() = reinterpret_cast<Cell*>(fp + 1);
    }
    auto fatal = new FatalErrorException("Stack overflow");
    if (reenter) fatal->throwException();
    unwindCpp(fatal);
    not_reached();
  }

  catch (Exception& e) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: Exception: {}\n", e.what());
    unwindCpp(e.clone());
    not_reached();
  }

  catch (std::exception& e) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: std::exception: {}\n", e.what());
    unwindCpp(new Exception("unexpected %s: %s", typeid(e).name(), e.what()));
    not_reached();
  }

  catch (...) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: unknown\n");
    unwindCpp(new Exception("unknown exception"));
    not_reached();
  }

  // We've come from the const Object& case, and need to restore
  // the vm state to where it was.
  // Note that we've preserved vmStack.top(), and vmFirstAR was invalid
  // prior to the popVMState above.
  // Also, we don't really need to null out vmfp() here, but doing so will
  // cause an assert to fire if we try to re-enter before the next
  // popVMState.
  vmpc() = nullptr;
  vmfp() = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
}
