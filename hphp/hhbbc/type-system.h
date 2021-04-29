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

#include <cstdint>
#include <vector>
#include <utility>

#include <folly/Optional.h>

#include "hphp/util/copy-ptr.h"
#include "hphp/util/low-ptr.h"

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/repo-auth-type-array.h"

#include "hphp/hhbbc/type-system-bits.h"

#include "hphp/hhbbc/array-like-map.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/misc.h"

//////////////////////////////////////////////////////////////////////

/*
 * Type system.
 *
 *   Types are represented by a "trep", which is a bitset, and an
 *   optional specialization. The trep specifies what combination of
 *   basic types the Type represents. The specialization (if present)
 *   gives more information about some subset of the Type. For any
 *   given Type, there's a B* variant which is just the trep, and a T*
 *   variant which can also contain the specialization.
 *
 *   For each "base" type, there's a single bit in the trep. In
 *   addition to the base types, there's many predefined unions of
 *   bits. See type-system-bits.h for the full list.
 *
 *
 * String type:
 *
 *   The String type is divided along one dimension: counted or
 *   uncounted. CStr and SStr represent the two possibilities, with
 *   Str representing the union of both.
 *
 * Array types:
 *
 *   Array types are divided along two dimensions: counted or
 *   uncounted, and empty or non-empty.  Unions of either are allowed.
 *   The naming convention is {S,C,}Arr{N,E,} (where Arr can be any
 *   array type, for example Vec, Dict, or Keyset), where leaving out
 *   either bit means it's unknown along that dimension. An example
 *   lattice looks like this:
 *
 *                         Arr
 *                          |
 *                  +----+--+--+---+
 *                  |    |     |   |
 *                  |  SArr   CArr |
 *                  |   |      |   |
 *              +-------+---------------+
 *              |   |          |   |    |
 *              |  ArrN  +-----+  ArrE  |
 *              | /   \  |     | /   \  |
 *             SArrN  CArrN   CArrE  SArrE
 *
 *
 * Specializations:
 *
 *   Specializations represent further information about some subset
 *   of the type (usually constant values). Each possible
 *   specialization is "supported" by some trep. That is, the
 *   specialization is only allowed if that trep is present. In
 *   addition (right now), only one specialization is allowed in a
 *   Type, and a specialization is only allowed to be present if
 *   exactly one "supportful" trep is present. For example, both Str
 *   and Int support specializations. So a type is not allowed to have
 *   any specialization if both Str and Int are present in a type (but
 *   its fine if only one of them is). The specializations are:
 *
 *     Int=n
 *
 *       Integer with known constant value. Supported by BInt.
 *
 *     Dbl=n
 *
 *       Double with known constant value. Supported by BDbl.
 *
 *     {S,C}Str=s
 *
 *       String with known constant value. Supported by any of B{C,S}Str.
 *
 *     Obj{<}=c
 *     Cls{<}=c
 *     Record{<}=c
 *
 *       Object, Class, or Record with a known class/record type. The
 *       class/record type can either be exact or a
 *       subclass. Supported by BObj, BCls, or BRecord.
 *
 *     Obj=WaitH<T>
 *
 *       Object is a wait handle known to produce T when awaited (or
 *       possibly throw). T must be a subtype of TInitCell. Supported
 *       by BObj.
 *
 *     Arr~(static array)
 *
 *       Array is a known static array.
 *
 *     Arr(T1, T2, .... TN)
 *
 *       Array is known to be a packed array containing precisely N
 *       values of type T1,T2...TN.
 *
 *     Arr([T])
 *
 *       Array is known to be a packed array of indeterminate
 *       length. All values have type T.
 *
 *     Arr([T1:T2])
 *
 *       Array is known to be an array with keys of type T1, and
 *       values of type T2. This potentially includes packed arrays.
 *
 *     Arr(Str=s1:T1, Str=s2:T2, .... Str=sn:TN, [TK:TV])
 *
 *       Array is known to be a (non-packed) array with known keys s1
 *       to sn (the keys can also be integers). Each known key has a
 *       known value type T1...TN. In addition, there's TK is the
 *       union of all possible non-statically known keys, and TV is
 *       the union of all possible values for those keys. TK and TV
 *       can be Bottom if there are none. TK/TV is disjoint from the
 *       known keys.
 *
 *   Array specializations are all supported by BArrLikeN (not
 *   BArrLike). This means the specialization only applies to the
 *   non-empty portion of the union.
 *
 *   NOTE: Having the S* types be a sibling of the C* types is
 *   problematic. There is an assumption that types in the index only
 *   ever get more refined, but eg we might "know" that a type is C*
 *   early on (because eg we stored a value to it, thus modifying it),
 *   but later we might know the value both before and after the
 *   modification, and just replace each with a static array. In fact,
 *   this is almost guaranteed to happen when building arrays from
 *   constants (not literals), because on the first pass we won't know
 *   the constant values, and will produce a CArr, but eventually we
 *   could figure them out, and produce an SArr. This means that in
 *   practice, we can't use CArr anywhere, because it might be
 *   improved to SArr, which is not a subtype. Even if we only
 *   generated CArr after all modifications are done (eg during the
 *   insert-assertions phase) we could still run into problems where
 *   we annotate an array as CArr but jit time analysis/optimization
 *   was able to produce a static array - so it doesn't appear to be
 *   useful, except as a superclass of SArr.
 *
 */

//////////////////////////////////////////////////////////////////////

namespace HPHP { namespace HHBBC {

struct Type;

#define DATATAGS                                                \
  DT(Str, SString, sval)                                        \
  DT(Int, int64_t, ival)                                        \
  DT(Dbl, double, dval)                                         \
  DT(ArrLikeVal, SArray, aval)                                  \
  DT(Obj, DObj, dobj)                                           \
  DT(WaitHandle, DWaitHandle, dwh)                              \
  DT(Cls, DCls, dcls)                                           \
  DT(LazyCls, SString, lazyclsval)                              \
  DT(Record, DRecord, drec)                                     \
  DT(ArrLikePacked, copy_ptr<DArrLikePacked>, packed)           \
  DT(ArrLikePackedN, copy_ptr<DArrLikePackedN>, packedn)        \
  DT(ArrLikeMap, copy_ptr<DArrLikeMap>, map)                    \
  DT(ArrLikeMapN, copy_ptr<DArrLikeMapN>, mapn)

// Tag for what kind of specialized data a Type object has.
enum class DataTag : uint8_t {
  None,
#define DT(name,...) name,
  DATATAGS
#undef DT
};

//////////////////////////////////////////////////////////////////////

/*
 * Information about a class type.  The class is either exact or a
 * subtype of the supplied class.
 */
struct DCls {
  enum Tag : uint16_t { Exact, Sub };

  DCls(Tag type, res::Class cls)
    : type(type)
    , cls(cls)
  {}

  Tag type;
  bool isCtx = false;
  res::Class cls;
};

/*
 * Information about a specific object type.  The class is either
 * exact or a subtype of the supplied class.
 *
 * If the class is known to be a wait handle, DWaitHandle will be used
 * instead.
 */
struct DObj {
  enum Tag : uint16_t { Exact, Sub };

  DObj(Tag type, res::Class cls)
    : type(type)
    , cls(cls)
  {}

  Tag type;
  bool isCtx = false;
  res::Class cls;
};

/*
 * Information about a wait handle (sub-class of HH\\Awaitable) carry a
 * type that awaiting the wait handle will produce.
 */
struct DWaitHandle {
  DWaitHandle(res::Class cls, copy_ptr<Type> inner)
    : cls{std::move(cls)}
    , inner{std::move(inner)} {}
  // Strictly speaking, we know that cls is HH\\Awaitable, but keeping
  // it around lets us demote to a DObj without having the Index
  // available.
  res::Class cls;
  copy_ptr<Type> inner;
};

/*
 * Information about a specific record type.  The record type is either
 * exact or a subtype of the supplied record.
 */
struct DRecord {
  enum Tag : uint16_t { Exact, Sub };

  DRecord(Tag type, res::Record rec)
    : type(type)
    , rec(rec)
  {}

  Tag type;
  res::Record rec;
};

struct DArrLikePacked;
struct DArrLikePackedN;
struct DArrLikeMap;
struct DArrLikeMapN;
struct ArrKey;
struct IterTypes;

//////////////////////////////////////////////////////////////////////

struct MapElem;
using MapElems = ArrayLikeMap<TypedValue, MapElem>;

//////////////////////////////////////////////////////////////////////

enum class LegacyMark : uint8_t {
  Bottom,   // NA (for when type does not include arrays)
  Marked,   // definitely mark
  Unmarked, // definitely unmarked
  Unknown   // either
};

// Bag of state for tracking HAM related metadata (array provenance
// and legacy marking).
struct HAMSandwich {
  // No state. Used for types which don't have any HAM relevance
  // (ints, keysets, etc).
  static const HAMSandwich None;
  // Unmarked legacy mark, but no array-provenance. Used for types
  // which don't care about array-provenance (vec, dict, etc).
  static const HAMSandwich Unmarked;

  // Create the appropriate HAMSandwich for the given static array.
  static HAMSandwich FromSArr(SArray);

  // Create the most general HAMSandwich allowed for the given type.
  static HAMSandwich TopForBits(trep b) {
    using HPHP::HHBBC::couldBe;
    return HAMSandwich {
      couldBe(b, kMarkBits) ? LegacyMark::Unknown : LegacyMark::Bottom
    };
  }

  // Return a new HAMSandwich refined based on the given type. For
  // example, if the given type no longer contains Vec or Dict, we
  // need to drop any marking information.
  HAMSandwich project(trep) const;

  // Return the legacy mark information in this HAMSandwich. If the
  // given type does not require legacy mark information, Unmarked is
  // always returned.
  LegacyMark legacyMark(trep) const;

  // Return true if this is the "bottom" HAMSandwich type. That is,
  // the result of intersecting together incompatible ones.
  bool isBottom(trep b) const;

  // Check if the intersection between this and another HAMSandwich is
  // non-empty.
  bool couldBe(HAMSandwich) const;
  // Check if this HAMSandwich is completely contained with another.
  bool subtypeOf(HAMSandwich) const;

  bool operator==(HAMSandwich) const;
  bool operator!=(HAMSandwich o) const { return !(*this == o); }

  // Union together or intersect two HAMSandwiches.
  HAMSandwich operator|(HAMSandwich) const;
  HAMSandwich& operator|=(HAMSandwich o) { return *this = *this | o; }

  HAMSandwich operator&(HAMSandwich) const;
  HAMSandwich& operator&=(HAMSandwich o) { return *this = *this & o; }

  // Testing:

  void setLegacyMarkForTesting(LegacyMark m) { m_mark = m; }

  bool checkInvariants(trep) const;

private:
  // Legacy marks are only tracked for Vec and Dict.
  static constexpr trep kMarkBits = BVec | BDict;

  explicit HAMSandwich(LegacyMark mark) : m_mark{mark} {}

  LegacyMark m_mark;
};

//////////////////////////////////////////////////////////////////////

enum class Emptiness {
  Empty,
  NonEmpty,
  Maybe
};

// Represents whether a promotion can happen, and whether that
// promotion might throw.
enum class Promotion {
  No,
  Yes,
  YesMightThrow
};

//////////////////////////////////////////////////////////////////////

struct Type {
  Type() : Type{BBottom} {}
  explicit Type(trep t) : Type{t, HAMSandwich::TopForBits(t)} {}
  Type(trep t, HAMSandwich h) : m_bits{t}, m_dataTag{DataTag::None}, m_ham{h} {
    assertx(checkInvariants());
  }

  Type(const Type& o) noexcept
    : m_raw{o.m_raw}
    , m_ham{o.m_ham}
  {
    SCOPE_EXIT { assertx(checkInvariants()); };
    if (LIKELY(m_dataTag == DataTag::None)) return;
    copyData(o);
  }

  Type(Type&& o) noexcept
    : m_raw{o.m_raw}
    , m_ham{o.m_ham}
  {
    SCOPE_EXIT { assertx(o.checkInvariants()); };
    if (LIKELY(m_dataTag == DataTag::None)) return;
    moveData(std::move(o));
  }

  Type& operator=(const Type&) noexcept;
  Type& operator=(Type&&) noexcept;

  ~Type() {
    assertx(checkInvariants());
    if (LIKELY(m_dataTag == DataTag::None)) return;
    destroyData();
  }

  /*
   * Exact equality or inequality of types, and hashing.
   */
  bool operator==(const Type& o) const;
  bool operator!=(const Type& o) const { return !(*this == o); }
  size_t hash() const;

  const Type& operator |= (const Type& other);
  const Type& operator |= (Type&& other);
  const Type& operator &= (const Type& other);
  const Type& operator &= (Type&& other);

  /*
   * Returns true if this type is equivalently refined, more refined or strictly
   * more refined than `o`.  This is similar to the `==` and subtype operations
   * defined below, except they take into account if a type is tagged as a
   * context.
   */
  bool equivalentlyRefined(const Type& o) const;
  bool moreRefined(const Type& o) const;
  bool strictlyMoreRefined(const Type& o) const;

  /*
   * Returns true if this type is definitely going to be a subtype or a strict
   * subtype of `o' at runtime.  If this function returns false, this may
   * still be a subtype of `o' at runtime, it just may not be known.
   */
  bool subtypeOf(const Type& o) const;
  bool strictSubtypeOf(const Type& o) const;

  /*
   * Similar, but only check the trep (same as subtypeOf(Type{bits}),
   * but cheaper).
   */
  bool subtypeOf(trep b) const { return HPHP::HHBBC::subtypeOf(bits(), b); }
  bool strictSubtypeOf(trep bits) const {
    // If the bits match, only specialized data can make it more
    // specific.
    return is(bits) ? hasData() : subtypeOf(bits);
  }

  /*
   * Similar to subtypeOf(), but only checks if *this is a subtype of
   * b1 excluding all types except for those in b2. For example, this
   * is useful if one wants to check if *this is a specific array
   * type, without caring if its unioned with non-array types.
   */
  bool subtypeAmong(trep b1, trep b2) const {
    return HPHP::HHBBC::subtypeAmong(bits(), b1, b2);
  }

  /*
   * Returns whether there are any values of this type that are also
   * values of the type `o'.
   * When this function returns false, it is known that this type
   * must not be in any subtype relationship with the argument Type 'o'.
   * When true is returned the two types may still be unrelated but it is
   * not possible to tell.
   * Essentially this function can conservatively return true but must be
   * precise when returning false.
   */
  bool couldBe(const Type& o) const;
  bool couldBe(trep b) const { return HPHP::HHBBC::couldBe(bits(), b); }

  // Equality, but more efficient
  bool is(trep b) const { return bits() == b; }

  bool hasData() const { return m_dataTag != DataTag::None; }

  struct ArrayCat {
    enum { None, Empty, Packed, Struct, Mixed } cat;
    bool hasValue;
  };

private:
  friend folly::Optional<int64_t> arr_size(const Type& t);
  friend ArrayCat categorize_array(const Type& t);
  friend CompactVector<LSString> get_string_keys(const Type& t);
  friend Type wait_handle(const Index&, Type);
  friend bool is_specialized_wait_handle(const Type&);
  friend bool is_specialized_array_like(const Type&);
  friend bool is_specialized_array_like_arrval(const Type&);
  friend bool is_specialized_array_like_packedn(const Type&);
  friend bool is_specialized_array_like_packed(const Type&);
  friend bool is_specialized_array_like_mapn(const Type&);
  friend bool is_specialized_array_like_map(const Type&);

  friend bool is_specialized_obj(const Type&);
  friend bool is_specialized_record(const Type&);
  friend bool is_specialized_cls(const Type&);
  friend bool is_specialized_lazycls(const Type&);
  friend bool is_specialized_string(const Type&);
  friend bool is_specialized_int(const Type&);
  friend bool is_specialized_double(const Type&);
  friend Type wait_handle_inner(const Type&);
  friend Type sval(SString);
  friend Type sval_nonstatic(SString);
  friend Type sval_counted(SString);
  friend Type ival(int64_t);
  friend Type dval(double);
  friend Type lazyclsval(SString);
  friend Type subObj(res::Class);
  friend Type objExact(res::Class);
  friend Type subCls(res::Class);
  friend Type clsExact(res::Class);
  friend Type exactRecord(res::Record);
  friend Type subRecord(res::Record);
  friend Type packed_impl(trep, HAMSandwich, std::vector<Type>);
  friend Type packedn_impl(trep, HAMSandwich, Type);
  friend Type map_impl(trep, HAMSandwich, MapElems, Type, Type);
  friend Type mapn_impl(trep, HAMSandwich, Type, Type);
  friend DObj dobj_of(const Type&);
  friend Type demote_wait_handle(Type);
  friend DRecord drec_of(const Type&);
  friend DCls dcls_of(Type);
  friend SString sval_of(const Type&);
  friend SString lazyclsval_of(const Type&);
  friend int64_t ival_of(const Type&);
  friend Type union_of(Type, Type);
  friend Type intersection_of(Type, Type);
  friend void widen_type_impl(Type&, uint32_t);
  friend Type widen_type(Type);
  friend Type widening_union(const Type&, const Type&);
  friend Emptiness emptiness(const Type&);
  friend Type opt(Type);
  friend Type unopt(Type);
  template<typename R, bool, bool>
  friend R tvImpl(const Type&);
  friend Type scalarize(Type t);

  friend Type return_with_context(Type, Type);
  friend Type setctx(Type, bool);
  friend Type unctx(Type);

  friend Type remove_data(Type, trep);
  friend Type remove_int(Type);
  friend Type remove_double(Type);
  friend Type remove_string(Type);
  friend Type remove_lazycls(Type);
  friend Type remove_cls(Type);
  friend Type remove_obj(Type);
  friend Type remove_keyset(Type);
  friend Type remove_bits(Type, trep);
  friend std::pair<Type, Type> split_obj(Type);
  friend std::pair<Type, Type> split_cls(Type);
  friend std::pair<Type, Type> split_array_like(Type);
  friend std::pair<Type, Type> split_string(Type);
  friend std::pair<Type, Type> split_lazycls(Type);

  friend std::string show(const Type&);
  friend std::pair<Type,bool> array_like_elem_impl(const Type&, const Type&);
  friend std::pair<Type,bool> array_like_set_impl(Type,
                                                  const Type&,
                                                  const Type&);
  friend std::pair<Type,bool> array_like_newelem_impl(Type, const Type&);
  friend std::pair<Type,bool> arr_val_elem(const Type&, const Type&);
  friend std::pair<Type,bool> arr_packed_elem(const Type&, const Type&);
  friend std::pair<Type,bool> arr_packedn_elem(const Type&, const Type&);
  friend std::pair<Type,bool> arr_map_elem(const Type&, const Type&);
  friend std::pair<Type,bool> arr_mapn_elem(const Type&, const Type&);
  friend bool arr_packed_set(Type&, const Type&, const Type&, bool);
  friend bool arr_packedn_set(Type&, const Type&, const Type&, bool);
  friend bool arr_map_set(Type&, const Type&, const Type&, bool);
  friend bool arr_map_newelem(Type&, const Type&, bool);
  friend IterTypes iter_types(const Type&);
  friend folly::Optional<RepoAuthType> make_repo_type_arr(ArrayTypeTable::Builder&,
                                                          const Type&);

  friend Type vec_val(SArray);
  friend Type vec_empty();
  friend Type some_vec_empty();
  friend Type dict_val(SArray);
  friend Type dict_empty();
  friend Type some_dict_empty();
  friend Type keyset_val(SArray);
  friend bool could_contain_objects(const Type&);
  friend Type loosen_interfaces(Type);
  friend Type loosen_staticness(Type);
  friend Type loosen_string_staticness(Type);
  friend Type loosen_array_staticness(Type);
  friend Type loosen_values(Type);
  friend Type loosen_string_values(Type);
  friend Type loosen_array_values(Type);
  friend Type loosen_emptiness(Type);
  friend Type loosen_likeness(Type);
  friend Type loosen_likeness_recursively(Type);
  friend Type add_nonemptiness(Type);
  friend Type assert_emptiness(Type);
  friend Type assert_nonemptiness(Type);
  friend Type remove_uninit(Type t);
  friend Type to_cell(Type t);
  friend bool inner_types_might_raise(const Type& t1, const Type& t2);
  friend std::pair<Type, Promotion> promote_clsmeth_to_vecish(Type);
  friend std::pair<Type, Promotion> promote_classlike_to_key(Type);

  friend Type set_trep_for_testing(Type, trep);
  friend trep get_trep_for_testing(const Type&);

  friend Type make_obj_for_testing(trep, res::Class, DObj::Tag, bool);
  friend Type make_cls_for_testing(trep, res::Class, DCls::Tag, bool);
  friend Type make_record_for_testing(trep, res::Record, DRecord::Tag);
  friend Type make_arrval_for_testing(trep, SArray);
  friend Type make_arrpacked_for_testing(trep, std::vector<Type>,
                                         folly::Optional<LegacyMark>);
  friend Type make_arrpackedn_for_testing(trep, Type);
  friend Type make_arrmap_for_testing(trep, MapElems, Type, Type,
                                      folly::Optional<LegacyMark>);
  friend Type make_arrmapn_for_testing(trep, Type, Type);

  friend Type set_mark_for_testing(Type, LegacyMark);
  friend Type loosen_mark_for_testing(Type);

private:
  union Data {
    Data() {}
    ~Data() {}

#define DT(tag_name,type,name) type name;
  DATATAGS
#undef DT
  };

  template<class Ret, class T, class Function>
  struct DDHelperFn;

private:
  trep bits() const { return trep(m_bits); }
  static Type unctxHelper(Type, bool&);

  template<class Ret, class T, class Function>
  DDHelperFn<Ret,T,Function> ddbind(const Function& f, const T& t) const;
  template<class Ret, class T, class Function>
  Ret dd2nd(const Type&, DDHelperFn<Ret,T,Function>) const;
  template<class Function> typename Function::result_type
  dualDispatchDataFn(const Type&, Function) const;
  template<typename Function> typename Function::result_type
  dispatchArrLikeNone(const Type&, Function) const;

  template<bool contextSensitive>
  bool equivImpl(const Type& o) const;
  template<bool contextSensitive>
  bool subtypeOfImpl(const Type& o) const;
  bool checkInvariants() const;

  void copyData(const Type& o);
  void moveData(Type&& o);
  void destroyData();

private:
  union {
    struct {
      uint64_t m_bits : kTRepBitsStored;
      DataTag m_dataTag;
    };
    uint64_t m_raw;
  };
  HAMSandwich m_ham;
  Data m_data;
};

//////////////////////////////////////////////////////////////////////

// The value portion of MapElems. Stores the known type of the value,
// and whether the key is static or not. (Modifying the key is
// problematic, so its easier to store with the value).

struct MapElem {
  Type val;
  TriBool keyStaticness;

  MapElem(Type val, TriBool keyStaticness)
    : val{std::move(val)}, keyStaticness{keyStaticness} {}

  // Keep the same value, but modify the staticness
  MapElem withStaticness(TriBool b) const {
    return MapElem{val, b};
  }
  // Keep the same staticness, but modify the value
  MapElem withType(Type t) const {
    return MapElem{std::move(t), keyStaticness};
  }

  // Create a MapElem using staticness inferred from the key's type.
  static MapElem KeyFromType(const Type& key, Type val);

  static MapElem IntKey(Type val) {
    return MapElem{std::move(val), TriBool::Yes};
  }
  static MapElem StrKey(Type val) {
    return MapElem{std::move(val), TriBool::Maybe};
  }
  static MapElem SStrKey(Type val) {
    return MapElem{std::move(val), TriBool::Yes};
  }
  static MapElem CStrKey(Type val) {
    return MapElem{std::move(val), TriBool::No};
  }
};

//////////////////////////////////////////////////////////////////////

struct DArrLikePacked {
  explicit DArrLikePacked(std::vector<Type> elems)
    : elems(std::move(elems)) {}

  std::vector<Type> elems;
};

struct DArrLikePackedN {
  explicit DArrLikePackedN(Type t) : type(std::move(t)) {}
  Type type;
};

struct DArrLikeMap {
  explicit DArrLikeMap(MapElems map, Type optKey, Type optVal)
    : map(std::move(map))
    , optKey(std::move(optKey))
    , optVal(std::move(optVal))
  {}
  bool hasOptElements() const { return !optKey.is(BBottom); }
  // The array always starts with these known keys
  MapElems map;
  // Key/value types for optional elements after known keys. Bottom if
  // none.
  Type optKey;
  Type optVal;
};

// DArrLikePackedN and DArrLikeMapN do not need the LegacyMark because they
// cannot be converted to a TypedValue
struct DArrLikeMapN {
  explicit DArrLikeMapN(Type key, Type val)
    : key(std::move(key))
    , val(std::move(val))
  {}
  Type key;
  Type val;
};

//////////////////////////////////////////////////////////////////////

#define X(y, ...) extern const Type T##y;
HHBBC_TYPE_PREDEFINED(X)
#undef X

//////////////////////////////////////////////////////////////////////

/*
 * Return WaitH<T> for a type t.
 */
Type wait_handle(const Index&, Type t);

/*
 * Return T from a WaitH<T>.
 *
 * Pre: is_specialized_handle(t);
 */
Type wait_handle_inner(const Type& t);

/*
 * Create Types that represent constant values.
 */
Type ival(int64_t);
Type dval(double);
Type lazyclsval(SString);
Type vec_val(SArray);
Type dict_val(SArray);
Type keyset_val(SArray);

/*
 * Create string types with different counted/static flavor.
 */
Type sval(SString);
Type sval_nonstatic(SString);
Type sval_counted(SString);

/*
 * Create empty string types with different counted/static flavor.
 */
Type sempty();
Type sempty_nonstatic();
Type sempty_counted();

/*
 * Create static empty array types
 */
Type vec_empty();
Type dict_empty();
Type keyset_empty();

/*
 * Create an any-countedness empty array/vec/dict type.
 */
Type some_vec_empty();
Type some_dict_empty();
Type some_keyset_empty();

/*
 * Create types for objects or classes with some known constraint on
 * which res::Class is associated with them.
 */
Type subObj(res::Class);
Type objExact(res::Class);
Type subCls(res::Class);
Type clsExact(res::Class);

/*
 * Create types for records with some known constraint on an associated
 * res::Record.
 */
Type exactRecord(res::Record);
Type subRecord(res::Record);

/*
 * vec types with known size.
 *
 * Pre: !v.empty()
 */
Type vec(std::vector<Type> v);
Type svec(std::vector<Type> v);

/*
 * Vec type of unknown size.
 */
Type vec_n(Type);
Type svec_n(Type);

/*
 * Struct-like dicts.
 *
 * Pre: !m.empty()
 */
Type dict_map(MapElems m, Type optKey = TBottom, Type optVal = TBottom);
Type sdict_map(MapElems m, Type optKey = TBottom, Type optVal = TBottom);

/*
 * Dict with key/value types.
 */
Type dict_n(Type, Type);
Type sdict_n(Type, Type);

/*
 * Dicts with vector-like data.
 */
Type dict_packed(std::vector<Type> v);
Type sdict_packed(std::vector<Type> v);
Type dict_packedn(Type);
Type sdict_packedn(Type);

/*
 * Keyset with key (same as value) type.
 */
Type keyset_n(Type);
Type ckeyset_n(Type);

/*
 * Keyset from MapElems
 */
Type keyset_map(MapElems);

/*
 * Unions the type `t' with TInitNull. This is identical to
 * union_of(t, TInitNull) but more efficient.
 */
Type opt(Type t);

/*
 * Removes TInitNull from `t' (if present). If `t' is TInitNull, this
 * results in TBottom.
 */
Type unopt(Type t);

/*
 * Improves the type `t` given the current context.  This returns the
 * intersection of the type `t` with the `context` if the `context` is a valid
 * class or object, and `t` is tagged as being the context.  If `context` is
 * a class we will first convert it to an object.
 */
Type return_with_context(Type t, Type context);

/*
 * If `to` is false.  This is an identity operation.  If `to` is true, and
 * `t` is a specialized object or class, this will return `t` tagged as a
 * context.
 */
Type setctx(Type t, bool to = true);

/*
 * This removes any context tags in the type `t`, even if nested inside other
 * types.
 */
Type unctx(Type t);

/*
 * Refinedness equivalence checks.
 */
inline bool equivalently_refined(const Type& a, const Type& b) {
  return a.equivalentlyRefined(b);
}

template<
  typename Iterable,
  typename = std::enable_if<
    std::is_same<typename Iterable::value_type, Type>::value
  >
>
bool equivalently_refined(const Iterable& a, const Iterable& b) {
  if (a.size() != b.size()) return false;
  for (auto ita = a.begin(), itb = b.begin(); ita != a.end(); ++ita, ++itb) {
    if (!equivalently_refined(*ita, *itb)) return false;
  }
  return true;
}

/*
 * Returns true if type 't' represents a "specialized" object, that is an
 * object of a known class, or an optional object of a known class.
 */
bool is_specialized_obj(const Type&);

/*
 * Returns true if type 't' represents a "specialized" class---i.e. a class
 * with a DCls structure.
 */
bool is_specialized_cls(const Type&);

/*
 * Returns true if type 't' represents a "specialized" record--i.e. a
 * record with a DRecord structure.
 */
bool is_specialized_record(const Type&);

/*
 * Returns true if type 't' represents a "specialized"
 * string/int/double/lazy class--i.e. with a known value.
 */
bool is_specialized_string(const Type&);
bool is_specialized_lazycls(const Type&);
bool is_specialized_int(const Type&);
bool is_specialized_double(const Type&);

/*
 * Returns whether `t' is a WaitH<T> or ?WaitH<T> for some T.
 *
 * Note that this function returns false for Obj<=WaitHandle with no
 * tracked inner type.
 */
bool is_specialized_wait_handle(const Type& t);

/*
 * Returns true if type 't' represents a specialized array. That is,
 * with either a constant value or some (maybe partially) known shape.
 */
bool is_specialized_array_like(const Type& t);
bool is_specialized_array_like_arrval(const Type& t);
bool is_specialized_array_like_packedn(const Type& t);
bool is_specialized_array_like_packed(const Type& t);
bool is_specialized_array_like_mapn(const Type& t);
bool is_specialized_array_like_map(const Type& t);

/*
 * Split the type `t' into two types, one representing the
 * TObj/TCls/TArrLike/TStr portion, and the other containing the
 * rest. If `t' doesn't contain one of those types, the first result
 * will be TBottom. Likewise if it's only one of those types, the
 * second result will be TBottom. `t' must be a subtype of TCell
 * (removing types form TTop produces unrepresentable types).
 */
std::pair<Type, Type> split_obj(Type);
std::pair<Type, Type> split_cls(Type);
std::pair<Type, Type> split_array_like(Type);
std::pair<Type, Type> split_string(Type);
std::pair<Type, Type> split_lazycls(Type);

/*
 * Remove TInt/TDbl/TStr/TCls/TObj from the type `t', including any
 * associated specialized data. `t' must be a subtype of TCell
 * (removing types from TTop produces unrepresentable types).
 */
Type remove_int(Type);
Type remove_double(Type);
Type remove_string(Type);
Type remove_lazycls(Type);
Type remove_cls(Type);
Type remove_obj(Type);

// Remove TKeyset from the type `t'. Unlike the other remove
// functions, this attempts to keep any appropriate array specialized
// data. `t' must be a subtype of TCell.
Type remove_keyset(Type);

/*
 * Remove arbitrary bits from the type `t', including any specialized
 * data associated with those bits. This is a more general version of
 * the above remove_* functions, but not as efficient. `t' must be a
 * subtype of TCell for the same reasons as above.
 *
 * Note: if you remove any of BArrLikeN bits, all array specialized
 * data is removed.
 */
Type remove_bits(Type, trep);

/*
 * Returns the best known instantiation of a class type.
 *
 * Pre: t.subypeOf(TObj)
 */
Type toobj(const Type& t);

/*
 * Returns the best known TCls subtype for an object type.
 *
 * Pre: t.subtypeOf(TCls)
 */
Type objcls(const Type& t);

/*
 * If the type t has a known constant value, return it as a
 * TypedValue. Otherwise return folly::none. tv() will fail to return
 * a constant value if the type is known to be counted. tvCounted()
 * will return the constant value regardless of the type's
 * countedness.
 *
 * The returned TypedValue can only contain non-reference-counted types.
 */
folly::Optional<TypedValue> tv(const Type& t);
folly::Optional<TypedValue> tvCounted(const Type& t);

/*
 * If the type t has a known constant value, return it as a TypedValue.
 * Otherwise return folly::none.
 *
 * The returned TypedValue may contain reference-counted types.
 *
 * You are responsible for any required ref-counting.
 */
folly::Optional<TypedValue> tvNonStatic(const Type& t);

/*
 * If the type t has a known constant value, return true. Otherwise
 * return false. is_scalar() will fail to return true if the type is
 * known to be counted. is_scalar_counted() will return true
 * regardless of the type's countedness.
 */
bool is_scalar(const Type& t);
bool is_scalar_counted(const Type& t);

/*
 * Return the canonical scalar type for t - equivalent to
 * from_cell(*tv(t)).
 *
 * This can be used to ensure that the arguments in a CallContext are
 * canonicalized, so that immaterial changes to them (eg TArrN ->
 * TSArrN or DArrLikeMap -> DArrLikeVal) don't affect which entry gets
 * looked up.
 *
 * pre: is_scalar(t).
 */
Type scalarize(Type t);

/*
 * Get the type in our typesystem that corresponds to an hhbc
 * IsTypeOp.
 *
 * Pre: op != IsTypeOp::Scalar
 */
Type type_of_istype(IsTypeOp op);

/*
 * Get the hhbc IsTypeOp that corresponds to the type in our typesystem.
 * Returns folly::none if no matching IsTypeOp is found.
 */
folly::Optional<IsTypeOp> type_to_istypeop(const Type& t);

/*
 * Get the type in our typesystem that corresponds to type given by the
 * potentially unresolved type structure.
 * Returns folly::none if the type structure is unresolved or
 * no matching Type is found.
 *
 */
folly::Optional<Type> type_of_type_structure(const Index&, Context, SArray ts);

/*
 * Return the DObj structure for a strict subtype of TObj or TOptObj.
 *
 * Pre: is_specialized_obj(t)
 */
DObj dobj_of(const Type& t);

/*
 * Return the DCls structure for a strict subtype of TCls.
 *
 * Pre: is_specialized_cls(t)
 */
DCls dcls_of(Type t);

/*
 * Return the SString for a strict subtype of TStr.
 *
 * Pre: is_specialized_string(t)
 */
SString sval_of(const Type& t);

/*
 * Return the SString for a strict subtype of TLazyCls.
 *
 * Pre: is_specialized_lazycls(t)
 */
SString lazyclsval_of(const Type& t);

/*
 * Return the specialized integer value for a type.
 *
 * Pre: is_specialized_int(t)
 */
int64_t ival_of(const Type& t);

/*
 * Create a Type from a TypedValue.
 *
 * Pre: the cell must contain a non-reference-counted type.
 * Post: returned type is a subtype of TUnc
 */
Type from_cell(TypedValue tv);

/*
 * Create a Type from a DataType. KindOfString and KindOfPersistentString
 * are both treated as TStr.
 *
 * Pre: dt is one of the DataTypes that actually represent php values
 * (or KindOfUninit).
 */
Type from_DataType(DataType dt);

/*
 * Create a Type from a builtin type specification string.
 *
 * This is used for HNI class properties.  We assume that these are
 * accurate.  `s' may be nullptr.
 */
Type from_hni_constraint(SString s);

/*
 * Make a type that represents values from the intersection of the
 * supplied types.
 */
Type intersection_of(Type a, Type b);

/*
 * Make a type that represents values from either of the supplied
 * types.
 *
 * Importantly, note that there are infinitely long chains of array
 * types that continue to become less specialized, so chains of
 * union_of operations are not guaranteed to reach a stable point in
 * finite steps.
 */
Type union_of(Type a, Type b);

// Helper function to union together multiple things
template<typename... Types>
Type union_of(Type a, Type b, Types... ts) {
  return union_of(std::move(a), union_of(std::move(b), std::move(ts)...));
}

/*
 * Widening union.
 *
 * This operation returns a type T, such that a is a subtype of T, b
 * is a subtype of T, and union_of(a, b) is a subtype of T.  The
 * widening union also has the property that every possible chain of
 * successive applications of the function eventually reaches a stable
 * point.
 *
 * This is currently implemented by unioning the types, then applying
 * widen_type() to the result.
 *
 * For portions of our analysis that rely on growing types reaching
 * stable points for termination, this function must occasionally be
 * used instead of union_of to guarantee termination.  See details in
 * analyze.cpp.
 */
Type widening_union(const Type& a, const Type& b);

/*
 * Widen a type to one which has a finite chain under the union operator. This
 * generally involves restricting the type's nesting depth to a fixed limit and
 * preventing a specialized array type from growing larger unbounded.
 */
Type widen_type(Type t);

/*
 * Check if the first type is more refined than the second type for
 * the purposes of use in the Index. This is basically moreRefined()
 * plus some additional rules for interfaces.
 */
bool more_refined_for_index(const Type&, const Type&);

/*
 * Returns what we know about the emptiness of the type.
 */
Emptiness emptiness(const Type&);

/*
 * Returns whether a Type could hold an object that has a custom
 * boolean conversion function.
 */
bool could_have_magic_bool_conversion(const Type&);

/*
 * Returns the smallest type that `a' is a subtype of, from the
 * following set: TInitCell, TUninit.
 *
 * Pre: `a' is a subtype of TCell.
 */
Type stack_flav(Type a);

/*
 * The HHBBC type system is not monotonic. However, we want types stored in
 * the index to become monotonically more refined. We call this helper before
 * updating function return types to help maintain that invariant. A function
 * return type should never be an object with some known interface.
 *
 * The monotonicity requirement on our types is that given a, b, A, and B
 * such that A <= a and B <= b, we must have union_of(A, B) <= union_of(a, b)
 * and intersection_of(A, B) <= intersection_of(a, b).
 *
 * The union_of case can fail if a and b are some interface and A and B are
 * concrete, unrelated classes that implement that interface.
 */
Type loosen_interfaces(Type);

/*
 * Discard any countedness information about the type. Force any type
 * (recursively) which contains only static or counted variants to contain
 * both. Doesn't change the type otherwise.
 */
Type loosen_staticness(Type);

/*
 * Like loosen_staticness, but non-recursive and only affects
 * arrays. Used to model the potential COW effects during defining
 * member instructions.
 */
Type loosen_array_staticness(Type);

/*
 * Discard any string countedness information. Force any string type
 * which contains only static or counted variants to contain
 * both. Doesn't change any non-string types.
 */
Type loosen_string_staticness(Type);

/*
 * Force any type which might contain TVec or TDict to contain TVec|TDict.
 * This discards any emptiness, staticness, or value information if the
 * type is modified.
 */
Type loosen_vec_or_dict(Type);

/*
 * Drop any data from the type (except for object class information) and force
 * TTrue or TFalse to TBool. Doesn't change the type otherwise.
 */
Type loosen_values(Type t);

/*
 * Drop any knowledge about the inner structure of array-like types
 * The type is unchanged otherwise. This is like loosen_values except
 * it only modifies TArrLikes.
 */
Type loosen_array_values(Type t);

/*
 * Drop any knowledge about known string values. The type is unchanged
 * otherwise.
 */
Type loosen_string_values(Type t);

/*
 * Discard any emptiness information about the type. Force any type which
 * contains only empty or non-empty variants to contain both. Doesn't change the
 * type otherwise.
 */
Type loosen_emptiness(Type t);

/*
 * Force any type which includes TCls or TLazyCls to also include
 * TSStr, and all TClsMeth to include TVecN.
 */
Type loosen_likeness(Type t);

/*
 * Like loosen_likeness, but operates recursively on any specialized
 * data present. This is usually not what you want. Note that
 * loosen_likeness can transform a TClsMeth into an array. If TClsMeth
 * is uncounted, this can result in a non-static type inside a static
 * array. As a result, loosen_likeness_recursively will also loosen
 * array staticness.
 */
Type loosen_likeness_recursively(Type t);

/*
 * Loosens staticness, emptiness, and values from the type. This forces a type
 * to its most basic form (except for object class information).
 */
Type loosen_all(Type t);

/*
 * If t contains TUninit, returns the best type we can that contains
 * at least everything t contains, but doesn't contain TUninit.  Note
 * that this function will return TBottom for TUninit.
 */
Type remove_uninit(Type t);

/*
 * If t is not a TCell, returns TInitCell. Otherwise, if t contains
 * TUninit, return union_of(remove_uninit(t), TInitCell).
 */
Type to_cell(Type t);

/*
 * Add non-empty variants of the type to the type if not already
 * present. Doesn't change the type otherwise.
 */
Type add_nonemptiness(Type t);

/*
 * Produced the most refined type possible, given that
 * t passed/failed an emptiness check.
 */
Type assert_emptiness(Type);
Type assert_nonemptiness(Type);

/*
 * If t is definitely an array with a known size, return
 * it. folly::none otherwise.
 */
folly::Optional<int64_t> arr_size(const Type&);

/*
 *
 * Returns the best known type of an array inner element given a type
 * for the key, along with whether the value will always be
 * present. The returned type is always a subtype of TInitCell.
 *
 * The returned type will be TBottom if there's guaranteed to be no
 * value for that key.
 *
 * Pre: first arg could be TArrLike, the second arg is a subtype of
 * TArrKey.
 *
 * Note: the array parameter merely "could be" TArrLike. Any
 * non-TArrLike bits in the type are ignored.
 */
std::pair<Type,bool> array_like_elem(const Type& arr, const Type& key);

/*
 * Perform an array-like set on types. Returns a type that represents
 * the effects of arr[key] = val and whether the set can possibly
 * throw.
 *
 * The returned type will not contain the TArrLike bits if the
 * operation will always throw (if you pass it in subtypes of
 * TArrLike, this means TBottom).
 *
 * Pre: first arg could be TArrLike, the second arg is a subtype of
 * TArrKey.
 *
 * Note: the array parameter merely "could be" TArrLike. Any
 * non-TArrLike bits in the type are ignored for the sake of modeling
 * the set. The returned type includes the non-TArrLike bits copied
 * into them.
 */
std::pair<Type,bool> array_like_set(Type arr, const Type& key, const Type& val);

/*
 * Perform a newelem operation on an array-like type.  Returns an
 * array that contains a new pushed-back element with the supplied
 * value, in the sense of arr[] = val, and whether the operation might
 * throw. If the operation always throws, the new base will not
 * contain the TArrLike bits (if you pass in a subtype of TArrLike,
 * this means TBottom).
 *
 * Pre: first arg could be TArrLike, second arg is a subtype of
 * TInitCell.
 *
 * Note: the array parameter merely "could be" TArrLike. Any
 * non-TArrLike bits in the type are ignored for the sake of modeling
 * the newelem. The returned type includes the non-TArrLike bits
 * copied into them.
 */
std::pair<Type,bool> array_like_newelem(Type arr, const Type& val);

/*
 * Return the best known information for iteration of the supplied type. This is
 * only intended for non-mutable iteration, so the returned types are at worst
 * InitCell.
 */
struct IterTypes {
  Type key;
  Type value;
  // The number of elements we're iterating over:
  enum struct Count {
    Empty,     // No elements
    Single,    // Exactly one element
    ZeroOrOne, // Less than 2 elements
    NonEmpty,  // Unknown upper bound, but non-empty
    Any        // Nothing known
  };
  Count count;
  // Can a IterInit[K] op throw on this iterator?
  bool mayThrowOnInit;
  // Can a IterNext[K] op throw on this iterator? Can only happen for object
  // types.
  bool mayThrowOnNext;
};
IterTypes iter_types(const Type&);

/*
 * Create a RepoAuthType for a Type.
 *
 * RepoAuthTypes may contain things like RepoAuthType::Array*'s or
 * SStrings for class names.  The emit code needs to handle making
 * sure these things are merged into the appropriate unit or repo.
 *
 */
RepoAuthType make_repo_type(ArrayTypeTable::Builder&, const Type& t);

/*
 * Returns true iff an IsType testing for testTy/testOp on valTy might raise.
 */
bool is_type_might_raise(const Type& testTy, const Type& valTy);
bool is_type_might_raise(IsTypeOp testOp, const Type& valTy);

/*
 * Returns true iff a compare of two types might raise a HAC notice
 */
bool compare_might_raise(const Type& t1, const Type& t2);

/*
 * Model the potential promotion of TClsMeth in varray/vec as a
 * side-effect of certain member instructions. Returns the best known
 * post-promotion type, and whether the promotion happens with
 * potential throwing.
 */
std::pair<Type, Promotion> promote_clsmeth_to_vecish(Type);

/*
 * Model potential promotion of TClsLike into a key for array
 * access. Class-likes will promote into a static string (of the class
 * name). Returns the best known post-promotion type, and whether the
 * promotion happens with potential throwing.
 */
std::pair<Type, Promotion> promote_classlike_to_key(Type);

/*
 * Given a type, adjust the type for the given type-constraint. If there's no
 * type-constraint, or if property type-hints aren't being enforced, then return
 * the type as is. This might return TBottom if the type is not compatible with
 * the type-hint.
 */
Type adjust_type_for_prop(const Index& index,
                          const php::Class& propCls,
                          const TypeConstraint* tc,
                          const Type& ty);

//////////////////////////////////////////////////////////////////////

}}
