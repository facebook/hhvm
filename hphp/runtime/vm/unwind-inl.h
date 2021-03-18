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

#ifndef incl_HPHP_VM_UNWIND_INL_H_
#error "unwind-inl.h should only be included by unwind.h"
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

template<class Action>
inline bool exception_handler(Action action) {
  Trace::Indent _i;
  try {
    action();
    return true;
  }

  catch (const Object& o) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: Object of class {}\n",
               o->getVMClass()->name()->data());
    assertx(o.get());
    if (vmfp() == nullptr) throw;
    unwindVM(o.get());
    return false;
  }

  catch (Exception& e) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: Exception: {}\n", e.what());
    if (vmfp() == nullptr) throw;
    unwindVM(e.clone());
    not_reached();
  }

  catch (std::exception& e) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: std::exception: {}\n", e.what());
    auto const exn =
      new Exception("unexpected %s: %s", typeid(e).name(), e.what());
    if (vmfp() == nullptr) exn->throwException();
    unwindVM(exn);
    not_reached();
  }

  catch (CppDummyException& e) {
    always_assert_flog(false,
                       "CppDummyException should not reach exception_handler");
  }

  catch (...) {
    checkVMRegState();
    ITRACE_MOD(Trace::unwind, 1, "unwind: unknown\n");
    auto const exn = new Exception("unknown exception");
    if (vmfp() == nullptr) exn->throwException();
    unwindVM(exn);
    not_reached();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
