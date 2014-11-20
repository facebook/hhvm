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
#ifndef incl_HPHP_ABSTRACT_LOCATION_H_
#define incl_HPHP_ABSTRACT_LOCATION_H_

#include <string>
#include <cstdint>

#include <folly/Optional.h>

namespace HPHP { struct StringData; }
namespace HPHP { namespace jit {

struct SSATmp;

//////////////////////////////////////////////////////////////////////

/*
 * An ALocation (abstract location) represents some part of memory that we know
 * something about.
 *
 * Each abstract location represents a set of potential concrete locations, and
 * the set of abstract locations is a lattice.  These support type-system-like
 * operations (operator<= for subset, operator| for union, and maybe() to test
 * for non-zero intersections).
 *
 * AUnknown is the top of the lattice (includes all possible locations).  We
 * don't subdivide very much yet, so many things should end up there.  AEmpty
 * is the bottom.  The various special location types AFoo all have an AFooAny,
 * which is the superset of all AFoos.
 */
struct ALocation;

//////////////////////////////////////////////////////////////////////

/*
 * Special ALocation data for locations known to be exactly a specific local on
 * the frame `fp'.
 */
struct AFrame { SSATmp* fp; uint32_t id; };

/*
 * A location inside of an object property, with base `obj' and byte offset
 * `offset' from the ObjectData*.
 */
struct AProp  { SSATmp* obj; uint32_t offset; };

/*
 * A integer index inside of an array, with base `arr'.  The `arr' tmp is any
 * kind of array (not necessarily kPackedKind or whatnot).
 */
struct AElemI { SSATmp* arr; uint64_t idx; };

/*
 * A location inside of an array, with base `arr', with a string key.  The
 * `arr' tmp is any kind of array.
 */
struct AElemS { SSATmp* arr; const StringData* key; };

//////////////////////////////////////////////////////////////////////

struct ALocation {
  enum rep : uint32_t {  // bits for various location classes
    BEmpty   = 0,

    BFrame   = 1 << 0,
    BProp    = 1 << 1,
    BElemI   = 1 << 2,
    BElemS   = 1 << 3,

    BElem    = BElemI | BElemS,

    BNonFrame = ~BFrame,

    BUnknown = static_cast<uint32_t>(-1),
  };

  /*
   * A hashing function for abstract locations.
   */
  struct Hash { size_t operator()(ALocation) const; };

  /*
   * Create a location from a bit representation.
   */
  explicit ALocation(rep bits) : m_bits{bits} {}

  /*
   * Create a location with more precise specialized information about where it
   * is.
   */
  /* implicit */ ALocation(AFrame);
  /* implicit */ ALocation(AProp);
  /* implicit */ ALocation(AElemI);
  /* implicit */ ALocation(AElemS);

  /*
   * Exact equality.
   */
  bool operator==(ALocation) const;

  /*
   * Create an abstract location that is at least as big as the union of this
   * location and another one.
   */
  ALocation operator|(ALocation) const;

  /*
   * Returns whether this location represents a non-strict-subset of the
   * possible locations in another one.
   */
  bool operator<=(ALocation) const;

  /*
   * Returns whether an abstract location could possibly refer to the same
   * concrete location as another abstract location.
   */
  bool maybe(ALocation) const;

  /*
   * Conditionally access specific known information of various kinds.  Return
   * folly::none if this abstract location is not specialized in that way.
   */
  folly::Optional<AFrame> frame() const;
  folly::Optional<AProp> prop() const;
  folly::Optional<AElemI> elemI() const;
  folly::Optional<AElemS> elemS() const;

private:
  enum class STag {
    None,
    Frame,
    Prop,
    ElemI,
    ElemS,
  };

private:
  friend std::string show(ALocation);
  bool checkInvariants() const;
  bool equivData(ALocation) const;

private:
  rep m_bits;
  STag m_stag{STag::None};
  union {
    AFrame m_frame;
    AProp  m_prop;
    AElemI m_elemI;
    AElemS m_elemS;
  };
};

//////////////////////////////////////////////////////////////////////

auto const AEmpty    = ALocation{ALocation::BEmpty};
auto const AFrameAny = ALocation{ALocation::BFrame};
auto const APropAny  = ALocation{ALocation::BProp};
auto const ANonFrame = ALocation{ALocation::BNonFrame};
auto const AElemIAny = ALocation{ALocation::BElemI};
auto const AElemAny  = ALocation{ALocation::BElem};
auto const AUnknown  = ALocation{ALocation::BUnknown};

//////////////////////////////////////////////////////////////////////

/*
 * Replace any SSATmps in an ALocation with their canonical name (chasing
 * passthrough instructions as with canonical() from analysis.h.)
 */
ALocation canonicalize(ALocation);

/*
 * Produce a debug string for an abstract location.
 */
std::string show(ALocation);

//////////////////////////////////////////////////////////////////////

}}

#endif
