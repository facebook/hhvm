/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_CODEGENHELPERS_H_
#define incl_HPHP_VM_CODEGENHELPERS_H_

#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/util/abi-cxx.h"

namespace HPHP { namespace JIT {

/*
 * Information about how to make different sorts of calls from the JIT
 * to C++ code.
 */
struct CppCall {
  enum class Kind {
    /*
     * Normal direct calls to a known address.  We don't distinguish
     * between direct calls to methods or direct calls to free
     * functions, with the assumption they have the same calling
     * convention.
     */
    Direct,
    /*
     * Limited support for C++ virtual function calls (simple single
     * inheritance situations---we're not supporting things like this
     * pointer adjustments, etc).
     */
    Virtual,
    /*
     * Calls through a register.
     */
    Indirect,
    /*
     * Call through the "rotated" ArrayData vtable.  This is used to
     * call ArrayData apis by loading a function pointer out of
     * g_array_funcs at an offset for the supplied function, indexing
     * by array kind.
     */
    ArrayVirt,
  };

  CppCall() = delete;
  CppCall(const CppCall&) = default;

  /*
   * Create a CppCall that represents a direct call to a non-member
   * function.
   */
  template<class Ret, class... Args>
  static CppCall direct(Ret (*pfun)(Args...)) {
    return CppCall { Kind::Direct, reinterpret_cast<void*>(pfun) };
  }

  /*
   * Create a CppCall that targets a /non-virtual/ C++ instance
   * method.
   *
   * Note: this uses ABI-specific tricks to get the address of the
   * code out of the pointer-to-member, and the fact that we just
   * treat it as the same Kind after this stuff requires that the
   * calling convention for member functions and non-member functions
   * is the same.
   */
  template<class Ret, class Cls, class... Args>
  static CppCall method(Ret (Cls::*fp)(Args...)) {
    return CppCall { Kind::Direct, getMethodPtr(fp) };
  }
  template<class Ret, class Cls, class... Args>
  static CppCall method(Ret (Cls::*fp)(Args...) const) {
    return CppCall { Kind::Direct, getMethodPtr(fp) };
  }

  /*
   * Call an array function.  Takes a pointer to an array of function
   * pointers to use for the particular entry point.  For example,
   *
   *   CppCall::array(&g_array_funcs.nvGetInt)
   *
   * The call mechanism assumes that the first argument to the
   * function is an ArrayData*, and loads the kind from there.
   */
  template<class Ret, class... Args>
  static CppCall array(Ret (*const (*p)[ArrayData::kNumKinds])(Args...)) {
    const void* vp = p;
    return CppCall { Kind::ArrayVirt, const_cast<void*>(vp) };
  }

  /*
   * Create a CppCall that represents a call to a virtual function.
   */
  static CppCall virt(int vtableOffset) {
    return CppCall { Kind::Virtual, vtableOffset };
  }

  /*
   * Indirect call through a register.
   */
  static CppCall indirect(PhysReg r) {
    return CppCall { Kind::Indirect, r };
  }

  /*
   * Return the type tag.
   */
  Kind kind() const { return m_kind; }

  /*
   * The point of this class is to discriminate these three fields are
   * mutually exclusive, depending on the kind.
   */
  void* address() const {
    assert(kind() == Kind::Direct);
    return m_u.fptr;
  }
  int vtableOffset() const {
    assert(m_kind == Kind::Virtual);
    return m_u.vtableOffset;
  }
  PhysReg reg() const {
    assert(m_kind == Kind::Indirect);
    return m_u.reg;
  }
  void* arrayTable() const {
    assert(kind() == Kind::ArrayVirt);
    return m_u.fptr;
  }

  /*
   * Change the target register for an indirect call.
   *
   * Pre: kind() == Kind::Indirect
   */
  void updateCallIndirect(PhysReg reg) {
    assert(m_kind == Kind::Indirect);
    m_u.reg = reg;
  }

private:
  union U {
    /* implicit */ U(void* fptr)       : fptr(fptr) {}
    /* implicit */ U(int vtableOffset) : vtableOffset(vtableOffset) {}
    /* implicit */ U(PhysReg reg)      : reg(reg) {}

    void* fptr;
    int vtableOffset;
    PhysReg reg;
  };

private:
  CppCall(Kind k, U u) : m_kind(k), m_u(u) {}

private:
  Kind m_kind;
  U m_u;
};

/*
 * SaveFP uses rVmFp, as usual. SavePC requires the caller to have
 * placed the PC offset of the instruction about to be executed in
 * rdi.
 */
enum class RegSaveFlags {
  None = 0,
  SaveFP = 1,
  SavePC = 2
};
inline RegSaveFlags operator|(const RegSaveFlags& l, const RegSaveFlags& r) {
  return RegSaveFlags(int(r) | int(l));
}
inline RegSaveFlags operator&(const RegSaveFlags& l, const RegSaveFlags& r) {
  return RegSaveFlags(int(r) & int(l));
}
inline RegSaveFlags operator~(const RegSaveFlags& f) {
  return RegSaveFlags(~int(f));
}

}}

#endif
