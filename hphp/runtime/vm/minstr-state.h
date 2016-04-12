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

#ifndef incl_HPHP_RUNTIME_VM_MINSTR_STATE_H_
#define incl_HPHP_RUNTIME_VM_MINSTR_STATE_H_

#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

/*
 * MInstrState contains VM registers used while executing member instructions.
 * It lives with the other VM registers in the RDS header, and is also saved and
 * restored with them when we reenter the VM.
 */
struct MInstrState {

  /*
   * This space is used for the return value of builtin functions that return by
   * reference, and for storing $this as the base for the BaseH bytecode,
   * without needing to acquire a reference to it.  Since we don't ever use the
   * two at the same time, it is okay to use a union.
   */
  union {
    TypedValue tvBuiltinReturn;
    TypedValue tvTempBase;
  };

  TypedValue tvRef;
  TypedValue tvRef2;
  TypedValue* base;
};

}

#endif
