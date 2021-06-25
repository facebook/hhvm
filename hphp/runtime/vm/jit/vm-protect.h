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

#pragma once

#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP { namespace jit {

/*
 * RAII assertion guard that ensures VM state (specifically, all of RDS,
 * including vmfp, vmsp, and vmpc) is not modified while it is in
 * scope. VMProtect::Pause may be used if the protection should be temporarily
 * disabled within a scope.
 *
 * The only rule for nesting is that no VMProtect structs may be constructed
 * inside a scope containing a VMProtect::Pause struct. The scope containing
 * the VMProtect::Pause struct may contain 0 or more VMProtect structs,
 * although only the outermost VMProtect struct will do any real work.
 */
struct VMProtect {
  struct Pause {
#ifndef NDEBUG
    Pause();
    ~Pause();
    bool m_active = true;
#else
    Pause() {}
#endif
  };

  static __thread bool is_protected;

#ifndef NDEBUG
  VMProtect();
  ~VMProtect();

private:
  void* m_oldBase;
  VMRegState m_oldState;
#else
  VMProtect() {}
#endif
};

}}
