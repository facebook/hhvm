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
#ifndef incl_HPHP_ALOCATION_ANALYSIS_H_
#define incl_HPHP_ALOCATION_ANALYSIS_H_

#include <bitset>
#include <string>
#include <cstdint>

#include "hphp/util/sparse-id-containers.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/abstract-location.h"
#include "hphp/runtime/vm/jit/cfg.h"

namespace HPHP { namespace jit {

struct IRUnit;

//////////////////////////////////////////////////////////////////////

/*
 * Sets of abstract locations.
 *
 * Right now we have a static maximum number of tracked locations---passes
 * using information from this module must be conservative about locations that
 * aren't assigned an id.  (E.g. via may_alias in ALocationAnalysis.)
 */
constexpr uint32_t kMaxTrackedALocs = 128;
using ALocBits = std::bitset<kMaxTrackedALocs>;

//////////////////////////////////////////////////////////////////////

struct ALocMeta {
  uint32_t index;      // id assigned to this location
  ALocBits conflicts;  // flow-insensitive may-alias set, without self
};

/*
 * Information about the various abstract locations an IR unit may be concerned
 * with.  See collect_locations.
 */
struct ALocationAnalysis {
  explicit ALocationAnalysis(const IRUnit&);

  ALocationAnalysis(const ALocationAnalysis&) = delete;
  ALocationAnalysis(ALocationAnalysis&&) = default;
  ALocationAnalysis& operator=(const ALocationAnalysis&) = delete;
  ALocationAnalysis& operator=(ALocationAnalysis&&) = default;

  /*
   * Bidirectional map from abstract locations to metadata about that location,
   * primarily an assigned id.  There is also an inverse map from location id
   * to the metadata structure.
   *
   * The keyed locations in this map take their canonical form.  You should use
   * canonicalize before doing lookups.
   */
  jit::hash_map<ALocation,ALocMeta,ALocation::Hash> locations;
  jit::vector<ALocMeta> locations_inv;

  /*
   * Short-hand to find an abstract location in the locations map, or get
   * folly::none.
   */
  folly::Optional<ALocMeta> find(ALocation) const;

  /*
   * Several larger sets of locations, we have a set of all the ids assigned to
   * properties, elemIs, and frame locals.  This is used by may_alias below.
   */
  ALocBits all_props;
  ALocBits all_elemIs;
  ALocBits all_frame;

  /*
   * Return a set of locations that we've assigned ids to that may be affected
   * by a memory operation.  This function is used to get information about
   * possible effects from an operation on a location that we aren't tracking.
   * This is often needed for instructions that affect very large abstract
   * locations like ANonFrame.
   *
   * Also, note that because of the kMaxTrackedALocs limit, this location could
   * be very 'concrete' (a prop on a known object for example).  But even in
   * those cases, since it's not tracked, we have to use things like all_props
   * to determine what it may alias.
   *
   * The precondition is just because you should generally be using the
   * conflict set in ALocMeta if we have one for `loc'---it'll be much less
   * conservative.
   *
   * Pre: find(loc) == folly::none
   */
  ALocBits may_alias(ALocation loc) const;

  /*
   * Map from frame SSATmp ids to the location bits for all of the frame's
   * locals.
   */
  jit::sparse_idptr_map<SSATmp,ALocBits> per_frame_bits;
};

//////////////////////////////////////////////////////////////////////

/*
 * Perform a flow-insensitive analysis on the supplied blocks, collecting all
 * the possibly distinct abstract memory locations that are explicitly
 * referenced, and assign them ids and may-alias sets.  Only certain types of
 * locations are assigned ids, based on whether it is an ALocation that passes
 * can currently plausibly optimize.
 *
 * Note: it is fine to continue to reuse one ALocationAnalysis structure after
 * mutating the IR, because the information it contains is both
 * flow-insensitive and conservative.  That is: if you change the IR to
 * reference new abstract memory locations, the fact that ALocationAnalysis
 * didn't know about it won't invalidate the things it knows about (and the
 * general may_alias function will still work on the new location).  Similarly,
 * removing references to locations or changing control flow won't invalidate
 * anything.
 */
ALocationAnalysis collect_locations(const IRUnit&, const BlockList&);

//////////////////////////////////////////////////////////////////////

/*
 * Produce summary information for debug printing.
 */
std::string show(const ALocationAnalysis&);
std::string show(ALocBits);

//////////////////////////////////////////////////////////////////////

}}


#endif
