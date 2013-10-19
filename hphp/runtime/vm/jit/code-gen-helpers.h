/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP { namespace JIT {

struct CppCall {
  template<class Ret, class... Args>
  explicit CppCall(Ret (*pfun)(Args...))
    : m_kind(Direct)
    , m_fptr(reinterpret_cast<void*>(pfun))
  {}

  explicit CppCall(void* p)
    : m_kind(Direct)
    , m_fptr(p)
  {}

  explicit CppCall(int off) : m_kind(Virtual), m_offset(off) {}

  CppCall(CppCall const&) = default;

  bool isDirect()  const { return m_kind == Direct;  }
  bool isVirtual() const { return m_kind == Virtual; }

  const void* getAddress() const { return m_fptr; }
  int         getOffset()  const { return m_offset; }

 private:
  enum { Direct, Virtual } m_kind;
  union {
    void* m_fptr;
    int   m_offset;
  };
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
