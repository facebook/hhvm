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

#ifndef incl_HPHP_JIT_VASM_H_
#define incl_HPHP_JIT_VASM_H_

#include "hphp/runtime/base/rds.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/containers.h"

#include "hphp/util/safe-cast.h"

#include <folly/Range.h>
#include <iosfwd>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Vunit;
struct Vinstr;
struct Vblock;
struct Vreg;
struct Abi;

///////////////////////////////////////////////////////////////////////////////

/*
 * If Trace::moduleEnabled(Trace::llvm) || RuntimeOption::EvalJitLLVMCounters,
 * these two RDS values are used to count the number of bytecodes executed by
 * code emitted from their respective backends.
 */
extern RDS::Link<uint64_t> g_bytecodesLLVM;
extern RDS::Link<uint64_t> g_bytecodesVasm;

///////////////////////////////////////////////////////////////////////////////

#define DECLARE_VNUM(Vnum, check)                         \
struct Vnum {                                             \
  Vnum() {}                                               \
  explicit Vnum(size_t n) : n(safe_cast<unsigned>(n)) {}  \
                                                          \
  /* implicit */ operator size_t() const {                \
    if (check) assert(n != 0xffffffff);                   \
    return n;                                             \
  }                                                       \
                                                          \
private:                                                  \
  unsigned n{0xffffffff};                                 \
}

/*
 * Vlabel wraps a block number.
 */
DECLARE_VNUM(Vlabel, true);

/*
 * Vpoint is a handle to record or retreive a code address.
 */
DECLARE_VNUM(Vpoint, false);

/*
 * Vtuple is an index to a tuple in Vunit::tuples.
 */
DECLARE_VNUM(Vtuple, true);

/*
 * VcallArgsId is an index to a VcallArgs in Vunit::vcallArgs.
 */
DECLARE_VNUM(VcallArgsId, true);

#undef DECLARE_VNUM

/*
 * Vreg discriminator.
 */
enum class VregKind : uint8_t { Any, Gpr, Simd, Sf };

///////////////////////////////////////////////////////////////////////////////

/*
 * Assert invariants on a Vunit.
 */
bool check(Vunit&);

/*
 * Check that each block has exactly one terminal instruction at the end.
 */
bool checkBlockEnd(Vunit& v, Vlabel b);

/*
 * Passes.
 */
void allocateRegisters(Vunit&, const Abi&);
void fuseBranches(Vunit&);
void optimizeExits(Vunit&);
void optimizeJmps(Vunit&);
void removeDeadCode(Vunit&);
void removeTrivialNops(Vunit&);
template<typename Folder> void foldImms(Vunit&);

///////////////////////////////////////////////////////////////////////////////

/*
 * Get the successors of a block or instruction.  If given a non-const
 * reference, the resulting Range will allow mutation of the Vlabels.
 */
folly::Range<Vlabel*> succs(Vinstr& inst);
folly::Range<Vlabel*> succs(Vblock& block);
folly::Range<const Vlabel*> succs(const Vinstr& inst);
folly::Range<const Vlabel*> succs(const Vblock& block);

/*
 * Sort blocks in reverse-postorder starting from `unit.entry'.
 */
jit::vector<Vlabel> sortBlocks(const Vunit& unit);

/*
 * Group blocks into main, cold, and frozen while preserving relative order
 * with each section.
 */
jit::vector<Vlabel> layoutBlocks(const Vunit& unit);

///////////////////////////////////////////////////////////////////////////////
}}

#endif
