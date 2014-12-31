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
#ifndef incl_HPHP_ALIAS_CLASS_H_
#define incl_HPHP_ALIAS_CLASS_H_

#include <string>
#include <cstdint>

#include <folly/Optional.h>

namespace HPHP { struct StringData; }
namespace HPHP { namespace jit {

struct SSATmp;

//////////////////////////////////////////////////////////////////////

/*
 * AliasClass represents a lattice of abstract memory locations.  These support
 * type-system-like operations (operator<= for subset, operator| for union, and
 * maybe() to test for non-zero intersections).
 *
 * AUnknown is the top of the lattice (is the class of all possible memory
 * locations we care about).  We don't subdivide very much yet, so many things
 * should end up there.  AEmpty is the bottom.  The various special location
 * types AFoo all have an AFooAny, which is the superset of all AFoos.
 */
struct AliasClass;

//////////////////////////////////////////////////////////////////////

/*
 * Special data for locations known to be exactly a specific local on the frame
 * `fp'.
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

struct AliasClass {
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
   * A hashing function for alias classes.
   */
  struct Hash { size_t operator()(AliasClass) const; };

  /*
   * Create an alias class from a bit representation.  Usually you should use
   * one of the other functions to create these.
   */
  explicit AliasClass(rep bits) : m_bits{bits} {}

  /*
   * Create an alias class with more precise specialized information about
   * where it is.
   */
  /* implicit */ AliasClass(AFrame);
  /* implicit */ AliasClass(AProp);
  /* implicit */ AliasClass(AElemI);
  /* implicit */ AliasClass(AElemS);

  /*
   * Exact equality.
   */
  bool operator==(AliasClass) const;

  /*
   * Create an alias class that is at least as big as the true union of this
   * alias class and another one.
   */
  AliasClass operator|(AliasClass) const;

  /*
   * Returns whether this alias class is a non-strict-subset of another one.
   */
  bool operator<=(AliasClass) const;

  /*
   * Returns whether an alias class could possibly refer to the same concrete
   * memory location as another one.  Basically, do they have a non-empty
   * intersection.
   */
  bool maybe(AliasClass) const;

  /*
   * Conditionally access specific known information of various kinds.  Return
   * folly::none if this alias class is not specialized in that way.
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
  friend std::string show(AliasClass);
  bool checkInvariants() const;
  bool equivData(AliasClass) const;

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

auto const AEmpty    = AliasClass{AliasClass::BEmpty};
auto const AFrameAny = AliasClass{AliasClass::BFrame};
auto const APropAny  = AliasClass{AliasClass::BProp};
auto const ANonFrame = AliasClass{AliasClass::BNonFrame};
auto const AElemIAny = AliasClass{AliasClass::BElemI};
auto const AElemAny  = AliasClass{AliasClass::BElem};
auto const AUnknown  = AliasClass{AliasClass::BUnknown};

//////////////////////////////////////////////////////////////////////

/*
 * Replace any SSATmps in an AliasClass with their canonical name (chasing
 * passthrough instructions as with canonical() from analysis.h.)
 */
AliasClass canonicalize(AliasClass);

/*
 * Produce a debug string for an alias class.
 */
std::string show(AliasClass);

//////////////////////////////////////////////////////////////////////

}}

#endif
