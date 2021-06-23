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

#include "hphp/runtime/vm/cti.h"
#include "hphp/util/asm-x64.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/vm/verifier/cfg.h"

namespace HPHP {
TRACE_SET_MOD(cti);
using jit::X64Assembler;
using jit::TCA;
using Verifier::funcInstrs;
using namespace jit::reg;

EntryStub g_enterCti;
CodeAddress g_exitCti;

namespace {

std::mutex g_mutex;
size_t bc_total;
thread_local std::vector<PcPair> tl_cti_cache;
__thread size_t cc_lookups, cc_misses;

// Number of bytes of x86 code for each opcode, used for interpolating
// the x86 pc for an unknown vmpc, and function size estimation.
// Must be the same for all instances of that opcode.
uint8_t g_cti_sizes[Op_count];

struct PatchTable {
  explicit PatchTable(Func* func, PC unitpc, TCA cti_base)
    : m_func(func), m_unitpc(unitpc), m_ctibase(cti_base)
  {}
  void addPC(PC pc, CodeAddress cti) {
    m_ctimap[pc - m_unitpc] = cti - m_ctibase;
  }
  void addPatch(PC pc, CodeAddress next_ip) {
    auto unit_pc = m_func->entry();
    auto targets = instrJumpTargets(unit_pc, pc - unit_pc);
    assert(targets.size() == 1);
    auto target_pc = unit_pc + targets[0];
    m_patches.push_back({next_ip, target_pc});
  }
  void finish(uint32_t size) {
    for (auto p : m_patches) {
      assert(checkPc(p.pc));
      auto cti = m_ctibase + m_ctimap[p.pc - m_unitpc];
      ((int32_t*)p.ip)[-1] = cti - p.ip;
    }
    auto& params = m_func->params();
    auto& bytecode = cti_code();
    for (int i = 0, n = params.size(); i < n; ++i) {
      if (params[i].hasDefaultValue()) {
        assert(checkPc(m_unitpc + params[i].funcletOff));
        auto cti = m_ctibase + m_ctimap[params[i].funcletOff];
        m_func->setCtiFunclet(i, cti - bytecode.base());
      }
    }
    m_func->setCtiEntry(m_ctibase - bytecode.base(), size);
  }
private:
  bool checkPc(PC pc) const {
    return m_ctimap.count(pc - m_unitpc) != 0;
  }
private:
  boost::container::flat_map<Offset,uint32_t> m_ctimap;
  Func* m_func;
  const PC m_unitpc;
  const TCA m_ctibase;
  std::vector<PcPair> m_patches;
};

TCA lookup_miss(const Func* func, Offset cti_entry, size_t h, PC misspc) {
  cc_misses++;
  auto cti = cti_code().base() + cti_entry;
  for (auto instrs = funcInstrs(func); !instrs.empty();) {
    auto pc = instrs.popFront();
    if (pc == misspc) {
      tl_cti_cache[h] = {cti, misspc};
      return cti;
    }
    cti += g_cti_sizes[(int)peek_op(pc)];
  }
  not_reached();
}

// calculate cti code size for func, or return 0 if we can't predict it yet.
size_t compute_size(const Func* func) {
  size_t size = 0;
  for (auto instrs = funcInstrs(func); !instrs.empty();) {
    auto pc = instrs.popFront();
    auto op = peek_op(pc);
    if (!g_cti_sizes[(int)op]) return 0;
    size += g_cti_sizes[(int)op];
  }
  return size;
}

inline bool isNop(Op opcode) {
  if (!RuntimeOption::RepoAuthoritative) return false;
  return opcode == OpNop ||
         opcode == OpEntryNop ||
         opcode == OpCGetCUNop ||
         opcode == OpUGetCUNop ||
         (!debug && isTypeAssert(opcode)) ||
         opcode == OpBreakTraceHint;
}

auto const pc_arg   = rdx;  // passed & returned by all opcodes
auto const next_ip  = rax;  // returned by opcodes with unpredictable targets
auto const next_reg = rbx;  // for conditional branches
auto const nextpc_arg = rdi; // for predicted calls
auto const tl_reg = r12;
auto const modes_reg = r13d;
auto const ra_reg = r14;

}

Offset compile_cti(Func* func, PC unitpc) {
  std::lock_guard<std::mutex> lock(g_mutex);
  auto cti_entry = func->ctiEntry();
  if (cti_entry) return cti_entry; // we lost the compile race
  auto cti_size = compute_size(func);
  auto mem = cti_size ? (TCA) cti_code().allocInner(cti_size) :
             nullptr;
  Optional<CodeBlock> inner_block;
  if (mem) {
    inner_block.emplace();
    inner_block->init(mem, cti_size, "");
  }
  auto cti_table = RuntimeOption::RepoAuthoritative ? cti_ops : ctid_ops;
  X64Assembler a{mem ? *inner_block : cti_code()};
  auto cti_base = a.frontier();
  PatchTable patches(func, unitpc, cti_base);
  for (auto instrs = funcInstrs(func); !instrs.empty(); ) {
    auto pc = instrs.popFront();
    auto ip = a.frontier();
    patches.addPC(pc, ip);
    auto op = peek_op(pc);
    auto cti = cti_table[(int)op];
    if (isNop(op)) {
      // compile op as just one instruction: addq size(op), vmpc
      auto bc_size = instrLen(pc);
      if (bc_size == 1) {
        a.incq  (pc_arg);
      } else {
        a.addq  (bc_size, pc_arg);
      }
      if (debug) {
        a.storeq(pc_arg, r12[rds::kVmpcOff]);
      }
    } else if (isSimple(op) || isThrow(op)) {
      a.  call  (cti);
    } else if (isBranch(op)) {
      a.  lea   (pc_arg[instrLen(pc)], next_reg);
      a.  call  (cti);
      a.  cmpq  (pc_arg, next_reg);
      a.  jne   (cti_base);
      patches.addPatch(pc, a.frontier());
    } else if (isUnconditionalJmp(op)) {
      a.  call  (cti);
      a.  jmp   (cti_base);
      patches.addPatch(pc, a.frontier());
    } else {
      // these ops jump to unpredictable targets by setting pc, and some
      // can halt by returning g_haltBytecode or g_pauseBytecode in rax.
      if (isFCall(op)) {
        // It's cheaper to pass next_pc as an arg than to calculate it in the
        // stubs that need it, since some of these opcodes have IVA params
        // which vary in size by callsite.
        //
        // For FCall, instrLen(pc) can use either 1 or 4 byte encoding,
        // triggering the g_cti_sizes assert below. Force 4-byte encoding.
        a.lea   (pc_arg[1000], nextpc_arg);
        ((int32_t*)a.frontier())[-1] = instrLen(pc);
      }
      a.  call  (cti);
      DEBUG_ONLY auto after = a.frontier();
      a.  jmp   (next_ip);
      assert(a.frontier() - after == kCtiIndirectJmpSize);
    }
    auto size = a.frontier() - ip;
    if (!g_cti_sizes[(int)op]) {
      g_cti_sizes[(int)op] = size;
    } else {
      assert(size == g_cti_sizes[(int)op]);
    }
  }
  if (!cti_size) {
    cti_size = a.frontier() - cti_base;
  } else {
    assert(cti_size == a.frontier() - cti_base); // check calculate_size()
  }
  // patch jumps, update func with code addresses.
  patches.finish(cti_size);
  bc_total += func->bclen();
  TRACE(1, "cti %s entry %p size %d %lu total %lu %lu\n",
        func->fullName()->size() > 0 ? func->fullName()->data() : "\"\"",
        func->entry(),
        func->bclen(), cti_size,
        bc_total, a.used());
  TRACE(2, "cti lookups %lu misses %lu\n", cc_lookups, cc_misses);
  return cti_base - cti_code().base();
}

// Return the cti ip for the given hhbc pc in func.
TCA lookup_cti(const Func* func, Offset cti_entry, PC unitpc, PC pc) {
  assert(pc && unitpc);
  if (tl_cti_cache.empty()) {
    tl_cti_cache.resize(jit::CodeCache::ABytecodeSize >> 10);
    always_assert(tl_cti_cache.size() > 0 &&
                  (tl_cti_cache.size() & (tl_cti_cache.size()-1)) == 0);
  }
  cc_lookups++;
  auto h = hash_int64(int64_t(pc)) & (tl_cti_cache.size() - 1);
  auto& pcp = tl_cti_cache[h];
  if (pcp.pc == pc) return pcp.ip;
  return lookup_miss(func, cti_entry, h, pc);
}

void free_cti(Offset cti_entry, uint32_t cti_size) {
  auto& bytecode = cti_code();
  auto ctibase = bytecode.base() + cti_entry;
  bytecode.free(ctibase, cti_size);
}

void compile_cti_stubs() {
  auto& bc_section = cti_code();
  X64Assembler a{bc_section};

  // pc is passed/returned in rdx, but we don't access it here
  // g_enterCti(modes, {ip, pc}, rds::Header*)
  //            edi    rsi  rdx  rcx           r8, r9 unused
  g_enterCti = (EntryStub) a.frontier();
  a.push  (rbp);
  a.movq  (rsp, rbp);
  a.movq  (rcx, tl_reg);
  a.movl  (edi, modes_reg);
  a.lea   (rbp[-8], ra_reg);
  a.jmp   (rsi);
  a.ud2   (); // cpu hint that we didn't indirectly jump to here.

  // bytecode jumps back here to stop the interpreter, because either:
  // 1. a canHalt() instruction set pc=0 (exiting this level of vm nesting)
  // 2. a control-flow instruction was executed in dispatchBB() mode.
  g_exitCti = a.frontier();
  a.movq  (rdx, rax); // move retAddr from halt path to rax
  a.pop   (rbp);
  a.ret   ();
}

}
