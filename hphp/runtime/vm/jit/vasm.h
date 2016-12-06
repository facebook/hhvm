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

#define DECLARE_VNUM(Vnum, check, prefix)                 \
struct Vnum {                                             \
  Vnum() {}                                               \
  explicit Vnum(size_t n) : n(safe_cast<uint32_t>(n)) {}  \
                                                          \
  /* implicit */ operator size_t() const {                \
    if (check) assertx(n != kInvalidId);                   \
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
  static constexpr uint32_t kInvalidId = 0xffffffff;      \
  uint32_t n{kInvalidId};                                 \
}

/*
 * Vlabel wraps a block number.
 */
DECLARE_VNUM(Vlabel, true, "B");

/*
 * Vtuple is an index to a tuple in Vunit::tuples.
 */
DECLARE_VNUM(Vtuple, true, "T");

/*
 * VcallArgsId is an index to a VcallArgs in Vunit::vcallArgs.
 */
DECLARE_VNUM(VcallArgsId, true, "V");

#undef DECLARE_VNUM

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
void allocateRegisters(Vunit&, const Abi&);
void fuseBranches(Vunit&);
void optimizeCopies(Vunit&, const Abi&);
void optimizeExits(Vunit&);
void optimizeJmps(Vunit&);
void optimizePhis(Vunit&);
void removeDeadCode(Vunit&);
void removeTrivialNops(Vunit&);
template<typename Folder> void foldImms(Vunit&);
void simplify(Vunit&);

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
 * Order blocks for lowering to machine code.  May use different layout
 * algorithms depending on the TransKind of `unit'.
 *
 * The output is guaranteed to be partitioned by area relative to `text'.  This
 * is almost the same as partitioning by AreaIndex, except we may interleave,
 * e.g., Main and Cold blocks in the same partition if their actual code areas
 * in `text' are the same.
 */
jit::vector<Vlabel> layoutBlocks(Vunit& unit, const Vtext& text);

/*
 * Return a bitset, keyed by Vlabel, indicating which blocks are targets of
 * backedges.
 */
boost::dynamic_bitset<> backedgeTargets(const Vunit& unit,
                                        const jit::vector<Vlabel>& rpoBlocks);

///////////////////////////////////////////////////////////////////////////////
}}

#endif
