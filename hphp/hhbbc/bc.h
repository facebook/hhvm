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
#ifndef incl_HHBBC_BC_H_
#define incl_HHBBC_BC_H_

#include <vector>
#include <utility>
#include <type_traits>

#include <boost/mpl/has_xxx.hpp>
#include <algorithm>

#include <folly/Hash.h>

#include "hphp/util/compact-vector.h"
#include "hphp/runtime/vm/hhbc.h"

#include "hphp/hhbbc/src-loc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

struct Bytecode;

namespace php {

struct Func;

}

//////////////////////////////////////////////////////////////////////

/*
 * The following creates a struct for each bytecode using the opcode
 * table.  Each opcode will be named bc::opcode, and has a single
 * constructor that takes its immediate types in order.
 *
 * E.g.
 *
 *   auto fpass = bc::FPassC { 2 };  // FPassC for arg 2
 */

//////////////////////////////////////////////////////////////////////

struct MKey {
  MKey()
    : mcode{MW}
    , int64{0}
  {}

  MKey(MemberCode mcode, LocalId local)
    : mcode{mcode}
    , local{local}
  {}

  MKey(MemberCode mcode, int32_t idx)
    : mcode{mcode}
    , idx{idx}
  {}

  MKey(MemberCode mcode, int64_t int64)
    : mcode{mcode}
    , int64{int64}
  {}

  MKey(MemberCode mcode, SString litstr)
    : mcode{mcode}
    , litstr{litstr}
  {}

  MemberCode mcode;
  union {
    SString litstr;
    int64_t int64;
    int64_t idx;
    LocalId local;
  };
};

inline bool operator==(MKey a, MKey b) {
  return a.mcode == b.mcode && a.int64 == b.int64;
}

inline bool operator!=(MKey a, MKey b) {
  return !(a == b);
}

// Represents a non-empty range of locals. There's always a first local,
// followed by a count of additional ones.
struct LocalRange {
  LocalId  first;
  uint32_t restCount;
};

inline bool operator==(const LocalRange& a, const LocalRange& b) {
  return a.first == b.first && a.restCount == b.restCount;
}

inline bool operator!=(const LocalRange& a, const LocalRange& b) {
  return !(a == b);
}

using IterTabEnt    = std::pair<IterKind,IterId>;
using IterTab       = CompactVector<IterTabEnt>;

using SwitchTab     = CompactVector<BlockId>;

// The final entry in the SSwitchTab is the default case, it will
// always have a nullptr for the string.
using SSwitchTabEnt = std::pair<LSString,BlockId>;
using SSwitchTab    = CompactVector<SSwitchTabEnt>;

//////////////////////////////////////////////////////////////////////

namespace bc {

//////////////////////////////////////////////////////////////////////

namespace detail {

/*
 * Trivial hasher overrides, which will cause bytecodes to hash according to
 * the hashes defined by hasher_impl.
 */
struct hasher_default {};

/*
 * Default bytecode immediate hashers.
 */
struct hasher_impl {
  static size_t hash(SString s)  { return s->hash(); }
  static size_t hash(LSString s) { return s->hash(); }
  static size_t hash(RepoAuthType rat) { return rat.hash(); }

  static size_t hash(std::pair<IterKind,IterId> kv) {
    return std::hash<IterId>()(kv.second);
  }

  static size_t hash(MKey mkey) {
    return HPHP::hash_int64_pair(mkey.mcode, mkey.int64);
  }

  template<class T>
  static size_t hash(const CompactVector<T>& v) {
    assert(!v.empty());
    return v.size() ^ hash(v.front());
  }

  static size_t hash(std::pair<LSString,BlockId> kv) {
    return HPHP::hash_int64_pair(kv.first->hash(), kv.second);
  }

  static size_t hash(LocalRange range) {
    return HPHP::hash_int64_pair(range.first, range.restCount);
  }

  template<class T>
  static typename std::enable_if<
    std::is_enum<T>::value,
    size_t
  >::type hash(T t) {
    using U = typename std::underlying_type<T>::type;
    return std::hash<U>()(static_cast<U>(t));
  }

  template<class T>
  static typename std::enable_if<
    !std::is_enum<T>::value,
    size_t
  >::type hash(const T& t) { return std::hash<T>()(t); }
};

/*
 * Hash operand wrapper.
 */
template<typename T, typename S>
struct hash_operand { const T& val; S type; };

/*
 * Hash T using H::operator() if it is compatible, else fall back to
 * hasher_impl (e.g., if H := hasher_default).
 */
template<typename T, typename S, class H>
auto hash(const hash_operand<T,S>& t, H h) -> decltype(h(t.val)) {
  return h(t.val);
}
template<typename T, typename S, class H>
auto hash(const hash_operand<T,S>& t, H h) -> decltype(h(t.val, t.type)) {
  return h(t.val, t.type);
}
template<typename T, typename S, typename... Unused>
typename std::enable_if<sizeof...(Unused) == 1, size_t>::type
hash(const hash_operand<T,S>& t, Unused...) {
  return hasher_impl::hash(t.val);
}

/*
 * Clone of folly::hash::hash_combine_generic(), but with a hasher argument
 * instead of a hasher template parameter.
 */
template <class H>
size_t hash_combine(H /*h*/) {
  return 0;
}

template<class H, typename T, typename... Ts>
size_t hash_combine(H h, const T& t, const Ts&... ts) {
  auto const seed = size_t{hash(t, h)};
  if (sizeof...(ts) == 0) return seed;

  auto const remainder = hash_combine(h, ts...);
  return static_cast<size_t>(folly::hash::hash_128_to_64(seed, remainder));
}

/*
 * Trivial equality overrides, which will cause bytecodes to compare via
 * operator==() on the various immediate types.
 */
struct eq_default {};

/*
 * Equality operand wrapper.
 */
template<typename T, typename S>
struct eq_operand { const T& l; const T& r; S type; };

/*
 * Compare two values, using E::operator() if it exists, else the default
 * operator==.
 */
template<typename T, typename S, class E>
auto equals(const eq_operand<T,S>& t, E e) -> decltype(e(t.l, t.r)) {
  return e(t.l, t.r);
}
template<typename T, typename S, class E>
auto equals(const eq_operand<T,S>& t, E e) -> decltype(e(t.l, t.r, t.type)) {
  return e(t.l, t.r, t.type);
}
template<typename T, typename S, typename... Unused>
typename std::enable_if<sizeof...(Unused) == 1, bool>::type
equals(const eq_operand<T,S>& t, Unused...) {
  return t.l == t.r;
}

/*
 * Check if a list of eq_operands are pairwise-equal.
 */
template <class E>
bool eq_pairs(E /*e*/) {
  return true;
}

template<class E, typename T, typename... Ts>
bool eq_pairs(E e, const T& t, const Ts&... ts) {
  return equals(t, e) && (sizeof...(ts) == 0 || eq_pairs(e, ts...));
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Bytecode immediate type tags.
 */
namespace imm {
#define ARGTYPE(name, type)     enum class name : uint8_t {};
#define ARGTYPEVEC(name, type)  enum class name : uint8_t {};
  ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
}

#define IMM_ID_BLA      BLA
#define IMM_ID_SLA      SLA
#define IMM_ID_ILA      ILA
#define IMM_ID_IVA      IVA
#define IMM_ID_I64A     I64A
#define IMM_ID_LA       LA
#define IMM_ID_IA       IA
#define IMM_ID_CAR      CAR
#define IMM_ID_CAW      CAW
#define IMM_ID_DA       DA
#define IMM_ID_SA       SA
#define IMM_ID_RATA     RATA
#define IMM_ID_AA       AA
#define IMM_ID_BA       BA
#define IMM_ID_OA(type) OA
#define IMM_ID_VSA      VSA
#define IMM_ID_KA       KA
#define IMM_ID_LAR      LAR

#define IMM_TY_BLA      SwitchTab
#define IMM_TY_SLA      SSwitchTab
#define IMM_TY_ILA      IterTab
#define IMM_TY_IVA      uint32_t
#define IMM_TY_I64A     int64_t
#define IMM_TY_LA       LocalId
#define IMM_TY_IA       IterId
#define IMM_TY_CAR      ClsRefSlotId
#define IMM_TY_CAW      ClsRefSlotId
#define IMM_TY_DA       double
#define IMM_TY_SA       SString
#define IMM_TY_RATA     RepoAuthType
#define IMM_TY_AA       SArray
#define IMM_TY_BA       BlockId
#define IMM_TY_OA(type) type
#define IMM_TY_VSA      CompactVector<LSString>
#define IMM_TY_KA       MKey
#define IMM_TY_LAR      LocalRange

#define IMM_NAME_BLA(n)     targets
#define IMM_NAME_SLA(n)     targets
#define IMM_NAME_ILA(n)     iterTab
#define IMM_NAME_IVA(n)     arg##n
#define IMM_NAME_I64A(n)    arg##n
#define IMM_NAME_LA(n)      loc##n
#define IMM_NAME_IA(n)      iter##n
#define IMM_NAME_CAR(n)     slot
#define IMM_NAME_CAW(n)     slot
#define IMM_NAME_DA(n)      dbl##n
#define IMM_NAME_SA(n)      str##n
#define IMM_NAME_RATA(n)    rat
#define IMM_NAME_AA(n)      arr##n
#define IMM_NAME_BA(n)      target
#define IMM_NAME_OA_IMPL(n) subop##n
#define IMM_NAME_OA(type)   IMM_NAME_OA_IMPL
#define IMM_NAME_VSA(n)     keys
#define IMM_NAME_KA(n)      mkey
#define IMM_NAME_LAR(n)     locrange

#define IMM_EXTRA_BLA
#define IMM_EXTRA_SLA
#define IMM_EXTRA_ILA
#define IMM_EXTRA_IVA
#define IMM_EXTRA_I64A
#define IMM_EXTRA_LA
#define IMM_EXTRA_IA
#define IMM_EXTRA_CAR       using has_car_flag = std::true_type;
#define IMM_EXTRA_CAW       using has_caw_flag = std::true_type;
#define IMM_EXTRA_DA
#define IMM_EXTRA_SA
#define IMM_EXTRA_RATA
#define IMM_EXTRA_AA
#define IMM_EXTRA_BA        using has_target_flag = std::true_type;
#define IMM_EXTRA_OA(x)
#define IMM_EXTRA_VSA
#define IMM_EXTRA_KA
#define IMM_EXTRA_LAR

#define IMM_MEM(which, n)          IMM_TY_##which IMM_NAME_##which(n)
#define IMM_MEM_NA
#define IMM_MEM_ONE(x)             IMM_MEM(x, 1);
#define IMM_MEM_TWO(x, y)          IMM_MEM(x, 1); IMM_MEM(y, 2);
#define IMM_MEM_THREE(x, y, z)     IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                   IMM_MEM(z, 3);
#define IMM_MEM_FOUR(x, y, z, l)   IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                   IMM_MEM(z, 3); IMM_MEM(l, 4);

#define IMM_EQ_WRAP(e, ...)       detail::eq_pairs(e, __VA_ARGS__)
#define IMM_EQ(which, n)          detail::eq_operand<     \
                                    IMM_TY_##which,       \
                                    imm::IMM_ID_##which   \
                                  > { \
                                    IMM_NAME_##which(n),  \
                                    o.IMM_NAME_##which(n) \
                                  }
#define IMM_EQ_NA                 detail::eq_operand<void*,imm::NA> { 0, 0 }
#define IMM_EQ_ONE(x)             IMM_EQ(x, 1)
#define IMM_EQ_TWO(x, y)          IMM_EQ_ONE(x), IMM_EQ(y, 2)
#define IMM_EQ_THREE(x, y, z)     IMM_EQ_TWO(x, y), IMM_EQ(z, 3)
#define IMM_EQ_FOUR(x, y, z, l)   IMM_EQ_THREE(x, y, z), IMM_EQ(l, 4)

#define IMM_HASH_WRAP(h, ...)       detail::hash_combine(h, __VA_ARGS__)
#define IMM_HASH(which, n)          detail::hash_operand<     \
                                      IMM_TY_##which,         \
                                      imm::IMM_ID_##which     \
                                    > { IMM_NAME_##which(n) }
#define IMM_HASH_NA                 detail::hash_operand<void*,imm::NA> { 0 }
#define IMM_HASH_ONE(x)             IMM_HASH(x, 1)
#define IMM_HASH_TWO(x, y)          IMM_HASH_ONE(x), IMM_HASH(y, 2)
#define IMM_HASH_THREE(x, y, z)     IMM_HASH_TWO(x, y), IMM_HASH(z, 3)
#define IMM_HASH_FOUR(x, y, z, l)   IMM_HASH_THREE(x, y, z), IMM_HASH(l, 4)

#define IMM_EXTRA_NA
#define IMM_EXTRA_ONE(x)           IMM_EXTRA_##x
#define IMM_EXTRA_TWO(x,y)         IMM_EXTRA_ONE(x)       IMM_EXTRA_ONE(y)
#define IMM_EXTRA_THREE(x,y,z)     IMM_EXTRA_TWO(x,y)     IMM_EXTRA_ONE(z)
#define IMM_EXTRA_FOUR(x,y,z,l)    IMM_EXTRA_THREE(x,y,z) IMM_EXTRA_ONE(l)

#define IMM_CTOR(which, n)         IMM_TY_##which IMM_NAME_##which(n)
#define IMM_CTOR_NA
#define IMM_CTOR_ONE(x)            IMM_CTOR(x, 1)
#define IMM_CTOR_TWO(x, y)         IMM_CTOR(x, 1), IMM_CTOR(y, 2)
#define IMM_CTOR_THREE(x, y, z)    IMM_CTOR(x, 1), IMM_CTOR(y, 2), \
                                   IMM_CTOR(z, 3)
#define IMM_CTOR_FOUR(x, y, z, l)  IMM_CTOR(x, 1), IMM_CTOR(y, 2), \
                                   IMM_CTOR(z, 3), IMM_CTOR(l, 4)

#define IMM_INIT(which, n)         IMM_NAME_##which(n) \
                                     ( std::move(IMM_NAME_##which(n)) )
#define IMM_INIT_NA
#define IMM_INIT_ONE(x)            : IMM_INIT(x, 1)
#define IMM_INIT_TWO(x, y)         : IMM_INIT(x, 1), IMM_INIT(y, 2)
#define IMM_INIT_THREE(x, y, z)    : IMM_INIT(x, 1), IMM_INIT(y, 2), \
                                     IMM_INIT(z, 3)
#define IMM_INIT_FOUR(x, y, z, l)  : IMM_INIT(x, 1), IMM_INIT(y, 2), \
                                     IMM_INIT(z, 3), IMM_INIT(l, 4)

#define POP_UV  if (i == 0) return Flavor::U
#define POP_CV  if (i == 0) return Flavor::C
#define POP_VV  if (i == 0) return Flavor::V
#define POP_FV  if (i == 0) return Flavor::F
#define POP_RV  if (i == 0) return Flavor::R
#define POP_CUV if (i == 0) return Flavor::CUV

#define POP_NOV             uint32_t numPop() const { return 0; } \
                            Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_ONE(x)          uint32_t numPop() const { return 1; } \
                            Flavor popFlavor(uint32_t i) const {  \
                              POP_##x; not_reached();             \
                            }

#define POP_TWO(x, y)       uint32_t numPop() const { return 2; }   \
                            Flavor popFlavor(uint32_t i) const {    \
                              POP_##x; --i; POP_##y; not_reached(); \
                            }

#define POP_THREE(x, y, z)  uint32_t numPop() const { return 3; }   \
                            Flavor popFlavor(uint32_t i) const {    \
                              POP_##x; --i; POP_##y; --i; POP_##z;  \
                              not_reached();                        \
                            }

#define POP_MFINAL  uint32_t numPop() const { return arg1; } \
                    Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_F_MFINAL uint32_t numPop() const { return arg2; } \
                     Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_C_MFINAL uint32_t numPop() const { return arg1 + 1; } \
                     Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_V_MFINAL POP_C_MFINAL

#define POP_CMANY   uint32_t numPop() const { return arg1; }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      assert(i < numPop());                   \
                      return Flavor::C;                       \
                    }

#define POP_SMANY   uint32_t numPop() const { return keys.size(); }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      assert(i < numPop());                   \
                      return Flavor::C;                       \
                    }

#define POP_FMANY   uint32_t numPop() const { return arg1; }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      assert(i < numPop());                   \
                      return Flavor::F;                       \
                    }

#define POP_CVUMANY uint32_t numPop() const { return arg1; }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      return Flavor::CVU;                     \
                    }

#define PUSH_NOV          uint32_t numPush() const { return 0; }

#define PUSH_ONE(x)       uint32_t numPush() const { return 1; }

#define PUSH_TWO(x, y)    uint32_t numPush() const { return 2; }

#define PUSH_INS_1(...)   uint32_t numPush() const { return 1; }

#define FLAGS_NF
#define FLAGS_TF
#define FLAGS_CF
#define FLAGS_FF
#define FLAGS_PF bool has_unpack;
#define FLAGS_CF_TF
#define FLAGS_CF_FF

#define FLAGS_CTOR_NF
#define FLAGS_CTOR_TF
#define FLAGS_CTOR_CF
#define FLAGS_CTOR_FF
#define FLAGS_CTOR_PF ,bool hu
#define FLAGS_CTOR_CF_TF
#define FLAGS_CTOR_CF_FF

#define FLAGS_INIT_NF
#define FLAGS_INIT_TF
#define FLAGS_INIT_CF
#define FLAGS_INIT_FF
#define FLAGS_INIT_PF ,has_unpack(hu)
#define FLAGS_INIT_CF_TF
#define FLAGS_INIT_CF_FF

#define O(opcode, imms, inputs, outputs, flags) \
  struct opcode {                               \
    static constexpr Op op = Op::opcode;        \
    explicit opcode ( IMM_CTOR_##imms           \
                      FLAGS_CTOR_##flags)       \
      IMM_INIT_##imms                           \
      FLAGS_INIT_##flags                        \
    {}                                          \
                                                \
    IMM_MEM_##imms                              \
    FLAGS_##flags                               \
    IMM_EXTRA_##imms                            \
    POP_##inputs                                \
    PUSH_##outputs                              \
                                                \
    bool operator==(const opcode& o) const {    \
      return equals(o, detail::eq_default{});   \
    }                                           \
                                                \
    bool operator!=(const opcode& o) const {    \
      return !(*this == o);                     \
    }                                           \
                                                \
    template<class E>                           \
    bool equals(const opcode& o, E e) const {   \
      return IMM_EQ_WRAP(e, IMM_EQ_##imms);     \
    }                                           \
                                                \
    size_t hash() const {                       \
      return IMM_HASH_WRAP(                     \
        detail::hasher_default{},               \
        IMM_HASH_##imms);                       \
    }                                           \
                                                \
    template<class H>                           \
    size_t hash(H h) const {                    \
      return IMM_HASH_WRAP(h, IMM_HASH_##imms); \
    }                                           \
  };
OPCODES
#undef O

#undef FLAGS_NA
#undef FLAGS_TF
#undef FLAGS_CF
#undef FLAGS_FF
#undef FLAGS_PF
#undef FLAGS_CF_TF
#undef FLAGS_CF_FF

#undef FLAGS_CTOR_NA
#undef FLAGS_CTOR_TF
#undef FLAGS_CTOR_CF
#undef FLAGS_CTOR_FF
#undef FLAGS_CTOR_PF
#undef FLAGS_CTOR_CF_TF
#undef FLAGS_CTOR_CF_FF

#undef FLAGS_INIT_NA
#undef FLAGS_INIT_TF
#undef FLAGS_INIT_CF
#undef FLAGS_INIT_FF
#undef FLAGS_INIT_PF
#undef FLAGS_INIT_CF_TF
#undef FLAGS_INIT_CF_FF

#undef PUSH_NOV
#undef PUSH_ONE
#undef PUSH_TWO
#undef PUSH_INS_1

#undef POP_UV
#undef POP_CV
#undef POP_VV
#undef POP_FV
#undef POP_RV

#undef POP_NOV
#undef POP_ONE
#undef POP_TWO
#undef POP_THREE
#undef POP_MFINAL
#undef POP_F_MFINAL
#undef POP_C_MFINAL
#undef POP_V_MFINAL
#undef POP_CMANY
#undef POP_SMANY
#undef POP_FMANY
#undef POP_CVUMANY

#undef IMM_TY_MA
#undef IMM_TY_BLA
#undef IMM_TY_SLA
#undef IMM_TY_ILA
#undef IMM_TY_IVA
#undef IMM_TY_I64A
#undef IMM_TY_LA
#undef IMM_TY_IA
#undef IMM_TY_CAR
#undef IMM_TY_CAW
#undef IMM_TY_DA
#undef IMM_TY_SA
#undef IMM_TY_RATA
#undef IMM_TY_AA
#undef IMM_TY_BA
#undef IMM_TY_OA
#undef IMM_TY_VSA
#undef IMM_TY_KA
#undef IMM_TY_LAR

// These are deliberately not undefined, so they can be used in other
// places.
// #undef IMM_NAME_BLA
// #undef IMM_NAME_SLA
// #undef IMM_NAME_ILA
// #undef IMM_NAME_IVA
// #undef IMM_NAME_I64A
// #undef IMM_NAME_LA
// #undef IMM_NAME_IA
// #undef IMM_NAME_CAR
// #undef IMM_NAME_CAW
// #undef IMM_NAME_DA
// #undef IMM_NAME_SA
// #undef IMM_NAME_RATA
// #undef IMM_NAME_AA
// #undef IMM_NAME_BA
// #undef IMM_NAME_OA
// #undef IMM_NAME_OA_IMPL
// #undef IMM_NAME_LAR

#undef IMM_EXTRA_BLA
#undef IMM_EXTRA_SLA
#undef IMM_EXTRA_ILA
#undef IMM_EXTRA_IVA
#undef IMM_EXTRA_I64A
#undef IMM_EXTRA_LA
#undef IMM_EXTRA_IA
#undef IMM_EXTRA_CAR
#undef IMM_EXTRA_CAW
#undef IMM_EXTRA_DA
#undef IMM_EXTRA_SA
#undef IMM_EXTRA_RATA
#undef IMM_EXTRA_AA
#undef IMM_EXTRA_BA
#undef IMM_EXTRA_OA
#undef IMM_EXTRA_KA
#undef IMM_EXTRA_LAR

#undef IMM_MEM
#undef IMM_MEM_NA
#undef IMM_MEM_ONE
#undef IMM_MEM_TWO
#undef IMM_MEM_THREE
#undef IMM_MEM_FOUR

#undef IMM_EQ
#undef IMM_EQ_NA
#undef IMM_EQ_ONE
#undef IMM_EQ_TWO
#undef IMM_EQ_THREE
#undef IMM_EQ_FOUR

#undef IMM_HASH
#undef IMM_HASH_DO
#undef IMM_HASH_NA
#undef IMM_HASH_ONE
#undef IMM_HASH_TWO
#undef IMM_HASH_THREE
#undef IMM_HASH_FOUR

#undef IMM_CTOR
#undef IMM_CTOR_NA
#undef IMM_CTOR_ONE
#undef IMM_CTOR_TWO
#undef IMM_CTOR_THREE
#undef IMM_CTOR_FOUR

#undef IMM_INIT
#undef IMM_INIT_NA
#undef IMM_INIT_ONE
#undef IMM_INIT_TWO
#undef IMM_INIT_THREE
#undef IMM_INIT_FOUR

}

//////////////////////////////////////////////////////////////////////

/*
 * Bytecode is a tagged-union that can hold any HHBC opcode struct
 * defined above.  You can visit a bytecode with a static_visitor
 * using visit(), defined below.
 *
 * Each Bytecode also carries a corresponding SrcLoc that indicates
 * which line of PHP code the bytecode was generated for.
 */
struct Bytecode {
  // Default construction creates a Nop.
  Bytecode()
    : op(Op::Nop)
    , Nop(bc::Nop{})
  {}

#define O(opcode, ...)                          \
  /* implicit */ Bytecode(bc::opcode data)      \
    : op(Op::opcode)                            \
  {                                             \
    new (&opcode) bc::opcode(std::move(data));  \
  }

  OPCODES

#undef O

  // Note: assuming bc::Nop is empty and has trivial dtor/ctor.

  Bytecode(const Bytecode& o) : op(Op::Nop) { *this = o; }
  Bytecode(Bytecode&& o) noexcept : op(Op::Nop) { *this = std::move(o); }

  Bytecode& operator=(const Bytecode& o) {
    destruct();
    op = Op::Nop;
    srcLoc = o.srcLoc;
#define O(opcode, ...) \
    case Op::opcode: new (&opcode) bc::opcode(o.opcode); break;
    switch (o.op) { OPCODES }
#undef O
    op = o.op;
    return *this;
  }

  Bytecode& operator=(Bytecode&& o) {
    destruct();
    srcLoc = o.srcLoc;
#define O(opcode, ...) \
    case Op::opcode: new (&opcode) bc::opcode(std::move(o.opcode)); break;
    switch (o.op) { OPCODES }
#undef O
    op = o.op;
    return *this;
  }

  ~Bytecode() { destruct(); }

  uint32_t numPop() const {
#define O(opcode, ...) \
    case Op::opcode: return opcode.numPop();
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  Flavor popFlavor(uint32_t i) const {
#define O(opcode, ...) \
    case Op::opcode: return opcode.popFlavor(i);
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  uint32_t numPush() const {
#define O(opcode, ...) \
    case Op::opcode: return opcode.numPush();
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  Op op;
  int32_t srcLoc{-1};

#define O(opcode, ...) bc::opcode opcode;
  union { OPCODES };
#undef O

private:
  void destruct() {
    switch (op) {
#define O(opcode, ...)                          \
      case Op::opcode:                          \
        { typedef bc::opcode X;                 \
          this->opcode.~X(); }                  \
        break;
      OPCODES
#undef O
    }
  }
};

//////////////////////////////////////////////////////////////////////

inline bool operator==(const Bytecode& a, const Bytecode& b) {
  if (a.op != b.op) return false;
#define O(opcode, ...) case Op::opcode: return a.opcode == b.opcode;
  switch (a.op) { OPCODES }
#undef O
  not_reached();
}

inline bool operator!=(const Bytecode& a, const Bytecode& b) {
  return !(a == b);
}

template<class E>
inline bool equals(const Bytecode& a, const Bytecode& b, E equals) {
  if (a.op != b.op) return false;
#define O(opcode, ...)  \
  case Op::opcode: return a.opcode.equals(b.opcode, equals);
  switch (a.op) { OPCODES }
#undef O
  not_reached();
}

template<class H>
inline size_t hash(const Bytecode& b, H hasher) {
  auto hash = 14695981039346656037ULL;
  auto o = static_cast<size_t>(b.op);
  hash ^= o;
#define O(opcode, ...)  \
  case Op::opcode:      \
    return folly::hash::hash_combine(b.opcode.hash(hasher), hash);
  switch (b.op) { OPCODES }
#undef O
  not_reached();
}

inline size_t hash(const Bytecode& b) {
  return hash(b, bc::detail::hasher_default{});
}

//////////////////////////////////////////////////////////////////////

/*
 * Helper for making a Bytecode with a given srcLoc.
 *
 * Ex:
 *    auto b = bc_with_loc(something.srcLoc, bc::Nop {});
 */
template<class T> Bytecode bc_with_loc(int32_t loc, const T& t) {
  Bytecode b = t;
  b.srcLoc = loc;
  return b;
}

//////////////////////////////////////////////////////////////////////

/*
 * Visit a bytecode using a StaticVisitor, similar to
 * boost::apply_visitor or match().
 *
 * The `v' argument should be a function object that accepts a call
 * operator for all the bc::Foo types, with a nested member typedef
 * called result_type that indicates the return type of the call.
 */
template<class Visit>
typename Visit::result_type visit(Bytecode& b, Visit v) {
#define O(opcode, ...) case Op::opcode: return v(b.opcode);
  switch (b.op) { OPCODES }
#undef O
  not_reached();
}

template<class Visit>
typename Visit::result_type visit(const Bytecode& b, Visit v) {
#define O(opcode, ...) case Op::opcode: return v(b.opcode);
  switch (b.op) { OPCODES }
#undef O
  not_reached();
}

//////////////////////////////////////////////////////////////////////

BOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(has_target, has_target_flag, false);
BOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(has_car, has_car_flag, false);
BOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(has_caw, has_caw_flag, false);

//////////////////////////////////////////////////////////////////////

struct ReadClsRefSlotVisitor : boost::static_visitor<ClsRefSlotId> {
  template <class T>
  typename std::enable_if<!has_car<T>::value, ClsRefSlotId>::type
  operator()(T const& /*t*/) const {
    return NoClsRefSlotId;
  }

  template<class T>
  typename std::enable_if<has_car<T>::value,ClsRefSlotId>::type
  operator()(T const& t) const { return t.slot; }
};

struct WriteClsRefSlotVisitor : boost::static_visitor<ClsRefSlotId> {
  template <class T>
  typename std::enable_if<!has_caw<T>::value, ClsRefSlotId>::type
  operator()(T const& /*t*/) const {
    return NoClsRefSlotId;
  }

  template<class T>
  typename std::enable_if<has_caw<T>::value,ClsRefSlotId>::type
  operator()(T const& t) const { return t.slot; }
};

//////////////////////////////////////////////////////////////////////

std::string show(const php::Func&, const Bytecode& bc);
inline std::string show(borrowed_ptr<const php::Func> func,
                        const Bytecode& bc) {
  return show(*func, bc);
}

//////////////////////////////////////////////////////////////////////

}

}

#endif
