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

#ifndef incl_HPHP_JIT_CALL_SPEC_H_
#define incl_HPHP_JIT_CALL_SPEC_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include <cstdint>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Information about how to make different sorts of calls from the JIT.
 */
struct CallSpec {
  enum class Kind : uint8_t {
    /*
     * Normal direct call to a known C++ function.
     *
     * We don't distinguish between our representations for direct calls to
     * free functions and those to member functions, with the assumption that
     * they have the same calling convention.
     */
    Direct,

    /*
     * Smashable direct call.
     *
     * Just like `Direct', except we ensure that the call target can be safely
     * mutated even in the presence of concurrent execution.
     */
    Smashable,

    /*
     * Call to an ArrayData vtable function.
     *
     * This is used to call ArrayData APIs by loading the appropriate function
     * table for the desired function family out of g_array_funcs, and indexing
     * into it with the ArrayKind of the passed ArrayData* argument.
     */
    ArrayVirt,

    /*
     * Call the appropriate destructor (i.e., release) function for the unitary
     * argument.
     *
     * A Vreg containing the data pointer's DataType is used to determine the
     * correct function to call.
     */
    Destructor,

    /*
     * Call a unique stub.
     *
     * Produce a call to JIT code which begins with a prologue{} Vinstr.
     *
     * TODO(#8425101): Make this true.
     */
    Stub,
  };

  CallSpec() = delete;
  CallSpec(const CallSpec&) = default;
  CallSpec& operator=(const CallSpec&) = default;

  /////////////////////////////////////////////////////////////////////////////
  // Static constructors.

  /*
   * A Direct call to the free C++ function `fp'.
   */
  template<class Ret, class... Args>
  static CallSpec direct(Ret (*fp)(Args...)) {
    return CallSpec { Kind::Direct, reinterpret_cast<void*>(fp) };
  }

  /*
   * A Direct call to the /non-virtual/ C++ instance method function `fp'.
   *
   * This uses ABI-specific tricks to get the address of the code out of the
   * pointer-to-member.  Additionally, we use the same Kind to represent both
   * free functions and member functions, which assumes that they have the same
   * calling convention.
   */
  template<class Ret, class Cls, class... Args>
  static CallSpec method(Ret (Cls::*fp)(Args...)) {
    return CallSpec { Kind::Direct, getMethodPtr(fp) };
  }
  template<class Ret, class Cls, class... Args>
  static CallSpec method(Ret (Cls::*fp)(Args...) const) {
    return CallSpec { Kind::Direct, getMethodPtr(fp) };
  }

  /*
   * A Smashable direct call to the free C++ function `fp'.
   */
  template<class Ret, class... Args>
  static CallSpec smashable(Ret (*fp)(Args...)) {
    return CallSpec { Kind::Smashable, reinterpret_cast<void*>(fp) };
  }

  /*
   * An ArrayVirt call, for the array function table `p'.
   *
   * Takes a pointer to an array of function pointers to use for the particular
   * entry point.  For example,
   *
   *   CallSpec::array(&g_array_funcs.nvGetInt)
   *
   * The call mechanism assumes that the first argument to the function is an
   * ArrayData*, and loads the kind from there.
   */
  template<class Ret, class... Args>
  static CallSpec array(Ret (*const (*p)[ArrayData::kNumKinds])(Args...)) {
    const void* vp = p;
    return CallSpec { Kind::ArrayVirt, const_cast<void*>(vp) };
  }

  /*
   * A Destructor call for the DataType in `r'.
   */
  static CallSpec destruct(Vreg r) {
    return CallSpec { Kind::Destructor, r };
  }

  /*
   * A Stub call to `addr'.
   */
  static CallSpec stub(TCA addr) {
    return CallSpec { Kind::Stub, addr };
  }

  /////////////////////////////////////////////////////////////////////////////
  // Accessors.

  /*
   * Return the type tag.
   */
  Kind kind() const { return m_kind; }

  /*
   * The address of a C++ function, for Direct or Smashable calls.
   */
  void* address() const {
    assertx(kind() == Kind::Direct ||
            kind() == Kind::Smashable);
    return m_u.fp;
  }

  /*
   * The table of array functions, for ArrayVirt calls.
   */
  void* arrayTable() const {
    assertx(kind() == Kind::ArrayVirt);
    return m_u.fp;
  }

  /*
   * The register containing the DataType, for Destructor calls.
   */
  Vreg reg() const {
    assertx(m_kind == Kind::Destructor);
    return m_u.reg;
  }

  /*
   * The address of the unique stub, for Stub calls.
   */
  TCA stubAddr() const {
    assertx(m_kind == Kind::Stub);
    return m_u.stub;
  }

  /////////////////////////////////////////////////////////////////////////////

private:
  union U {
    /* implicit */ U(void* fp) : fp(fp) {}
    /* implicit */ U(Vreg reg) : reg(reg) {}
    /* implicit */ U(TCA stub) : stub(stub) {}

    void* fp;
    Vreg reg;
    TCA stub;
  };

private:
  CallSpec(Kind k, U u) : m_kind(k), m_u(u) {}

private:
  Kind m_kind;
  U m_u;
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif
