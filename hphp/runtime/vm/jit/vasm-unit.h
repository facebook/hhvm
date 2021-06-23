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

#include "hphp/runtime/base/datatype.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-data.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/data-block.h"
#include "hphp/util/immed.h"

#include <folly/sorted_vector_types.h>

#include <functional>
#include <type_traits>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * Block of Vinstrs, managed by Vunit.
 *
 * A Vblock, in addition to containing a Vinstr stream, also specifies where it
 * should be emitted to.
 */
struct Vblock {
  explicit Vblock(AreaIndex area_idx, uint64_t w)
    : area_idx(area_idx)
    , weight(w) {}

  AreaIndex area_idx;
  int frame{-1};
  int pending_frames{-1};
  uint64_t weight;
  jit::vector<Vinstr> code;
};

/*
 * Source operands for vcall/vinvoke instructions, packed into a struct for
 * convenience and to keep the instructions compact.
 */
struct VcallArgs {
  VregList args;
  VregList simdArgs;
  VregList stkArgs;
  VregList indRetArgs;

  // If the index of the associated VregList has an entry in the
  // Spills map, it means that Vreg in the VregList is the type field
  // of a TypedValue to be spilled. The Vreg in the map is the
  // matching data field. Since spilled TypedValues are rare, this
  // lets us avoid making the VregLists bigger.
  using Spills = folly::sorted_vector_map<size_t, Vreg>;
  Spills argSpills; // For "args"
  Spills stkSpills; // For "stkArgs"

  bool operator==(const VcallArgs& o) const {
    return
      std::tie(args, simdArgs, stkArgs, indRetArgs, argSpills, stkSpills) ==
      std::tie(o.args, o.simdArgs, o.stkArgs, o.indRetArgs,
               o.argSpills, o.stkSpills);
  }
  bool operator!=(const VcallArgs& o) const { return !(*this == o); }
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Vasm constant.
 *
 * Either a 1, 4, or 8 byte unsigned value, 8 byte double, or the disp32 part
 * of a thread-local address of an immutable constant that varies by thread.
 * Constants may also represent an undefined value, indicated by the isUndef
 * member.
 *
 * Also contains convenience constructors for various pointer and enum types.
 */
struct Vconst {
  enum Kind { Quad, Long, Byte, Double };

  using ullong = unsigned long long;
  using ulong = unsigned long;

  Vconst() : kind(Quad), val(0) {}
  explicit Vconst(Kind k)        : kind(k), isUndef(true), val(0) {}
  explicit Vconst(bool b)        : kind(Byte), val(b) {}
  explicit Vconst(uint8_t b)     : kind(Byte), val(b) {}
  explicit Vconst(int8_t b)      : Vconst(uint8_t(b)) {}
  explicit Vconst(uint32_t i)    : kind(Long), val(i) {}
  // For historical reasons Vconst(int) produces an 8-byte constant.
  explicit Vconst(int32_t i)     : Vconst(int64_t(i)) {}
  explicit Vconst(uint16_t)      = delete;
  explicit Vconst(int16_t)       = delete;
  explicit Vconst(ullong i)      : kind(Quad), val(i) {}
  explicit Vconst(long long i)   : Vconst(ullong(i)) {}
  explicit Vconst(ulong i)       : Vconst(ullong(i)) {}
  explicit Vconst(long i)        : Vconst(ullong(i)) {}
  explicit Vconst(const void* p) : Vconst(uintptr_t(p)) {}
  explicit Vconst(double d)      : kind(Double), doubleVal(d) {}

  template<
    class E,
    class Enable = typename std::enable_if<std::is_enum<E>::value>::type
  >
  explicit Vconst(E e) : Vconst(typename std::underlying_type<E>::type(e)) {}

  template<class R, class... Args>
  explicit Vconst(R (*fn)(Args...)) : Vconst(uintptr_t(fn)) {}

  bool operator==(Vconst other) const {
    return kind == other.kind && isUndef == other.isUndef && val == other.val;
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
  };
};

struct Vframe {
  Vframe(
    const Func* func,
    int32_t callOff,
    int parent,
    int cost,
    uint64_t entry_weight
  ) : func(func)
    , callOff(callOff)
    , parent(parent)
    , entry_weight(entry_weight)
    , inclusive_cost(cost)
    , exclusive_cost(cost)
  {}

  struct Section {
    size_t inclusive{0};
    size_t exclusive{0};
  };

  static constexpr int Top = -1;

  LowPtr<const Func> func;
  int32_t callOff{-1};
  int parent;

  uint64_t entry_weight;
  int inclusive_cost;
  int exclusive_cost;

  int num_inner_frames{0};

  jit::array<Section, kNumAreas> sections;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * A Vunit contains all the assets that make up a vasm compilation unit.  It is
 * responsible for allocating new blocks, Vregs, and tuples.
 */
struct Vunit {
  /*
   * Create a new block in the given area and weight, returning its id.
   */
  Vlabel makeBlock(AreaIndex area);
  Vlabel makeBlock(AreaIndex area, uint64_t weight);

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

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Make various Vunit-managed vasm structures.
   */
  Vreg makeReg() { return Vreg{next_vr++}; }
  Vaddr makeAddr() { return Vaddr{next_vaddr++}; }
  Vtuple makeTuple(VregList&& regs);
  Vtuple makeTuple(const VregList& regs);
  VcallArgsId makeVcallArgs(VcallArgs&& args);

  /*
   * Create or return a register representing the given constant value.
   */
  Vreg makeConst(Vconst);
  template<typename T> Vreg makeConst(T v) { return makeConst(Vconst{v}); }

  /*
   * Allocate a block of data to hold n objects of type T.
   *
   * Any instructions with VdataPtr members that point inside the buffer
   * returned by allocData() will automatically be fixed up during a relocation
   * pass immediately before final code emission.
   */
  template<typename T>
  T* allocData(size_t n = 1) {
    auto const size = sizeof(T) * n;
    dataBlocks.emplace_back();

    auto& block = dataBlocks.back();
    block.data.reset(new uint8_t[size]);
    block.size = size;
    block.align = alignof(T);
    return (T*)block.data.get();
  }

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Return true iff this Vunit needs register allocation before it can be
   * emitted, either because it uses virtual registers or contains instructions
   * that must be lowered by xls.
   */
  bool needsRegAlloc() const;
  /*
   * Return true iff this Vunit needs to have frames computed for
   * its blocks before being emitted.
   */
  bool needsFramesComputed() const;

  /////////////////////////////////////////////////////////////////////////////
  // Data members.

  unsigned next_vr{Vreg::V0};
  Vlabel entry;
  jit::vector<Vframe> frames;
  jit::vector<Vblock> blocks;
  jit::hash_map<Vconst,Vreg,Vconst::Hash> constToReg;
  jit::hash_map<size_t,Vconst> regToConst;
  jit::vector<VregList> tuples;
  jit::vector<VcallArgs> vcallArgs;
  jit::vector<VdataBlock> dataBlocks;
  unsigned next_vaddr{0};
  uint16_t cur_voff{0};  // current instruction index managed by Vout
  bool padding{false};
  bool profiling{false};
  Optional<TransContext> context;
  StructuredLogEntry* log_entry{nullptr};
  Annotations annotations;
};

inline tracing::Props traceProps(const Vunit& v) {
  if (v.context) return traceProps(*v.context);
  return tracing::Props{}.add("func_name", "(stub)");
}

///////////////////////////////////////////////////////////////////////////////
}}

