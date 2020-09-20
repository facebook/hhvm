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

#include "hphp/runtime/base/rds.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/containers.h"

#include "hphp/util/safe-cast.h"

#include <boost/dynamic_bitset.hpp>
#include <folly/Range.h>
#include <iosfwd>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Abi;
struct Vblock;
struct Vinstr;
struct Vreg;
struct Vunit;
struct Vtext;

///////////////////////////////////////////////////////////////////////////////

#define DECLARE_VNUM(Vnum, type, check, prefix)           \
struct Vnum {                                             \
  Vnum() {}                                               \
  explicit Vnum(size_t n) : n(safe_cast<type>(n)) {}      \
                                                          \
  /* implicit */ operator size_t() const {                \
    if (check) assertx(n != kInvalidId);                  \
    return n;                                             \
  }                                                       \
                                                          \
  bool isValid() const {                                  \
    return n != kInvalidId;                               \
  }                                                       \
                                                          \
  std::string toString() const {                          \
    if (n == kInvalidId) return prefix "?";               \
    return folly::to<std::string>(prefix, n);             \
  }                                                       \
                                                          \
private:                                                  \
  static constexpr type kInvalidId =                      \
    static_cast<type>(0xffffffff);                        \
  type n{kInvalidId};                                     \
}

/*
 * Vlabel wraps a block number.
 */
DECLARE_VNUM(Vlabel, uint32_t, true, "B");

/*
 * Vaddr is a reference to an address in the instruction stream.
 */
DECLARE_VNUM(Vaddr, uint32_t, false, "A");

/*
 * Vtuple is an index to a tuple in Vunit::tuples.
 */
DECLARE_VNUM(Vtuple, uint32_t, true, "T");

/*
 * VcallArgsId is an index to a VcallArgs in Vunit::vcallArgs.
 */
DECLARE_VNUM(VcallArgsId, uint32_t, true, "V");

#undef DECLARE_VNUM

///////////////////////////////////////////////////////////////////////////////

using VinstrId = unsigned int;
using MaybeVinstrId = folly::Optional<VinstrId>;

///////////////////////////////////////////////////////////////////////////////

/*
 * Assert invariants on a Vunit.
 */
bool check(Vunit& unit);

/*
 * Assert that Vreg widths match between defs and uses.
 *
 * This should only be run before any zero-extending or truncating copies get
 * reduced to regular copies---so, before simplify() or the various lowering
 * passes.
 */
bool checkWidths(Vunit& unit);

/*
 * Check that each block has exactly one terminal instruction at the end.
 */
bool checkBlockEnd(const Vunit& v, Vlabel b);

/*
 * Passes.
 */
void allocateRegistersWithXLS(Vunit&, const Abi&);
void allocateRegistersWithGraphColor(Vunit&, const Abi&);
void annotateSFUses(Vunit&);
void fuseBranches(Vunit&);
void optimizeCopies(Vunit&, const Abi&);
void optimizeExits(Vunit&, MaybeVinstrId = {});
void optimizeJmps(Vunit&, MaybeVinstrId = {});
void optimizePhis(Vunit&);
void removeDeadCode(Vunit&, MaybeVinstrId = {});
void removeTrivialNops(Vunit&);
void reuseImmq(Vunit&);
template<typename Folder> void foldImms(Vunit&);
void simplify(Vunit&);
void postRASimplify(Vunit&);
void sfPeepholes(Vunit&, const Abi&);
void fixupVmfpUses(Vunit& unit);

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
 * Make block weights more consistent by enforcing that the weight of each block
 * doesn't exceed the sums of the weights of its predecessors or its successors.
 */
void fixBlockWeights(Vunit& unit);

/*
 * Order blocks for lowering to machine code.  May use different layout
 * algorithms depending on the TransKind of `unit'.
 *
 * The output is guaranteed to be partitioned by code area.
 */
jit::vector<Vlabel> layoutBlocks(Vunit& unit);

/*
 * Return a bitset, keyed by Vlabel, indicating which blocks are targets of
 * backedges.
 */
boost::dynamic_bitset<> backedgeTargets(const Vunit& unit,
                                        const jit::vector<Vlabel>& rpoBlocks);

///////////////////////////////////////////////////////////////////////////////
}}

namespace std {
template<> struct hash<HPHP::jit::Vlabel> {
  size_t operator()(HPHP::jit::Vlabel l) const { return (size_t)l; }
};
}

///////////////////////////////////////////////////////////////////////////////

