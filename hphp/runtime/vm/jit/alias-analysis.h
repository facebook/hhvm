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

#include <bitset>
#include <string>
#include <cstdint>

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/alias-class.h"
#include "hphp/runtime/vm/jit/cfg.h"

#include "hphp/util/sparse-id-containers.h"

namespace HPHP { namespace jit {

struct IRUnit;

//////////////////////////////////////////////////////////////////////

/*
 * Sets of abstract locations tracked by AliasAnalysis come in ALocBits.
 *
 * Right now we have a static maximum number of tracked locations---passes
 * using information from this module must be conservative about locations that
 * aren't assigned an id.  (E.g. via calls to may_alias in AliasAnalysis.)
 */
constexpr uint32_t kMaxTrackedALocs = 256;
using ALocBits = std::bitset<kMaxTrackedALocs>;

//////////////////////////////////////////////////////////////////////

struct ALocMeta {
  uint32_t index;      // id assigned to this location
  ALocBits conflicts;  // flow-insensitive may-alias set, without self
};

/*
 * Information about various abstract locations an IR unit may be concerned
 * with.  See collect_aliases.
 */
struct AliasAnalysis {
  explicit AliasAnalysis(const IRUnit&);

  AliasAnalysis(const AliasAnalysis&) = delete;
  AliasAnalysis(AliasAnalysis&&) = default;
  AliasAnalysis& operator=(const AliasAnalysis&) = delete;
  AliasAnalysis& operator=(AliasAnalysis&&) = default;

  /*
   * Bidirectional map from alias classes to metadata about that abstract
   * memory location, primarily an assigned id.  There is also an inverse map
   * from id to the metadata structure.
   *
   * Two different alias classes in this map should not alias each other.  If
   * an alias class contains multiple locations (e.g., a range of stack slots,
   * multiple frame locals), we need to use multiple bits in this map.
   *
   * The keyed locations in this map take their canonical form.  You should use
   * canonicalize before doing lookups.
   */
  jit::hash_map<AliasClass,ALocMeta,AliasClass::Hash> locations;
  jit::vector<ALocMeta> locations_inv;

  using LocationMap = jit::hash_map<AliasClass,ALocBits,AliasClass::Hash>;

  /*
   * If an AStack covers multiple locations, it will have an entry in this
   * map. It is OK if not all locations covered by the AStack are tracked. We
   * only store the tracked subset here.
   */
  LocationMap stack_ranges;

  /*
   * Similar to `stack_ranges', if an ALocal covers multiple locations, it will
   * have an entry in this map.
   */
  LocationMap loc_expand_map;

  LocationMap iter_expand_map;

  /*
   * Short-hand to find an alias class in the locations map, or get std::nullopt
   * if the alias class wasn't assigned an ALocMeta structure.
   */
  Optional<ALocMeta> find(AliasClass) const;

  /*
   * Several larger sets of locations, we have a set of all the ids assigned to
   * properties, elemIs, frame locals, etc.  This is used by may_alias below.
   */
  ALocBits all_props;
  ALocBits all_elemIs;
  ALocBits all_elemSs;
  ALocBits all_local;
  ALocBits all_stack;
  ALocBits all_rds;
  ALocBits all_iter;
  ALocBits all_fcontext;
  ALocBits all_ffunc;
  ALocBits all_fmeta;

  /*
   * Return the number of distinct locations we're tracking by id
   */
  size_t count() const { return locations_inv.size(); }

  /*
   * Return a set of locations that we've assigned ids to that may alias a
   * given AliasClass.  Note that (as usual) memory locations we haven't
   * assigned bits to may still be affected, but this module only reports
   * effects on locations assigned bits.
   *
   * This function may conservatively return more bits than actually may
   * overlap `acls'.
   */
  ALocBits may_alias(AliasClass acls) const;

  /*
   * Return a set of locations that we've assigned ids to that are definitely
   * contained in `acls'.  This function may conservatively return a smaller
   * set of bits: every bit that is set in the returned ALocBits is contained
   * in `acls', but there may be locations contained in `acls' that don't have
   * a bit set in the returned vector. As a consequence of this, any caller of
   * expand() must produce correct (but potentially suboptimal) results if
   * expand is hardcoded to always return an empty bitset.
   *
   * This should generally be used with AliasClasses that are exhaustive,
   * must-style information.  That is, AliasClasses that should be interpreted
   * as referring to every point they contain.  Right now, the primary example
   * of that sort of AliasClasses is the class of locations in `kills' sets in
   * certain memory effects structs: these sets indicate every location inside
   * the class is affected.
   *
   * Right now, this function will work for specific AliasClasses we've
   * assigned ids---for larger classes, it only supports stack ranges observed
   * during alias collection, ALocalAny, and some cases of unions of those---if
   * you need more to work, the implementation will need some improvements.
   */
  ALocBits expand(AliasClass acls) const;

  /*
   * Sets of alias classes that are used by expand().
   */
  jit::hash_map<AliasClass,ALocBits,AliasClass::Hash> stk_expand_map;
};

//////////////////////////////////////////////////////////////////////

/*
 * Perform a flow-insensitive analysis on the supplied blocks, collecting
 * possibly distinct abstract memory locations that are explicitly referenced,
 * and assigning them ids and may-alias sets.  Only certain types of locations
 * are assigned ids, based on whether it maps to an AliasClass that that passes
 * can currently plausibly optimize (because it is sufficiently concrete).
 *
 * Note: it is fine to continue to reuse one AliasAnalysis structure after
 * mutating the IR, because the information it contains is both
 * flow-insensitive and conservative.  That is: if you change the IR to
 * reference new abstract memory locations, the fact that AliasAnalysis didn't
 * know about it won't invalidate the things it knows about (and the general
 * may_alias function will still work on the new location).  Similarly,
 * removing references to locations or changing control flow won't invalidate
 * anything.
 */
AliasAnalysis collect_aliases(const IRUnit&, const BlockList&);

//////////////////////////////////////////////////////////////////////

/*
 * Produce summary information for debug printing.
 */
std::string show(const AliasAnalysis&);
std::string show(ALocBits);

//////////////////////////////////////////////////////////////////////

}}

