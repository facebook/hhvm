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

#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/abi-cxx.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

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

template <class T, class F>
Vreg cond(Vout& v, ConditionCode cc, Vreg sf, Vreg dst, T t, F f) {
  auto fblock = v.makeBlock();
  auto tblock = v.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, sf, {fblock, tblock}};
  v = tblock;
  auto treg = t(v);
  v << phijmp{done, v.makeTuple(VregList{treg})};
  v = fblock;
  auto freg = f(v);
  v << phijmp{done, v.makeTuple(VregList{freg})};
  v = done;
  v << phidef{v.makeTuple(VregList{dst})};
  return dst;
}

//////////////////////////////////////////////////////////////////////

/*
 * Information about an array key (this represents however much we know about
 * whether the key is going to behave like an integer or a string).
 */
struct ArrayKeyInfo {
  int64_t convertedInt{0};
  KeyType type{KeyType::Any};

  // If true, the string could dynamically contain an integer-like string,
  // which needs to be checked.
  bool checkForInt{false};

  // If true, useKey is an integer constant we've materialized, by converting a
  // string `key' that was strictly an integer.
  bool converted{false};
};

inline ArrayKeyInfo checkStrictlyInteger(Type key) {
  auto ret = ArrayKeyInfo{};

  if (key <= Type::Int) {
    ret.type = KeyType::Int;
    return ret;
  }
  assertx(key <= Type::Str);
  ret.type = KeyType::Str;
  if (key.hasConstVal()) {
    int64_t i;
    if (key.strVal()->isStrictlyInteger(i)) {
      ret.converted    = true;
      ret.type         = KeyType::Int;
      ret.convertedInt = i;
    }
  } else {
    ret.checkForInt = true;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}

#endif
