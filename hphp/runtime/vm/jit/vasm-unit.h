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

#ifndef incl_HPHP_JIT_VASM_UNIT_H_
#define incl_HPHP_JIT_VASM_UNIT_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/immed.h"

#include <functional>
#include <type_traits>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * Block of Vinstrs, managed by Vunit.
 */
struct Vblock {
  explicit Vblock(AreaIndex area) : area(area) {}

  AreaIndex area;
  jit::vector<Vinstr> code;
};

/*
 * Vector of Vregs, for Vtuples and VcallArgs.
 */
using VregList = jit::vector<Vreg>;

/*
 * Source operands for vcall/vinvoke instructions, packed into a struct for
 * convenience and to keep the instructions compact.
 */
struct VcallArgs {
  VregList args;
  VregList simdArgs;
  VregList stkArgs;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Vasm constant.
 *
 * Either a 1, 4, or 8 byte unsigned value, 8 byte double, or the disp32 part
 * of a thread-local address of an immutable constant that varies by
 * thread. Constants may also represent an undefined value, indicated by the
 * isUndef member.
 */
struct Vconst {
  enum Kind { Quad, Long, Byte, Double, ThreadLocal };

  Vconst() : kind(Quad), val(0) {}
  explicit Vconst(Kind k) : kind(k), isUndef(true), val(0) {}
  explicit Vconst(bool b) : kind(Byte), val(b) {}
  explicit Vconst(uint8_t b) : kind(Byte), val(b) {}
  explicit Vconst(uint32_t i) : kind(Long), val(i) {}
  explicit Vconst(uint64_t i) : kind(Quad), val(i) {}
  explicit Vconst(double d) : kind(Double), doubleVal(d) {}
  explicit Vconst(Vptr tl) : kind(ThreadLocal), disp(tl.disp) {
    assertx(!tl.base.isValid() &&
           !tl.index.isValid() &&
           tl.seg == Vptr::FS);
  }

  bool operator==(Vconst other) const {
    return kind == other.kind &&
      ((isUndef && other.isUndef) || val == other.val);
  }

  struct Hash {
    size_t operator()(Vconst c) const {
      return
        std::hash<uint64_t>()(c.val) ^ std::hash<int>()(c.kind) ^ c.isUndef;
    }
  };

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

  Kind kind;
  bool isUndef{false};
  union {
    uint64_t val;
    double doubleVal;
    int64_t disp; // really, int32 offset from %fs
  };
};

///////////////////////////////////////////////////////////////////////////////

/*
 * A Vunit contains all the assets that make up a vasm compilation unit.  It is
 * responsible for allocating new blocks, Vregs, and tuples.
 */
struct Vunit {
  /*
   * Create a new block in the given area, returning its id.
   */
  Vlabel makeBlock(AreaIndex area);

  /*
   * Create a block intended to be used temporarily, as part of modifying
   * existing code.
   *
   * Although not necessary for correctness, the block may be freed with
   * freeScratchBlock when finished.
   */
  Vlabel makeScratchBlock();

  /*
   * Free a scratch block when finished with it.  There must be no references
   * to this block in reachable code.
   */
  void freeScratchBlock(Vlabel);

  /*
   * Make various Vunit-managed vasm structures.
   */
  Vreg makeReg() { return Vreg{next_vr++}; }
  Vtuple makeTuple(VregList&& regs);
  Vtuple makeTuple(const VregList& regs);
  VcallArgsId makeVcallArgs(VcallArgs&& args);

  Vreg makeConst(bool);
  Vreg makeConst(uint32_t);
  Vreg makeConst(uint64_t);
  Vreg makeConst(double);
  Vreg makeConst(Vptr);
  Vreg makeConst(const void* p) { return makeConst(uint64_t(p)); }
  Vreg makeConst(int64_t v) { return makeConst(uint64_t(v)); }
  Vreg makeConst(int32_t v) { return makeConst(int64_t(v)); }
  Vreg makeConst(DataType t) { return makeConst(uint64_t(t)); }
  Vreg makeConst(Immed64 v) { return makeConst(uint64_t(v.q())); }
  Vreg makeConst(Vconst::Kind k);

  template<class R, class... Args>
  Vreg makeConst(R (*fn)(Args...)) { return makeConst(CTCA(fn)); }

  template<class T>
  typename std::enable_if<std::is_integral<T>::value, Vreg>::type
  makeConst(T l) { return makeConst(uint64_t(l)); }

  /*
   * Return true iff this Vunit needs register allocation before it can be
   * emitted, either because it uses virtual registers or contains instructions
   * that must be lowered by xls.
   */
  bool needsRegAlloc() const;


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

  unsigned next_vr{Vreg::V0};
  unsigned next_point{0};
  Vlabel entry;
  jit::vector<Vblock> blocks;
  jit::hash_map<Vconst,Vreg,Vconst::Hash> constToReg;
  jit::hash_map<size_t,Vconst> regToConst;
  jit::vector<VregList> tuples;
  jit::vector<VcallArgs> vcallArgs;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif
