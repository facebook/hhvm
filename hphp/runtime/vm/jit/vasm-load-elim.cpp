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

/*
 * Vasm-level load elimination: cross-block load CSE, store-to-load forwarding,
 * and precise alias-based kills. See eliminateRedundantLoads() below.
 *
 * This pass is wired into arm::optimize() only, and is generally not worthwhile
 * on x86:
 *
 *  - The motivating redundancy is the IncRef/DecRef refcount double-load. ARM
 *    has no compare-to-memory or read-modify-write decrement instructions, so
 *    cmplim/declm are lowered to load/op/store and the refcount is preloaded
 *    into a register (see irlower-refcount.cpp) — producing redundant loads to
 *    eliminate. On x86, cmplim and declm are single CISC instructions that
 *    micro-fuse and operate directly on memory, so there is no preloaded
 *    refcount and no redundant load in that path.
 *  - More broadly, x86 folds most loads into their consuming instruction
 *    (foldable_load in vasm-simplify.cpp), so few explicit load vinstrs survive
 *    to be redundant. ARM is RISC: every access is an explicit load/store, so
 *    the redundant-load and store-to-load-forwarding surface is much larger.
 *
 * Nothing here is ARM-specific for correctness: it forwards 8-byte loads with a
 * full-width copy, 4-byte loads with a zero-extending movl, and uses generic
 * Vptr aliasing. It does assume every Vptr carries a base register, which holds
 * on ARM; see the assertion in findAvailable().
 */

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/vasm-info.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/configs/jit.h"

#include <algorithm>
#include <iterator>

TRACE_SET_MOD(vasm)

namespace HPHP::jit {

namespace {

// Access widths this pass tracks. A load and a store at the same address but
// with different widths read and write different values, so the width is part
// of an entry's identity and must match exactly for forwarding.
//
// Only Long (loadl/storel) and Quad (load/store) are tracked today. Adding a
// width means adding the instruction that reproduces its exact register
// footprint to makeForward() below — the byte widths in particular are not
// interchangeable (loadb defines a Vreg8, loadzbq zero-extends, loadsbq
// sign-extends, loadtqb truncates), so each needs its own forward and its own
// test before being tracked here.
// Width alone would not even key them apart: loadb/loadzbq/loadsbq share an
// address and width, and loadtqb's Vptr64 keys as Quad next to plain load.
bool isTrackedWidth(Width w) {
  return w == Width::Long || w == Width::Quad;
}

uint8_t widthBytes(Width w) {
  switch (w) {
    case Width::Byte: return 1;
    case Width::Word: return 2;
    case Width::Long: return 4;
    case Width::Quad: return 8;
    default: break;
  }
  always_assert_flog(false, "load-elim: unsupported access width {}", show(w));
}

// Overwrite `slot` with the instruction that forwards `src` into `dst` while
// reproducing the eliminated load's full register footprint. Assigning the
// instruction (rather than a whole Vinstr) preserves the slot's origin, voff
// and pos, which the per-opcode operator= is defined to keep.
//
// A loadl (and a storel source forwarded to one) defines the whole 64-bit
// register, zero-extending bits [32,64); a Vreg32 value may legally be read at
// full width by a later consumer (e.g. `b = (Vreg)a; testq imm64, b`), which
// passes checkWidths precisely because the defining loadl zero-extended. A
// 32-bit-semantics copy would leave the upper half holding whatever `src`
// carries, so such a wider read would see garbage. Forward 4-byte values with a
// zero-extending movl (matching loadl exactly) and use copy only for full-width
// 8-byte loads, where the whole register is defined.
void makeForward(Vinstr& slot, Width w, Vreg src, Vreg dst) {
  switch (w) {
    case Width::Long: slot = movl{src, dst}; return;
    case Width::Quad: slot = copy{src, dst}; return;
    default: break;
  }
  always_assert_flog(false, "load-elim: no forward for width {}", show(w));
}

struct AvailableLoad {
  Vptr ptr;
  Vreg reg;
  Width width;
};

using AvailableLoads = jit::vector<AvailableLoad>;

// A redundant load that, in verification mode (Cfg::Jit::ArmLoadElimVerify), is
// replaced by a straight-line (branchless) runtime check instead of being
// rewritten to a plain copy. The check reloads the real value, forwards
// `expected` into `dst` (as shipping would), and crashes if they disagree.
// `expected` is the value the pass would have forwarded; `dst` is the register
// the load defines. Sites are collected during the RPO walk and applied
// afterwards (insertLoadElimChecks), since inserting the check shifts the indices
// the walk relies on. The check adds no blocks and no branches — see
// insertLoadElimCheck for why that matters.
struct VerifySite {
  Vlabel b;
  size_t idx;    // index of the redundant load within block b at collection
  Vreg expected; // value available from a prior load/store
  Vreg dst;      // register the load defines
  Width width;
};

Vreg findAvailable(const AvailableLoads& available, const Vptr& ptr, Width w) {
  assertx(ptr.base.isValid() &&
          "cannot look up a load with an invalid base register");
  for (auto const& entry : available) {
    if (entry.width == w && entry.ptr == ptr) return entry.reg;
  }
  return Vreg{};
}

void addAvailable(AvailableLoads& available, Vptr ptr, Vreg reg, Width w) {
  assertx(reg.isValid() && "cannot track a load into an invalid register");
  for (auto& entry : available) {
    if (entry.width == w && entry.ptr == ptr) {
      entry.reg = reg;
      return;
    }
  }
  available.push_back({ptr, reg, w});
}

// Drop every available entry. Used for instructions that may write anywhere:
// calls (which can clobber arbitrary memory), killeffects, any memory writer
// whose location we cannot pin down (getMemOpAndSize size 0), and anything
// effectful we have not positively cleared (see preservesTrackedMemory).
void killClobbered(AvailableLoads& available) {
  available.clear();
}

// Precise alias-based kill: drop only entries whose memory range a store at
// (ptr, size) might overlap. Entries proven disjoint by memRangesDisjoint (same
// base/index/scale/seg, non-overlapping displacement) survive. Hack has no
// volatile stores, so reordering loads across a provably-disjoint store is safe.
void killAliasing(AvailableLoads& available, const Vptr& ptr, uint8_t size) {
  assertx(size != 0 && "killAliasing requires a known store size");
  available.erase(
    std::remove_if(available.begin(), available.end(),
      [&](const AvailableLoad& entry) {
        return !memRangesDisjoint(ptr, size, entry.ptr, widthBytes(entry.width));
      }),
    available.end()
  );
}

// For a register-source store that we track for forwarding, return the stored
// value register and its width; otherwise return an invalid register. We only
// forward widths that the load side tracks (Quad for `store`, Long for
// `storel`); other stores still kill aliasing entries but are not forwarded.
//
// Soundness of forwarding a `storel` source to a later `loadl`: forwarding only
// fires when widths match (findAvailable keys on width), so a 4-byte store can
// only be forwarded to a 4-byte load, and makeForward() emits the
// zero-extending movl that a bare copy would get wrong.
struct StoreValue {
  Vreg reg;
  Width width;
};

StoreValue forwardableStore(const Vinstr& inst) {
  switch (inst.op) {
    case Vinstr::store:  return {inst.store_.s, Width::Quad};
    case Vinstr::storel: return {inst.storel_.s, Width::Long};
    default:             return {Vreg{}, Width::None};
  }
}

// Drop every entry that depends on register r — whether r is the entry's base,
// its index, or the loaded/stored value itself. Used when r is redefined, so no
// stale entry keyed on the old value of r survives. Returns true if anything
// was dropped.
bool killRegDeps(AvailableLoads& available, Vreg r) {
  assertx(r.isValid() && "killRegDeps called with invalid register");
  auto const it = std::remove_if(available.begin(), available.end(),
    [&](const AvailableLoad& entry) {
      return entry.ptr.base == r || entry.ptr.index == r || entry.reg == r;
    });
  if (it == available.end()) return false;
  available.erase(it, available.end());
  return true;
}

// Kill entries invalidated by a register this instruction defines. A virtual
// dst is freshly defined — vasm is SSA for virtual registers and check() runs
// checkSSA on entry — so it cannot appear in any existing entry and the scan is
// dead work. Only a physical dst can alias something we already track. In debug
// builds we still scan, to prove the SSA assumption rather than rely on it.
void killDefinedReg(AvailableLoads& available, Vreg r) {
  if (r.isPhys()) {
    killRegDeps(available, r);
    return;
  }
  if (debug) {
    always_assert_flog(!killRegDeps(available, r),
                       "load-elim: virtual reg {} was already tracked; vasm "
                       "is supposed to be SSA for virtuals", show(r));
  }
}

// Kill entries invalidated by everything this instruction defines: its declared
// defs plus the physical registers it defines *implicitly*. visitDefs() sees
// only the declared ones. getEffects() is the only model we have for the rest —
// rsp for the whole push/pop family, rvmfp for pushvmfp, rax/rdx for cqo/idiv.
// Without it, e.g. `load [rsp+8] => v0; push v1; load [rsp+8] => v2` would
// forward v0 into v2 even though push moved rsp out from under the address.
void killDefs(const Vunit& unit, const Abi& abi, const Vinstr& inst,
              AvailableLoads& available) {
  // getEffects() hard-fails on vcall/vinvoke, and a call's implicit clobber set
  // is not something we want to reason about entry-by-entry anyway. Callers
  // must route calls to killClobbered() before getting here.
  assertx(!isCall(inst.op) &&
          "calls must be handled by killClobbered, not killDefs");

  visitDefs(unit, inst, [&](Vreg r) {
    if (r.isValid()) killDefinedReg(available, r);
  });

  RegSet uses, across, defs;
  getEffects(abi, inst, uses, across, defs);
  defs.forEach([&](PhysReg r) { killRegDeps(available, Vreg{r}); });
}

// Effectful instructions that provably do not write memory and do not
// invalidate a tracked address beyond the registers they define. Anything else
// effectful is treated as an unknown memory writer — writesMemory() is derived
// from the declared Vptr operands, so it misses instructions whose memory
// access is implicit (push/pushp/pushm write [rsp], popm/poppm write through a
// bare Vptr), and defaulting those to "harmless" is how a stale entry survives
// a write.
//
// Block-end instructions must be on this list: they are all effectful, and
// clearing at a block end would empty exitLoads[b] and defeat cross-block CSE
// entirely. phidef is likewise effectful, but it is just a register definition
// at the top of a merge block — clearing there would kill exactly the merges
// this pass exists to exploit.
bool preservesTrackedMemory(const Vinstr& inst) {
  if (isBlockEnd(inst)) {
    // Terminators transfer control and write no memory. The ones that do call
    // (vinvoke, tailcallstub, ...) are filtered out by isCall() earlier.
    assertx(!isCall(inst.op) && "calls must be handled before this point");
    return true;
  }
  switch (inst.op) {
    case Vinstr::phidef:
    // Pseudo-instructions: metadata for the unwinder, profiler and inliner.
    // They emit no code and touch no memory.
    case Vinstr::nothrow:
    case Vinstr::syncpoint:
    case Vinstr::landingpad:
    case Vinstr::inlinestart:
    case Vinstr::inlineend:
    case Vinstr::conjure:
      return true;
    default:
      return false;
  }
}

// Available loads entering a block = intersection of its predecessors' exit
// sets (an entry survives only if every predecessor makes the same value
// available at the same address+width). This is a deliberately single-pass,
// no-fixpoint dataflow: blocks are visited once in RPO, so a loop header's
// back-edge predecessor has not been processed yet and contributes an empty
// (bottom) exit set, intersecting away any loop-carried entry. That is safe
// (never over-claims availability) but pessimistic: we do not CSE loop-carried
// loads. Do not "optimize" this into reusing a back-edge's later-computed set
// without a real fixpoint, or forwarding could become unsound.
//
// TODO: when predecessors agree on the address and width but disagree on the
// register, the entry is dropped. Those could be merged by appending an operand
// to each predecessor's phijmp and to this block's phidef. That needs every
// predecessor to already carry a phijmp/phidef pair, and needs critical edges
// split — arm::optimize() has not run splitCriticalEdges() at this point. Worth
// sizing (count the address+width matches that differ only in register) after
// the dominating-value preference in tryEliminateLoad, which should shrink how
// often predecessors disagree in the first place.
AvailableLoads mergeAvailable(
    const jit::vector<AvailableLoads>& exitLoads,
    const PredVector::value_type& preds) {
  if (preds.empty()) return {};

  assertx(size_t{preds[0]} < exitLoads.size() &&
          "predecessor block index out of range");
  if (preds.size() == 1) return exitLoads[preds[0]];

  auto result = exitLoads[preds[0]];
  for (size_t p = 1; p < preds.size(); ++p) {
    assertx(size_t{preds[p]} < exitLoads.size() &&
            "predecessor block index out of range");
    auto const& other = exitLoads[preds[p]];
    result.erase(
      std::remove_if(result.begin(), result.end(),
        [&](const AvailableLoad& entry) {
          return findAvailable(other, entry.ptr, entry.width) != entry.reg;
        }),
      result.end()
    );
  }
  return result;
}

bool tryEliminateLoad(Vinstr& inst, Vptr src, Vreg dst, Width width,
                      AvailableLoads& available, Vlabel b, size_t i,
                      bool verify, jit::vector<VerifySite>& sites) {
  assertx(src.base.isValid() && "load must have a valid base register");
  assertx(isTrackedWidth(width) && "load-elim tracks Long and Quad only");

  // If the destination register is also part of the load's own address
  // computation, the load overwrites that address. Recording it would leave a
  // self-referential, stale entry, so we never make such a load available (we
  // still invalidate prior entries via killDefinedReg below). This can only
  // happen for physical registers, since a virtual (SSA) dest is freshly
  // defined and cannot equal an existing base/index.
  auto const destClobbersAddr =
    dst == src.base || (src.index.isValid() && dst == src.index);

  auto const prev = findAvailable(available, src, width);
  if (!prev.isValid()) {
    killDefinedReg(available, dst);
    if (!destClobbersAddr) {
      // The lookup above missed, and killDefinedReg only removes entries, so no
      // matching entry can exist — append directly instead of rescanning.
      assertx(!findAvailable(available, src, width).isValid() &&
              "miss path must not have a matching entry");
      available.push_back({src, dst, width});
    }
    return false;
  }

  if (verify) {
    // Verification mode: keep the load and record a site so a runtime check
    // (does the loaded value really equal the value we would have forwarded?)
    // can be spliced in after the walk. Availability bookkeeping is identical
    // to the rewrite path below: `dst` ends up holding the value at `src`
    // either way. The check itself turns the kept load into the shipping
    // forward and reloads into a scratch to compare — see insertLoadElimCheck.
    FTRACE(kVasmLoadElimLevel,
           "load-elim: B{} #{}: {} [{}] => {} kept; will verify against "
           "{} (Cfg::Jit::ArmLoadElimVerify)\n",
           size_t{b}, i, vinst_names[inst.op], show(src), show(dst),
           show(prev));
    sites.push_back({b, i, prev, dst, width});
  } else {
    FTRACE(kVasmLoadElimLevel,
           "load-elim: B{} #{}: {} [{}] => {} replaced with forward of {} "
           "(available from a prior {}-byte load or store)\n",
           size_t{b}, i, vinst_names[inst.op], show(src), show(dst),
           show(prev), widthBytes(width));
    makeForward(inst, width, prev, dst);
  }

  killDefinedReg(available, dst);

  // The value is now live in both `prev` and `dst`, and we keep tracking
  // `prev`: its definition dominates `dst`, so it is available on strictly more
  // paths. Re-keying the entry to `dst` inside one arm of a diamond guarantees
  // the entry dies at the merge, because mergeAvailable() only keeps entries
  // that map to the *identical* Vreg in every predecessor.
  //
  // The exception is a physical `prev`, which is liable to be clobbered; there
  // the virtual `dst` is the better long-lived representative.
  if (prev.isPhys() && !destClobbersAddr) {
    addAvailable(available, src, dst, width);
  }
  return true;
}

// Splice a *branchless* runtime correctness check in after the kept load
// described by `site`, entirely within block `site.b`. Crucially it adds no
// blocks and no branches, so the CFG — and therefore the phi/edge-copy
// resolution the register allocator performs at merge points — is byte-for-byte
// identical to the shipping build. An earlier implementation split the block and
// branched to a cold `trap`; that extra edge, when the load sat at a
// parallel-copy merge, changed edge-copy resolution and let an unrelated value
// be routed into a live register. A straight-line sequence cannot do that.
//
// The kept load is turned into the exact forward the shipping path emits, so
// `dst`'s value and register allocation match shipping precisely. Around it a
// straight-line check reloads the real memory value and faults iff the forward
// was unsound (i.e. the alias analysis let us forward a value the load did not
// actually read):
//
//     scratch = load/loadl [src]       // reload FIRST, before dst is clobbered
//     movl/copy expected => dst        // the forward (was: load [src] => dst)
//     sf      = cmp{q,l} dst, scratch  // Z set iff sound
//     poison  = ldimmq 1               // an unmapped address
//     safe    = lea [rsp - 8]          // dead, writable stack scratch
//     target  = cmovq NE, sf, safe, poison   // unsound -> poison, else safe
//     store scratch => [target]        // SIGSEGVs at this site iff unsound
//
// The reload must precede the forward: `dst` can alias `src`'s base or index
// register (destClobbersAddr in tryEliminateLoad, reachable for physical dsts
// such as a pointer chase `load [x0+8] => x0`). Forwarding first would overwrite
// that base and make the reload read the wrong address — a spurious mismatch.
// Reloading first reads the true address while the address regs are still live.
//
// `store` is effectful, so DCE keeps it; that in turn keeps target -> cmov ->
// sf -> cmp -> the reload alive (a bare load into a dead reg would be removed).
// On a sound forward the store scribbles a dead stack slot ([rsp - 8]) and is
// harmless; on an unsound one it dereferences address 1 and crashes at the
// offending site.
//
// The safe address is materialized into a vreg with `lea` rather than feeding
// physical `rsp` straight into the cmov: `sp` is register 31, which `csel` (the
// ARM lowering of cmov) reads as the zero register, so a raw rsp operand would
// select 0 on the sound path and fault there. `lea` is one of the few forms
// where reg 31 means sp, so it captures the real stack address.
//
// Forwarding `dst` in every case (rather than keeping the load and comparing
// `expected` against it) means the self-forward case (expected == dst) needs no
// special handling: `copy dst => dst` is a nop and the scratch reload still
// supplies the real value to compare.
void insertLoadElimCheck(Vunit& unit, const VerifySite& site) {
  assertx(site.idx < unit.blocks[site.b].code.size() &&
          "verify site index out of range");
  assertx(isTrackedWidth(site.width) && "verify site has an untracked width");

  // Sites are applied in descending index order (see insertLoadElimChecks), and
  // inserting after site.idx only shifts strictly-higher indices, so site.idx
  // still names the kept load. Assert it (of the recorded width): if a future
  // change to the apply order ever moved this index off its load, this fires
  // here rather than mis-placing the check.
  assertx((site.width == Width::Long
             ? unit.blocks[site.b].code[site.idx].op == Vinstr::loadl
             : unit.blocks[site.b].code[site.idx].op == Vinstr::load) &&
          "verify site no longer points at its kept load");

  auto& bcode = unit.blocks[site.b].code;
  auto const irctx = bcode[site.idx].irctx();

  // Address of the load being verified, captured before we overwrite the slot.
  // load_.s is Vptr64 and loadl_.s is Vptr32; widen both to a plain Vptr.
  Vptr src;
  if (site.width == Width::Quad) {
    src = bcode[site.idx].load_.s;
  } else {
    src = bcode[site.idx].loadl_.s;
  }

  auto const scratch = unit.makeReg();
  auto const sf = unit.makeReg();
  auto const poison = unit.makeReg();
  auto const safe = unit.makeReg();
  auto const target = unit.makeReg();

  // Build the whole straight-line check, then splice it in over the kept load.
  jit::vector<Vinstr> seq;

  // 1. Reload the real value first, while src's base/index registers are still
  //    intact (the forward below may overwrite dst, which can alias them). Only
  //    the low 32 bits are meaningful for a 4-byte load, so loadl/cmpl are the
  //    right widths there; load/cmpq for 8 bytes.
  // 2. Forward into dst via the same instruction shipping uses (makeForward).
  // 3. Compare the forwarded dst against the reload; equality is direction-
  //    independent, so operand order does not matter.
  if (site.width == Width::Quad) {
    seq.emplace_back(load{src, scratch}, irctx);
    seq.emplace_back(copy{site.expected, site.dst}, irctx);
    seq.emplace_back(cmpq{site.dst, scratch, sf}, irctx);
  } else {
    seq.emplace_back(loadl{src, scratch}, irctx);
    seq.emplace_back(movl{site.expected, site.dst}, irctx);
    seq.emplace_back(cmpl{site.dst, scratch, sf}, irctx);
  }
  // 4. Branchless fault: select the poison address on mismatch (CC_NE: Z == 0),
  //    else the dead writable stack slot, then store through it. Addresses are
  //    always 64-bit, so cmovq regardless of the load width.
  seq.emplace_back(ldimmq{1, poison}, irctx);
  seq.emplace_back(lea{rsp()[-8], safe}, irctx);
  seq.emplace_back(cmovq{CC_NE, sf, safe, poison, target}, irctx);
  seq.emplace_back(store{scratch, target[0]}, irctx);

  // Overwrite the kept load with the first instruction of the sequence and
  // insert the rest after it. This keeps the block a single straight line.
  bcode[site.idx] = seq[0];
  bcode.insert(bcode.begin() + site.idx + 1,
               std::make_move_iterator(seq.begin() + 1),
               std::make_move_iterator(seq.end()));
}

// Apply all collected verification sites. Within a single block, sites must be
// spliced in descending index order: inserting the check after the highest
// index only shifts strictly-higher indices, leaving every lower-index site at
// its original index. Sites in different blocks are independent. Sorting all
// sites by descending index gives a valid order because, per block, it is
// descending (cross-block order is irrelevant since each splice only mutates its
// own block).
void insertLoadElimChecks(Vunit& unit, jit::vector<VerifySite>& sites) {
  std::sort(sites.begin(), sites.end(),
            [](const VerifySite& a, const VerifySite& b) {
              return a.idx > b.idx;
            });
  for (auto const& site : sites) insertLoadElimCheck(unit, site);
}

}

void eliminateRedundantLoads(Vunit& unit, const Abi& abi) {
  assertx(check(unit) && "unit must pass validation before load elimination");

  auto changed = false;
  VpassTracer tracer{&unit, Trace::vasm, "vasm-load-elim", &changed};

  auto const rpo = sortBlocks(unit);
  if (rpo.empty()) return;

  auto const preds = computePreds(unit);

  // When set, redundant loads are kept and checked at runtime rather than
  // rewritten to copies; see VerifySite and insertLoadElimChecks. Read once so
  // the behavior is fixed for the whole pass.
  //
  // Only verify real Hack translations (unit.context present). Hand-written
  // vasm — the unique stubs and other Vauto-emitted code — has no context, runs
  // the register allocator with a fixed frame that cannot always absorb the
  // check's extra live registers (XLS aborts with a spill/`slots` failure), and
  // is not where the alias-analysis soundness we are validating actually
  // matters.
  auto const verify = Cfg::Jit::ArmLoadElimVerify && unit.context.has_value();
  jit::vector<VerifySite> sites;

  jit::vector<AvailableLoads> exitLoads(unit.blocks.size());

  for (auto const b : rpo) {
    auto available = mergeAvailable(exitLoads, preds[b]);

    auto& code = unit.blocks[b].code;
    for (size_t i = 0; i < code.size(); ++i) {
      auto& inst = code[i];

      if (inst.op == Vinstr::loadl) {
        if (tryEliminateLoad(inst, inst.loadl_.s, inst.loadl_.d, Width::Long,
                             available, b, i, verify, sites)) {
          changed = true;
        }
        continue;
      }

      if (inst.op == Vinstr::load) {
        if (tryEliminateLoad(inst, inst.load_.s, inst.load_.d, Width::Quad,
                             available, b, i, verify, sites)) {
          changed = true;
        }
        continue;
      }

      if (writesMemory(inst.op)) {
        // Calls and killeffects may write arbitrary memory; drop everything.
        // Note: callm has a single Vptr operand, so getMemOpAndSize would
        // report a precise location for it — but the callee can still clobber
        // anything, so calls must be handled here before the precise path.
        if (isCall(inst.op) || inst.op == Vinstr::killeffects) {
          killClobbered(available);
          continue;
        }

        auto const [ptr, size] = getMemOpAndSize(inst);
        if (size == 0) {
          // Non-simple writer (multi-memory operand, etc.): be conservative.
          killClobbered(available);
          continue;
        }

        killAliasing(available, ptr, size);
        killDefs(unit, abi, inst, available);

        // Store-to-load forwarding: make the stored value available so a later
        // load from the same (address, width) becomes a copy of it.
        auto const sv = forwardableStore(inst);
        if (sv.reg.isValid()) {
          assertx(widthBytes(sv.width) == size &&
                  "store forward width must match its access size");
          assertx(isTrackedWidth(sv.width) &&
                  "only Long and Quad stores are forwardable");
          FTRACE(kVasmLoadElimLevel,
                 "load-elim: B{} #{}: store {} => [{}] now available "
                 "for forwarding\n",
                 size_t{b}, i, show(sv.reg), show(ptr));
          addAvailable(available, ptr, sv.reg, sv.width);
        }
        continue;
      }

      // Neither a load nor a declared memory writer. Default-deny: anything
      // effectful that we have not positively cleared may write memory we
      // cannot see (push/pop/pushvmfp have no Vptr operand, so writesMemory()
      // misses them entirely).
      if (effectful(inst) && !preservesTrackedMemory(inst)) {
        killClobbered(available);
        continue;
      }

      killDefs(unit, abi, inst, available);
    }

    exitLoads[b] = std::move(available);
  }

  // Verification sites must be spliced in only after the RPO walk: each splice
  // shifts indices, which would corrupt the in-progress walk.
  if (!sites.empty()) {
    assertx(verify && "verify sites recorded without verification enabled");
    insertLoadElimChecks(unit, sites);
  }

  assertx(check(unit) && "unit must pass validation after load elimination");

  if (changed) {
    printUnit(kVasmLoadElimLevel, "after vasm-load-elim", unit);
  }
}

}
