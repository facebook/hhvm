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

#include "hphp/runtime/base/tv-val.h"
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

  // The JIT passes &tvBuiltinReturn::m_data to builtins returning
  // Array/Object/String, which perform RVO in C++, thus writing valid
  // pointers without updating m_type, preventing the GC from scanning
  // the pointer. But conservative scanning doesn't really hurt here
  // (given that the pointer is also passed into a C++ function), and
  // it allows us to keep rds::Header below 128 bytes.
  TYPE_SCAN_CONSERVATIVE_FIELD(tvBuiltinReturn);

  tv_lval base;

  // type-scan driven scanner
  TYPE_SCAN_IGNORE_FIELD(base);
};

}

