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

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct AsmInfo;
struct Vtext;
struct Vunit;

///////////////////////////////////////////////////////////////////////////////

/*
 * State maintained by vasm_emit().
 */
struct Venv {
  /*
   * Patch data collected at emit-time for post-processing.
   */
  struct LabelPatch { CodeAddress instr; Vlabel target; };
  struct AddrPatch { CodeAddress instr; Vaddr target; };
  struct VaddrBind { Vaddr vaddr; Vlabel target; };
  struct LdBindRetAddrPatch {
    CodeAddress instr;
    SrcKey target;
    SBInvOffset spOff;
  };

  Venv(Vunit& unit, Vtext& text, CGMeta& meta);

  void record_inline_stack(TCA);

  Vunit& unit;
  Vtext& text;
  CGMeta& meta;

  CodeBlock* cb;

  Vlabel current{0};
  Vlabel next{0};

  uint32_t pending_frames{0}; // unpushed inlined frames
  int frame{-1};
  CodeAddress framestart;
  const IRInstruction* origin;

  jit::vector<CodeAddress> addrs;
  jit::vector<CodeAddress> vaddrs;
  jit::vector<VaddrBind> pending_vaddrs;
  jit::vector<AddrPatch> leas;
  jit::vector<LabelPatch> jmps, jccs;
  jit::vector<LabelPatch> catches;
  jit::vector<LdBindRetAddrPatch> ldbindretaddrs;
  jit::vector<std::pair<TCA,IStack>> stacks;
};

/*
 * Toplevel vasm assembly-emitter loop.
 *
 * This is an emit loop implementation which provides the two-phase emitter
 * pattern of (1) emitting all code to the TC; then (2) performing patches that
 * we track at emit-time.
 *
 * vasm_emit() creates and updates a Venv object, and delegates the work of
 * emitting single Vinstrs to the Vemit template class.  Vemit is expected to
 * update the various patch point structures on the Venv as needed.
 *
 * Vemit is expected to have the following interface:
 *
 * struct Vemit {
 *   Vemit(Venv&);
 *
 *   // One emit routine for each Vinstr type.
 *   template<class Inst> void emit(Inst& i);
 *
 *   // Add arch-specific trap instruction padding up to the end of the block.
 *   static void pad(CodeBlock&);
 *
 *   // Perform all the accumulated patches.
 *   static void patch(Venv&);
 * };
 */
template<class Vemit>
void vasm_emit(Vunit& u, Vtext& text, CGMeta& fixups,
               AsmInfo* asm_info);

///////////////////////////////////////////////////////////////////////////////

/*
 * Allocate memory to hold the given value and return a pointer to it.  If a
 * previous translation allocated the same literal, a pointer to that may be
 * returned instead.
 */
const uint64_t* alloc_literal(Venv& env, uint64_t val);

///////////////////////////////////////////////////////////////////////////////

/*
 * When the JIT is serializing profile data, keep track that the `callRetAddr'
 * corresponds to the outer-most function for which the current translation is
 * being created.
 */
void setCallFuncId(Venv& env, TCA callRetAddr);

///////////////////////////////////////////////////////////////////////////////
}}

#include "hphp/runtime/vm/jit/vasm-internal-inl.h"

