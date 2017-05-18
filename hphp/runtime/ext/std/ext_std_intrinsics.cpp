/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/ext/std/ext_std_intrinsics.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(disable_inlining, const Variant& function) {
  ObjectData* obj = nullptr;
  HPHP::Class* cls = nullptr;
  StringData* invName = nullptr;
  const HPHP::Func* f = vm_decode_function(function, GetCallerFrame(),
                                           false, obj, cls, invName,
                                           DecodeFlags::LookupOnly);
  if (f == nullptr || f->isAbstract()) {
    raise_warning("disable_inlining(): undefined function");
    return;
  }

  jit::InliningDecider::forbidInliningOf(f);
}

void HHVM_FUNCTION(trigger_oom, bool oom) {
  if (oom) setSurpriseFlag(MemExceededFlag);
}

TypedValue HHVM_FUNCTION(launder_value, const Variant& val) {
  return tvReturn(val);
}

void StandardExtension::initIntrinsics() {
  if (!RuntimeOption::EnableIntrinsicsExtension) return;

  HHVM_FALIAS(__hhvm_intrinsics\\disable_inlining, disable_inlining);
  HHVM_FALIAS(__hhvm_intrinsics\\trigger_oom, trigger_oom);
  HHVM_FALIAS(__hhvm_intrinsics\\launder_value, launder_value);

  loadSystemlib("std_intrinsics");
}

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
