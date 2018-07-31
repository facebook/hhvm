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

#ifndef incl_HPHP_RUNTIME_VM_MINSTR_STATE_H_
#define incl_HPHP_RUNTIME_VM_MINSTR_STATE_H_

#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

/*
 * Member instruction property state. If we're checking property type-hints,
 * this indicates whether the current base is a property. This is needed so we
 * can forbid the base promoting to a type which isn't allowed by the
 * type-hint. This state is only live in situations where we have not proven
 * that promotion cannot happen.
 *
 * We use the lower bit of the Class* to store whether this property is static
 * or not. Furthermore, in LOWPTR builds we can pack everything into 64-bits,
 * which allows for efficient initialization from the TC.
 */
struct MInstrPropState {
  MInstrPropState(): m_cls{0} {}
  MInstrPropState(const Class* cls, Slot slot, bool isStatic)
    : m_cls{
       static_cast<decltype(m_cls)>(
         reinterpret_cast<uintptr_t>(cls) | (isStatic ? 0x1 : 0x0)
       )
      }
    , m_slot{slot}
    {}

  const Class* getClass() const {
    return reinterpret_cast<const Class*>(
      m_cls & ~static_cast<decltype(m_cls)>(0x1)
    );
  }
  Slot getSlot() const { return m_slot; }
  bool isStatic() const { return m_cls & 0x1; }

  static constexpr ptrdiff_t slotOff() {
    return offsetof(MInstrPropState, m_slot);
  }
  static constexpr ptrdiff_t clsOff() {
    return offsetof(MInstrPropState, m_cls);
  }

  static constexpr size_t slotSize() {
    return sizeof(m_slot);
  }
  static constexpr size_t clsSize() {
    return sizeof(m_cls);
  }
private:
  LowPtr<const Class>::storage_type m_cls;
  Slot m_slot;
};

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

  TypedValue tvRef;
  TypedValue tvRef2;
  tv_lval base;

  // type-scan driven scanner
  TYPE_SCAN_IGNORE_FIELD(base);

  MInstrPropState propState;
};

}

#endif
